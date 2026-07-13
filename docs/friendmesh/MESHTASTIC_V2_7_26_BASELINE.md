# Meshtastic v2.7.26 compatibility baseline

Status: Phase 1 baseline provenance verified; byte-vector and physical interoperability gates remain open  
Captured: 2026-07-12  
FriendMeshOS working base: `876ec5691f78bd12545304ef623f8f1b58fb9eeb`

## Upstream endpoints

| Component | Exact endpoint | Evidence |
| --- | --- | --- |
| Firmware release | `v2.7.26.54e0d8d` | [Official release](https://github.com/meshtastic/firmware/releases/tag/v2.7.26.54e0d8d) |
| Firmware commit | `54e0d8d0ab2ff56b3a9ce967e53f79e49af560fb` | [Official commit](https://github.com/meshtastic/firmware/commit/54e0d8d0ab2ff56b3a9ce967e53f79e49af560fb) |
| Upstream firmware protobuf revision | `6b1ded439633cd03d4af85b44231b91d1d106278` | Upstream records this as a submodule revision; FriendMeshOS vendors the matching schemas as ordinary tracked files. See the [official firmware tree](https://github.com/meshtastic/firmware/tree/54e0d8d0ab2ff56b3a9ce967e53f79e49af560fb) and [protobuf tree](https://github.com/meshtastic/protobufs/tree/6b1ded439633cd03d4af85b44231b91d1d106278). |

The fork's original upstream Git objects are not ancestors in the current local Git history. Exact equivalence is therefore established per boundary through hashes/vectors, not by claiming local ancestry that does not exist.

## Vendored schema equivalence

The following local SHA-256 values exactly matched the same files fetched from protobuf commit `6b1ded439633cd03d4af85b44231b91d1d106278`:

| Schema | SHA-256 |
| --- | --- |
| `protobufs/meshtastic/mesh.proto` | `aae813b7cc47e8d6eb0d720aa8df0132461852a2de6f59c5be105b751e9cd562` |
| `protobufs/meshtastic/channel.proto` | `415b5865b57b004ff474dc6be93982dea3f40ea85fcd9adcbbbef88bd9349dce` |
| `protobufs/meshtastic/config.proto` | `0fcf7faa64583654adf43e9f9ceeec49470883eb7d0d86755667f4aacd2cb9e5` |
| `protobufs/meshtastic/telemetry.proto` | `cfd942cc3965b4e617b9c2e64ba697d2a39dea66badf1034d41c06835b9ef5d9` |
| `protobufs/meshtastic/portnums.proto` | `2bbfd0f78ba21d65e45db47fca5736907803a5d5114b9b9bab1f36ec4e7aceda` |

Current generated-binding fingerprints:

| Generated file | SHA-256 |
| --- | --- |
| `src/mesh/generated/meshtastic/mesh.pb.h` | `de0e6589d1af8aa56786d43f1c95332c7f879dab3de9131490d113001a5c31bf` |
| `src/mesh/generated/meshtastic/mesh.pb.cpp` | `34cbb88aca1578ba4b6c90838dff44e6611009b107728771f8097da9f407a297` |

## Tool manifest

| Tool | Captured value | Reproducibility status |
| --- | --- | --- |
| PlatformIO Core | `6.1.19` | Captured from local build environment |
| Python | `3.11.9` | Captured from local build environment |
| Firmware Nanopb runtime | `0.4.9.1` archive / reported library `0.4.9+1` | Pinned in `platformio.ini` and build output |
| Generator | `nanopb-0.4.9/generator-bin/protoc` required by `bin/regen-protos.sh` | Not present in this checkout; regeneration remains blocked until the exact generator bundle is installed and hashed |

Do not regenerate bindings and call them baseline-equivalent until the generator bundle/version and full generated-tree diff are recorded.

## Compatibility fixture matrix

All fixture payloads must use synthetic node numbers, keys, coordinates, channel names, and text. No real PSK, channel URL, identity, or location may enter source or test reports.

| Fixture | Required assertions | Current evidence |
| --- | --- | --- |
| Public text | `TEXT_MESSAGE_APP`, channel 0, broadcast destination, stock display/relay | Synthetic protobuf vector decodes and re-encodes byte-for-byte through checked-in Nanopb; physical exchange pending |
| Private channel text | `TEXT_MESSAGE_APP`, nonzero channel, channel encryption path, stock peer round trip | Synthetic channel-2 protobuf vector passes; encrypted air-frame and physical round trip pending |
| PKI DM | Direct destination, X25519-derived AES-CCM path, `pki_encrypted`, authentication failure cases | Metadata observer implemented; crypto byte/physical vector pending |
| Position | `POSITION_APP`, source/precision/time fields, hidden precision, stale/future time | Synthetic position vector and freshness observer implemented; physical exchange pending |
| NodeInfo | `NODEINFO_APP`, node number, user/public key and key-change handling | Synthetic node/user/public-key vector passes; key-change state-machine work remains |
| Telemetry | `TELEMETRY_APP`, bounded metric variant and unknown-field behavior | Synthetic device-metrics vector passes; unknown-field and physical cases remain |
| Traceroute | `TRACEROUTE_APP`, route arrays, forward/back path and unknown hop | Synthetic route/SNR vector and hop observer pass; physical multihop pending |
| ACK/NAK | `ROUTING_APP`, `NONE` as ACK and nonzero `Routing_Error` as NAK | Fixed ACK and `NO_ROUTE` vectors pass the standalone checker; native suite remains host-blocked |
| BLE config | `ToRadio`/`FromRadio` config sequence through completion marker | Fixed request/completion-ID boundary vectors pass; physical supported-phone round trip pending |
| Protected FriendMesh channel | Stock read allowed; write/delete rejected by default; explicit recovery path | Contract/wireframe only; implementation belongs to Phases 4 and 15 |

## Implemented Phase 1 source anchors

- `src/friendmesh/observability/FriendMeshObservability.*` — read-only destination, channel, decoded/encrypted, port, ACK/NAK, hop, receive-age, transport, queue, and position-freshness observations.
- `src/friendmesh/platform/TDeckCapabilityService.*` — bounded capability-state model for GPS, magnetometer, SD, maps, touch, trackball, keyboard, and LoRa.
- `src/friendmesh/observability/DiagnosticFormatter.*` — bounded semantic names and secret-free detail/export formatting for diagnostic events.
- `test/test_friendmesh_observability/test_main.cpp` — native cases for time skew, Meshtastic hop semantics, packet metadata, ACK/NAK decoding, queue/freshness states, formatter redaction/error names, and independent capability degradation.
- `test/fixtures/MeshtasticV2726Vectors.h` — synthetic, secret-free fixed protobuf vectors for ordinary text, private-channel text, position, NodeInfo, telemetry, traceroute, routing, and BLE configuration boundaries.
- `test/test_friendmesh_compatibility_vectors/test_main.cpp` — semantic assertions plus byte-for-byte decode/re-encode tests.
- `bin/check-friendmesh-vectors.sh` — host-only checker against the checked-in Nanopb bindings; it requires no Portduino or connected device.
- `lib/meshtastic-device-ui/source/graphics/common/ViewController.cpp` — forwards received queue status to the view through a default no-op read-only hook.
- `lib/meshtastic-device-ui/source/graphics/TFT/TFTView_320x240.cpp` — records redacted packet/queue/position observations, automatically refreshes D-01/D-02, and provides non-destructive view clearing plus two-step SD-only redacted plaintext export of all retained events, including Clear view records.

The seed bytes were produced with the locally installed Meshtastic Python `2.7.7` package, then independently decoded and deterministically re-encoded byte-for-byte by this checkout's generated Nanopb `0.4.9.1` bindings. The values use synthetic identities, key bytes, coordinates, and timestamps. These are protobuf-boundary fixtures, not encrypted LoRa air-frame or PKI cryptographic vectors.

The observer intentionally contains no payload text, coordinates, keys, fingerprints, PSKs, or channel URLs.

## Gate status

- `t-deck-tft` firmware build: passed; inherited warnings remain.
- Standalone protobuf-vector checker: passed all ten fixed vectors using the local generated bindings.
- T-Deck diagnostics integration build: passed; the D-01/D-02 screens, navigation, automatic refresh, and SD export require physical verification.
- Native test execution: blocked before test compilation because the local Portduino toolchain requires `pkg-config`/`argp.h`; no system packages were installed.
- Physical original T-Deck, T-Deck Plus, stock-node, multihop, and BLE phone tests: not run; no device mutation was authorized.
- Six-theme diagnostics screen: implemented with shared semantic styles; the six-theme/three-input physical matrix remains open.

Phase 1 remains incomplete until fixed byte vectors, native execution, diagnostics UI, stock two-node exchange, BLE sync, protected-channel behavior, and six-theme/three-input evidence pass.
