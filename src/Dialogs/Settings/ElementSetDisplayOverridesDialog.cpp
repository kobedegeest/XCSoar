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
#include "Renderer/TextRowRenderer.hpp"
#include "Screen/Layout.hpp"
#include "UIGlobals.hpp"
#include "Units/Units.hpp"
#include "Widget/LargeTextWidget.hpp"
#include "Widget/MultiSelectListWidget.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Widget/StaticHelpTextWidget.hpp"
#include "Widget/TwoWidgets.hpp"
#include "Widget/VScrollWidget.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <memory>
#include <utility>
#include <vector>

using SettingList = std::vector<const DisplaySettingDescriptor *>;

// Keep the labels and order of the corresponding Map Display config pages.
static constexpr DisplaySettingGroup groups[] = {
  DisplaySettingGroup::ORIENTATION,
  DisplaySettingGroup::WAYPOINTS,
  DisplaySettingGroup::TERRAIN,
  DisplaySettingGroup::AIRSPACE,
};

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

static const char *
GetAirspaceHelp() noexcept
{
  return _("Airspace includes warning settings. Warning overrides can enable "
           "or disable warnings and change warning timing when a page or "
           "flight mode selects this set.");
}

/** A section of the override list, using the main config menu's controls. */
class DisplaySettingsWidget final : public RowFormWidget {
  ElementSetDisplayOverrides &overrides;
  const SettingList items;

public:
  // Reserve one of RowFormWidget's 32 rows for the submenu heading.
  static constexpr unsigned MAX_ITEMS = 31;

  DisplaySettingsWidget(const DialogLook &look,
                        ElementSetDisplayOverrides &_overrides,
                        SettingList _items) noexcept
    : RowFormWidget(look), overrides(_overrides), items(std::move(_items)) {
    assert(!items.empty() && items.size() <= MAX_ITEMS);
  }

  void Prepare(ContainerWindow &parent,
               const PixelRect &rc) noexcept override;
  bool Save(bool &changed) noexcept override;

  PixelSize GetMinimumSize() const noexcept override {
    auto size = RowFormWidget::GetMinimumSize();
    size.height = RowFormWidget::GetMaximumSize().height;
    return size;
  }
};

void
DisplaySettingsWidget::Prepare(ContainerWindow &parent,
                               const PixelRect &rc) noexcept
{
  RowFormWidget::Prepare(parent, rc);
  AddReadOnly(GetGroupLabel(items.front()->group));

  for (const auto *item : items) {
    const auto &descriptor = *item;
    const auto value = *overrides.Get(descriptor.key);
    const char *label = gettext(descriptor.label);
    const char *help = GetHelp(descriptor);

    switch (descriptor.value_type) {
    case DisplaySettingValueType::BOOLEAN:
      AddBoolean(label, help, value.AsBoolean());
      break;

    case DisplaySettingValueType::ENUM: {
      auto *control = AddEnum(label, help);
      auto &field = static_cast<DataFieldEnum &>(*control->GetDataField());
      for (uint16_t i = 0; i < descriptor.enum_choice_count; ++i) {
        const auto &choice = descriptor.enum_choices[i];
        field.AddChoice(static_cast<unsigned>(choice.value),
                        gettext(choice.label), nullptr,
                        choice.help != nullptr ? gettext(choice.help) : nullptr);
      }

      field.SetValue(static_cast<unsigned>(value.value));
      control->RefreshDisplay();
      break;
    }

    case DisplaySettingValueType::INTEGER:
      switch (descriptor.numeric_format) {
      case DisplaySettingNumericFormat::NUMBER:
      case DisplaySettingNumericFormat::PERCENT: {
        const char *format =
          descriptor.numeric_format == DisplaySettingNumericFormat::PERCENT
          ? "%d %%" : "%d";
        AddInteger(label, help, format, "%d",
                   descriptor.minimum.value, descriptor.maximum.value,
                   std::max(descriptor.integer_step, int32_t{1}), value.value);
        break;
      }

      case DisplaySettingNumericFormat::ALTITUDE: {
        const auto unit = Units::GetUserAltitudeUnit();
        AddFloat(label, help, "%.0f %s", "%.0f",
                 Units::ToUserUnit(descriptor.minimum.value, unit),
                 Units::ToUserUnit(descriptor.maximum.value, unit),
                 descriptor.integer_step, false, UnitGroup::ALTITUDE,
                 value.value);
        break;
      }

      case DisplaySettingNumericFormat::DURATION:
        AddDuration(label, help,
                    std::chrono::seconds{descriptor.minimum.value},
                    std::chrono::seconds{descriptor.maximum.value},
                    std::chrono::seconds{descriptor.integer_step},
                    std::chrono::seconds{value.value});
        break;
      }
      break;
    }
  }
}

bool
DisplaySettingsWidget::Save(bool &changed) noexcept
{
  auto candidate = overrides;
  for (unsigned i = 0; i < items.size(); ++i) {
    const auto &descriptor = *items[i];
    const unsigned row = i + 1;
    DisplaySettingValue value{};

    switch (descriptor.value_type) {
    case DisplaySettingValueType::BOOLEAN:
      value = DisplaySettingValue::Boolean(GetValueBoolean(row));
      break;

    case DisplaySettingValueType::ENUM:
      value = DisplaySettingValue::Enum(
        static_cast<int32_t>(GetValueEnum(row)));
      break;

    case DisplaySettingValueType::INTEGER:
      switch (descriptor.numeric_format) {
      case DisplaySettingNumericFormat::NUMBER:
      case DisplaySettingNumericFormat::PERCENT:
        value = DisplaySettingValue::Integer(GetValueInteger(row));
        break;

      case DisplaySettingNumericFormat::ALTITUDE: {
        double altitude = overrides.Get(descriptor.key)->value;
        SaveValue(row, UnitGroup::ALTITUDE, altitude);
        value = DisplaySettingValue::Integer(
          static_cast<int32_t>(std::lround(altitude)));
        break;
      }

      case DisplaySettingNumericFormat::DURATION:
        value = DisplaySettingValue::Integer(
          static_cast<int32_t>(GetValueTime(row).count()));
        break;
      }
      break;
    }

    if (!descriptor.IsValid(value))
      return false;

    candidate.Set(descriptor, value);
  }

  changed |= candidate != overrides;
  overrides = candidate;
  return true;
}

class SettingSelectionWidget final : public MultiSelectListWidget {
  const SettingList &items;
  Button *apply_button = nullptr;

public:
  explicit SettingSelectionWidget(const SettingList &_items) noexcept
    : items(_items) {}

  void SetApplyButton(Button &button) noexcept {
    apply_button = &button;
    apply_button->SetEnabled(false);
  }

  void Prepare(ContainerWindow &parent,
               const PixelRect &rc) noexcept override {
    CreateList(parent, UIGlobals::GetDialogLook(), rc,
               Layout::GetMaximumControlHeight());
    SetLengthWithSelection(items.size());
    MultiSelectListWidget::Prepare(parent, rc);
  }

protected:
  void OnPaintItem(Canvas &canvas, const PixelRect rc,
                   unsigned index) noexcept override {
    assert(index < items.size());
    DrawCheckboxText(canvas, rc, gettext(items[index]->label),
                     IsSelected(index));
  }

  void OnSelectionChanged() noexcept override {
    if (apply_button != nullptr)
      apply_button->SetEnabled(GetSelectedCount() > 0);
  }
};

/** The result remains empty on Cancel, including after checking items. */
static SettingList
SelectSettings(UI::SingleWindow &parent, const DialogLook &look,
               const char *caption, const char *action,
               const SettingList &items)
{
  auto picker = std::make_unique<SettingSelectionWidget>(items);
  auto *const picker_ptr = picker.get();
  std::unique_ptr<Widget> content = std::move(picker);
  if (!items.empty() && items.front()->group == DisplaySettingGroup::AIRSPACE)
    content = std::make_unique<StaticHelpTextWidget>(std::move(content),
                                                    GetAirspaceHelp());
  WidgetDialog dialog(WidgetDialog::Full{}, parent, look, caption,
                      content.release());
  picker_ptr->SetApplyButton(*dialog.AddButton(action, mrOK));
  dialog.AddButton(_("Select all"), [picker_ptr](){ picker_ptr->SelectAll(); });
  dialog.AddButton(_("Clear"), [picker_ptr](){ picker_ptr->ClearSelection(); });
  dialog.AddButton(_("Cancel"), mrCancel);
  dialog.EnableCursorSelection();

  SettingList selected;
  if (dialog.ShowModal() == mrOK)
    for (const auto i : picker_ptr->GetSelectedIndices())
      selected.push_back(items[i]);

  return selected;
}

class SettingGroupRenderer final : public ListItemRenderer {
  const std::vector<DisplaySettingGroup> &items;
  TextRowRenderer renderer;

public:
  explicit SettingGroupRenderer(
    const std::vector<DisplaySettingGroup> &_items) noexcept
    : items(_items) {}

  unsigned OnListResized() noexcept override {
    return renderer.CalculateLayout(*UIGlobals::GetDialogLook().list.font);
  }

  void OnPaintItem(Canvas &canvas, const PixelRect rc,
                   unsigned index) noexcept override {
    renderer.DrawTextRow(canvas, rc, GetGroupLabel(items[index]));
  }
};

class ElementSetDisplayOverridesDialog final : public WidgetDialog {
  UI::SingleWindow &parent;
  const DialogLook &look;
  ElementSetDisplayOverrides &overrides;
  const std::span<const DisplaySettingDescriptor> catalog;
  const GlobalDisplaySettingValueGetter get_global_value;

  Button *add_button;
  Button *remove_button;

  SettingList GetItems(DisplaySettingGroup group, bool adding) const;
  std::unique_ptr<Widget> CreateForm();
  void UpdateButtons() noexcept;
  void Refresh();
  void AddClicked();
  void RemoveClicked();

public:
  ElementSetDisplayOverridesDialog(
    UI::SingleWindow &_parent, const DialogLook &_look,
    ElementSetDisplayOverrides &_overrides,
    std::span<const DisplaySettingDescriptor> _catalog,
    GlobalDisplaySettingValueGetter _get_global_value)
    : WidgetDialog(Full{}, _parent, _look, _("Setting overrides")),
      parent(_parent), look(_look), overrides(_overrides), catalog(_catalog),
      get_global_value(_get_global_value) {
    add_button = AddButton(C_("Button", "Add"), [this](){ AddClicked(); });
    remove_button = AddButton(_("Remove"), [this](){ RemoveClicked(); });
    AddButton(_("OK"), mrOK);
    AddButton(_("Cancel"), mrCancel);
    FinishPreliminary(CreateForm());
    UpdateButtons();
  }
};

SettingList
ElementSetDisplayOverridesDialog::GetItems(DisplaySettingGroup group,
                                           bool adding) const
{
  SettingList items;
  for (const auto &descriptor : catalog)
    if (descriptor.group == group && descriptor.element_set_overwritable &&
        (overrides.Get(descriptor.key) == nullptr) == adding)
      items.push_back(&descriptor);

  return items;
}

std::unique_ptr<Widget>
ElementSetDisplayOverridesDialog::CreateForm()
{
  std::unique_ptr<Widget> form;
  for (const auto group : groups) {
    const auto items = GetItems(group, false);
    for (std::size_t i = 0; i < items.size();
         i += DisplaySettingsWidget::MAX_ITEMS) {
      const auto end = std::min(items.size(),
                               i + DisplaySettingsWidget::MAX_ITEMS);
      auto section = std::make_unique<DisplaySettingsWidget>(
        look, overrides, SettingList(items.begin() + i, items.begin() + end));

      // Compose bounded forms instead of exceeding RowFormWidget's capacity.
      if (form)
        form = std::make_unique<TwoWidgets>(std::move(form), std::move(section));
      else
        form = std::move(section);
    }
  }

  if (!form)
    return std::make_unique<LargeTextWidget>(
      look, _("This element set inherits all display settings. "
              "Choose Add, then a submenu, to select setting overrides."));

  return std::make_unique<VScrollWidget>(std::move(form), look);
}

void
ElementSetDisplayOverridesDialog::UpdateButtons() noexcept
{
  add_button->SetEnabled(get_global_value != nullptr &&
                        overrides.Size() < overrides.Capacity() &&
                        std::any_of(catalog.begin(), catalog.end(),
                                    [this](const auto &item) {
    return item.element_set_overwritable && overrides.Get(item.key) == nullptr;
  }));
  remove_button->SetEnabled(!overrides.IsEmpty());
}

void
ElementSetDisplayOverridesDialog::Refresh()
{
  widget.Set(CreateForm());
  widget.Show();
  UpdateButtons();
  widget.SetFocus();
}

void
ElementSetDisplayOverridesDialog::AddClicked()
{
  bool changed = false;
  if (get_global_value == nullptr || !widget.Save(changed))
    return;

  std::vector<DisplaySettingGroup> available_groups;
  for (const auto group : groups)
    if (!GetItems(group, true).empty())
      available_groups.push_back(group);
  if (available_groups.empty())
    return;

  unsigned cursor = 0;
  while (true) {
    SettingGroupRenderer renderer(available_groups);
    const int selected = ListPicker(
      _("Add setting override"), available_groups.size(), cursor,
      renderer.OnListResized(), renderer, false,
      _("Choose a submenu, then check the settings to add. "
        "New overrides start with their current global values."));
    if (selected < 0)
      return;

    cursor = selected;
    const auto group = available_groups[selected];
    const auto selected_items = SelectSettings(
      parent, look, GetGroupLabel(group), C_("Button", "Add"),
      GetItems(group, true));
    if (selected_items.empty())
      continue;

    // Add the whole selection atomically: a full set or invalid global value
    // must not leave only part of the checked settings added.
    auto candidate = overrides;
    bool valid = true;
    for (const auto *item : selected_items) {
      const auto value = get_global_value(*item);
      if (!item->IsValid(value)) {
        ShowMessageBox(_("The current global value is not valid for this setting."),
                       gettext(item->label), MB_OK | MB_ICONEXCLAMATION);
        valid = false;
        break;
      }

      if (candidate.Set(*item, value) == SetDisplayOverrideResult::FULL) {
        ShowMessageBox(_("No more setting overrides can be added."),
                       _("Setting override"), MB_OK | MB_ICONEXCLAMATION);
        valid = false;
        break;
      }
    }

    if (!valid)
      continue;

    overrides = candidate;
    Refresh();
    return;
  }
}

void
ElementSetDisplayOverridesDialog::RemoveClicked()
{
  bool changed = false;
  if (!widget.Save(changed))
    return;

  std::vector<DisplaySettingGroup> available_groups;
  for (const auto group : groups)
    if (!GetItems(group, false).empty())
      available_groups.push_back(group);
  if (available_groups.empty())
    return;

  SettingGroupRenderer renderer(available_groups);
  const int selected = ListPicker(
    _("Remove setting overrides"), available_groups.size(), 0,
    renderer.OnListResized(), renderer, false,
    _("Removed overrides inherit their global settings again."));
  if (selected < 0)
    return;

  const auto group = available_groups[selected];
  const auto selected_items = SelectSettings(
    parent, look, GetGroupLabel(group), _("Remove"), GetItems(group, false));
  if (selected_items.empty())
    return;

  for (const auto *item : selected_items)
    overrides.Remove(item->key);
  Refresh();
}

bool
ShowElementSetDisplayOverridesDialog(
  UI::SingleWindow &parent, const DialogLook &look,
  ElementSetDisplayOverrides &overrides,
  std::span<const DisplaySettingDescriptor> catalog,
  GlobalDisplaySettingValueGetter get_global_value)
{
  ElementSetDisplayOverrides working = overrides;
  ElementSetDisplayOverridesDialog dialog(parent, look, working, catalog,
                                         get_global_value);
  if (dialog.ShowModal() != mrOK || working == overrides)
    return false;

  overrides = working;
  return true;
}
