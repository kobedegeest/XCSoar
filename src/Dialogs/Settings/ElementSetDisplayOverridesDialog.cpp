// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "ElementSetDisplayOverridesDialog.hpp"

#include "Dialogs/ListPicker.hpp"
#include "Dialogs/Message.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Form/Button.hpp"
#include "Form/DataField/Enum.hpp"
#include "Language/Language.hpp"
#include "Look/DialogLook.hpp"
#include "Renderer/TwoTextRowsRenderer.hpp"
#include "UIGlobals.hpp"
#include "Widget/ListWidget.hpp"
#include "Widget/RowFormWidget.hpp"
#include "util/StaticString.hxx"

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

static const char *
GetGroupLabel(DisplaySettingGroup group) noexcept
{
  switch (group) {
  case DisplaySettingGroup::TERRAIN:
    return _("Terrain");

  case DisplaySettingGroup::ORIENTATION:
    return _("Orientation");

  case DisplaySettingGroup::WAYPOINTS:
    return _("Waypoints");

  case DisplaySettingGroup::AIRSPACE:
    return _("Airspace");
  }

  return "";
}

static const char *
GetHelp(const DisplaySettingDescriptor &descriptor) noexcept
{
  return descriptor.help != nullptr ? gettext(descriptor.help) : nullptr;
}

static const DisplaySettingEnumChoice *
FindEnumChoice(const DisplaySettingDescriptor &descriptor,
               DisplaySettingValue value) noexcept
{
  for (uint16_t i = 0; i < descriptor.enum_choice_count; ++i)
    if (descriptor.enum_choices[i].value == value.value)
      return &descriptor.enum_choices[i];

  return nullptr;
}

static const char *
FormatValue(const DisplaySettingDescriptor &descriptor,
            DisplaySettingValue value, StaticString<64> &buffer) noexcept
{
  switch (descriptor.value_type) {
  case DisplaySettingValueType::BOOLEAN:
    return value.AsBoolean() ? _("Yes") : _("No");

  case DisplaySettingValueType::ENUM:
    if (const auto *choice = FindEnumChoice(descriptor, value))
      return gettext(choice->label);

    [[fallthrough]];

  case DisplaySettingValueType::INTEGER:
    buffer.Format("%d", value.value);
    return buffer.c_str();
  }

  return "";
}

class DisplaySettingValueWidget final : public RowFormWidget {
  const DisplaySettingDescriptor &descriptor;
  DisplaySettingValue value;

public:
  DisplaySettingValueWidget(const DialogLook &look,
                            const DisplaySettingDescriptor &_descriptor,
                            DisplaySettingValue _value) noexcept
    : RowFormWidget(look), descriptor(_descriptor), value(_value) {}

  DisplaySettingValue GetValue() const noexcept {
    return value;
  }

  void Prepare(ContainerWindow &parent,
               const PixelRect &rc) noexcept override;
  bool Save(bool &changed) noexcept override;
};

void
DisplaySettingValueWidget::Prepare(
  [[maybe_unused]] ContainerWindow &parent,
  [[maybe_unused]] const PixelRect &rc) noexcept
{
  const char *help = GetHelp(descriptor);

  switch (descriptor.value_type) {
  case DisplaySettingValueType::BOOLEAN:
    AddBoolean(_("Value"), help, value.AsBoolean());
    break;

  case DisplaySettingValueType::ENUM: {
    auto *control = AddEnum(_("Value"), help);
    auto &field = static_cast<DataFieldEnum &>(*control->GetDataField());
    for (uint16_t i = 0; i < descriptor.enum_choice_count; ++i) {
      const auto &choice = descriptor.enum_choices[i];
      const char *choice_help = choice.help != nullptr
        ? gettext(choice.help)
        : nullptr;
      field.AddChoice(static_cast<unsigned>(choice.value),
                      gettext(choice.label), nullptr, choice_help);
    }

    field.SetValue(static_cast<unsigned>(value.value));
    control->RefreshDisplay();
    break;
  }

  case DisplaySettingValueType::INTEGER:
    AddInteger(_("Value"), help, "%d", "%d",
               descriptor.minimum.value, descriptor.maximum.value,
               std::max(descriptor.integer_step, int32_t{1}), value.value);
    break;
  }
}

bool
DisplaySettingValueWidget::Save(bool &changed) noexcept
{
  DisplaySettingValue candidate{};
  switch (descriptor.value_type) {
  case DisplaySettingValueType::BOOLEAN:
    candidate = DisplaySettingValue::Boolean(GetValueBoolean(0));
    break;

  case DisplaySettingValueType::ENUM:
    candidate = DisplaySettingValue::Enum(
      static_cast<int32_t>(GetValueEnum(0)));
    break;

  case DisplaySettingValueType::INTEGER:
    candidate = DisplaySettingValue::Integer(GetValueInteger(0));
    break;
  }

  if (!descriptor.IsValid(candidate))
    return false;

  if (candidate != value) {
    value = candidate;
    changed = true;
  }

  return true;
}

enum class ValueDialogResult {
  CANCEL,
  SAVE,
  REMOVE,
};

static ValueDialogResult
ShowValueDialog(UI::SingleWindow &parent, const DialogLook &look,
                const DisplaySettingDescriptor &descriptor,
                DisplaySettingValue &value, bool allow_remove)
{
  auto widget = std::make_unique<DisplaySettingValueWidget>(
    look, descriptor, value);
  auto *const widget_ptr = widget.get();

  WidgetDialog dialog(WidgetDialog::Auto{}, parent, look,
                      gettext(descriptor.label), widget.release());
  if (allow_remove)
    dialog.AddButton(_("Remove"), mrExtra);
  dialog.AddButton(_("OK"), mrOK);
  dialog.AddButton(_("Cancel"), mrCancel);

  const int result = dialog.ShowModal();
  if (result == mrExtra)
    return ValueDialogResult::REMOVE;
  if (result != mrOK)
    return ValueDialogResult::CANCEL;

  value = widget_ptr->GetValue();
  return ValueDialogResult::SAVE;
}

class AddableSettingRenderer final : public ListItemRenderer {
  const std::vector<const DisplaySettingDescriptor *> &items;
  TwoTextRowsRenderer row_renderer;

public:
  explicit AddableSettingRenderer(
    const std::vector<const DisplaySettingDescriptor *> &_items) noexcept
    : items(_items) {}

  unsigned CalculateLayout(const DialogLook &look) noexcept {
    return row_renderer.CalculateLayout(*look.list.font_bold,
                                        look.small_font);
  }

  unsigned OnListResized() noexcept override {
    const auto &look = UIGlobals::GetDialogLook();
    return row_renderer.CalculateLayout(*look.list.font_bold,
                                        look.small_font);
  }

  void OnPaintItem(Canvas &canvas, const PixelRect rc,
                   unsigned index) noexcept override {
    assert(index < items.size());
    row_renderer.DrawFirstRow(canvas, rc, gettext(items[index]->label));
    row_renderer.DrawSecondRow(canvas, rc,
                               GetGroupLabel(items[index]->group));
  }
};

class ElementSetDisplayOverridesWidget final : public ListWidget {
  ElementSetDisplayOverrides &overrides;
  const std::span<const DisplaySettingDescriptor> catalog;
  const GlobalDisplaySettingValueGetter get_global_value;

  std::vector<const DisplaySettingDescriptor *> items;
  TwoTextRowsRenderer row_renderer;

  Button *edit_button = nullptr;
  Button *add_button = nullptr;
  Button *remove_button = nullptr;

  void UpdateList();
  void UpdateButtons() noexcept;
  bool CanAdd() const noexcept;
  void EditClicked();
  void AddClicked();
  void RemoveClicked() noexcept;

public:
  ElementSetDisplayOverridesWidget(
    ElementSetDisplayOverrides &_overrides,
    std::span<const DisplaySettingDescriptor> _catalog,
    GlobalDisplaySettingValueGetter _get_global_value) noexcept
    : overrides(_overrides), catalog(_catalog),
      get_global_value(_get_global_value) {}

  void CreateButtons(WidgetDialog &dialog);

  void Prepare(ContainerWindow &parent,
               const PixelRect &rc) noexcept override;
  bool Save([[maybe_unused]] bool &changed) noexcept override {
    return true;
  }

protected:
  void OnPaintItem(Canvas &canvas, const PixelRect rc,
                   unsigned index) noexcept override;
  void OnCursorMoved([[maybe_unused]] unsigned index) noexcept override {
    UpdateButtons();
  }
  bool CanActivateItem([[maybe_unused]] unsigned index) const noexcept override {
    return !items.empty();
  }
  void OnActivateItem([[maybe_unused]] unsigned index) noexcept override {
    EditClicked();
  }
};

void
ElementSetDisplayOverridesWidget::CreateButtons(WidgetDialog &dialog)
{
  edit_button = dialog.AddButton(_("Edit"), [this](){ EditClicked(); });
  add_button = dialog.AddButton(C_("Button", "Add"),
                                [this](){ AddClicked(); });
  remove_button = dialog.AddButton(_("Remove"),
                                   [this](){ RemoveClicked(); });
}

void
ElementSetDisplayOverridesWidget::Prepare(ContainerWindow &parent,
                                           const PixelRect &rc) noexcept
{
  const auto &look = UIGlobals::GetDialogLook();
  CreateList(parent, look, rc,
             row_renderer.CalculateLayout(*look.list.font_bold,
                                          look.small_font));
  UpdateList();
}

bool
ElementSetDisplayOverridesWidget::CanAdd() const noexcept
{
  if (get_global_value == nullptr ||
      overrides.Size() == overrides.Capacity())
    return false;

  return std::any_of(catalog.begin(), catalog.end(), [this](const auto &item) {
    return item.element_set_overwritable && overrides.Get(item.key) == nullptr;
  });
}

void
ElementSetDisplayOverridesWidget::UpdateButtons() noexcept
{
  const bool has_items = !items.empty();
  edit_button->SetEnabled(has_items);
  remove_button->SetEnabled(has_items);
  add_button->SetEnabled(CanAdd());
}

void
ElementSetDisplayOverridesWidget::UpdateList()
{
  items.clear();
  for (const auto &descriptor : catalog)
    if (overrides.Get(descriptor.key) != nullptr)
      items.push_back(&descriptor);

  auto &list = GetList();
  list.SetLength(std::max(items.size(), std::size_t{1}));
  list.Invalidate();
  UpdateButtons();
}

void
ElementSetDisplayOverridesWidget::OnPaintItem(
  Canvas &canvas, const PixelRect rc, unsigned index) noexcept
{
  if (items.empty()) {
    assert(index == 0);
    row_renderer.DrawFirstRow(canvas, rc, _("No setting overrides"));
    row_renderer.DrawSecondRow(
      canvas, rc, _("This element set inherits all display settings."));
    return;
  }

  assert(index < items.size());
  const auto &descriptor = *items[index];
  const auto *value = overrides.Get(descriptor.key);
  assert(value != nullptr);

  StaticString<64> value_buffer;
  row_renderer.DrawFirstRow(canvas, rc, gettext(descriptor.label));
  row_renderer.DrawSecondRow(canvas, rc, GetGroupLabel(descriptor.group));
  row_renderer.DrawRightSecondRow(canvas, rc,
                                  FormatValue(descriptor, *value,
                                              value_buffer));
}

void
ElementSetDisplayOverridesWidget::EditClicked()
{
  if (items.empty())
    return;

  const auto &descriptor = *items[GetList().GetCursorIndex()];
  DisplaySettingValue value = *overrides.Get(descriptor.key);
  switch (ShowValueDialog(UIGlobals::GetMainWindow(),
                          UIGlobals::GetDialogLook(), descriptor,
                          value, true)) {
  case ValueDialogResult::CANCEL:
    return;

  case ValueDialogResult::SAVE:
    overrides.Set(descriptor, value);
    break;

  case ValueDialogResult::REMOVE:
    overrides.Remove(descriptor.key);
    break;
  }

  UpdateList();
}

void
ElementSetDisplayOverridesWidget::AddClicked()
{
  if (!CanAdd())
    return;

  std::vector<const DisplaySettingDescriptor *> addable;
  for (const auto &descriptor : catalog)
    if (descriptor.element_set_overwritable &&
        overrides.Get(descriptor.key) == nullptr)
      addable.push_back(&descriptor);

  AddableSettingRenderer renderer(addable);
  const auto &look = UIGlobals::GetDialogLook();
  const int selected = ListPicker(
    _("Add setting override"), addable.size(), 0,
    renderer.CalculateLayout(look), renderer, false,
    _("The new override starts with the current global value."));
  if (selected < 0)
    return;

  const auto &descriptor = *addable[selected];
  DisplaySettingValue value = get_global_value(descriptor);
  if (!descriptor.IsValid(value)) {
    ShowMessageBox(_("The current global value is not valid for this setting."),
                   _("Setting override"), MB_OK | MB_ICONEXCLAMATION);
    return;
  }

  if (ShowValueDialog(UIGlobals::GetMainWindow(), look, descriptor,
                      value, false) != ValueDialogResult::SAVE)
    return;

  if (overrides.Set(descriptor, value) == SetDisplayOverrideResult::FULL) {
    ShowMessageBox(_("No more setting overrides can be added."),
                   _("Setting override"), MB_OK | MB_ICONEXCLAMATION);
    return;
  }

  UpdateList();
}

void
ElementSetDisplayOverridesWidget::RemoveClicked() noexcept
{
  if (items.empty())
    return;

  const auto &descriptor = *items[GetList().GetCursorIndex()];
  overrides.Remove(descriptor.key);
  UpdateList();
}

bool
ShowElementSetDisplayOverridesDialog(
  UI::SingleWindow &parent, const DialogLook &look,
  ElementSetDisplayOverrides &overrides,
  std::span<const DisplaySettingDescriptor> catalog,
  GlobalDisplaySettingValueGetter get_global_value)
{
  ElementSetDisplayOverrides working = overrides;
  auto widget = std::make_unique<ElementSetDisplayOverridesWidget>(
    working, catalog, get_global_value);
  auto *const widget_ptr = widget.get();

  WidgetDialog dialog(WidgetDialog::Full{}, parent, look,
                      _("Setting overrides"), widget.release());
  widget_ptr->CreateButtons(dialog);
  dialog.AddButton(_("OK"), mrOK);
  dialog.AddButton(_("Cancel"), mrCancel);
  dialog.EnableCursorSelection();

  if (dialog.ShowModal() != mrOK || working == overrides)
    return false;

  overrides = working;
  return true;
}
