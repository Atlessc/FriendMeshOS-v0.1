# FriendMesh Phase 3 storage cryptography design

Status: authenticated record, wrapped-master-key format, two-slot internal-key backend, and asynchronous isolated D-01 provider self-test implemented and physically passed; runtime provisioning, journal, reboot/interrupted recovery, and production identity persistence remain gated
Target: LilyGO T-Deck / T-Deck Plus runtime-capability build only  
Base: Meshtastic 2.7.26

## Security boundary

FriendMesh security-sensitive state must not use Meshtastic `SafeFile` readback as an integrity or confidentiality boundary. `SafeFile` may later be used beneath the FriendMesh transaction layer for temporary-file/rename mechanics, but a record is trusted only after strict framing checks and AEAD authentication.

This design protects confidentiality and detects modification of records copied from internal flash or SD. It does not claim hardware-backed security. A six-digit PIN has low entropy, and a person who can extract flash and run custom firmware can attempt offline PIN guesses. Retry delays help only against the compliant on-device UI. Full-snapshot rollback also cannot be detected reliably without protected monotonic hardware state. Release copy must describe this as software-only, casual-extraction resistance.

The existing Device UI PIN is stored as a numeric UI configuration value and compared directly. It is not yet an acceptable storage-key derivation interface. Phase 3 must either migrate that PIN into the reviewed wrapping flow or introduce a storage-unlock credential without ever storing the credential itself.

## Frozen primitive choices

- Record encryption: libsodium XChaCha20-Poly1305 IETF, 32-byte key, 24-byte nonce, 16-byte authentication tag.
- Record nonce: fresh CSPRNG output for every encryption. An all-zero result is regenerated. A nonce is never derived from sequence, time, path, record ID, or reboot state.
- Password KDF: explicit Argon2id v1.3 (`crypto_pwhash_ALG_ARGON2ID13`), 16-byte random salt, 32-byte output. The algorithm identifier, salt, operations limit, and memory limit are persisted in the wrapped-key header.
- Domain separation: libsodium `crypto_kdf_derive_from_key()` with fixed eight-byte contexts and independently assigned subkey IDs.
- Storage master key: 32 random bytes generated once. PIN changes rewrap this key transactionally instead of re-encrypting every record.

The exact Argon2id operations and memory limits are deliberately not frozen yet. The bundled interactive profile requests 64 MiB, while the target has 8 MiB PSRAM and substantially less internal RAM. The corrected physical boot reported approximately 2.90 MiB free PSRAM and 175,624 bytes free internal heap after normal UI/radio initialization, so total installed PSRAM cannot be treated as KDF budget. Phase 3 requires a physical benchmark over conservative feasible memory/pass combinations, measured unlock latency, watchdog behavior, allocation location, and simultaneous firmware headroom. The selected values must be stored per wrapped key so later upgrades remain readable.

The first physical benchmark build reset before completing its first case because it pinned a continuously runnable Argon2 task to CPU 0, whose idle task is protected by the five-second task watchdog in this ESP32-S3 configuration. The corrected benchmark runs at idle priority on CPU 1, logs before entering each monolithic KDF call, and records the task stack low-water mark. The first corrected physical run completed all five cases, but its 8 KiB task stack retained only 588 bytes, so the dedicated benchmark/KDF worker reserve is increased to 12 KiB before further use.

| Memory | Operations | Physical duration | Result |
| ---: | ---: | ---: | --- |
| 256 KiB | 1 | 296 ms | Pass |
| 512 KiB | 1 | 266 ms | Pass |
| 1024 KiB | 1 | 493 ms | Pass |
| 1024 KiB | 2 | 679 ms | Pass |
| 1024 KiB | 3 | 1089 ms | Pass |

The run reduced PSRAM from approximately 2.90 MiB free to a 1.85 MiB boot low-water mark, supporting that the memory-hard workspace used external RAM. Internal heap retained a 126,860-byte low-water mark and 131,060-byte largest block. LoRa telemetry and position events were captured during the same diagnostic session. `1024 KiB / 3 operations` is the leading v1 candidate because it completed in approximately 1.1 seconds with useful remaining headroom, but it remains provisional until UI/radio coexistence and wrapped-key recovery are tested.

Active BLE coexistence is not a Phase 3 KDF or storage acceptance gate. FriendMesh does not use BLE, and the product owner does not plan to operate it with a connected phone. Stock Meshtastic BLE pairing/configuration remains an inherited compatibility boundary and is retested only when storage work changes shared client, configuration, protobuf, PhoneAPI, or StreamAPI behavior. The absence of simultaneous-BLE measurements remains documented and must not be misreported as a physical pass.

## Version 1 authenticated record

All integers are unsigned little-endian. Structures are encoded field by field; compiler structure layout is never serialized.

| Offset | Bytes | Field | Rule |
| ---: | ---: | --- | --- |
| 0 | 4 | Magic | ASCII `FMSR` |
| 4 | 1 | Format version | `1` |
| 5 | 1 | Record type | Known bounded value `1..8` |
| 6 | 2 | Flags/reserved | Must be zero in v1 |
| 8 | 4 | Schema version | Nonzero |
| 12 | 4 | Key epoch | Nonzero |
| 16 | 8 | Monotonic logical sequence | Nonzero; journal semantics enforce monotonicity later |
| 24 | 16 | Scope ID | Group ID or all-zero device scope |
| 40 | 16 | Record ID | Nonzero stable random identifier |
| 56 | 24 | XChaCha nonce | Nonzero CSPRNG output |
| 80 | 4 | Plaintext length | `0..4096` |
| 84 | variable | Ciphertext | Same length as plaintext |
| end-16 | 16 | Poly1305 tag | Produced by combined-mode AEAD |

The exact 84-byte header is AEAD additional authenticated data. Record type, schema, epoch, sequence, scope, record identity, nonce, and length therefore cannot be altered without authentication failure. Decode requires an exact total length; trailing bytes and truncation are rejected. Callers may require an expected type and scope before decryption, preventing a valid record from being consumed in the wrong domain.

Record types v1: signing identity, replay state, transaction, group state, history, outbox, contact, and notification. Unknown types, versions, or flags fail closed. The plaintext output buffer is wiped after authentication failure and no parsed metadata becomes authoritative before authentication succeeds.

## Version 1 wrapped master key

The storage master key is wrapped in a fixed 108-byte record. All integers are unsigned little-endian and encoded field by field.

| Offset | Bytes | Field | Rule |
| ---: | ---: | --- | --- |
| 0 | 4 | Magic | ASCII `FMWK` |
| 4 | 1 | Format version | `1` |
| 5 | 1 | KDF identifier | `1`, Argon2id v1.3 |
| 6 | 1 | AEAD identifier | `1`, XChaCha20-Poly1305 |
| 7 | 1 | Flags/reserved | Must be zero in v1 |
| 8 | 4 | Operations limit | Bounded to `1..6` before invoking the KDF |
| 12 | 4 | Memory limit, KiB | Bounded to `8..2048` before invoking the KDF |
| 16 | 4 | Key generation | Nonzero; incremented by the future transactional provider |
| 20 | 16 | Argon2id salt | Nonzero, fresh random value |
| 36 | 24 | XChaCha nonce | Nonzero, fresh random value |
| 60 | 32 | Encrypted master key | Ciphertext for the 32-byte storage master key |
| 92 | 16 | Poly1305 tag | Authentication tag |

The AEAD additional authenticated data is the exact 60-byte header followed by a caller-supplied, nonzero 16-byte device binding. The binding is deliberately not serialized beside the wrapped key: moving the record to a device with a different binding must fail authentication. The future storage provider must define a stable binding that can be reproduced after an ordinary firmware update and power loss. This software binding is not represented as a hardware-backed secret.

The codec accepts `1..64` credential bytes so the storage layer is not tied to one UI encoding. The product UI still owns the approved six-digit numeric PIN policy. Wrong credentials, the wrong device binding, and authenticated-record tampering all fail closed without returning master-key bytes. Unsupported or excessive KDF parameters are rejected before Argon2id runs, preventing a modified header from forcing an unbounded allocation or denial-of-service workload.

The stored v1 defaults are currently 1024 KiB and three operations. They remain provisional until transactional wrap recovery, normal UI/radio coexistence, and power-loss tests pass. Persisting the parameters in each record allows later profiles to remain readable without guessing which KDF cost created the record.

## Frozen key hierarchy and storage domains

```text
six-digit PIN --Argon2id(salt, stored parameters)--> wrapping key
                                                      |
random storage master key <--- XChaCha20-Poly1305 wrapped-key record
        |
        +-- fixed storage-domain subkeys
```

Each subkey is derived with libsodium `crypto_kdf_derive_from_key()`. Both the numeric ID and exact eight-byte ASCII context are protocol constants:

| ID | Domain | Context |
| ---: | --- | --- |
| 1 | Signing identity | `FMIDEN01` |
| 2 | Replay state | `FMREPL01` |
| 3 | Transaction journal | `FMTXNJ01` |
| 4 | Group state | `FMGRUP01` |
| 5 | History | `FMHIST01` |
| 6 | Durable outbox | `FMOUTB01` |
| 7 | Contacts | `FMCONT01` |
| 8 | Notifications | `FMNOTI01` |
| 9 | Key audit | `FMKAUD01` |
| 10 | Drafts | `FMDRAF01` |

Unknown domains fail closed and wipe the output. These subkeys protect local storage only; they cannot be reused as Meshtastic channel keys, FriendMesh group keys, radio-envelope keys, or any other protocol secret.

## Transaction and provider contract

The next implementation slices must keep these invariants:

1. Security/membership/key events are durably journaled before radio enqueue.
2. Ordinary chat enters a durable outbox before transmission. SOS and Help may bypass failed persistence only with an explicit degraded warning.
3. Internal storage owns the last-known-good essential state. SD is an expansion provider, not the only copy of membership/key/replay state.
4. Each mutation writes an authenticated prepare record, required data records, and an authenticated commit record. Recovery applies only complete, internally consistent transactions.
5. Snapshots name the last committed journal sequence and hash/identify included records. Compaction never deletes the prior known-good snapshot until the replacement commit is authenticated and readable.
6. Provider states are explicit: unavailable, locked, ready, full, read-only, corrupt/quarantined, and recovery-required.
7. A KDF/wrapped-key failure never falls back to plaintext or silently creates a new master key. PIN reset without an approved recovery mechanism makes old ciphertext unavailable.
8. Nonce generation happens immediately before encryption and a failed write never causes that nonce/ciphertext pair to be repurposed.
9. Wrapped master keys use two independent internal slots. A prepared generation is written only to the inactive slot and read back exactly before success is reported. Recovery authenticates both slots and selects the highest valid generation; a partial/corrupt inactive slot cannot replace the last-known-good generation.
10. Same-generation slots that authenticate to different master keys are quarantined as a generation conflict. A valid slot may be loaded in an explicit degraded state when its peer is missing, corrupt, or unreadable.

## Implemented evidence

- `FriendMeshRecordCodec` provides bounded, versioned framing independent of the storage provider.
- `FriendMeshStorageCrypto` adapts the pinned ESP32-S3 libsodium XChaCha implementation and generates random nonces.
- `FriendMeshMasterKey` implements the fixed 108-byte wrapped-key format, bounded pre-KDF validation, device-binding AAD, and the ten fixed domain subkeys without generating or persisting a production key.
- `FriendMeshWrappedKeyStore` implements backend-independent two-slot recovery, authenticated generation selection, stale-generation rejection before writing, exact readback, degraded-state reporting, and divergent-generation quarantine.
- `FriendMeshInternalKeySlots` maps its default two slots onto internal LittleFS at `/friendmesh/keys/master_a.bin` and `master_b.bin` using full-atomic `SafeFile` writes. No startup or UI path constructs the default production mapping, so production key files remain disabled; D-01 supplies separate diagnostic paths only when the operator starts its test.
- `FriendMeshKeySlotSelfTest` is an explicit asynchronous D-01 action that uses fixed non-secret material and separate `/friendmesh/diagnostics/keyslot_test_a.bin` and `keyslot_test_b.bin` paths. It commits generations one and two, authenticates both, corrupts the old slot, proves degraded recovery from generation two, and removes both records and `.tmp` companions. It never opens the production slot paths.
- A device known-answer self-test uses a fixed key/nonce/header/plaintext and independently generated XChaCha ciphertext/tag.
- The host structural harness covers record round trip, wrong key, every single-byte mutation, every truncation length, and expected type/scope mismatch under undefined-behavior and bounds sanitizers. Its wrapped-key checks freeze an exact 108-byte vector, reject wrong credentials and device bindings, mutate every byte, test every truncation, prove excessive KDF costs are rejected before the KDF call, and verify all ten subkeys are stable and pairwise distinct.
- The wrapped-key host vector uses a deterministic test KDF and test AEAD to freeze framing, state transitions, and rejection behavior. It is not an independent Argon2id/XChaCha cryptographic known-answer vector; physical Argon2id and the firmware XChaCha self-test provide separate primitive evidence.
- Fault-injected host slot tests cover empty storage, initial commit, alternating generations, partial inactive-slot writes at two offsets, corrupt post-write readback, wrong PIN, stale generations, peer-slot I/O failure, and same-generation divergent master keys. Every failure preserves or selects the last authenticated generation and wipes failed unlock output.
- D-01 refreshes current/total, boot low-water, and largest contiguous block for internal heap and PSRAM every second, and shows internal filesystem use. Diagnostic exports capture the same resource snapshot for test evidence.
- An explicit D-01 action runs five non-secret Argon2id v1.3 candidates asynchronously: 256 KiB/one pass, 512 KiB/one pass, and 1 MiB/one through three passes. It records success and elapsed milliseconds, logs each result, wipes derived output, and never persists the fixed benchmark password, salt, or result as key material.
- D-01 and diagnostic exports expose the key-slot self-test state, failed step, elapsed time, recovered generation, authenticated-slot mask, degraded recovery result, and cleanup result. The action is never automatic and still requires physical execution evidence.
- The first physical press exposed a deterministic lock-order bug: the TFT task holds the global `spiLock` while LVGL dispatches callbacks, while the original synchronous slot test tried to acquire `spiLock` again at its first `clearSlots()` call. Because the lock is non-recursive and waits indefinitely, the UI stalled until the device rebooted. The corrected callback only creates a low-priority 12 KiB worker on CPU 1 and returns; the worker publishes thread-safe state and begins LittleFS work after the TFT callback can release `spiLock`. The failed attempt blocked before any diagnostic file operation and never referenced production slot paths.
- The corrected physical D-01 run passed in 5172 ms: failed step `NONE`, generation 2 selected from authenticated-slot mask `0x02`, degraded recovery `true`, and cleanup `PASS`. The export retained healthy heap/PSRAM largest-block and low-water measurements and showed 73728/3538944 bytes of internal LittleFS in use. This proves the isolated normal-execution provider path, not reboot or interrupted-write recovery.
- No production storage key, signing seed, record, provider invocation, device binding, or PIN-derived value is created by this slice. FriendMesh signing remains `STORAGE_UNAVAILABLE` and transmit-disabled.

## Remaining Phase 3 gates

- Repeat the 12 KiB-worker physical benchmark on the original T-Deck and separately document the T-Deck Plus capability profile when hardware is available; freeze the final KDF parameters only after recovery/coexistence evidence.
- Define the stable 16-byte device-binding source and prove ordinary firmware updates do not strand the wrapped key.
- Separately exercise interrupted creation, PIN change/rewrap, and reboot recovery without using `uploadfs` or erasing existing Meshtastic configuration; the isolated normal-execution D-01 slot test is physically passed.
- Implement the general internal essential-record and SD providers, transaction journal, snapshots, recovery, compaction, capacity accounting, and durable outbox.
- Perform power-cut, SD removal/full/read-only/corrupt, reboot nonce campaign, and plaintext-on-disk scans.
- Design and test locked/degraded/recovery UI in all six themes and all three input methods.

## Primary references

- [Libsodium XChaCha20-Poly1305 construction](https://doc.libsodium.org/secret-key_cryptography/aead/chacha20-poly1305/xchacha20-poly1305_construction)
- [Libsodium password hashing API](https://doc.libsodium.org/password_hashing/default_phf)
- [Libsodium key derivation](https://doc.libsodium.org/key_derivation)
- [RFC 9106: Argon2](https://www.rfc-editor.org/rfc/rfc9106)
