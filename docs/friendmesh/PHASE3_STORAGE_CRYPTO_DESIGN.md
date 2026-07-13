# FriendMesh Phase 3 storage cryptography design

Status: authenticated record, wrapped-master-key format, two-slot internal-key backend, stable ESP32-S3 factory-ID device binding, isolated D-01 provider/reboot/update diagnostics, and the explicit production key-lifecycle manager are implemented; physical PIN provisioning, journal, and production identity persistence remain gated
Target: LilyGO T-Deck / T-Deck Plus runtime-capability build only  
Base: Meshtastic 2.7.26

## Security boundary

FriendMesh security-sensitive state must not use Meshtastic `SafeFile` readback as an integrity or confidentiality boundary. `SafeFile` may later be used beneath the FriendMesh transaction layer for temporary-file/rename mechanics, but a record is trusted only after strict framing checks and AEAD authentication.

This design protects confidentiality and detects modification of records copied from internal flash or SD. It does not claim hardware-backed security. A six-digit PIN has low entropy, and a person who can extract flash and run custom firmware can attempt offline PIN guesses. Retry delays help only against the compliant on-device UI. Full-snapshot rollback also cannot be detected reliably without protected monotonic hardware state. Release copy must describe this as software-only, casual-extraction resistance.

The existing Device UI PIN is stored as a numeric UI configuration value and compared directly. It is not an acceptable storage-key derivation interface. Production storage uses a separate exactly-six-ASCII-digit credential policy, rejects `000000`, and never persists the credential itself. The T-Deck setup/unlock/rewrap UI for that credential remains to be implemented and physically reviewed.

## Frozen primitive choices

- Record encryption: libsodium XChaCha20-Poly1305 IETF, 32-byte key, 24-byte nonce, 16-byte authentication tag.
- Record nonce: fresh CSPRNG output for every encryption. An all-zero result is regenerated. A nonce is never derived from sequence, time, path, record ID, or reboot state.
- Password KDF: explicit Argon2id v1.3 (`crypto_pwhash_ALG_ARGON2ID13`), 16-byte random salt, 32-byte output. The algorithm identifier, salt, operations limit, and memory limit are persisted in the wrapped-key header.
- Domain separation: libsodium `crypto_kdf_derive_from_key()` with fixed eight-byte contexts and independently assigned subkey IDs.
- Storage master key: 32 random bytes generated once. PIN changes rewrap this key transactionally instead of re-encrypting every record.

The v1 Argon2id profile is frozen at 1024 KiB and three operations. The bundled interactive profile requests 64 MiB, while the target has 8 MiB PSRAM and substantially less internal RAM, so it is not feasible here. The corrected physical boot reported approximately 2.90 MiB free PSRAM and 175,624 bytes free internal heap after normal UI/radio initialization, and the physical benchmark/recovery campaign measured the conservative feasible candidates before selecting the v1 default. Parameters remain stored per wrapped key so later profiles can remain readable without guessing which KDF cost created a record.

The first physical benchmark build reset before completing its first case because it pinned a continuously runnable Argon2 task to CPU 0, whose idle task is protected by the five-second task watchdog in this ESP32-S3 configuration. The corrected benchmark runs at idle priority on CPU 1, logs before entering each monolithic KDF call, and records the task stack low-water mark. The first corrected physical run completed all five cases, but its 8 KiB task stack retained only 588 bytes, so the dedicated benchmark/KDF worker reserve is increased to 12 KiB before further use.

| Memory | Operations | Physical duration | Result |
| ---: | ---: | ---: | --- |
| 256 KiB | 1 | 296 ms | Pass |
| 512 KiB | 1 | 266 ms | Pass |
| 1024 KiB | 1 | 493 ms | Pass |
| 1024 KiB | 2 | 679 ms | Pass |
| 1024 KiB | 3 | 1089 ms | Pass |

The run reduced PSRAM from approximately 2.90 MiB free to a 1.85 MiB boot low-water mark, supporting that the memory-hard workspace used external RAM. Internal heap retained a 126,860-byte low-water mark and 131,060-byte largest block. LoRa telemetry and position events were captured during the same diagnostic session. `1024 KiB / 3 operations` completed in approximately 1.1 seconds with useful remaining headroom. It is the frozen v1 profile after both isolated wrapped-key recovery actions physically passed and public messaging, private direct messaging, and configuration loading remained healthy afterward.

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

The AEAD additional authenticated data is the exact 60-byte header followed by a caller-supplied, nonzero 16-byte device binding. The binding is deliberately not serialized beside the wrapped key: moving the record to a device with a different binding must fail authentication.

On the ESP32-S3 target, `FriendMeshDeviceBinding` reads the factory-burned 128-bit optional unique ID from `ESP_EFUSE_OPTIONAL_UNIQUE_ID` and uses those exact 16 bytes as the binding. A read error, unsupported target, wrong length, or all-zero ID fails closed and wipes the output. The temporary raw ID is wiped after derivation and is never serialized, persisted, logged, or exported. This value is stable across ordinary firmware updates and power loss, but it is not secret and does not create a hardware security root; it is only a software-enforced device-copy binding.

The codec accepts `1..64` credential bytes so the storage layer is not tied to one UI encoding. The product UI still owns the approved six-digit numeric PIN policy. Wrong credentials, the wrong device binding, and authenticated-record tampering all fail closed without returning master-key bytes. Unsupported or excessive KDF parameters are rejected before Argon2id runs, preventing a modified header from forcing an unbounded allocation or denial-of-service workload.

The stored v1 defaults are frozen at 1024 KiB and three operations. The separate power-cut campaign still gates the general transaction engine and production persistence, but it no longer gates this measured KDF cost selection. Persisting the parameters in each record allows later profiles to remain readable without guessing which KDF cost created the record.

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
- `FriendMeshDeviceBinding` derives the exact 16-byte T-Deck binding from the ESP32-S3 factory optional unique ID, fails closed when the ID is unavailable or invalid, and wipes temporary/output buffers on failure.
- `FriendMeshWrappedKeyStore` implements backend-independent two-slot recovery, authenticated generation selection, stale-generation rejection before writing, exact readback, degraded-state reporting, and divergent-generation quarantine.
- `FriendMeshInternalKeySlots` maps its default two slots onto internal LittleFS at `/friendmesh/keys/master_a.bin` and `master_b.bin` using full-atomic `SafeFile` writes. Startup now constructs this production mapping only for a read-only presence probe; it does not generate, wrap, write, unlock, or derive a production key. D-01 diagnostic actions continue to use separate paths.
- `FriendMeshStorageKeyManager` is the production lifecycle boundary. It implements explicit `probe`, `provision`, `unlock`, `rewrap`, `lock`, and domain-subkey derivation operations; validates the dedicated six-digit credential policy; generates the 32-byte master key, salts, and nonces through the firmware CSPRNG; binds wrapped records to the factory ID; verifies every committed generation; maintains two authenticated generations when provisioning and after a completed rewrap; wipes temporary/master-key buffers; and never silently provisions or falls back to plaintext.
- Host lifecycle checks cover PIN policy, unavailable binding and RNG, explicit-only provisioning, two-slot readiness, subkey derivation, lock/unlock, wrong credentials and binding, transactional rewrap, old-credential rejection, and a partial-write failure that preserves the authenticated prior generation.
- `FriendMeshKeySlotSelfTest` is an explicit asynchronous D-01 action that uses fixed non-secret material and separate `/friendmesh/diagnostics/keyslot_test_a.bin` and `keyslot_test_b.bin` paths. It commits generations one and two, authenticates both, corrupts the old slot, proves degraded recovery from generation two, and removes both records and `.tmp` companions. It never opens the production slot paths.
- `FriendMeshKeySlotRebootTest` is a separate explicit asynchronous D-01 action with fixed non-secret credentials, key, binding, salts, and nonces. Its first stage commits generation one at `/friendmesh/diagnostics/keyslot_reboot_a.bin`, leaves a deliberately truncated inactive `keyslot_reboot_b.bin.tmp`, writes a diagnostic marker, and requires a real device reboot. After reboot, a second operator press authenticates the last-known-good generation, removes the interrupted temporary, wraps the same test master key as generation two under the new fixed credential, removes the old slot, proves the old credential no longer authenticates, reloads with the new credential, and cleans every diagnostic artifact. It never constructs the production slot mapping.
- `FriendMeshDeviceBindingTest` is a separate explicit asynchronous three-stage D-01 action. Build A wraps one fixed non-secret test key under the real factory-derived binding, proves same-device unlock and altered-binding rejection, and records only an internal application fingerprint marker before requiring a real reboot. The reboot stage requires the same application fingerprint and repeats both checks before requiring a genuinely changed Build B. The update stage requires a different fingerprint, repeats the checks, and removes its isolated slots, temporaries, and marker. It uses only `/friendmesh/diagnostics/device_binding_a.bin`, `device_binding_b.bin`, and `device_binding.marker`; no production path is constructed.
- A device known-answer self-test uses a fixed key/nonce/header/plaintext and independently generated XChaCha ciphertext/tag.
- The host structural harness covers record round trip, wrong key, every single-byte mutation, every truncation length, and expected type/scope mismatch under undefined-behavior and bounds sanitizers. Its wrapped-key checks freeze an exact 108-byte vector, reject wrong credentials and device bindings, mutate every byte, test every truncation, prove excessive KDF costs are rejected before the KDF call, and verify all ten subkeys are stable and pairwise distinct. Binding checks prove identical 128-bit hardware IDs derive identically, distinct IDs derive distinctly, and short/all-zero inputs fail closed with wiped output.
- The wrapped-key host vector uses a deterministic test KDF and test AEAD to freeze framing, state transitions, and rejection behavior. It is not an independent Argon2id/XChaCha cryptographic known-answer vector; physical Argon2id and the firmware XChaCha self-test provide separate primitive evidence.
- Fault-injected host slot tests cover empty storage, initial commit, alternating generations, partial inactive-slot writes at two offsets, corrupt post-write readback, wrong PIN, stale generations, peer-slot I/O failure, and same-generation divergent master keys. Every failure preserves or selects the last authenticated generation and wipes failed unlock output.
- D-01 refreshes current/total, boot low-water, and largest contiguous block for internal heap and PSRAM every second, and shows internal filesystem use. Diagnostic exports capture the same resource snapshot for test evidence.
- An explicit D-01 action runs five non-secret Argon2id v1.3 candidates asynchronously: 256 KiB/one pass, 512 KiB/one pass, and 1 MiB/one through three passes. It records success and elapsed milliseconds, logs each result, wipes derived output, and never persists the fixed benchmark password, salt, or result as key material.
- D-01 and diagnostic exports expose the key-slot self-test state, failed step, elapsed time, recovered generation, authenticated-slot mask, degraded recovery result, and cleanup result.
- The first physical press exposed a deterministic lock-order bug: the TFT task holds the global `spiLock` while LVGL dispatches callbacks, while the original synchronous slot test tried to acquire `spiLock` again at its first `clearSlots()` call. Because the lock is non-recursive and waits indefinitely, the UI stalled until the device rebooted. The corrected callback only creates a low-priority 12 KiB worker on CPU 1 and returns; the worker publishes thread-safe state and begins LittleFS work after the TFT callback can release `spiLock`. The failed attempt blocked before any diagnostic file operation and never referenced production slot paths.
- The corrected physical D-01 run passed in 5172 ms: failed step `NONE`, generation 2 selected from authenticated-slot mask `0x02`, degraded recovery `true`, and cleanup `PASS`. The export retained healthy heap/PSRAM largest-block and low-water measurements and showed 73728/3538944 bytes of internal LittleFS in use. This proves the isolated normal-execution provider path, not reboot or interrupted-write recovery.
- D-01 and diagnostic exports expose the staged reboot test state, failed step, duration, generation, authenticated-slot mask, interrupted-write recovery, credential rewrap, old-credential rejection, and cleanup. The first stage intentionally retains only fixed non-secret test artifacts and disables same-process continuation; after reboot the operator must press `Run TEST Reboot/Rewrap` again to execute verification and cleanup.
- The staged test proves only that physical LittleFS preserves the last-known-good primary while the backend ignores a truncated inactive `.tmp`, and that the diagnostic two-slot codec can rewrap one fixed test master key under a new fixed credential. It does not prove automatic startup recovery, a production PIN UI, stable production device binding, secure erase, every rewrap power boundary, the general transaction journal, or production identity persistence.
- The physical two-stage run passed in 8982 ms after the required reboot: failed step `NONE`, generation 2 selected from authenticated-slot mask `0x02`, interrupted-write recovery `PASS`, credential rewrap `PASS`, old credential rejection `PASS`, and cleanup `PASS`. The final export showed 73728/3538944 bytes of internal LittleFS in use, matching the prior baseline, with healthy heap and PSRAM headroom. The operator then confirmed public messaging, private direct messaging, and configuration loading all remained healthy.
- D-01 and diagnostic exports expose only the binding-test state, failed step, duration, binding availability, same-device result, reboot result, firmware-change result, wrong-device rejection, and cleanup. The factory ID, derived binding, application fingerprint, fixed test credential, and fixed test key never enter logs or exports.
- The physical Build A/reboot/Build B sequence reached final screen result `SAME PASS REBOOT PASS UPDATE PASS WRONG REJECT CLEAN PASS`. Three retained exports independently record `PASS`, failed step `NONE`, duration 6938 ms, binding available, same-device pass, reboot pass, changed-firmware pass, wrong-device rejection, and cleanup pass. They exclude keys, message bodies, node IDs, coordinates, factory ID, binding value, and firmware fingerprint. This proves the same factory binding reopened the isolated wrapped test key after both a real reboot and a genuinely changed application image, while an altered binding failed authentication and cleanup removed the diagnostic artifacts.
- Startup invokes the production provider and factory-binding source only to report `NOT_CONFIGURED`, `LOCKED`, or a fail-closed availability/corruption state. It never invokes Argon2id, creates a credential-derived value, generates a production master key, or writes a production record. No production storage key or signing seed exists yet; FriendMesh signing remains `STORAGE_UNAVAILABLE` and transmit-disabled.

## Remaining Phase 3 gates

- Implement the explicit T-Deck storage-PIN setup, unlock, change-PIN, lock, and recovery-state UI without reusing the plaintext Device UI PIN; keep all KDF/filesystem work outside LVGL callbacks.
- Physically provision the first production storage master key only after the PIN UI is reviewed, then verify reboot unlock, wrong-PIN rejection, rewrap, old-PIN rejection, production-slot redundancy, cleanup/recovery messaging, and all six themes/input methods before allowing any consumer to persist a signing seed.
- Document a separate T-Deck Plus capability profile when that hardware becomes available; this does not block the original T-Deck v1 profile.
- Implement the general internal essential-record and SD providers, transaction journal, snapshots, recovery, compaction, capacity accounting, and durable outbox.
- Perform power-cut, SD removal/full/read-only/corrupt, reboot nonce campaign, and plaintext-on-disk scans.
- Design and test locked/degraded/recovery UI in all six themes and all three input methods.

## Primary references

- [Libsodium XChaCha20-Poly1305 construction](https://doc.libsodium.org/secret-key_cryptography/aead/chacha20-poly1305/xchacha20-poly1305_construction)
- [Libsodium password hashing API](https://doc.libsodium.org/password_hashing/default_phf)
- [Libsodium key derivation](https://doc.libsodium.org/key_derivation)
- [RFC 9106: Argon2](https://www.rfc-editor.org/rfc/rfc9106)
