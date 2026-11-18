# AirPods as the vocalist's ALT switch (AirCursor experiment)

The vocalist already has AirPods. [AirCursor](https://github.com/m-afham/AirCursor) (MIT) is a macOS
menu-bar app that reads the AirPods' head-motion sensors and turns gestures into mouse clicks and
keyboard actions. It is a plausible way to trigger an ALT toggle without carrying another button.

**Confirmed rig (2026-11-18):** the vocalist has **AirPods Pro**, and the show has an **M1 Air** and
an **M2 Pro** with current macOS. All AirCursor requirements are therefore met. The question is no
longer "will it run" but "will a stem press reliably reach the show Mac and not a music app".

## What AirCursor actually requires (from its README, checked 2026-11-18)

- **macOS 15 Sequoia or later.**
- Headphones with motion sensors: **AirPods Pro (any gen), AirPods Max, AirPods 3rd gen+**, or
  Beats Fit Pro. Plain AirPods 1/2 will **not** work.
- Permissions: Accessibility, Motion, Bluetooth.
- The AirPods must be **playing audio on that Mac** — otherwise no motion data arrives. AirCursor
  tries to switch the Mac's output to the AirPods automatically.
- Gestures available: stem press 1×/2×/3×, nod, shake, tilt; hover-click; remappable actions
  include clicks and shortcut-type actions. It can synthesize keyboard events (`CGEvent`).

## Stem click mapping — verified from AirCursor source (2026-11-18)

The earlier uncertainty is resolved. AirCursor's `StemControlService.swift` registers with macOS's
`MPRemoteCommandCenter` and captures **stem presses directly** (no head motion needed):

| AirPods action | Media command AirCursor captures | Trigger |
|---|---|---|
| Press stem once | play / pause / togglePlayPause | `stemSingle` |
| Press stem **twice** | **next track** | `stemDouble` |
| Press stem three times | previous track | `stemTriple` |

AirCursor claims the "Now Playing" role while active (`AudioKeepAlive` keeps it genuinely playing),
which is how it receives those commands instead of a music app.

`Actions.swift` shows the remappable actions. There is **no arbitrary "custom hotkey" action**, but
there are fixed actions that emit real keyboard events: `back` = **⌘[**, `forward` = **⌘]**, plus
Mission Control / Spaces / screenshot. A free global listener (e.g. **Hammerspoon**) can catch ⌘[ and
run our sender — so the chain works without modifying AirCursor:

```mermaid
flowchart LR
    AP[AirPods Pro stem DOUBLE press] --> AC[AirCursor: stemDouble → Back (⌘[ )]
    AC -->|⌘[| HK[Hammerspoon global listener]
    HK -->|executes| PY[mac/send_alt_toggle.py vocals]
    PY -->|UDP OSC /alt/vocals/toggle :39051| AS[AbleSet project script]
    AS --> W[WING channels 12/15/16]
```

**Recommended binding:** `stemDouble → Back (⌘[)`. It is deliberate (a vocalist never double-presses
by accident while singing), and ⌘[ is unlikely to be used elsewhere on the show Mac. Avoid `nod` and
avoid single press (play/pause) for the same reason.

AirCursor itself does **not** send OSC. The missing pieces are the hotkey choice above and a small
listener that runs the prototype sender in [`mac/`](../mac/README.md).

### Remaining risks

- **Now Playing conflict:** if another app (music player, even a browser tab) owns Now Playing on
  that Mac, stem presses may go there instead. AirCursor claims the role while tracking, but test it
  with the actual show apps open.
- **Permissions:** Accessibility is needed for AirCursor and for the hotkey listener; after an
  AirCursor update macOS may require toggling it off/on again.
- **Single show Mac (confirmed):** Ableton + AbleSet + Videosync all run on the same MacBook, so
  AirCursor and the hotkey listener must run on that machine too. The vocalist's AirPods should be
  paired to it. Test the chain with Ableton and Videosync actually open: whichever app owns macOS's
  "Now Playing" can steal the stem press. AirCursor claims the role while tracking, but this rig has
  more audio apps than a plain desktop, so a rehearsal test is mandatory. If the M1 Air and M2 Pro
  swap show duty, install/configure AirCursor + listener on both.

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
