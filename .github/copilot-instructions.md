# FriendMeshOS Agent Instructions

This is the canonical agent-facing instruction file for this repository. Read it completely before non-trivial work. `AGENTS.md` is the quick-reference companion; if the two files conflict, stop and resolve the conflict from live source and tracked project decisions before changing code.

## Project and supported product scope

This repository is the Meshtastic C++17 firmware plus the Python hardware-test MCP server in `mcp-server/`. FriendMeshOS is currently a LilyGO T-Deck / T-Deck Plus product built with the `t-deck-tft` PlatformIO environment.

FriendMesh features must preserve the pinned Meshtastic packet, routing, radio, protobuf, BLE, NodeDB, and ordinary-channel behavior. Keep reusable protocol and service logic separate from T-Deck-specific input and display code so another device port can follow the documented approach later. Do not claim or implement support for another board until its own capability and physical-test matrix exists.

The live compiled source is authoritative for existing firmware behavior. These tracked documents are authoritative for planned FriendMesh behavior:

- `FRIENDMESH_FULL_IMPLEMENTATION_GAME_PLAN.md` — product decisions, phase ordering, and acceptance gates.
- `MESHTASTIC_CORE_AND_FRIENDMESH_ARCHITECTURE.md` — Meshtastic internals and the FriendMesh compatibility boundary.
- `FRIENDMESHOS_ROADMAP.md` — whole-project sequencing, milestone log, and handoff state.
- `branding/TDECK_STYLING.md` — six-theme design contract.

If documentation and source disagree, record the discrepancy. Never alter protocol or security behavior merely to match stale prose.

## Required session start

Before editing:

1. Read this file, `AGENTS.md`, the relevant phase in `FRIENDMESH_FULL_IMPLEMENTATION_GAME_PLAN.md`, and the latest milestone/handoff in `FRIENDMESHOS_ROADMAP.md`.
2. Inspect `git status --short --branch` and recent history. Existing changes belong to the user unless proven otherwise.
3. Trace the live implementation and generated protobuf definitions that own the behavior.
4. State the exact phase and acceptance bullets being attempted.
5. Do not begin later-phase feature code until preceding security and compatibility dependencies are complete, or explicitly record the work as an isolated scaffold that cannot ship.

A phase is complete only when every build, design, automated-test, physical-test, recovery, documentation, and six-theme gate has recorded evidence. `Implemented` and `verified` are different states.

## Repository map

| Path | Purpose |
| --- | --- |
| `src/mesh/` | Router, channels, crypto, NodeDB, radio interfaces, PhoneAPI, and StreamAPI |
| `src/modules/` | Port-number modules and feature modules |
| `src/graphics/` | On-device display and UI code |
| `src/input/` | Keyboard, touch, trackball, rotary, and input broker code |
| `src/gps/`, `src/motion/` | Location and motion hardware support |
| `src/mesh/generated/` | Generated Nanopb C/C++ bindings; do not hand-edit |
| `protobufs/` | Vendored `.proto` source used by `bin/regen-protos.sh` |
| `variants/` | Board definitions and PlatformIO environments |
| `test/` | Native Unity firmware tests |
| `mcp-server/` | Python device/build/flash/test automation server |
| `mcp-server/tests/` | Unit and physical hardware test tiers |
| `.github/workflows/` | CI definitions |

`protobufs/`, `meshtestic/`, and `lib/meshtastic-device-ui/` are vendored ordinary repository content in FriendMeshOS. Do not run `git submodule update`, require submodule initialization, or assume developers will pull these directories from separate repositories. Upstream may use submodules; that provenance is documentation only and does not define this fork's checkout workflow.

Search with `rg` or `rg --files`. Follow declarations to definitions and callers before deciding ownership.

## C++ and firmware conventions

- Use C++17 and match the surrounding Meshtastic style and naming.
- Keep headers focused. Prefer small value types and pure helpers for logic that can be native-tested.
- Use the existing module framework (`SinglePortModule`, `ProtobufModule`, `MeshModule`, and `concurrency::OSThread`) instead of adding a parallel dispatcher or scheduler.
- Use `Observable`, `Observer`, and `CallbackObserver` for existing event flows. An observer callback can run in the notifier's context: copy transient data that outlives the callback, keep callbacks short, and move substantial work to the owning thread or queue.
- Respect ownership of `meshtastic_MeshPacket` instances and the packet pool. Do not retain borrowed pointers or free a packet owned by another layer.
- Avoid unbounded containers, recursion, and dynamic growth in radio/UI hot paths. Bound every protobuf field, queue, cache, fragment count, history request, and retry schedule.
- Treat shared globals, callbacks, radio events, UI events, and filesystem callbacks as concurrent unless the source proves otherwise. Keep mutations in one owning context or use the repository's synchronization pattern.
- Use `Throttle::isWithinTimespanMs` or `Throttle::execute` for elapsed-millisecond checks. Do not compare `millis()` with `last + interval`; that is rollover-unsafe.
- Keep code comments to one or two lines and explain only non-obvious intent or invariants.
- Do not hand-edit files in `src/mesh/generated/`; change the `.proto` source and run `bin/regen-protos.sh`.

## FriendMesh architecture rules

- Ordinary Meshtastic traffic remains available in its own OS area and must retain stock interoperability.
- FriendMesh structured group traffic uses a bounded, versioned envelope over the approved private application port. Unknown versions fail closed without disrupting unrelated Meshtastic packets.
- A normal Meshtastic relay may forward FriendMesh traffic without understanding its payload.
- One FriendMesh member is one verified device identity. Never silently treat an alias as cryptographic identity.
- Meshtastic X25519 keys perform key agreement and cannot sign FriendMesh events. FriendMesh administrative and durable events require the separately planned signing identity.
- Never describe channel encryption as authenticated encryption. Meshtastic channel AES-CTR has no cryptographic authentication tag.
- Never log or expose PSKs, private keys, derived shared keys, plaintext history, PIN-derived keys, raw security transcripts, or secret-bearing protobufs in diagnostics.
- Private FriendMesh group operation is blocked in licensed/Ham mode because encrypted operation is unavailable there.
- FriendMesh-owned private channels require strong 32-byte keys and MQTT disabled. A weak/default key or MQTT violation removes secure status and blocks FriendMesh group traffic.
- Destructive membership, rekey, erase, and disband operations must use the game plan's signed transaction and recovery rules. Do not prototype irreversible shortcuts in shipping paths.
- SOS and Help are best-effort radio features, not guaranteed emergency services. Preserve the prominent incomplete/unsafe warning until release qualification removes it through an explicit decision.

## Encryption and key management

`src/mesh/CryptoEngine.cpp`, `src/mesh/Channels.cpp`, `src/mesh/Channels.h`, `src/mesh/Router.cpp`, and `src/modules/AdminModule.cpp` are the starting source set.

### Channel encryption

- Channel payloads use AES-CTR with the channel PSK.
- Supported effective key sizes include AES-128 and AES-256; one-byte PSKs are indices into well-known keys.
- The nonce is derived from packet ID, sender node number, and block counter. Confirm the exact byte layout in `CryptoEngine.cpp` before touching it.
- Channel hash is a routing/filter hint, not a message authentication code.

### Per-peer PKI

- Meshtastic derives a shared secret using X25519 ECDH and SHA-256, then uses AES-256-CCM with an eight-byte authentication tag.
- A fresh 32-bit `extraNonce` is transmitted with the tag; the wire overhead is `MESHTASTIC_PKC_OVERHEAD`.
- PKI selection has port-number and infrastructure exceptions. Trace both send and receive branches before changing it.
- Remote admin also requires the configured admin-key authorization and session/passkey checks in `AdminModule.cpp`; encryption alone is not authorization.
- Licensed/Ham mode disables encryption.

### Rotation hazards

- Full device factory reset with BLE-bond erasure removes the private key and creates a new keypair. Peers holding the prior public key can no longer decrypt new PKI DMs until NodeInfo exchange updates them.
- Partial configuration reset preserves the private key.
- Explicitly blanking the private key through admin configuration also triggers regeneration.
- Never trigger or simulate a real key reset on connected hardware without operator approval.

## Build, format, and test

Use the narrowest relevant checks first, then the supported target build.

```text
pio test -e native
pio test -e native -f <suite>
pio run -e t-deck-tft
trunk fmt
git diff --check
```

- Run `trunk fmt` before proposing a commit. Inspect its changes and preserve unrelated user work.
- A documentation-only edit needs `git diff --check` and a Markdown-link audit; it does not need a firmware build unless it changes generated/configured inputs.
- A firmware change needs relevant native tests and `pio run -e t-deck-tft`.
- A protobuf change needs regeneration, bounded-field review, compatibility vectors, decoder rejection tests, and the target build.
- Never claim physical verification from a native test, compile, simulator, or screenshot alone.
- Record commands, results, limitations, and hardware not exercised in the roadmap milestone log.

CI matrix generation is `./bin/generate_ci_matrix.py all [--level pr]`. Native host firmware is `pio run -e native-macos` and has additional prerequisites documented in `variants/native/portduino/platformio.ini`.

## MCP Server and hardware test harness

The MCP server is configured by `.mcp.json`. Its Python package and argument details live in `mcp-server/README.md`.

Setup:

```text
cd mcp-server
python3 -m venv .venv
.venv/bin/pip install -e '.[test]'
```

The tool groups include device/board discovery, PlatformIO build and flash, serial sessions, device reads, confirmed device writes, `userPrefs` administration, UI capture, and vendor recovery tools.

### Hardware-test rules

- No destructive or state-changing device operation without explicit operator approval. This includes flash/upload, erase-and-flash, update flash, factory reset, reboot, shutdown, channel/config writes, owner writes, text transmission, key changes, and power cycling.
- `confirm=True` is a real operator gate and never a convenience flag.
- One operation may own a serial port at a time. Sequence open/read/close or one-shot calls; never run concurrent calls against the same port.
- Read-only device inspection is allowed, but report which device and port were observed.
- `userPrefs.jsonc` is test session state. The `_session_userprefs` fixture snapshots and restores it; never edit it from inside a test.
- Run the hardware suite with `./mcp-server/run-tests.sh` or the test TUI. Interpret results from the generated report and firmware log, not just a terminal summary.
- If a test is flaky, use the repository repro workflow. A consistent failure is a regression, not a flake.
- Do not classify a firmware root cause without evidence. Say `unknown` and list the observation needed to distinguish candidates.

### Test-fixture contract

- Tests must declare the hardware role and capability they require.
- Tests must leave devices and `userPrefs.jsonc` in their prior state, unless an operator-approved provisioning test explicitly documents otherwise.
- Never bake personal or production secrets into fixtures, logs, reports, screenshots, packet vectors, or source.
- Compatibility vectors must use synthetic keys and payloads and state their provenance/version.
- Physical FriendMesh gates require the exact stock/FriendMesh versions, regional/radio configuration, board type, and observed result to be recorded.

### Recovery boundaries

Read-only diagnosis may identify a busy port, missing device, stale process, bootloader state, or failed test. Killing processes, entering a bootloader, flashing, resetting, or power-cycling changes external state; explain the proposed action and wait for approval.

## UI and theme contract

- Design for 320x240 first and preserve usable keyboard, trackball, and touch focus order.
- All six themes use the same information hierarchy and behavior. Theme color may change; semantic meaning, focus order, warning severity, and emergency behavior may not.
- Color is never the sole signal. Pair it with icon, text, border/state, and alert feedback as specified.
- Emergency styling is fixed and theme-independent.
- Do not introduce animation into FriendMesh flows.
- Every screen must account for loading, empty, focused, pressed, disabled, warning, error, stale, pending/undo, and relevant alert states.
- A UI phase is not verified until every theme and input method in its test matrix has evidence.

## Documentation and handoff

Update the living documents in the same change whenever behavior, protocol, risk, or phase status changes.

- Add decisions and superseded decisions to the decision log; do not erase historical rationale.
- Add unresolved security, safety, interoperability, airtime, storage, and hardware risks to the risk register with owner and mitigation/evidence needed.
- Record every intentional divergence from upstream Meshtastic in the delta ledger with file, purpose, compatibility impact, upstream comparison, and removal/rebase guidance.
- Mark only individually evidenced bullets complete. Leave a phase unchecked when physical or six-theme evidence is missing.
- End a work session with exact files changed, tests run, results, known gaps, and the next safe action in the roadmap milestone log.

## Git and workspace safety

- Preserve unrelated modifications and untracked files.
- Do not use `git reset --hard`, destructive checkout, history rewriting, or cleanup commands without explicit approval.
- Do not commit, push, create a branch, or open a pull request unless the user asks.
- Inspect the final diff for secrets, generated churn, debug output, and accidental platform expansion.
- Never describe uncommitted work as committed or an unrun test as passing.

## Quick command reference

| Action | Command |
| --- | --- |
| Build T-Deck firmware | `pio run -e t-deck-tft` |
| Clean target | `pio run -e t-deck-tft -t clean` |
| Run all native tests | `pio test -e native` |
| Run one native suite | `pio test -e native -f <suite>` |
| Format | `trunk fmt` |
| Regenerate protobufs | `bin/regen-protos.sh` |
| Run hardware tests | `./mcp-server/run-tests.sh` |
| Launch test TUI | `mcp-server/.venv/bin/meshtastic-mcp-test-tui` |
