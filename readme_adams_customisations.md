# adams_customisations branch

This branch consolidates work that used to live across several separate branches
(`14.3_conditional_triggering`, `14.3_pete`, `14.3_flashlight`, `0_15`), each of
which was forked from a different, older point in WLED's history.

Instead of maintaining multiple long-lived branches/forks, this branch tracks
upstream `main` directly, and per-board/per-device customisation is done via
`platformio_override.ini` (which is not gitignored in this repo) by
enabling/disabling usermods and setting build flags per PlatformIO environment.

## What's different from upstream

### New usermod: Conditional_Triggering
- `usermods/Conditional_Triggering/` — ported from the old `14.3_conditional_triggering`
  branch. Not present upstream (yet — this is intended to be submitted as a PR).
- Triggers one of two presets based on a sensor/switch's state, with optional
  debounce and auto-toggling between the two presets.
- Registered as `USERMOD_ID_CONDITIONAL_TRIGGER` (59) in `wled00/const.h`.
- Ported to the current self-registering usermod pattern (a `.cpp` with a static
  instance + `REGISTER_USERMOD`, plus `library.json`) rather than the old
  `usermods_list.cpp` `#ifdef`-based include — that mechanism no longer exists on
  main, since the `dynarray_multi` refactor.
- Enable it on a board by adding `Conditional_Triggering` to that env's
  `custom_usermods` in `platformio_override.ini`.

### Core changes: overridable button timing
- `wled00/button.cpp`: `WLED_DEBOUNCE_THRESHOLD`, `WLED_LONG_PRESS` and
  `WLED_DOUBLE_PRESS` are now `#ifndef`-guarded (previously plain `#define`s),
  so they can be set per-board with `-D WLED_LONG_PRESS=2000` etc. in
  `platformio_override.ini`, instead of editing the file directly.
- Carries forward the intent of the old `14.3_pete`/`14.3_flashlight` branches'
  button timing tweaks (faster debounce, longer long-press/double-press windows),
  without hardcoding them globally.

### platformio_override.ini
- Defines per-board build:
- Each env enables only the usermods it needs via `custom_usermods`, and
  configures them (GPIO pins, battery chemistry, LED count, etc.) via
  build flags — this is "one branch, per-board config" mechanism, 
  instead of separate branches per device.

## Not carried forward (superseded by upstream)

- **Battery usermod "15s startup delay" fix** (from `14.3_flashlight`): upstream's
  Battery usermod was rewritten (now split into `Battery.cpp`/`UMBattery.h`/
  `battery_defaults.h`/`types/*.h`) and already includes an equivalent initial
  voltage-stabilisation delay (`USERMOD_BATTERY_INITIAL_DELAY`). Nothing to port.
- **PWM frequency change** (`WLED_PWM_FREQ` → 2020, from `14.3_flashlight`): not
  hardcoded here — set `-D WLED_PWM_FREQ=2020` per-board in
  `platformio_override.ini` instead if a board needs it.
- **`MULTI_RELAY_MAX_RELAYS` 4 → 6** (from `14.3_pete`): already `#ifndef`-guarded
  upstream, so no branch was needed — set `-D MULTI_RELAY_MAX_RELAYS=6` per-board.
- **`0_15` branch**: contained no unique commits, just a checkout of the
  `v0.15.0-b4` tag.

## Old branches

`0_15`, `14.3_conditional_triggering`, `14.3_pete` and `14.3_flashlight` are kept
around for reference (e.g. until the Conditional_Triggering usermod is PR'd
upstream), but are no longer the basis for new work — use this branch instead.
