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
#define WING_IP        "192.168.1.3"   // direct-to-WING path (phone-canvas proven)
#define WING_PORT      2223
#define WING_CHANNEL   1               // guitar=1, bass=9, flute=10, vocals=12, drums=TBD
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
- WiFi must be on the same `192.168.1.0/24` network as the **WING** (`192.168.1.3`) — that is the
  only address the toggle path needs. The show Macs can be on the same network, but they are not in
  the toggle path on this design.
- **DHCP reservation is keyed by MAC address — one per network interface.** Both show Macs are
  connected by **WiFi and Ethernet at the same time**, so each has *two* MACs and can receive *two*
  different reserved IPs on the same subnet. That is ambiguous: a device sending to the wrong one
  reaches nothing. Pick **one interface as the show interface** (Ethernet is more reliable for a
  live rig), reserve **that interface's MAC**, and keep the other interface off the show network (or
  at least know which IP AbleSet actually answers on). Verify from the Mac with `ifconfig` / Network
  Settings which MAC ↔ IP pair is the reserved one.
- If the M1 Air and M2 Pro swap show duty, it does **not** affect the recommended direct-to-WING
  toggle path — the ESP32s only care about `WING_IP` (`192.168.1.3`). It matters only for the
  experimental Option B and for AirPods/AirCursor (which must run on the show Mac). If you use B,
  either dedicate one "show Mac" role with a fixed reserved IP, or keep both reservations and update
  `ABLESET_IP` (or resolve `show-mac.local` via mDNS) before the show.
- **ESP32 devices: yes, a MAC/IP reservation per unit solves the addressing problem.** Each ESP32
  has a unique WiFi STA MAC; reserve one IP per foot switch (e.g. `.10` guitar, `.11` bass, ...).
  Then each unit is stable and identifiable. Optionally also give it an mDNS name such as
  `alt-guitar.local` in the firmware. Note the reservation must use the MAC the ESP32 actually
  presents (it is burned in unless the firmware overrides it).
- **Still unverified:** that AbleSet accepts a *remote* OSC command on UDP 39051 from another device.
  The documented `/subscribe` examples all use the local web UI. Test this early — see
  [ABLESET-CONTROL.md](ABLESET-CONTROL.md#what-must-be-verified-before-committing-to-this-design).
  If AbleSet only listens locally, use the fallback: devices send their OSC **directly to the WING**
  (the same packets the current phone canvases send), which needs no AbleSet inbound at all.
- Keep a phone with the canvases charged — hardware is a convenience, not the only path.
