# HamCore

**Off-grid LoRa mesh messaging for US amateur radio operators.**

HamCore is multi-hop mesh text messaging firmware for LoRa radios, built to be operated
legally under a US amateur radio license (FCC Part 97). Messages hop node-to-node with no
internet, no infrastructure, and no central server — across the 33cm (902–928 MHz) and
70cm (420–450 MHz) ham bands, at amateur power levels, with your callsign in every frame.

HamCore is a fork of [MeshCore](https://github.com/meshcore-dev/MeshCore) by
**Scott Powell (ripplebiz)** and the meshcore-dev contributors, released under the MIT
License. The mesh routing engine, hardware support, and companion protocol are their
work; HamCore's changes are the Part 97 compliance layer described below. If you don't
hold a ham license and want license-free (Part 15) meshing, use upstream MeshCore —
it's excellent.

> ⚠️ **HamCore is *not* over-the-air compatible with stock MeshCore.** HamCore frames
> use a different payload version and carry a callsign trailer, so the two networks
> reject each other's packets cleanly instead of half-working.

## How HamCore differs from MeshCore

| | MeshCore (Part 15) | HamCore (Part 97) |
|---|---|---|
| Message content | AES-128 encrypted | **Plaintext** (§97.113(a)(4) prohibits obscuring meaning) |
| Authentication | Ed25519 identities + MACs | **Kept** — signed adverts, keyed MACs, HMAC login proof |
| Station ID | None | **Callsign in every transmitted frame**; TX inhibited until configured (§97.119) |
| Frequencies | Regional ISM presets | **Enforced 420–450 / 902–928 MHz** (§97.301) |
| Admin login | Password (encrypted) | **HMAC proof-of-password** — the password never goes on the air |
| Privacy | Encrypted | **None.** Everything is publicly readable, by design and by law |

The full compliance rationale — what each rule requires and exactly what was changed —
is in **[HAMCORE.md](HAMCORE.md)**. Read it before transmitting.

## What you need

- **A US amateur radio license, Technician class or higher**, to transmit.
  Receive-only monitoring needs no license.
- **A supported LoRa board.** HamCore inherits MeshCore's hardware support — 87 board
  variants including Heltec V3/V4, RAK4631/WisMesh, LilyGO T-Deck / T-Echo / T-Beam,
  Seeed T1000-E and Xiao, Station G2/G3, and more. See `variants/` for the full list.
  One hardware note: nearly all of these boards ship with RF front ends tuned for
  868/915 MHz — great for 33cm. For 70cm you want a 433 MHz-band radio module variant.
- [PlatformIO](https://platformio.org/) to build and flash.

## Getting started

### 1. Build and flash

Each board has environments for every node role, defined in
`variants/<board>/platformio.ini`:

```bash
git clone https://github.com/tek126/hamcore
cd hamcore

# build (example: Heltec V3 companion radio with BLE)
pio run -e Heltec_v3_companion_radio_ble

# build + flash over USB
pio run -t upload -e Heltec_v3_companion_radio_ble

# or produce versioned release artifacts
./build.sh build-firmware RAK_4631_repeater
```

Common environment name patterns: `<Board>_companion_radio_ble|usb|wifi`,
`<Board>_repeater`, `<Board>_room_server`, `<Board>_sensor`, `<Board>_terminal_chat`.

### 2. First boot — set your callsign

A freshly flashed HamCore node **will not transmit**. TX is inhibited until the node
name begins with a valid US callsign. Connect over USB serial (or with
[hamcore-cli](https://github.com/tek126/hamcore-cli)) and:

```
set name W1AW           # your callsign — or W1AW-2, "W1AW Base", etc.
```

The callsign prefix of the name is validated (e.g. `W1AW`, `KD2ABC`), embedded in every
frame you transmit, and shown in your advert.

### 3. Pick a band and frequency

```
set band 33cm           # 906.875 MHz, BW 250, SF 10, CR 5 (the default)
set band 70cm           # 433.500 MHz, BW 250, SF 10, CR 5
set radio 927.5,250,10,5   # or any in-band value: freq,bw,sf,cr
set tx 22               # TX power in dBm (clamped to the board's safe max)
reboot
```

Frequencies outside 420–450 / 902–928 MHz are refused everywhere: CLI, companion app
commands, and stored-preference loading. All nodes on a mesh must share the same
frequency, bandwidth, spreading factor, and coding rate.

### 4. Talk

```
advert                  # announce yourself (flood)
```

Then message other nodes from the CLI client or a companion app. Repeaters extend
range automatically; a `set advert.interval 10` gives an infrastructure node a
10-minute ID beacon (belt-and-suspenders — every frame already carries the callsign).

## Node roles

| Firmware | What it does |
|---|---|
| [Companion Radio](examples/companion_radio) | Your handheld/desk node — pairs with a client over BLE, USB, or Wi-Fi |
| [Simple Repeater](examples/simple_repeater) | Relays traffic to extend mesh coverage; remote-manageable |
| [Simple Room Server](examples/simple_room_server) | A BBS-style shared message board nodes can sync from |
| [Simple Sensor](examples/simple_sensor) | Remote telemetry node with alerting |
| [Terminal Chat](examples/simple_secure_chat) | Standalone chat over a plain serial terminal |
| [KISS Modem](examples/kiss_modem) | Raw LoRa TNC for host applications — bypasses the mesh stack, so **station ID is the host's responsibility** |

## Client software

- **[hamcore-cli](https://github.com/tek126/hamcore-cli)** — terminal client
  (interactive chat, contact management, remote repeater admin). The supported client.
- **[hamcore_py](https://github.com/tek126/hamcore_py)** — the Python library behind
  it, for building your own tools and bots.

Stock MeshCore phone/web apps speak the same BLE/serial companion protocol and may
partially work, but they are untested against HamCore and their radio-preset screens
offer Part 15 frequencies that HamCore firmware will refuse. Use the CLI until a
dedicated app exists.

## Remote administration

Repeaters, room servers, and sensors accept remote logins. HamCore replaces MeshCore's
password-in-packet login with an **HMAC-SHA256 proof-of-password**: the client transmits
a 16-byte tag computed from the password, the timestamp, and both stations' public keys —
never the password itself. Replays are rejected via the server's monotonic timestamp
check, and changing a password over RF is refused (local serial only), so passwords
never appear on the air.

Remember: everything on this network is plaintext and publicly readable. Treat repeater
passwords as access tokens, not secrets, and never reuse a password from anything else.

## Operating notes (the part between you and the FCC)

- **No expectation of privacy.** Every message, including "direct" ones, is readable by
  anyone with a receiver. That is the law of the band, not a bug.
- **Follow your local band plan.** Both bands are shared; 33cm also carries heavy
  Part 15 ISM traffic. Coordinate wideband use regionally.
- **§97.313 — minimum power necessary.** You *may* run more power than Part 15 allows;
  that doesn't mean you should.
- **You are the control operator.** Content rules (§97.113 — third-party traffic,
  business use, prohibited content) apply to what your station transmits, including
  what it repeats.

## Development

```bash
pio test -e native            # unit tests (gtest) — includes the callsign validator
pio test -e native_kiss_modem # KISS modem tests
```

Core mesh code is in `src/`, shared helpers in `src/helpers/` (the HamCore-specific
logic lives mainly in `src/helpers/HamRadio.*`, `src/helpers/AuthHelpers.*`,
`src/Dispatcher.cpp`, and `src/Utils.cpp`), and per-role firmware in `examples/`.

Issues and PRs are welcome at [github.com/tek126/hamcore](https://github.com/tek126/hamcore).
Keep upstream's embedded coding style: no dynamic allocation outside setup paths, match
the existing brace/indent style, keep it simple. To pull upstream MeshCore fixes:
`git fetch upstream && git merge upstream/main`.

## Credits & license

- **[MeshCore](https://github.com/meshcore-dev/MeshCore)** — Scott Powell (ripplebiz)
  and contributors. HamCore exists because MeshCore is well-built, portable, and
  MIT-licensed. Support upstream: [buymeacoffee.com/ripplebiz](https://buymeacoffee.com/ripplebiz).
- HamCore modifications © 2026, released under the same **MIT License**
  (see [license.txt](license.txt)).

*HamCore is independent of and unaffiliated with the MeshCore project. Nothing here is
legal advice — verify against the current Part 97 text before operating.*
