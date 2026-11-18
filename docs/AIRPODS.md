# AirPods as the vocalist's ALT switch (AirCursor experiment)

The vocalist already has AirPods. [AirCursor](https://github.com/m-afham/AirCursor) (MIT) is a macOS
menu-bar app that reads the AirPods' head-motion sensors and turns gestures into mouse clicks and
keyboard actions. It is a plausible way to trigger an ALT toggle without carrying another button.

## What AirCursor actually requires (from its README, checked 2026-11-18)

- **macOS 15 Sequoia or later.**
- Headphones with motion sensors: **AirPods Pro (any gen), AirPods Max, AirPods 3rd gen+**, or
  Beats Fit Pro. Plain AirPods 1/2 will **not** work.
- Permissions: Accessibility, Motion, Bluetooth.
- The AirPods must be **playing audio on that Mac** — otherwise no motion data arrives. AirCursor
  tries to switch the Mac's output to the AirPods automatically.
- Gestures available: stem press 1×/2×/3×, nod, shake, tilt; hover-click; remappable actions
  include clicks and shortcut-type actions. It can synthesize keyboard events (`CGEvent`).

## How it would connect to the show

```mermaid
flowchart LR
    AP[AirPods motion] --> AC[AirCursor on Mac]
    AC -->|gesture → hotkey| HK[Small local listener]
    HK -->|UDP OSC /alt/vocals/toggle :39051| AS[AbleSet project script]
    AS --> W[WING channels 12/15/16]
```

AirCursor itself does **not** send OSC. The missing piece is a tiny local listener that catches a
chosen hotkey and sends the same `/alt/vocals/toggle` message as an ESP32. A prototype sender lives
in [`mac/`](../mac/README.md).

## Risks — decide before relying on it at a show

1. **False triggers.** A vocalist nods to the music constantly. A nod configured as the toggle would
   fire accidentally. Prefer a deliberate **stem double-press** mapped to the action.
2. **Audio route conflict.** AirPods must be the Mac's audio output for motion data; if the vocalist
   needs in-ear monitoring from the WING/PC device instead, that conflicts.
3. **"Now Playing" conflict.** macOS may route stem presses to the music app instead of AirCursor
   (documented in their README). Nod/tilt gestures still work.
4. **Permissions/version drift.** After an AirCursor update, macOS may need Accessibility toggled
   off/on again. That is build-time maintenance, not show-time.
5. **Third-party dependency.** It is not affiliated with this repo or Apple. Verify the exact
   AirPods model and test a full show rehearsal before trusting it.

## Recommendation

Try a **cheap handheld button first** — it is deterministic, needs no permissions, and behaves the
same on every show. Treat AirPods/AirCursor as an experiment for rehearsals; keep the phone canvas
as the always-working fallback. If the experiment succeeds, document the exact gesture + hotkey here.
