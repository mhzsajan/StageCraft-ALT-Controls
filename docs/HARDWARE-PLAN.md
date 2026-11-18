# Hardware plan (prototype — nothing built or tested yet)

## Per-unit bill of materials (suggested)

| Part | Why | Notes |
|---|---|---|
| ESP32-WROOM-32 dev board | WiFi + enough GPIO, cheap | WROVER also fine |
| Momentary foot switch (normally open) | Guitar / bass / flute / drummer | Use a proper "kill switch" style housing so it does not slide |
| Small handheld push button (same switch) | Vocalist, if not using AirPods | A single large button is easier under stage stress than a gesture |
| Optional I²C OLED (SSD1306 0.96") | Show `WIFI OK` / `SENT` / `MAIN/ALT` | Not required for function |
| Optional LED + 330 Ω resistor | Cheap visual confirmation of a press | Strongly recommended on a dark stage |
| Power: USB power bank (5 V) or 18650 + TP4056 | Portable | Power banks are simplest and safe |
| Enclosure | Protect the electronics | 3D print or small project box; cable strain relief |

## GPIO / behaviour (draft)

- Button on a GPIO with `INPUT_PULLUP` (connect switch between pin and GND).
- Debounce ~50 ms; send **one** message per press, never on release.
- On boot: send **nothing**. Only connect WiFi, show status, wait for a press.
- On press: send `/alt/<unit>/toggle` to AbleSet IP, UDP 39051; blink LED / show `SENT`.
- If WiFi is down: do not send; indicate it on the display/LED. The **phone canvas remains the
  fallback** for every member.

## Per-unit config

Each unit gets its own `config.h` (copied from `firmware/esp32/src/config.h.example`, never
committed with real WiFi credentials):

```cpp
#define WIFI_SSID      "SHOW_NETWORK"
#define WIFI_PASS      "..."
#define ABLESET_IP     "192.168.1.x"   // the show machine running AbleSet
#define ABLESET_PORT   39051
#define UNIT_NAME      "guitar"        // guitar | bass | flute | vocals | drums
#define BUTTON_PIN     4
```

The **channel map is not in the device** — it lives in AbleSet's `project-script.js`. Changing a WING
channel means editing one script and restarting AbleSet, not reflashing five switches.

## Drummer note

The drummer's feet are busy on kick/hi-hat. Options to decide with the human:

1. A button mounted on the rack within reach of a hand.
2. A trigger pad already connected to the kit, mapped to the same OSC via an intermediate helper.
3. A toe-side switch on the kick pedal (mechanical fit must be checked).

## Power / reliability notes

- Test with a power bank before committing to a Li-ion build.
- WiFi must be on the same `192.168.1.0/24` network as the **show MacBook** (the machine running
  Ableton + AbleSet + Videosync). `ABLESET_IP` in each unit's config must be that Mac's IP.
- The ZTE H196A already has DHCP reservations for several Macs (`.5`, `.6`, `.7`, `.52`). Either:
  - **recommended:** dedicate one reservation to the "show Mac" role and always use that Mac for the
    rig, so no ESP32 config ever changes; or
  - if the M1 Air and M2 Pro swap, give each its own reservation and update `ABLESET_IP` (or resolve
    `show-mac.local` via mDNS) before the show.
- DHCP reservation for each ESP32 is also strongly recommended so they are easy to identify on the
  network.
- Keep a phone with the canvases charged — hardware is a convenience, not the only path.
