// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

struct DialogLook;
struct MapElementSet;
namespace UI { class SingleWindow; }

/**
 * @return true when the #MapElementSet object has been modified
 */
bool
dlgConfigMapElementsShowModal(UI::SingleWindow &parent,
                              const DialogLook &dialog_look,
                              MapElementSet &data,
                              bool allow_name_change);
