# Protocol evidence (verified, not guessed)

Sources are the vendored WING MCP server docs in
`C:\Users\Admin\Documents\wing-mcp-server-vendored\docs\wing-protocol\` and a read-only live query
on 2026-11-18.

| Fact | Evidence |
|---|---|
| WING control is OSC over UDP **2223**; console IP is static **192.168.1.3** | `02-osc-protocol.md`, `01-overview.md`; also StageCraft docs |
| ALT input address: `/ch/<N>/in/set/altsrc`, **0 = main/live, 1 = ALT/backing** | WING MCP `README.md` L114; live AbleSet canvases use the same |
| Live read-only check: `/ch/1/in/set/altsrc` → `valueKind=int`, `display="1"`, `value=1` | `wing_get` on 2026-11-18 |
| **WING silently ignores integer-tagged numeric SETs** — send whole numbers with `,f` (float) tag | `02-osc-protocol.md` "SET — simple", *Live caveat (verified on hardware)* |
| GET on a leaf returns `,sfi` triplet `(ascii, raw 0..1, real int)` for integer params | `02-osc-protocol.md` "GET — leaf value" |
| GET can be redirected to another UDP port with address prefix `/%<port>/...` | `02-osc-protocol.md` "Response redirection" |
| `SET -1` flips a boolean param without reading state first | `02-osc-protocol.md` "Toggle — the -1 trick". **Not yet tested on `altsrc`** — test only with operator sign-off, it is audible |
| OSC subscribe exists (`/*b`, `/*s`, `/*S`) but dies after ~10 s inactivity unless renewed | `02-osc-protocol.md` "Subscribe". Avoid: only **one OSC subscriber** is allowed on the console (`docs/bridge.md` L13) |
| AbleSet listens for inbound OSC on UDP **39051**; AI bridge listener is **39052**; `/subscribe` is for AI state reading only | `ableset-show-profile/docs/WHY-THESE-SETTINGS.md` L36–57 |
| **ALT canvas buttons send raw OSC straight from the client to the WING** — `/shared/...` only updates the on-screen label | `ableset-show-profile/docs/ABLESET-DIAGRAMS.md` L45/L58; canvas `onPress` = `"/shared/altKick 0; 192.168.1.3:2223/ch/1/in/set/altsrc 0"` |
| **User's live test (2026-11-18):** iPhone, Android and a second iPhone all opened AbleSet canvases and toggled ALT successfully — proving that a network device can send the WING OSC directly | reported by the human; consistent with the canvas code above |
| **Not proven:** a *remote* device sending an arbitrary OSC command into AbleSet on UDP 39051 and having `onOscChange()` react | all documented `/subscribe` examples use the local `http://127.0.0.1/settings/osc` tester |

## Consequence for this project

Under the recommended design ([ABLESET-CONTROL.md](ABLESET-CONTROL.md)) the device sends a single
OSC message with **no arguments** to AbleSet. No WING OSC encoding, no subscription, no state read
and no float-tag trap on the device side. All the WING-specific complexity stays in one place —
AbleSet's `project-script.js` — where it can be verified once.
