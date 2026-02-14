// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

namespace InfoBoxFactory
{
  enum Type {
#define INFOBOX_ENTRY(id, name, caption, description, create, update, panels) id,
#include "InfoBoxes/Content/InfoBoxList.inc"
#undef INFOBOX_ENTRY
    e_NUM_TYPES
  };

  static constexpr Type NUM_TYPES = e_NUM_TYPES;
  static constexpr Type MIN_TYPE_VAL = (Type)0;
  static constexpr Type MAX_TYPE_VAL = (Type)(e_NUM_TYPES - 1);
}
