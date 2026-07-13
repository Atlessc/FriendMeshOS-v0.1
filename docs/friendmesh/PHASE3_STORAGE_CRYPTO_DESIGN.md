# FriendMesh Phase 3 storage cryptography design

Status: first record-layer slice implemented; master-key wrapping, KDF tuning, providers, journal, and recovery remain gated  
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

## Key hierarchy draft

```text
six-digit PIN --Argon2id(salt, stored parameters)--> wrapping key
                                                      |
random storage master key <--- XChaCha20-Poly1305 wrapped-key record
        |
        +-- identity subkey
        +-- replay/journal subkey
        +-- group-state subkey
        +-- history subkey
        +-- outbox subkey
        +-- contacts/notifications subkeys
```

Contexts and subkey IDs will be frozen before the first real master key is generated. A subkey cannot be reused for both record storage and a radio/group protocol. The wrapped-key record format and device-binding AAD require their own deterministic vectors.

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

## Implemented evidence

- `FriendMeshRecordCodec` provides bounded, versioned framing independent of the storage provider.
- `FriendMeshStorageCrypto` adapts the pinned ESP32-S3 libsodium XChaCha implementation and generates random nonces.
- A device known-answer self-test uses a fixed key/nonce/header/plaintext and independently generated XChaCha ciphertext/tag.
- The host structural harness covers round trip, wrong key, every single-byte mutation, every truncation length, and expected type/scope mismatch under undefined-behavior and bounds sanitizers.
- D-01 refreshes current/total, boot low-water, and largest contiguous block for internal heap and PSRAM every second, and shows internal filesystem use. Diagnostic exports capture the same resource snapshot for test evidence.
- No production storage key, signing seed, record, provider, or PIN-derived value is created by this slice.

## Remaining Phase 3 gates

- Physically run and expose the XChaCha known-answer self-test.
- Benchmark Argon2id configurations on original T-Deck and T-Deck Plus capability profiles.
- Freeze wrapped-master-key and domain-context formats with deterministic vectors.
- Implement internal and SD providers, transaction journal, snapshots, recovery, compaction, capacity accounting, and durable outbox.
- Perform power-cut, SD removal/full/read-only/corrupt, reboot nonce campaign, and plaintext-on-disk scans.
- Design and test locked/degraded/recovery UI in all six themes and all three input methods.

## Primary references

- [Libsodium XChaCha20-Poly1305 construction](https://doc.libsodium.org/secret-key_cryptography/aead/chacha20-poly1305/xchacha20-poly1305_construction)
- [Libsodium password hashing API](https://doc.libsodium.org/password_hashing/default_phf)
- [Libsodium key derivation](https://doc.libsodium.org/key_derivation)
- [RFC 9106: Argon2](https://www.rfc-editor.org/rfc/rfc9106)
