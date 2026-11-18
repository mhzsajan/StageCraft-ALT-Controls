# ESP32 foot-switch firmware — PROTOTYPE (not hardware-tested)

This talks **directly to the WING**, using the exact path the AbleSet phone canvases use (proven on
iPhone/Android in the user's live test): on each press it GETs the current
`/ch/<WING_CHANNEL>/in/set/altsrc` value, flips it, and SETs it back with a **float** tag. No AbleSet
inbound, no subscribe, no computer. See [docs/PROTOCOL-EVIDENCE.md](../../docs/PROTOCOL-EVIDENCE.md).

**Status:** code skeleton only. Nothing has been flashed or tested on hardware. Do not take it to a
show until you have verified: WiFi connect, one message per press, and AbleSet actually reacting.

## Build (Arduino IDE)

1. Install the ESP32 board package (Espressif `esp32`) and the Arduino `WiFi` library (ships with it).
2. Copy `src/config.h.example` → `src/config.h` and fill in **your** WiFi credentials, `WING_IP`
   (`192.168.1.3`), `WING_CHANNEL` for this member, and the unit name. `src/config.h` is gitignored —
   credentials must never be committed.
3. Open `src/main.cpp`, select your ESP32 board + port, upload.
4. Watch Serial at 115200: it prints the assigned IP and each GET/SET result.
5. Press the button and confirm the corresponding **WING channel's altsrc** flips (read it back with
   the WING MCP `wing_get`). The canvas status label will **not** follow a hardware switch on this
   direct path — that is expected; see the README's comparison table.

## Behaviour

- Boots silently (sends nothing), waits for WiFi.
- 50 ms debounce, one message per press (on press, not release).
- Optional LED blink and optional SSD1306 display (`#define USE_DISPLAY` in `config.h`).
- If WiFi is down, prints a warning and does not send.

## Changing a WING channel

Do **not** edit this firmware. Edit `project-script.js` in `ableset-show-profile` and restart
AbleSet — the channel map lives there.
