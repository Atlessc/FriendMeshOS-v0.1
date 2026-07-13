# FriendMeshOS Upstream Delta Ledger

Status: active template and baseline tracker  
Last reviewed: 2026-07-12  
Pinned Meshtastic line documented by the architecture handbook: `v2.7.26.54e0d8d`  
FriendMeshOS repository snapshot used to create this ledger: `876ec5691f78bd12545304ef623f8f1b58fb9eeb` on branch `develop`

This ledger makes upstream Meshtastic adoption selective, reviewable, and reproducible. It records exact comparison endpoints, local adaptations, intentionally skipped changes, generated artifacts, verification evidence, and residual risks. It must be updated before an upstream change is declared complete.

The compatibility contract is defined in [`MESHTASTIC_CORE_AND_FRIENDMESH_ARCHITECTURE.md`](../../MESHTASTIC_CORE_AND_FRIENDMESH_ARCHITECTURE.md). Product behavior is defined in [`FRIENDMESH_FULL_IMPLEMENTATION_GAME_PLAN.md`](../../FRIENDMESH_FULL_IMPLEMENTATION_GAME_PLAN.md). Related decisions and risks live in [`DECISION_LOG.md`](DECISION_LOG.md) and [`RISK_REGISTER.md`](RISK_REGISTER.md).

## Status and classification vocabulary

Delta status:

- `CANDIDATE`: range identified; review not started.
- `REVIEWING`: schema/core/security impact analysis underway.
- `ADAPTING`: selected changes are being ported against FriendMesh boundaries.
- `VERIFYING`: implementation complete; required tests are incomplete.
- `ACCEPTED`: exact range, adaptations, tests, docs, and residual risks approved.
- `PARTIAL`: only explicitly listed commits/paths were adopted.
- `DEFERRED`: useful but intentionally postponed with rationale.
- `REJECTED`: reviewed and deliberately not adopted.
- `SUPERSEDED`: replaced by a later ledger entry.

Change classification:

1. `REQUIRED COMPATIBILITY`: needed to communicate safely/correctly with supported current nodes or clients.
2. `T-DECK RELIABILITY/SECURITY`: applicable bug, regulatory, safety, security, or hardware correction.
3. `OPTIONAL CAPABILITY`: useful but not required for the compatibility contract.
4. `NOT APPLICABLE`: another board/UI/platform or disabled feature with no shared dependency effect.
5. `CONFLICT / ADAPT`: overlaps FriendMeshOS architecture and must be reimplemented rather than copied wholesale.

## Baseline identity

Do not start an import until every `REQUIRED` field is exact. A version string alone is not an auditable Git range.

| Field | Value | State | Evidence / next action |
| --- | --- | --- | --- |
| FriendMeshOS product version at planning | `v0.2.1` | DOCUMENTED | Game plan introduction |
| Meshtastic base label | `v2.7.26.54e0d8d` | DOCUMENTED | Game plan introduction and architecture handbook |
| Exact upstream firmware commit | `54e0d8d0ab2ff56b3a9ce967e53f79e49af560fb` | VERIFIED TAG ENDPOINT | [Official Meshtastic release `v2.7.26.54e0d8d`](https://github.com/meshtastic/firmware/releases/tag/v2.7.26.54e0d8d) links to the [full commit](https://github.com/meshtastic/firmware/commit/54e0d8d0ab2ff56b3a9ce967e53f79e49af560fb); local checkout equivalence still requires Phase 1 file/vector comparison |
| Exact upstream protobuf commit | `6b1ded439633cd03d4af85b44231b91d1d106278` | VERIFIED ENDPOINT | Firmware commit `54e0d8d` records this submodule; five critical local schemas match its SHA-256 values in [`MESHTASTIC_V2_7_26_BASELINE.md`](MESHTASTIC_V2_7_26_BASELINE.md) |
| FriendMeshOS baseline commit for Phase 0 ledger | `876ec5691f78bd12545304ef623f8f1b58fb9eeb` | CAPTURED | `git rev-parse HEAD` on 2026-07-12 |
| FriendMeshOS baseline description | `v0.1.0-baseline-11-g876ec56` | CAPTURED | `git describe --tags --always` on 2026-07-12 |
| Active branch at capture | `develop` | CAPTURED | `git branch --show-current` on 2026-07-12 |
| Upstream firmware remote | Not configured as a distinct remote in the captured checkout | REQUIRED | Add/identify read-only upstream source before comparison; do not confuse `origin` fork with upstream |
| Fork origin | `https://github.com/Atlessc/FriendMeshOS-v0.1.git` | CAPTURED | `git remote -v` on 2026-07-12 |
| Protobuf generator command | `bin/regen-protos.sh` | DOCUMENTED | `AGENTS.md` quick commands |
| Generator/tool versions | PlatformIO `6.1.19`; Python `3.11.9`; runtime Nanopb `0.4.9.1`; regeneration script requires Nanopb generator `0.4.9` | PARTIAL | Exact generator bundle is absent and must be installed/hashed before regeneration; see baseline manifest |
| Primary supported environment | `t-deck-tft` | APPROVED | Game plan capability contract |

## Upstream import index

One row represents one coherent reviewed comparison range. Split unrelated radio, schema, security, UI, and board-only work when doing so makes rollback and evidence clearer.

| Delta ID | Date opened | Local base commit | Upstream from | Upstream through | Scope | Classification summary | Status | Owner | Detailed record | Test evidence | Residual risks |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| UPD-000 | 2026-07-12 | `876ec569...` | `54e0d8d0ab2ff56b3a9ce967e53f79e49af560fb` | `54e0d8d0ab2ff56b3a9ce967e53f79e49af560fb` | Establish pinned release equivalence, protobuf provenance, tools, and vectors; no code import | REQUIRED COMPATIBILITY baseline | REVIEWING | Upstream/Protocol | This document: Baseline identity | Firmware tag endpoint verified; local/schema/vector evidence pending Phase 1 | RSK-003, RSK-004 |
| UPD-001 | Not opened | TBD | TBD | TBD | Phase 15 rehearsal against one later Meshtastic point; comparison only unless separately approved | To classify | CANDIDATE | Upstream/Protocol | Create from template below | Pending | RSK-003, RSK-004 |

## Meshtastic boundary inventory

Review every row affected by a candidate range. `No direct diff` still requires dependency-impact consideration when shared headers, generated types, build flags, or call contracts change.

| Boundary | Current source anchors | FriendMesh contract | Required classification questions | Minimum evidence | Last reviewed delta |
| --- | --- | --- | --- | --- | --- |
| Radio frame | `src/mesh/RadioInterface.h`, `RadioInterface.cpp`, `RadioLibInterface.cpp` | Preserve 16-byte header, limits, flags, regional legality | Header layout? payload ceiling? CAD/contention? airtime/regional tables? | Static/layout assertions, byte capture, region/preset build | UPD-000 pending |
| Routing and hops | `src/mesh/Router.cpp`, `FloodingRouter.cpp`, `ReliableRouter.cpp`, `NextHopRouter.cpp` | Preserve flooding, next-hop, duplicate, retry, ACK/NAK and hop semantics | Hop math? retry window? implicit ACK? queue/priority behavior? | Direct/multi-hop captures, ACK/NAK/timeout/duplicate cases | UPD-000 pending |
| Packet identity | Router and packet-history sources | Preserve node/packet IDs, duplicate identity, ACK correlation, nonce inputs | ID width/generation? history eviction? collision handling? | Packet-ID/replay vectors and long-run test | UPD-000 pending |
| Channel crypto | `src/mesh/CryptoEngine.cpp`, `Channels.cpp`, `protobufs/meshtastic/channel.proto` | Preserve channel AES-CTR selection and key lifecycle; FriendMesh adds signed app envelope above it | Nonce/key/PSK shorthand/hash/reset changes? | Known-answer vectors and stock encrypted channel exchange | UPD-000 pending |
| PKI crypto | `src/mesh/CryptoEngine.cpp`, NodeInfo/key verification sources | Preserve X25519/SHA-256/AES-CCM wire behavior and identity lifecycle | AEAD layout? extra nonce? key advertisement? reset/verification behavior? | Bidirectional PKI DM, key-change/reset, official vector comparison | UPD-000 pending |
| Protobuf wire | `protobufs/meshtastic/*.proto`, generated Nanopb bindings | Preserve field numbers/types, enum values, bounds, unknown handling | Added/deprecated fields? enum renumber? max sizes? generator drift? | Schema diff, generated diff, old/new client vectors, unknown enums | UPD-000 pending |
| Port numbers | `protobufs/meshtastic/portnums.proto` | Preserve standard ports; FriendMesh uses magic/version on `PRIVATE_APP` pending registered-port review | New/conflicting port? changed semantics? deprecation? | Dispatch tests, stock opaque-relay test, unknown port behavior | UPD-000 pending |
| NodeDB/identity | `src/mesh/NodeDB.cpp`, `NodeInfoModule.cpp`, device-only schema | NodeDB remains discovery/cache, never membership authority | Eviction/schema/key/fingerprint/channel changes? | Missing/stale/evicted/key-change/collision cases | UPD-000 pending |
| Position/privacy | `src/modules/PositionModule.cpp`, `src/mesh/PositionPrecision.cpp`, `mesh.proto` | Preserve position source/precision; never restore hidden precision | Field/source/precision/transmit schedule changes? | Stock position exchange, precision capture, stale/source UI tests | UPD-000 pending |
| Queue/airtime | Router/radio queues and `airtime.cpp` | FriendMesh background work cannot starve ACK/text/admin/SOS | Priority enum? queue length/eviction? utilization threshold? | Congestion/eviction measurement on slow presets | UPD-000 pending |
| Client/BLE/serial | `src/mesh/PhoneAPI.cpp`, `StreamAPI.cpp`, `MeshService.cpp`, `mesh.proto` | Stock phone/client configuration round trips remain functional | Framing/config sequencing/auth/channel mutation changes? | Current supported phone clients, BLE reconnect, serial/TCP round trip | UPD-000 pending |
| Admin/config | `src/modules/AdminModule.cpp`, `admin.proto`, `config.proto` | Protected FriendMesh channels coexist with standard admin paths and deliberate recovery | Auth/session passkey? channel mutation? managed mode? reset behavior? | Protected-write/recovery tests and unrelated config round trip | UPD-000 pending |
| Storage/migrations | NodeDB/config persistence plus FriendMesh storage providers | Upstream migrations cannot corrupt FriendMesh essential state or key references | File/schema/key lifecycle/reset/partition changes? | Upgrade/downgrade backup, reboot, power-cut, last-known-good recovery | UPD-000 pending |
| T-Deck platform | `variants/t-deck-tft`, graphics/input/GPS/SD/platform sources | One runtime-detected T-Deck image; original/Plus results separate | Pin/peripheral/input/display/PSRAM/SPI/power changes? | `t-deck-tft` build plus original/Plus physical matrix | UPD-000 pending |
| Public Meshtastic UI | existing Meshtastic area and device UI library | FriendMesh may redesign T-Deck UI but ordinary Meshtastic functions remain available | Shared navigation/config regression? upstream UI dependency API change? | Public text/node/map/config smoke test in six themes | UPD-000 pending |

## Planned local FriendMesh delta inventory

These are intentional local application-layer extensions, not upstream imports. Recording them helps reviewers distinguish an upstream conflict from an owned FriendMesh change. `CONTRACT ONLY` means the plan exists but implementation evidence has not yet been linked.

| Local delta ID | Owned behavior | Expected source area | Must preserve | State | Implementation commit(s) | Verification evidence | Upstream conflict notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| FMD-001 | Read-only packet/event and capability observability | Diagnostics services/UI, test fixtures | Secret/precise-location redaction; no packet behavior changes | VERIFYING | Uncommitted Phase 1 working tree on `876ec56` | Fixed vectors and `t-deck-tft` build pass; automatic refresh, position transitions, semantic D-02, and redacted export compile; physical D-01/D-02 matrix pending | Adds a no-op queue-status view hook and T-Deck-only dynamic diagnostics UI; rebase against queue/enum/client changes without forking semantics |
| FMD-002 | Versioned signed `FriendMeshEnvelope` dispatch on `PRIVATE_APP` | New protobuf/module/service | Standard ports and opaque relay compatibility | VERIFYING | Uncommitted Phase 2 working tree on `876ec56` | Generated 229-byte maximum frame; strict host protocol/replay checks and `t-deck-tft` build pass; physical opaque-relay/stock regression and fuzzing pending | Revisit if upstream assigns a registered port or changes private-app handling |
| FMD-003 | FriendMesh Ed25519 signing identity bound to Meshtastic PKI identity | Identity/security service and protected storage | Existing X25519/PKI DM behavior and reset warnings | VERIFYING | Uncommitted Phase 2 working tree on `876ec56` | RFC 8032 OpenSSL vector and ESP32 libsodium compile/link pass; NodeDB binding and firmware self-test exist; protected persistence, UI, and physical two-device proof pending | Never replace or derive from upstream X25519 key bytes |
| FMD-004 | Authenticated FriendMesh storage, journals, durable outbox, and SD provider | FriendMesh storage services | Upstream config/NodeDB persistence and reset behavior | CONTRACT ONLY | Pending Phase 3 | Power-cut/storage matrix pending | Review every upstream storage/reset migration |
| FMD-005 | Eight-group/eight-member signed domain model over one protected shared carrier | FriendMesh group services plus carrier/channel API adapters | Meshtastic channel table, independent inner group security, and stock config round trips | CONTRACT ONLY | Pending Phases 2-4 | Carrier, capacity, cross-group isolation, and protection tests pending | Adapt to upstream channel/admin API instead of bypassing it; never treat the outer carrier PSK as group authorization |
| FMD-006 | Nearby signed invitation, in-person approval, and member-specific key grants | FriendMesh membership protocol/UI | Standard NodeInfo/key verification and ordinary channel import/recovery | CONTRACT ONLY | Pending Phase 5 | Spoof/multi-candidate tests pending | Channel URLs do not become approval authority |
| FMD-007 | Epoch rekey, replacement, succession, erase, and disband transactions | FriendMesh security/storage services | Upstream channel PSK lifecycle; full-reset identity behavior | CONTRACT ONLY | Pending Phase 6 | Cut-point/offline/partition tests pending | Security-sensitive overlap with channel/admin/reset changes |
| FMD-008 | Signed chat, fragmentation, reactions, tombstones, and outbox | FriendMesh module/UI | Ordinary text and queue priorities | CONTRACT ONLY | Pending Phase 7 | Loss/reorder/stock-client tests pending | Re-measure bounds if packet size/queue changes |
| FMD-009 | Bounded FriendMesh event history synchronization | FriendMesh sync service | Live routing, ACK, text, admin, and SOS priority | CONTRACT ONLY | Pending Phase 8 | Congestion/replica tests pending | Re-tune only from measured upstream queue/airtime changes |
| FMD-010 | Group map, navigation, markers, and meetups above standard Position/Waypoint state | FriendMesh navigation/map services/UI | Position precision/source and standard waypoint interoperability | CONTRACT ONLY | Pending Phases 9-11 | Geographic/privacy/waypoint tests pending | Never infer precision removed upstream |
| FMD-011 | Signed SOS and Help incident state machines plus deliberate standard-text fallback | FriendMesh alert services/UI | Standard text fallback, legal airtime, ordinary Meshtastic availability | CONTRACT ONLY | Pending Phases 12-13 | Physical drills pending | Revisit `ALERT_APP`/priority behavior without assuming emergency dispatch |
| FMD-012 | SD-only verified Contacts and notification center | FriendMesh contacts/storage/UI | Standard PKI DM behavior and SD capability checks | CONTRACT ONLY | Pending Phase 14 | Consent/key-change/storage tests pending | Reconcile upstream PKI/reset changes |
| FMD-013 | Stock-client protected-channel adapter and advanced recovery | Channel/admin adapters/UI | Unrelated stock configuration and BLE operations | CONTRACT ONLY | Pending Phase 15 | Supported-client matrix pending | High-conflict area for upstream admin/channel API changes |
| FMD-014 | Application-embedded T-Deck startup-only splash, separate device-controls screen, and temporarily offline-only map tiles | FriendMesh branding and T-Deck map construction | Saved configuration, reboot/pairing/shutdown choices, SD/offline maps, ordinary Meshtastic UI, and future bounded online-map restoration | VERIFYING | Uncommitted physical-regression working tree on `876ec56` | `t-deck-tft` build passes; startup splash, offline-only no-stall map, chooser visibility, labels, focus/navigation, cancel, restart, pairing mode, and power-off behavior physically pass | Recheck upstream boot-asset, reboot-screen, and asynchronous tile-service changes before removing the local gate |

## Required review sequence for a candidate range

- [ ] Record exact local base, upstream start, upstream end, repositories/remotes, and comparison command.
- [ ] Save commit list and high-level diffstat.
- [ ] Review protobuf sources before generated code or firmware implementation.
- [ ] Record every field/enum/bound/generator change and old-client behavior.
- [ ] Review radio header, packet limits, region tables, presets, CAD, airtime, and queue behavior.
- [ ] Review routing, duplicate, next-hop, ACK/NAK, retry, and priority behavior.
- [ ] Review channel crypto, PKI, nonce construction, key generation/storage, admin authorization, and reset behavior.
- [ ] Review NodeDB, Position, telemetry, text, traceroute, client transport, BLE, and storage migrations.
- [ ] Review T-Deck pin/peripheral/display/input/GPS/SD/power effects.
- [ ] Classify each relevant commit/path with one of the five classifications above.
- [ ] Identify every conflict with `FMD-*` owned behavior and describe the adaptation.
- [ ] Port only the smallest coherent selected change set.
- [ ] Record intentionally skipped commits and why their dependencies are not required.
- [ ] Regenerate protobufs only from the reviewed schema and record exact tools.
- [ ] Run the evidence matrix and attach artifacts.
- [ ] Update architecture, decision log, risk register, roadmap milestone, changelog, and this ledger.
- [ ] Obtain protocol/security/T-Deck review before `ACCEPTED`.

## Detailed delta record template

Copy this section for every `UPD-*` row and link it from the index.

### UPD-XXX — Short range name

| Field | Value |
| --- | --- |
| Status | CANDIDATE |
| Opened / last updated | YYYY-MM-DD / YYYY-MM-DD |
| Owner / reviewers | Role or name / role or name |
| Local repository and base commit | URL / full SHA |
| Upstream firmware repository | URL |
| Upstream compare range | Full start SHA..full end SHA |
| Upstream protobuf repository/range | URL / full start SHA..full end SHA |
| Comparison commands | Exact non-destructive commands |
| Generator/tool versions | PlatformIO, compiler, Python, protoc/Nanopb, regen script SHA |
| Target environment | `t-deck-tft` |
| Related decisions | DEC-* |
| Related risks | RSK-* |

#### Commit and path classification

| Upstream commit/path | Summary | Classification | Dependency notes | Local action | Local commit | Reviewer |
| --- | --- | --- | --- | --- | --- | --- |
| Full SHA / path | What behavior changes | REQUIRED / T-DECK / OPTIONAL / N/A / CONFLICT | Required adjacent changes | Port / adapt / skip / defer | Full SHA or pending | Pending |

#### Protobuf and generated-code manifest

| Schema/message/enum | Wire change | Numeric compatibility | Nanopb bound/memory effect | Older client behavior | Generated files | Evidence |
| --- | --- | --- | --- | --- | --- | --- |
| None recorded | — | — | — | — | — | Pending review |

#### FriendMesh adaptation record

| FMD ID / source path | Conflict | Adaptation | Behavior intentionally retained | Migration/recovery | Evidence |
| --- | --- | --- | --- | --- | --- |
| FMD-XXX | Describe overlap | Describe local reimplementation | Name invariant | Upgrade/rollback path | Test/report link |

#### Intentionally skipped changes

| Upstream commit/path | Reason skipped | Dependency checked | Revisit trigger | Reviewer |
| --- | --- | --- | --- | --- |
| None recorded | — | — | — | — |

#### Verification results

| Gate | Required result | Status | Artifact / command / device IDs |
| --- | --- | --- | --- |
| Clean `t-deck-tft` build | Pass | NOT RUN | Pending |
| Native unit/protocol vectors | Pass | NOT RUN | Pending |
| Static/format checks | Pass | NOT RUN | Pending |
| Direct and multi-hop packet exchange | Compatible | NOT RUN | Pending |
| Public/channel text | Bidirectional with untouched node/client | NOT RUN | Pending |
| PKI DM and key-change behavior | Compatible | NOT RUN | Pending |
| Position/NodeInfo/telemetry/traceroute | Compatible | NOT RUN | Pending |
| ACK/NAK/retry/timeout/duplicate/queue pressure | Expected enums and timing | NOT RUN | Pending |
| BLE phone config and reconnect | Round trip without unrelated mutation | NOT RUN | Pending |
| Protected FriendMesh channel | Read allowed; default edit/delete protected; recovery works | NOT RUN | Pending |
| FriendMesh envelope opaque relay | Relay succeeds; stock client does not render payload | NOT RUN | Pending |
| Reboot/update/recovery | Essential state retained or documented migration succeeds | NOT RUN | Pending |
| Original T-Deck physical run | Pass with capability record | NOT RUN | Pending |
| T-Deck Plus physical run | Pass with capability record | NOT RUN | Pending |
| Six-theme affected-screen matrix | Pass | NOT RUN | Pending |

#### Acceptance

| Field | Value |
| --- | --- |
| Accepted/rejected/deferred by | Pending |
| Date | Pending |
| Final local commit range | Pending |
| Residual risks and owner | Pending |
| Architecture/roadmap/changelog links | Pending |
| Rollback point and recovery steps | Pending |

## Ledger review history

| Date | Scope | Reviewer | Result | Evidence |
| --- | --- | --- | --- | --- |
| 2026-07-12 | Initial Phase 0 template, boundary map, and planned local-delta inventory | Phase 0 documentation pass | Created; exact upstream baseline remains unresolved | UPD-000, DEC-R09, RSK-004 |
| 2026-07-12 | Resolve official firmware tag endpoint | Phase 0 implementation pass | Official `v2.7.26.54e0d8d` release resolves to full commit `54e0d8d0ab2ff56b3a9ce967e53f79e49af560fb`; vendored checkout/schema/tool/vector equivalence remains open | Official Meshtastic release and commit pages; UPD-000 |
