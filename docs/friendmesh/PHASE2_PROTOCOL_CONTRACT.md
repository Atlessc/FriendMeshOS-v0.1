# FriendMesh Phase 2 Protocol Contract

Status: implemented receiver foundation; not a complete secure-group protocol  
Protocol version: 1  
Meshtastic base: 2.7.26  
Hardware scope: `t-deck-tft` only

## What this slice establishes

Phase 2 adds a bounded FriendMesh application envelope above Meshtastic without changing the LoRa header, routing, flooding, ACK/NAK behavior, channel encryption, PKI direct messages, NodeDB schema, or BLE/client framing. FriendMesh frames use Meshtastic `PRIVATE_APP` and are still transported and relayed as ordinary Meshtastic application payloads.

This slice establishes:

- a generated Nanopb schema with fixed bounds;
- unambiguous FriendMesh magic and protocol-version dispatch;
- deterministic canonical signing bytes;
- a separate Ed25519 device signing identity primitive;
- an identity-binding payload that binds the signing public key to the sender node and current Meshtastic X25519 public key;
- signature, sender, group, epoch, timestamp, event-ID, and sender-sequence validation;
- bounded in-memory replay protection;
- strict rejection of unknown versions, unknown event types, malformed protobufs, and noncanonical encodings;
- a T-Deck-only `PRIVATE_APP` receiver that does not consume unrelated private application traffic.

It does **not** yet establish a private group. There is no group-key encryption, membership database, identity-key persistence, invitation UI, verification UI, transmit pipeline, or durable replay state in this slice. An accepted identity-binding frame is logged as untrusted until a later in-person verification flow commits it.

## Wire layout

One FriendMesh frame is contained in one Meshtastic `Data.payload`:

| Offset | Size | Meaning |
| --- | ---: | --- |
| 0 | 4 | ASCII `FMSH` |
| 4 | 1 | outer protocol version, currently `0x01` |
| 5 | bounded | canonical Nanopb `FriendMeshEnvelope` |

The hard frame limit is 233 bytes. The generated maximum envelope is 224 bytes, so the five-byte prefix produces a maximum of 229 bytes and leaves four bytes of headroom. Phase 2 never fragments an envelope.

`FriendMeshSignedFields` contains:

| Field | Bound / rule | Purpose |
| --- | --- | --- |
| `protocol_version` | exactly 1 | protects against outer/inner version confusion |
| `group_id` | fixed 16 bytes | opaque application-group identity |
| `group_epoch` | fixed32 | current cryptographic/authorization generation |
| `sender_node` | nonzero fixed32 | full Meshtastic node number, never a relay byte or display name |
| `sender_sequence` | nonzero fixed64 | per-signing-identity monotonic replay coordinate |
| `event_id` | fixed 16 bytes, not all zero | stable deduplication identity |
| `created_at` | nonzero fixed64 Unix seconds | queue age and clock-skew validation |
| `event_type` | known v1 enum only | bounded dispatch |
| `signing_public_key` | fixed 32 bytes | Ed25519 verification identity |
| `payload` | at most 48 bytes | compact event-specific protobuf |

The 64-byte detached Ed25519 signature authenticates the domain-separated canonical encoding of every signed field, including the inline payload. No standalone payload-hash field is needed for this single-envelope v1 representation; payload mutation changes the signed bytes and fails verification. Future fragmented content must define its own signed digest and bounds before adding an event type.

## Canonical encoding and dispatch

Signing input is:

```text
ASCII "FriendMeshOS Signed Envelope v1" || canonical Nanopb FriendMeshSignedFields
```

The decoder re-encodes a decoded envelope and requires an exact byte-for-byte match with the received frame. Therefore alternate field order, duplicate/default encodings, and unknown protobuf fields are rejected instead of becoming multiple encodings of the same signed object.

Dispatch order is intentionally bounded:

1. Check `FMSH` magic without parsing. A mismatch returns `CONTINUE` so another `PRIVATE_APP` consumer can handle it.
2. Enforce the 233-byte frame limit and outer version.
3. Decode within generated Nanopb bounds.
4. Re-encode and reject noncanonical bytes.
5. Validate the event-specific payload shape without changing state.
6. Validate sender, optional group/epoch/time context, canonical signature, then replay state.
7. Dispatch the accepted event. The current receiver only logs secret-free metadata.

## Signing identity and Meshtastic binding

FriendMesh Ed25519 keys are separate from Meshtastic X25519 keys. They are never derived from a channel PSK, the X25519 private key, or human input. On ESP32-S3, generation and detached signing/verification use the framework-bundled libsodium implementation. The firmware executes an RFC 8032 known-answer self-test at module construction and fails closed if it does not pass.

An identity binding carries the current 32-byte Meshtastic X25519 public key and a key-generation counter. The receiver requires that key to match the full public key currently stored for `packet.from` in NodeDB. The comparison fingerprint is:

```text
SHA-256(
  "FriendMeshOS Identity Binding v1" ||
  sender_node_le32 ||
  meshtastic_x25519_public_key ||
  friendmesh_ed25519_public_key ||
  key_generation_le32
)
```

The UI may derive a six-digit comparison number from the first four fingerprint bytes, but approval must retain the full fingerprint. NodeDB matching proves consistency with the currently advertised key; it does not prove the person holding the device. In-person comparison and protected persistence remain required.

## Replay and time rules

The Phase 2 receiver keeps at most 64 sender windows and 128 recent event IDs. A sender window is keyed by group ID, group epoch, full sender node, and signing public key. Each sender has a 64-sequence sliding bitmap:

- a higher sequence advances the window;
- an unseen out-of-order sequence within 64 is accepted;
- a duplicate sequence is rejected;
- a sequence older than the 64-entry window is rejected;
- a duplicate recent event ID is rejected;
- when sender capacity is exhausted, a new sender fails closed.

When the T-Deck has at least device-quality RTC time, events older than 24 hours or more than 10 minutes in the future are rejected. With no valid RTC, cryptographic, sender, event-ID, and sequence checks still apply, but time rejection is deferred. These limits are receiver safeguards, not a promise that all later event types may legitimately remain queued for 24 hours.

The replay store is RAM-only and resets on reboot. Durable, encrypted replay state belongs to Phase 3. No security-sensitive state transition may rely on the RAM-only window as its sole idempotency control.

## Event types in version 1

| Value | Name | Payload rule | Current effect |
| ---: | --- | --- | --- |
| 0 | `UNSPECIFIED` | rejected | none |
| 1 | `IDENTITY_BINDING` | canonical `FriendMeshIdentityBinding` | validate NodeDB key consistency, then log as untrusted |
| 2 | `PROTOCOL_PROBE` | empty | validate the envelope and log metadata |

New event values require schema bounds, authorization rules, failure semantics, vectors, fuzz coverage, and a version-compatibility decision before implementation.

## Verification commands

```bash
bin/regen-friendmesh-protos.sh
bin/check-friendmesh-protocol.sh
bin/check-friendmesh-ed25519.sh
bin/check-friendmesh-vectors.sh
$HOME/.platformio/penv/bin/pio run -e t-deck-tft
git diff --check
```

The protocol checker covers canonical round trips, maximum frame size, unknown outer/signed versions, unknown protobuf fields, bad signatures, wrong sender/group/epoch, stale/future time, duplicate event IDs, duplicate/stale/out-of-order sequences, and fail-closed sender-capacity exhaustion. The Ed25519 checker independently verifies an RFC 8032 known-answer vector and mutated-signature rejection using OpenSSL. The firmware performs its own libsodium self-test at boot; physical verification must confirm the success log on the T-Deck.

## Remaining Phase 2 gate

- Persist the generated signing seed and binding through the reviewed encrypted-storage design instead of inventing an insecure temporary store.
- Implement identity setup, full fingerprint/security-number comparison, verified/pending/key-changed states, and non-dismissible key-change UX in all six themes and all three input modes.
- Add a transmit path and two-FriendMesh-device physical exchange without changing ordinary Meshtastic behavior.
- Add malformed-input fuzzing and runtime resource measurements.
- Re-run stock private channel, PKI DM, position, traceroute, BLE configuration, and opaque relay regression after physical upload.
- Complete the deferred Phase 1 native and controlled multihop evidence before release qualification.
