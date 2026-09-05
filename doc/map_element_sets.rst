Map Element Sets
================

``MapElementSettings`` owns eight reusable sets. ``PageActions`` selects a
fixed set or resolves ``Auto`` from the flight mode. A set combines directly
owned map controls with sparse overrides of global settings. Removing an
override restores inheritance; storing the same value as the global setting
still creates an explicit override.

Adding an overridable setting
----------------------------

Keep these mappings consistent when extending the catalog:

1. Allocate an explicit, unused numeric key in
   ``src/MapDisplay/DisplaySettingCatalog.hpp`` and a unique profile suffix.
   Never derive either from array position or reuse a retired identifier.
2. Add the descriptor in ``DisplaySettingCatalog.cpp`` with the owning group,
   translated label/help, type, validation bounds, enum choices or integer
   step, and required effects. Integer descriptors must specify their numeric
   presentation: number, percentage, altitude, or duration. Altitudes persist
   in metres and durations in seconds, regardless of the editor's units.
3. Extend the group's global/effective bundle and accessors, validated profile
   loader, and application callback. Check both supported and unsupported
   feature configurations. Enable ``element_set_overwritable`` only when
   these paths are implemented.
4. Keep global panels and quick actions editing the global baseline. They
   save profile values and call ``ReloadGlobalElementSetDisplaySettings()``
   so an active override continues to win. Do not write the effective value
   back into the global profile.
5. Update catalog counts and the relevant catalog, profile, and domain tests.
   Check defaults, invalid values, round trips, global inheritance, and a
   selected override surviving global reload. Add new source files to Make
   and the Xcode navigation groups.
6. Check the editor and manual. The picker uses the corresponding Map Display
   submenu; each form section reserves one of its 32 rows for a heading and
   contains at most 31 inputs. Larger groups span multiple sections in one
   scrollable list. Adding settings validates the entire selection before
   changing the dialog's working copy. Only the outer OK writes that copy
   back to the enclosing set editor.

Airspace warning enablement, dialogue visibility, timing, sound, and margin
are currently eligible overrides. These affect active warning behaviour on
page or automatic flight-mode changes. The editor and manual explain this;
class visibility overrides affect display only. Changes to warning policy
eligibility require an explicit product decision and migration review.

Verification on a build-capable device
-------------------------------------

Run ``TestElementSetDisplayOverrides``,
``TestElementSetDisplayOverrideService``, ``TestDisplaySettingProfile``,
``TestWaypointDisplaySettings``, ``TestAirspaceDisplaySettings``, and
``TestProfile`` using the project's normal test workflow. These cover the
model, catalog, service, and profile/domain mappings; they do not substitute
for testing the dialog or the real runtime side effects together.

Exercise the following in the simulator, using a disposable profile:

* Start with an empty set. Add opens submenus in the main configuration
  order. Check several items, use Select all and Clear, and cancel a
  selection. Already overridden settings must not be offered again.
* Add all Airspace settings and then all remaining settings. Scroll to the
  last input with touch/mouse and keyboard, in portrait and landscape, and
  after resizing. Confirm every selected setting remains accessible.
* Change a boolean, enum, percentage, altitude, and duration directly through
  their input fields. Check metres and feet, including unchanged values and
  the altitude limits, then add/remove other settings before saving.
* Cancel the override editor after mixed additions, edits, and removals.
  Reopen it and verify the old state. Also accept the override editor and
  cancel the enclosing set editor. Verify copy/paste includes overrides and
  that accepting both dialogs survives a profile reload.
* Switch Circling/Cruise/Final Glide with an Auto page, then with a fixed-set
  page. Enter and leave full-screen and pan layouts. Confirm the selected set
  and effective values remain correct.
* Change a global terrain, waypoint, or airspace setting while its override
  is active. Confirm the override still wins; remove it and confirm the new
  global value takes effect. Observe terrain cache, waypoint appearance,
  redraw/projection, and airspace-computer updates.

Runtime robustness follow-up
---------------------------

The service's bool application result currently conflates no change with
failure. Accessor failures can leave partially applied state, and callers
discard some results. Resolving this needs an explicit result contract,
failure-injection tests, and integration tests of the actual adapters before
changing runtime application or initialisation semantics.
