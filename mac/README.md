# Mac helper — hotkey / AirCursor gesture → AbleSet OSC

AirCursor itself is a mouse/keyboard app; it does not send OSC. This tiny Python sender is the bridge:
a global hotkey listener (Hammerspoon, Keyboard Maestro, BetterTouchTool, or a small Swift helper)
calls `send_alt_toggle.py vocals`, and the script sends `/alt/vocals/toggle` to AbleSet on UDP 39051.
From there AbleSet's Project Script does the WING work.

**Status:** prototype helper, unit-tested for the OSC bytes only. Not integrated with a hotkey
listener yet — that is the next development step, to be decided with the human.

## Usage

```bash
python3 send_alt_toggle.py vocals --host 192.168.1.2
python3 send_alt_toggle.py guitar --host 192.168.1.2 --port 39051
```

Units: `guitar`, `bass`, `flute`, `vocals`, `drums`.

## Suggested integration with AirCursor

1. In AirCursor, set trigger **"Press stem twice" (`stemDouble`)** → action **"Back (⌘[)"**.
   AirCursor has no arbitrary-custom-hotkey action; Back/Forward are fixed actions that emit real
   ⌘[ / ⌘] key events, which a listener can catch. Avoid `nod` (the vocalist nods to music constantly)
   and single press (play/pause).
2. In a global listener such as **Hammerspoon**, bind ⌘[ to run
   `python3 .../send_alt_toggle.py vocals`. AirCursor captures the stem press via macOS's
   `MPRemoteCommandCenter` — see [docs/AIRPODS.md](../docs/AIRPODS.md) for the source evidence.
3. Rehearse once: double-pressing the AirPods stem must toggle the WING channel and the canvas
   status label, with the show apps open (to catch any "Now Playing" conflict).

See [docs/AIRPODS.md](../docs/AIRPODS.md) for the AirPods model/permission requirements and risks.
