# HamCore

HamCore is a fork of [MeshCore](https://github.com/meshcore-dev/MeshCore) modified to be
operable under a US amateur radio license (FCC Part 97). It is an off-grid LoRa mesh
messaging system — same hardware, same mesh routing, same companion protocol — with the
changes required to transmit legally as a licensed amateur station.

**The HamCore ecosystem**

| Repository | Contents |
|---|---|
| [hamcore](https://github.com/tek126/hamcore) | This repo — HamCore firmware (fork of MeshCore v1.17.1) |
| [hamcore_py](https://github.com/tek126/hamcore_py) | Python companion library (fork of `meshcore_py`) |
| [hamcore-cli](https://github.com/tek126/hamcore-cli) | Terminal client (fork of `meshcore-cli`) |

---

## What Part 97 requires, and how HamCore complies

### 1. No obscured message content — §97.113(a)(4)

Amateur stations may not transmit "messages encoded for the purpose of obscuring their
meaning." Stock MeshCore encrypts every direct message, channel message, and admin
exchange with AES-128 under ECDH-derived or pre-shared keys.

**HamCore change:** the payload cipher is now an identity transform
(`firmware/src/Utils.cpp`). All message content travels as plaintext (zero-padded to the
old 16-byte block size so the wire framing is unchanged).

What is deliberately **kept**, because authentication does not obscure meaning and is
permitted:

- **Ed25519 identity keys and advert signatures** (`src/Identity.cpp`, untouched).
  Nodes still prove who they are; adverts with bad signatures are still dropped.
- **The truncated keyed HMAC prefix** on each payload. In MeshCore's design this
  2-byte MAC doubles as the recipient/channel selector (destination hashes are only
  1 byte), so it is retained as an authentication/addressing code. The message it
  covers is plaintext.
- **Login authentication** uses an HMAC proof-of-password (see Operator
  responsibilities below) — an authentication code, not content encryption, and
  likewise permitted under §97.113(a)(4).

### 2. Station identification — §97.119

Every station must transmit its callsign at the end of each communication and at least
every 10 minutes.

**HamCore changes:**

- **Callsign in every transmitted frame.** All RF frames carry a fixed 8-byte ASCII
  callsign trailer, appended at the single radio choke point
  (`Dispatcher::checkSend`) and identifying the *transmitting* station — so a repeater
  retransmitting someone else's packet correctly identifies itself. HamCore frames are
  marked payload version 2 in the packet header; stock MeshCore (version 1) frames are
  cleanly rejected rather than misparsed, and vice versa.
- **TX inhibit until a callsign is configured.** A node with no valid callsign will
  not key the transmitter at all.
- **Node name must start with your callsign** (e.g. `W1AW`, `W1AW-2`, `KD2ABC Base`),
  validated against the US callsign format at every path that can set the name (CLI
  `set name`, companion app command, prefs load). The advert — which is signed and
  plaintext — therefore also identifies the station.
- **10-minute ID beacons permitted:** the minimum periodic advert interval was lowered
  from 60 to 10 minutes (`set advert.interval 10`). With the per-packet trailer this is
  belt-and-suspenders, not a requirement.

### 3. Frequency privileges — §97.301

**HamCore changes:**

- Frequency settings are validated against the US amateur allocations this hardware can
  reach: **70cm (420–450 MHz)** and **33cm (902–928 MHz)** — at the CLI, the companion
  protocol handler, and on prefs load.
- Default preset is **906.875 MHz / BW 250 / SF 10 / CR 5** (33cm). `set band 33cm` or
  `set band 70cm` applies a preset; `set radio` still allows any in-band value.
- The CLI `set tx` command now enforces the per-board `MAX_LORA_TX_POWER` limit
  (previously unchecked). Note that as a licensed amateur you are *not* bound by
  Part 15 EIRP limits, but you are bound by §97.313 (minimum power necessary).

---

## Operator responsibilities (things firmware cannot do for you)

- **You must hold at least a Technician-class license** to transmit. Receive-only
  monitoring requires no license.
- **Band plans:** 420–450 and 902–928 MHz are shared allocations with regional band
  plans (and, for 33cm, heavy Part 15 ISM occupancy). Coordinate wideband LoRa use with
  your local band plan; the 70cm preset (433.5 MHz) sits in the auxiliary/experimental
  segment but local plans vary.
- **No expectation of privacy:** everything you send — including direct messages — is
  readable by anyone. Do not send anything sensitive.
- **Treat repeater/room-server passwords as access tokens, not secrets.** Login never
  transmits the password: the client sends a 16-byte HMAC-SHA256 proof
  (key = password, message = timestamp || server pubkey || client pubkey), so the
  password never leaves the device, and replayed logins are rejected by the server's
  timestamp check. Remote `password` / `guest.password` changes over RF are refused
  (they only work over the local serial console) so a new password is never
  transmitted either. Even so, never reuse a password you care about.
- **Third-party traffic, prohibited content, pecuniary interest** rules (§97.113) still
  apply to what people type.
- **Control operator:** unattended repeaters/room servers operate under automatic
  control rules; you remain responsible for what your station transmits.

## Interoperability

HamCore is **not over-the-air compatible with stock MeshCore** — deliberately. Frames
use payload version 2 with the callsign trailer, so mixed networks fail cleanly instead
of half-working. The BLE/serial companion protocol is unchanged except the device name
prefix (`HamCore-`); the bundled `client-lib`/`client-cli` handle both.

The KISS modem firmware target drives the radio directly and bypasses the mesh stack —
if you use it, callsign identification is the host application's responsibility (as
with any TNC).

## Building

```
pio run -e Heltec_v3_companion_radio_ble    # or any of the ~580 env targets
pio test -e native                          # unit tests (includes callsign validator suite)
```

First-boot checklist: flash, connect (serial CLI or client), then
`set name <YOURCALL>` — the radio will not transmit until you do.

---

*HamCore is developed with substantial assistance from generative AI (Anthropic's
Claude), directed and reviewed by a human maintainer — see the AI disclosure in the
README. HamCore is not legal advice. Rules cited are current as of 2026; verify against the
latest Part 97 text. MeshCore is a trademark of its respective owners; this fork is
independent and unaffiliated.*
