# ESP32 foot-switch firmware — PROTOTYPE (not hardware-tested)

This sends one OSC message, `/alt/<UNIT_NAME>/toggle`, to AbleSet (UDP 39051) whenever the button is
pressed. AbleSet's Project Script then flips the shared state and writes the WING OSC. See
[docs/ABLESET-CONTROL.md](../../docs/ABLESET-CONTROL.md).

**Status:** code skeleton only. Nothing has been flashed or tested on hardware. Do not take it to a
show until you have verified: WiFi connect, one message per press, and AbleSet actually reacting.

## Build (Arduino IDE)

1. Install the ESP32 board package (Espressif `esp32`) and the Arduino `WiFi` library (ships with it).
2. Copy `src/config.h.example` → `src/config.h` and fill in **your** WiFi credentials, the AbleSet
   machine's IP, and the unit name. `src/config.h` is gitignored — credentials must never be committed.
3. Open `src/main.cpp`, select your ESP32 board + port, upload.
4. Watch Serial at 115200: it prints the assigned IP and each sent message.
5. With AbleSet running, press the button and confirm the corresponding canvas status changes.

## Behaviour

- Boots silently (sends nothing), waits for WiFi.
- 50 ms debounce, one message per press (on press, not release).
- Optional LED blink and optional SSD1306 display (`#define USE_DISPLAY` in `config.h`).
- If WiFi is down, prints a warning and does not send.

## Changing a WING channel

Do **not** edit this firmware. Edit `project-script.js` in `ableset-show-profile` and restart
AbleSet — the channel map lives there.
