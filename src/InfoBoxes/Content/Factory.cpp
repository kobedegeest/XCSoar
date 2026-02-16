// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "InfoBoxes/Content/Factory.hpp"

#include "InfoBoxes/Content/Base.hpp"
#include "InfoBoxes/Content/Alternate.hpp"
#include "InfoBoxes/Content/Altitude.hpp"
#include "InfoBoxes/Content/Direction.hpp"
#include "InfoBoxes/Content/Glide.hpp"
#include "InfoBoxes/Content/MacCready.hpp"
#include "InfoBoxes/Content/Other.hpp"
#include "InfoBoxes/Content/Speed.hpp"
#include "InfoBoxes/Content/Task.hpp"
#include "InfoBoxes/Content/Places.hpp"
#include "InfoBoxes/Content/Contest.hpp"
#include "InfoBoxes/Content/Team.hpp"
#include "InfoBoxes/Content/Terrain.hpp"
#include "InfoBoxes/Content/Thermal.hpp"
#include "InfoBoxes/Content/Time.hpp"
#include "InfoBoxes/Content/Trace.hpp"
#include "InfoBoxes/Content/Weather.hpp"
#include "InfoBoxes/Content/Airspace.hpp"
#include "InfoBoxes/Content/Radio.hpp"
#include "InfoBoxes/Content/Engine.hpp"

#include "util/Macros.hpp"
#include "util/StringAPI.hxx"
#include "Language/Language.hpp"

#include <cstddef>
#include <cassert>
#include <optional>

/**
 * An #InfoBoxContent implementation that invokes a callback.  This is
 * used for those contents that would implement only the Update()
 * method and need no context.
 */
class InfoBoxContentCallback : public InfoBoxContent {
  void (*update)(InfoBoxData &data) noexcept;
  const InfoBoxPanel *panels;

public:
  InfoBoxContentCallback(void (*_update)(InfoBoxData &data) noexcept,
                         const InfoBoxPanel *_panels) noexcept
    :update(_update), panels(_panels) {}

  void Update(InfoBoxData &data) noexcept override {
    update(data);
  }

  const InfoBoxPanel *GetDialogContent() noexcept override {
    return panels;
  }
};

template<class T>
struct IBFHelper {
  static InfoBoxContent *Create() noexcept {
    return new T();
  }
};

template<class T, int param>
struct IBFHelperInt {
  static InfoBoxContent *Create() noexcept {
    return new T(param);
  }
};

using namespace InfoBoxFactory;

struct MetaData {
  /** Stable identifier for name-based storage (e.g. "e_HeightGPS"). */
  const char *id;
  const char *name;
  const char *caption;
  const char *description;
  InfoBoxContent *(*create)() noexcept;
  void (*update)(InfoBoxData &data) noexcept;
  const InfoBoxPanel *panels;

  /**
   * Implicit instances shall not exist.  This declaration ensures at
   * compile time that the meta_data array is not larger than the
   * number of explicitly initialised elements.
   */
  MetaData() = delete;

  constexpr MetaData(const char *_id,
                     const char *_name,
                     const char *_caption,
                     const char *_description,
                     InfoBoxContent *(*_create)() noexcept) noexcept
    :id(_id), name(_name), caption(_caption), description(_description),
     create(_create), update(nullptr), panels(nullptr) {}

  constexpr MetaData(const char *_id,
                     const char *_name,
                     const char *_caption,
                     const char *_description,
                     void (*_update)(InfoBoxData &data) noexcept) noexcept
    :id(_id), name(_name), caption(_caption), description(_description),
     create(nullptr), update(_update), panels(nullptr) {}

  constexpr MetaData(const char *_id,
                     const char *_name,
                     const char *_caption,
                     const char *_description,
                     void (*_update)(InfoBoxData &data) noexcept,
                     const InfoBoxPanel _panels[]) noexcept
    :id(_id), name(_name), caption(_caption), description(_description),
     create(nullptr), update(_update), panels(_panels) {}
};

/* WARNING: Never insert or delete items or rearrange the order of the items
 * in this array. This will break existing infobox configurations of all users!
 */

constexpr MetaData MakeMetaData(const char *id,
                                const char *name,
                                const char *caption,
                                const char *description,
                                InfoBoxContent *(*create)() noexcept,
                                void (*update)(InfoBoxData &data) noexcept,
                                const InfoBoxPanel *panels) noexcept
{
  if (create != nullptr)
    return MetaData(id, name, caption, description, create);
  else if (panels != nullptr)
    return MetaData(id, name, caption, description, update, panels);
  else
    return MetaData(id, name, caption, description, update);
}

/** Stringify enum id for stable name-based storage (e.g. e_HeightGPS -> "e_HeightGPS"). */
#define INFOBOX_ENTRY(id, name, caption, description, create, update, panels) \
  MakeMetaData(_T(#id), name, caption, description, create, update, panels),

#define CYCLE_CREATE(name, idx) IBFHelperInt<name, idx>::Create

static constexpr MetaData meta_data[] = {
#include "InfoBoxes/Content/InfoBoxList.inc"
};

#undef INFOBOX_ENTRY
#undef CYCLE_CREATE

static_assert(ARRAY_SIZE(meta_data) == NUM_TYPES,
              "Wrong InfoBox factory size");

const char *
InfoBoxFactory::GetName(Type type) noexcept
{
  assert(type < NUM_TYPES);

  return meta_data[type].name;
}

const char *
InfoBoxFactory::GetCaption(Type type) noexcept
{
  assert(type < NUM_TYPES);

  return meta_data[type].caption;
}

/**
 * Returns the long description (help text) of the info box type.
 */
const char *
InfoBoxFactory::GetDescription(Type type) noexcept
{
  assert(type < NUM_TYPES);

  return meta_data[type].description;
}

const TCHAR *
InfoBoxFactory::GetId(Type type) noexcept
{
  assert(type < NUM_TYPES);

  return meta_data[type].id;
}

std::unique_ptr<InfoBoxContent>
InfoBoxFactory::Create(Type type) noexcept
{
  assert(type < NUM_TYPES);
  const auto &m = meta_data[type];

  assert(m.create != nullptr ||
         m.update != nullptr);

  if (m.create != nullptr)
    return std::unique_ptr<InfoBoxContent>(m.create());
  else
    return std::make_unique<InfoBoxContentCallback>(m.update, m.panels);
}

std::optional<InfoBoxFactory::Type>
InfoBoxFactory::FindTypeByName(const char *name) noexcept
{
  if (name == nullptr)
    return std::nullopt;

  for (unsigned i = 0; i < NUM_TYPES; ++i) {
    if (StringIsEqual(meta_data[i].id, name))
      return static_cast<Type>(i);
  }
  return std::nullopt;
}
