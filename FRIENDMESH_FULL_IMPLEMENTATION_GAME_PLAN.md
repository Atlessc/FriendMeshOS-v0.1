# FriendMesh Full Implementation Game Plan

Status: active implementation; Phase 2 foundation in progress
Target: LilyGO T-Deck / T-Deck Plus class hardware using `t-deck-tft`  
FriendMeshOS version at planning time: `v0.2.1`  
Meshtastic base: `v2.7.26.54e0d8d`

This is the canonical execution plan for the friends, private groups, group chat, history, map, compass, meetup, marker, SOS, Help Request, and later verified-contact features of FriendMeshOS. It turns the approved product interview into implementable contracts, phase gates, screen behavior, theme requirements, test requirements, and honest feasibility boundaries.

Read this with:

- [`FRIENDMESHOS_ROADMAP.md`](FRIENDMESHOS_ROADMAP.md) for whole-project ordering.
- [`MESHTASTIC_CORE_AND_FRIENDMESH_ARCHITECTURE.md`](MESHTASTIC_CORE_AND_FRIENDMESH_ARCHITECTURE.md) for the underlying Meshtastic packet, routing, encryption, protobuf, and update boundaries.
- [`branding/TDECK_STYLING.md`](branding/TDECK_STYLING.md) for the six-theme styling contract.

If these documents conflict, this plan owns approved FriendMesh product behavior, the core architecture handbook owns Meshtastic behavior, and the live compiled source owns existing implementation facts. Resolve and document the conflict before coding.

---

## Master phase tracker

Status rules:

- Check a phase only after its build, design, automated-test, physical-test, recovery, documentation, and six-theme gates have evidence.
- `Implemented` does not mean `verified`.
- A partially completed phase stays unchecked and records its exact completed bullets in the session milestone log.
- No later phase may silently weaken a completed security or Meshtastic-compatibility gate.

- [x] Product requirements interview and final decisions completed.
- [x] Canonical FriendMesh game plan created before feature implementation.
- [ ] Phase 0 — Freeze contracts and warn the public.
- [ ] Phase 1 — Meshtastic compatibility and observability harness.
- [ ] Phase 2 — FriendMesh protobuf and signing identity.
- [ ] Phase 3 — Encrypted storage and transaction engine.
- [ ] Phase 4 — Group domain model and security status.
- [ ] Phase 5 — Nearby invitation, verification, and membership.
- [ ] Phase 6 — Rekey, leave, kick, replacement, succession, and disband.
- [ ] Phase 7 — Group chat and durable outbox.
- [ ] Phase 8 — Offline history synchronization.
- [ ] Phase 9 — Map, position privacy, and group filters.
- [ ] Phase 10 — Compass, navigation, and breadcrumbs.
- [ ] Phase 11 — Markers, meetups, and Emergency Rally Point.
- [ ] Phase 12 — SOS.
- [ ] Phase 13 — Help Requests.
- [ ] Phase 14 — SD-only Contacts and notification center.
- [ ] Phase 15 — Interoperability, protected channels, and upstream-update rehearsal.
- [ ] Phase 16 — Full security, reliability, and six-theme qualification.
- [ ] Phase 17 — Release qualification.

Current implementation focus: **Phase 2 FriendMesh protobuf and signing identity**. Phase 0 artifacts and Phase 1 compatibility/observability implementation remain unchecked because the working tree is uncommitted and the focused native suite, encrypted-air/PKI vectors, and controlled multihop evidence are still open. The operator explicitly deferred those remaining gates on 2026-07-12 and authorized Phase 2 work; the deferral is not a completion claim and the missing evidence remains mandatory before release qualification.

---

## 1. Product definition and non-negotiable rules

FriendMesh is a T-Deck-only, security-first social and field-coordination system built above Meshtastic. It is not a generic contacts skin and not a replacement mesh protocol.

### Required release scope

The first approved FriendMesh release is withheld until all required systems are implemented and qualified:

- Verified one-device identities.
- Up to eight FriendMesh application groups per T-Deck.
- Eight tested members per group.
- Secure nearby invitation and admin approval.
- One admin plus ordinary members.
- Signed membership and administrative events.
- Private FriendMesh-only group chat.
- Reactions, deletion tombstones, long-message fragmentation, durable drafts with SD, unread counts, and delivery states.
- Encrypted local history plus best-effort offline synchronization.
- Group map, filters, stale positions, marker details, and navigation.
- North-up/GPS-course compass fallback with optional magnetometer support.
- Meetup proposals, voting, attendance, Emergency Rally Point, and markers.
- SOS and non-emergency Help Requests.
- Membership removal, key rotation, offline rekey, admin succession, disbanding, and group erasure.
- Mandatory BLE/Meshtastic interoperability regression.
- Complete six-theme, touch, trackball, keyboard, error-state, and physical-device qualification.

Internal milestones and public source snapshots may be incomplete. No incomplete build is described as safe, production-ready, private, or suitable for emergency reliance.

### Compatibility boundary

FriendMeshOS continues to:

- Send and receive ordinary Meshtastic traffic in a separate `Meshtastic` area.
- Use the existing Meshtastic radio header, routing, hops, channel machinery, queue priorities, ACK/NAK behavior, NodeDB, position state, BLE transport, and phone compatibility.
- Allow normal Meshtastic nodes to relay FriendMesh frames without understanding their application payload.
- Use standard public text only as an explicit SOS/outside-help fallback.
- Keep FriendMesh-owned channels readable but protected from edit/delete through stock admin clients, with a deliberate advanced recovery unlock.

FriendMesh group chat and structured group events are not ordinary Meshtastic text. Every group participant needs FriendMeshOS to interpret them.

### Privacy boundary

FriendMesh can protect payload content but cannot hide that LoRa traffic exists. The radio header still exposes addressing/routing metadata. Privacy claims must never imply traffic-analysis resistance, guaranteed sender anonymity, guaranteed delivery, guaranteed remote erasure, or protection after a key is voluntarily shared.

Private FriendMesh groups are disabled in licensed/Ham mode because Meshtastic disables encryption there. Normal unencrypted Meshtastic remains available.

MQTT uplink and downlink are forced off for FriendMesh-owned channels. Any violation removes the `Secure` status and blocks group traffic until corrected.

---

## 2. T-Deck capability contract

Use one `t-deck-tft` build with runtime capability detection.

| Capability | Required behavior |
| --- | --- |
| TFT/touch/trackball/keyboard | Required for supported FriendMeshOS release |
| SX1262 LoRa | Required |
| GPS | Detect at runtime; map/navigation degrades clearly when absent |
| Magnetometer | Optional; absence never blocks progress or release of north-up navigation |
| microSD | Optional; expands history, drafts, contacts, notification center, maps, breadcrumbs, incident logs, and personal markers |
| Offline map tiles | Optional SD capability; arrow-only navigation remains available |
| BLE phone connection | Must remain compatible; no FriendMesh phone-app dependency |

Physical validation must record original T-Deck and T-Deck Plus results separately even when the same firmware image is used.

### Navigation fallback chain

1. Calibrated supported magnetometer: device-relative arrow labeled `MAG`.
2. Valid GPS course while moving: travel-relative arrow labeled `GPS`.
3. Otherwise: north-up destination bearing labeled `NORTH-UP`.

Magnetometer implementation stays a non-blocking later task until hardware is available. The interface and simulator tests are still built early.

---

## 3. Identity and trust architecture

### One device is one member

A FriendMesh member represents one verified device identity, not an abstract person. One human may join with several devices, but every device occupies a separate member slot and alias.

### Required identity fields

Each member record contains:

- Full Meshtastic node number.
- Meshtastic X25519 public-key fingerprint.
- FriendMesh Ed25519 signing public key and fingerprint.
- Group-specific alias, maximum 32 display characters.
- Join ordinal and join event ID.
- Role: `ADMIN` or `MEMBER`.
- Status: `PENDING`, `APPROVED`, `LEAVING`, `KICK_PENDING`, `REKEY_PENDING`, `REPLACED`, `BLOCKED_LOCAL`, `REMOVED`, or `DISBANDED`.
- Current membership epoch.
- Last authenticated activity.
- Emergency-contact permission for later Contacts support.

Alias input is trimmed, normalized deterministically, and compared case-insensitively for uniqueness. The storage encoding must cap UTF-8 bytes safely even when the display-character count is 32.

### Why FriendMesh needs a signing key

Meshtastic X25519 keys provide key agreement, not digital signatures. FriendMesh administrative events, messages, votes, deletions, alerts, and history records require an attributable signing identity.

Each device therefore creates a separate Ed25519 keypair:

- Private signing seed stored in protected essential storage.
- Public signing key included in the in-person verification transcript.
- Binding record ties node number, Meshtastic PKI fingerprint, and FriendMesh signing fingerprint together.
- A factory reset/device replacement creates a new identity requiring admin approval and in-person verification.
- Never derive signing keys from group PSKs or reuse X25519 private-key bytes.

Before implementation, validate the bundled Ed25519 primitive with published test vectors and review RNG, key storage, canonical encoding, and signature-malleability behavior.

### Device replacement

Replacement requires admin approval and a new in-person security-number verification. It preserves:

- Alias.
- Join date/order.
- Role.
- Historical messages.
- Reactions.
- Alerts and incident history.

The record gains an undeletable `DEVICE_REPLACED` administrative event. The old device identity is revoked and never automatically accepted again.

### Personal blocking

Blocking is local to one device:

- Future chat from the blocked identity is hidden.
- Member disappears from personal rosters, maps, target selection, and ordinary notifications.
- Historical messages remain with a `Blocked` indicator.
- Emergency policy must offer a separate explicit setting; local blocking must not silently suppress an active SOS without warning.
- Blocking does not remove the person from the group and does not rotate the PSK.

---

## 4. Group model

### Limits

- Maximum eight FriendMesh application groups configured per device.
- Maximum eight tested approved members per group in release one.
- Persistence schema reserves room for a future 16/32-member version without changing existing field meanings.
- All FriendMesh groups share one reserved Meshtastic secondary carrier channel, consuming one slot total.
- The carrier channel is transport only. Its outer PSK is not a group confidentiality or authorization boundary; every group payload requires independent authenticated encryption plus signed group/epoch authorization.
- Creating or joining additional groups does not allocate additional Meshtastic slots. Initial carrier setup fails safely if no slot exists and never overwrites an ordinary channel.

### Group fields

- Random 128-bit group ID unrelated to name or PSK.
- Display name.
- Shared FriendMesh carrier reference plus an opaque group dispatch tag that does not expose the raw group ID.
- Current epoch and random 32-byte channel PSK.
- Admin signing identity.
- Ordered membership records.
- Activity timestamp.
- Creation timestamp.
- Location-sharing policy: precise or hidden.
- One optional active impromptu meetup.
- One optional Emergency Rally Point.
- Security state.
- Admin-event sequence and previous-event hash.
- Pending transactions and tombstones.

There is no group icon, group color, description, guest role, observer role, or announcement mode in release one.

### Group inactivity

- Any authenticated chat, position, membership, marker, meetup, vote, help, or alert event counts as activity.
- After 90 days without activity, expiration begins with visible warnings.
- Expiration uses the same safe transaction principles as disbanding and must not silently delete an active group after a clock correction.

### Admin model

- Creator is initial admin.
- Exactly one active admin exists per membership epoch.
- Admin can rename group, set location policy, approve joins, remove members, rotate keys, set Emergency Rally Point, cancel their own/admin-permitted group actions, transfer administration, and disband.
- Any member may open an invitation window or propose an impromptu meetup.
- Members control only their own group alias.

### Admin transfer and succession

Normal transfer is signed by the current admin and accepted by the new admin.

After ten days without authenticated admin activity:

- Succession becomes available.
- Earliest-joined eligible member is default candidate.
- More than half of the full current membership must sign approval.
- Certificate includes group ID, current membership epoch, candidate identity, prior admin identity, vote event IDs, and expiry.
- No two disjoint partitions can both satisfy a strict majority of the same fixed membership.
- Successful succession advances the administrative epoch.
- Returning former admin becomes an ordinary member; restoration requires a deliberate transfer.

If only the admin remains, a 30-day self-deletion timer begins only after that state is locally confirmed. Warn at 7 days, 3 days, 24 hours, and 1 hour.

---

## 5. FriendMesh wire protocol

### Application transport

Use Meshtastic `PRIVATE_APP` (`256`) initially with:

- Fixed FriendMesh magic bytes.
- Protocol major/minor version.
- Compact Nanopb `FriendMeshEnvelope`.
- One managed Meshtastic secondary carrier channel shared by all FriendMesh groups. Its outer channel encryption is defense in depth and transport compatibility, not group isolation.
- Independent authenticated encryption for every group epoch using a random 32-byte group key; a carrier participant outside that group cannot decrypt or authorize its events.
- MQTT disabled.
- Sender signature over the canonical envelope content.

Before release, evaluate requesting a registered upstream port number. Until then, magic/version prevents accidental interpretation of other private applications.

### Envelope header

Every FriendMesh application event includes:

- Protocol version.
- Group ID.
- Group membership/key epoch.
- Event type.
- Event ID.
- Sender member/device ID.
- Sender sequence number.
- Sender-reported timestamp and time-quality flags.
- Expiration/TTL where applicable.
- Fragment metadata where applicable.
- Payload hash.
- Ed25519 signature.

### Event families

1. `CHAT`: message, reaction, deletion tombstone, long-message fragments.
2. `MEMBERSHIP`: join request, approval, alias change, replacement, leave, kick, role transfer, succession vote/certificate.
3. `KEY`: rekey prepare, member grant, commit, receipt, retired-key observation.
4. `SYNC`: inventory summary, range request, event batch, receipt, gap report.
5. `POSITION`: FriendMesh position/freshness metadata referencing standard position state.
6. `MARKER`: create, update, reconfirm, remove.
7. `MEETUP`: proposal, vote, accepted, attendance, arrived, canceled, expired.
8. `HELP`: create, response, escalation vote, state, close.
9. `SOS`: create, delivered, responding, unable, arrived, beacon, close, public-fallback decision.
10. `CONTROL`: disband, erase vote, erase commit, expiration, security status.

### Administrative event chain

Admin-authorized state changes use a strictly ordered chain:

- Monotonic admin sequence.
- Previous administrative event hash.
- Membership epoch.
- Admin signature or valid quorum certificate.

Reject stale epochs, broken chains, invalid signatures, unauthorized actions, and replayed event IDs. Store rejected-event diagnostics without exposing keys or plaintext unnecessarily.

Chat/events can arrive concurrently and do not need one global total order. Order them using sender sequence, trustworthy receive/sync metadata, and display-time rules.

### Fragmentation

Long messages are split at the FriendMesh layer:

- UTF-8-safe boundaries.
- Shared event ID.
- Fragment index/count.
- Whole-message hash and length.
- Per-fragment integrity/authentication through the signed envelope design.
- Bounded maximum total message size and fragment count established by airtime tests.
- Partial messages display `Incomplete message` and can request missing fragments.
- Expire unsent ordinary fragments after 24 hours.

Do not monopolize the mesh with arbitrary long text. Establish an explicit character/byte ceiling after LongFast and slower-preset testing.

---

## 6. Key epochs, removal, rekey, and retired-key audit

### Group key requirements

- Exactly 32 random bytes from a reviewed CSPRNG.
- Never derived from name, invitation code, PIN, member list, or public PSK.
- Never shown in logs, exports, screenshots, group metadata, or diagnostics.
- Never sent through public group broadcasts.
- Delivered only in a member-specific encrypted grant tied to the member's verified key.

### Leave and kick state machine

```text
ACTIVE
  -> LEAVE_PENDING or KICK_PENDING
  -> one-hour countdown; compliant device is muted
  -> CANCELLED (undo before expiry)
  -> REKEY_PREPARING
  -> REKEY_DISTRIBUTING
  -> REKEY_COMMITTED
  -> removed device REMOVED
  -> offline remaining devices REKEY_PENDING until receipt
```

- Voluntary leave waits for admin or an authorized replicated coordinator.
- Admin kick follows the same one-hour reversible window.
- Once rekey begins, no undo is allowed.
- Removed device never receives the new key.
- A device still technically possesses the old key during the countdown; UI muting is not cryptographic revocation.

### Forward-only transaction recovery

Use a durable transaction journal:

- Before any new key grant is distributed, a failed prepare may safely abort.
- Once any new-key grant is distributed, recovery resumes forward and never reverts to the compromised old key.
- Commit only after the new group record and member grants are durably journaled.
- Offline members show `REKEY PENDING` and cannot read new group traffic until they receive and acknowledge the new epoch.

### Offline rekey envelopes

The admin creates one sealed, admin-signed grant per remaining member:

- Bound to group ID, new epoch, member identity, member public-key fingerprint, and expiration policy.
- Encrypted only for that target device.
- Safe ciphertext replicas retained by remaining FriendMeshOS members.
- Retained until confirmed receipt or later membership change invalidates it.
- Never resent if the target public key changes.
- Admin has a visible `Resend key` action.

Perform a dedicated cryptographic design review before selecting the sealed-envelope KDF/AEAD construction. Prefer vetted primitives already available in the firmware and include deterministic interoperability vectors.

### Retired-key audit

- Admin may retain retired group keys for 30 days in an encrypted audit vault.
- Retired keys are never selected for transmission.
- Candidate frames may be decoded only when channel hash/magic suggests a match.
- A valid FriendMesh signature can attribute the event to a stored device signing key, but header identity remains spoofable and replay remains possible.
- Display: `Traffic using a retired group key was detected` plus confidence and signature result.
- Never claim a physical attacker was located or definitively identified.
- Bound CPU/storage: cap retained epochs and prune at 30 days.

---

## 7. Storage and transactional integrity

### Storage tiers

Without SD:

- Essential group/member/key/transaction state in internal persistent storage.
- Forty encrypted event records per group retained for sync duties.
- Only newest 15 chat messages displayed locally.
- No durable drafts, Contacts, notification center, personal markers, full breadcrumbs, or permanent incident log.

With SD:

- Up to 1,000 events/messages per group.
- Durable drafts.
- Notification center.
- Verified Contacts.
- Offline map tiles.
- Navigation breadcrumbs.
- Personal markers.
- SOS incident logs.
- Human-readable explicit exports.

### Per-device storage encryption

- Generate a per-device storage master key.
- Protect it using the screen PIN and a reviewed embedded-appropriate KDF/wrapping design.
- Derive separate subkeys for group history, key audit, contacts, drafts, notifications, and transaction journals.
- Use AEAD for stored records; AES-CTR alone is insufficient.
- Nonce reuse is forbidden and tested across reboot/power-loss recovery.
- PIN reset without the recovery material may intentionally make old encrypted history unrecoverable.
- Document that at-rest protection without ESP32 flash encryption/secure boot is resistance to casual extraction, not a hardware-backed security guarantee.

### Files and journals

Proposed SD structure:

```text
/friendmesh/
  device.meta
  groups/<group-id>/
    state.snapshot
    admin.journal
    events.journal
    events.index
    outbox.journal
    sync.state
    incidents/
    breadcrumbs/
  contacts/
  notifications/
  exports/
  audit/
```

Use append-only authenticated records, checksummed/AEAD snapshots, atomic temporary-file replacement, monotonic schema versions, bounded compaction, and last-known-good recovery.

### Failure policy

- SOS and Help Requests transmit even when they cannot be stored; warn that history persistence failed.
- Ordinary chat does not transmit unless its durable outbox record succeeds.
- Membership, key rotation, kick, disband, erase, and security events transmit only after the transaction journal is durable.
- On SD removal/full/failure, fall back to internal capacity where possible.
- Corrupt nonessential group metadata may be quarantined/deleted rather than supported by a large repair engine.
- Essential membership/key state must keep a last-known-good transactional copy.

---

## 8. History and offline synchronization

### Reality and scope

- Release-one sync occurs only among FriendMeshOS T-Decks.
- Ordinary Meshtastic nodes relay live frames but do not retain FriendMesh history.
- A future Linux history keeper is optional later infrastructure.
- No reachable replica means history is incomplete, not failed.

### Retention

- Internal: 40 total records per group for replication, newest 15 chat entries visible.
- SD: 1,000 records per group.
- Newly approved members may receive pre-join history after the inviter/admin sees a prominent disclosure warning.
- Removed members cannot request events beyond their removal epoch.
- Public Meshtastic channels receive no FriendMesh persistent-history feature.

### Event identity and deduplication

Use stable signed event IDs and per-sender monotonic sequences. Maintain compact per-member high-water marks plus bounded missing ranges. Identical IDs deduplicate silently. Conflicting content for one ID is quarantined and shown only in diagnostics.

### Sync protocol

1. Detect a returning approved member.
2. Exchange compact inventory/high-water summary.
3. Compute missing ranges without enumerating 1,000 IDs.
4. Respect channel utilization; sync only when low enough.
5. Send small bounded batches with resumable receipts.
6. Apply signature, epoch, membership, duplicate, expiration, and deletion checks.
7. Persist before acknowledging.
8. Continue newest/highest-priority first.

Priority order:

1. Active SOS.
2. Active Help Requests.
3. Membership and key events.
4. Meetup and marker events.
5. Newest chat backward.
6. Reactions and deletion tombstones.

UI status: `History incomplete — waiting for another member`, progress count, pause reason, and channel-utilization wait. Sync must never block live chat, routing ACKs, SOS, or user navigation.

### Clock behavior

- Preserve original sender timestamp.
- Preserve local receive timestamp and sync receipt time.
- Display delayed duration for queued messages.
- Reorder conversation using trusted metadata without silently rewriting the original timestamp.
- Mark `Sender clock appears incorrect` when skew is detected.
- Long-press details show original and adjusted display time.
- Correct local clock only from trusted GPS/phone time.

---

## 9. Group chat product design

### Layout

```text
──────── DATE ────────

Alias · time
[Received message bubble aligned left]

                         You · time
          [Sent message bubble aligned right]
```

Visible per ordinary message:

- Sender alias.
- Timestamp.
- Delivery state.
- Secure lock/group status.
- Thumbs-up/down counts when present.

Long-press details:

- Event ID.
- Queued/transmitting/relayed/delivered/timed-out/failed state.
- ACK information.
- Observed hops and age.
- RSSI/SNR when available.
- Encryption/channel epoch.
- Original and adjusted timestamps.
- Delayed duration.
- Fragment/reassembly state.

### Operations

- No message editing.
- No reply/quote threads.
- No attachments.
- No disappearing messages.
- No search.
- No pins/announcements.
- Reactions: thumbs up/down only; one current reaction per member per message.
- Sender deletes own ordinary message.
- Admin can delete any ordinary message.
- Deletion is a signed best-effort tombstone and displays `Message deleted`.
- Join/leave/kick/rekey/admin/SOS/meetup/security system records are undeletable while group exists.

### Drafts and unread state

- SD present: draft survives reboot per group.
- No SD: draft is volatile.
- Unread count per group.
- Muted groups suppress ordinary sound but never SOS.
- FriendMesh home loads active alerts and group unread state before public Meshtastic views.
- Screen lock/PIN protects access to FriendMesh storage, but approved product behavior does not hide message text in locked-screen notifications.
- Screenshots remain available on FriendMesh screens without a special sensitive-screen warning.

### Queue policy

- Normal FriendMesh chat uses standard text-message radio priority.
- Elevated priority only for SOS, Help, key rotation, and critical control.
- Ordinary chat, reactions, and deletion events expire after 24 hours.
- Delayed sent message shows original composition time plus `Delayed X`.

---

## 10. T-Deck information architecture

### Boot home

Primary tiles:

1. `Meshtastic` — ordinary upstream-compatible features.
2. `FriendMesh` — private groups and later Contacts.
3. `Map` — all known nodes by default with filters.
4. `SOS` — fixed emergency entry.

Persistent top bar:

- Active area/group.
- Mesh/LoRa state.
- GPS state.
- Security/rekey warning.
- Unread count.
- Active SOS/Help indicator.
- SD status when a dependent feature is open.

### FriendMesh group chooser

- Tiled list of up to eight groups with bounded paging/scrolling on 320x240.
- Each tile: group name, unread count, reachable/total members, sync state, security state, and active alert badge.
- Empty tile: Create or Join.
- One selected FriendMesh group at a time.

### Group dashboard

Required destinations:

- Chat.
- Map.
- Members.
- Navigate.
- Meetup.
- Alerts/Help.
- Markers.
- Security/Sync/More.

Switching groups changes chat, group map filter, compass candidates, meetup, alert context, and group dashboard state. Map zoom may persist per group; center resets to the user's current location.

### Input behavior

- Every action works by touch, trackball, and keyboard.
- Focus always visible.
- Back/cancel path on every non-emergency screen.
- Destructive actions use two-step confirmation.
- Pending destructive actions expose visible undo until irreversible commit.
- No decorative animations; use immediate state changes, restrained progress indicators, and clear banners.

---

## 11. Invitation and joining UX

### Open invitation

Any member can open an invitation screen:

- Shows six-character alphanumeric code.
- Remains active while inviter keeps the screen open.
- Multiple nearby candidates may submit.
- Closing stops new submissions.
- Already-submitted requests remain pending.

### Candidate flow

1. Select `Join nearby group`.
2. Enter code.
3. Enter unique group alias.
4. Review precise-or-hidden location policy and pre-join history disclosure.
5. Submit node number, PKI key, signing key, alias, and proof-of-possession challenge.
6. Wait for admin review.

### Admin flow

- See pending candidate identity and inviter.
- Detect alias conflict and full group/channel limit.
- Compare in-person verification/security number.
- Approve or reject.
- Approval creates signed membership event and member-specific key grant.
- Chat receives `Alias joined the group`.

No unverified member can enter group state or receive the group key.

---

## 12. Map, positions, and member details

### Map defaults and filters

Global map defaults to all known Meshtastic nodes. Fast filters:

- Selected group.
- All FriendMesh group members.
- All known nodes.
- Alerts only.
- Meetup/navigation.

Filters reset after reboot. Group zoom persists; map center returns to local current location.

### Marker display

- Basic marker until tapped.
- Fixed identity colors independent of theme.
- Multi-group member uses multiple badges/rings without duplicating the physical node.
- Stale after five minutes and rendered faded.
- Last-known location retained indefinitely until replaced/deleted by group lifecycle.

Tapped member card:

- Alias and underlying node identity.
- Group badges.
- Distance.
- Last observed hops and age.
- Position age/source.
- Last-heard age.
- Alert status.
- GPS accuracy.
- `Navigate` button.

Location can be precise or hidden. Manual member-position entry is not supported. Clearly distinguish onboard GPS and phone-provided position.

### No map tiles

If tiles are missing:

- Show arrow-only navigation.
- Display distance, bearing, position age, hops, accuracy, and last-known status.
- Never block navigation because map storage is absent.

---

## 13. Person navigation and breadcrumbs

### Target selection

Targets can be opened from map, group member list, active alert, recent target, and later Contacts. Selecting a target opens details first; navigation starts only after pressing `Navigate`.

### Map mode

- User and destination markers.
- Route-independent straight-line guidance.
- Distance and GPS accuracy visible.
- Stale target banner is prominent but dismissible.
- Getting closer/farther trend.
- Breadcrumb line where storage permits.

### Arrow-only mode

- Large destination arrow.
- Distance in meters/kilometers.
- Bearing degrees/cardinal direction.
- Position age.
- Last-heard age if different.
- Hops plus observation age.
- GPS accuracy.
- Heading source (`MAG`, `GPS`, `NORTH-UP`).
- Signal information.
- Target alert state.

Update calculations every five seconds while navigating, with faster UI refresh only where sensor data supports it without excess power.

### Arrival

- Arrival radius is `max(10 m, reported GPS accuracy)`.
- Do not auto-stop navigation.
- Offer `I'm here` when inside the radius.
- Trip ends only on manual completion or cancellation.

### Breadcrumb sharing

- Breadcrumbs are private to navigator.
- When destination is another group device, the destination may receive the active navigator trail.
- Stop and erase/close active breadcrumb sharing when trip completes or is canceled.
- No indefinite breadcrumb retention.

Starting navigation to a member creates a notification for that destination.

---

## 14. Meetup and marker system

### Meetup types

1. One active impromptu meetup.
2. One persistent `Emergency Rally Point` controlled by admin.
3. Private `Return to start` point stored locally.

Meetup coordinates come only from the proposer's current valid position.

### Impromptu meetup proposal

- Any member proposes.
- Proposer automatically votes yes.
- Voting window chosen from 2-10 minutes.
- Every current member is eligible; nonresponses abstain.
- Without admin approval, yes votes must exceed 50% of full membership.
- Admin approval accepts immediately unless at least 75% of full membership votes no before deadline.
- Admin has one vote in denominators.
- Ties reject.

Proposal appears as a chat event with Yes/No controls, proposer, point, optional description, and countdown.

### Attendance and lifetime

- Members mark attending/declined.
- Group sees who is navigating there.
- Arrival uses navigation accuracy radius plus manual confirmation.
- Impromptu meetup expires after all attending members arrive or after one hour.
- Creator or admin may cancel/move it.
- Move/cancel uses prominent confirmation and creates group system message.

### Marker types

- Meetup.
- Danger.
- Avoid.
- Resource.
- Vehicle.
- Camp.
- Last seen.
- Pickup.
- Help.
- SOS.

Ordinary non-sensitive markers may use standard Meshtastic waypoints. Meetup, Help, SOS, and control markers remain FriendMesh-only.

Marker rules:

- Group markers are signed and recorded in chat history.
- Personal markers require SD and do not enter group history.
- Only creator edits a normal marker; admin removal rules must be explicit in UI.
- One-button navigation.
- Detail includes description, distance, creator, time, and members within 100 m based on GPS.
- Danger marker gives prominent notification, not immediate alarm sound.

Reconfirmation:

- Danger: 24 hours.
- Avoid: 7 days.
- Resource/vehicle/camp/pickup: 30 days.
- Last seen: expires after 24 hours unless renewed.
- SOS/Help: incident lifetime.
- Emergency Rally Point: never expires until admin changes it.

---

## 15. SOS state machine and design

### Activation

- Dedicated home action.
- Five-second intentional hold.
- Then five-second cancel countdown.
- Countdown includes `Include location publicly` toggle, off by default.
- One deduplicated incident is sent across every current FriendMesh group plus verified Contacts that explicitly accepted emergency alerts; recipients appearing through several paths must render it once.
- Group/verified emergency-contact payload receives current exact or labeled last-known coordinates.
- Public fallback omits coordinates unless explicitly approved.

### States

```text
IDLE
 -> HOLDING
 -> CANCEL_COUNTDOWN
 -> ACTIVE/SENDING
 -> DELIVERED (per recipient)
 -> RESPONDING / UNABLE / ARRIVED (per recipient)
 -> CLOSED: FALSE_ALARM | SAFE | HELP_ARRIVED
```

- Originator alone normally closes SOS.
- `ARRIVED` is manually confirmed.
- Aggressive retries stop after three distinct `DELIVERED` acknowledgments.
- Low-rate beacons continue while active.
- Retry cadence is selected from airtime research and physical testing, uses `Throttle`, respects duty cycle, and uses one deduplicated incident ID.

### Recipient experience

SOS overrides mute and quiet hours:

- Sound.
- Vibration/haptic where available.
- Flashing/emergency screen treatment.
- Persistent notification.
- Fixed red/white/black visual language in every theme.

Actions:

- Navigate to sender.
- Mark responding.
- Mark unable.
- Send group message.
- Mark arrived.
- Call outside help.

`Call outside help` cannot call a phone. It presents a second explicit confirmation, then sends a public Meshtastic SOS fallback with coordinates. It must state that this broadcasts location and does not contact emergency services automatically.

### Missing GPS and persistence

- Use last-known location with stale age warning.
- If no location exists, send identity/status without coordinates.
- Transmit even when storage fails; warn that incident could not be logged.
- SD present: preserve incident log.
- No SD: do not promise permanent incident history.

### Safety language

Every SOS screen and manual states:

- Best-effort peer assistance only.
- Not guaranteed delivery.
- Not emergency-service dispatch.
- Coordinates may be stale or inaccurate.
- Radio range, interference, power, and offline devices can prevent receipt.

---

## 16. Help Request system

### Categories

- Need a ride.
- Uncomfortable situation.
- Lost/separated/no GPS.
- Equipment problem.
- Medical but not emergency.
- Need someone to call me.
- Discreet extraction.
- Free text.

Help is urgent but visually below SOS. Default device notification behavior applies unless sender explicitly requests silent delivery. Discreet extraction uses neutral locked-screen wording: `A contact requested a check-in`.

### Responses

- I'm coming.
- I can call.
- I can relay.
- Unable.
- Need more information.

The whole group sees `Unable`. Multiple people may volunteer; there is no lead responder. Responders share location while responding and stop when closed.

### Escalation

- Any verified responder may request escalation.
- Escalation requires requester, admin, or two verified responders.
- If requester cannot act, two responders can escalate.
- SOS lists the person needing help and the escalation initiator(s).
- Requester normally closes.
- Admin or two responders may resolve when requester cannot act.
- De-escalation uses requester, admin, or two responders.

After closure, display `Handled` for three hours, then remove from active UI while retaining history according to storage policy.

Help transmits even when storage fails, with persistence warning.

---

## 17. Group erasure, disbanding, and expiration

### Best-effort total group-history erasure

- Vote starts only if at least 75% of members are reachable.
- Five-minute vote.
- Every current member has one vote; admin has one ordinary vote.
- At least 75% of full membership, rounded up, must vote yes.
- Admin cannot force or block a successful threshold result.
- Commit is signed/quorum-certified.
- Connected compliant devices erase chat, markers, breadcrumbs, alerts, meetups, audit history, and group content.
- Offline compliant devices erase when they receive the tombstone.
- Cannot guarantee deletion from malicious devices, screenshots, or exports.

### Disbanding

1. Admin performs two-step confirmation.
2. One-hour countdown visible to group.
3. Admin may undo during countdown.
4. Commit becomes irreversible.
5. Connected members erase group keys and state.
6. Replicated disband tombstone waits for offline members.
7. Group cannot be restored; recreate it.

### System records

Join, leave, kick, rekey, admin, SOS, and meetup control records are undeletable while group exists. Successful total erasure/disband removes them from compliant devices as specified above.

---

## 18. Later SD-only Contacts

Contacts are later but part of the complete planned release if required by final SOS/navigation behavior.

- Requires SD.
- Independent of group membership.
- One verified device per contact entry.
- Can belong to zero, one, or multiple groups.
- Supports PKI direct messages and person navigation without group history exposure.
- Final shared-group departure does not remove an explicitly saved contact.
- `Emergency alerts` permission is explicitly accepted during verification.
- SOS goes only to verified contacts with that permission and that are reachable through the mesh.

No contact record bypasses public-key change verification.

---

## 19. Security status and diagnostics

### Group security states

- `SECURE`: random 32-byte PSK, MQTT off, valid current epoch, all members verified, no transaction fault.
- `REKEY PENDING`: some approved members lack current epoch.
- `KEY CHANGE`: member identity changed and requires verification.
- `SYNC INCOMPLETE`: history not fully available.
- `STORAGE DEGRADED`: persistence below requested tier.
- `UNSAFE CONFIG`: protected channel changed/unlocked/MQTT enabled.
- `HAM MODE`: FriendMesh private group disabled.

### Security screen

- Group ID and epoch.
- Channel slot.
- Admin fingerprint.
- Member signing and Meshtastic PKI fingerprints.
- Verification/rekey status.
- MQTT forced-off state.
- Pending destructive transactions.
- Retired-key audit state and expiry.
- Advanced channel recovery unlock.

Never display PSK bytes.

### Diagnostic privacy

- Exact coordinates redacted by default.
- Temporary explicit `Include precise location` diagnostic option.
- Keys/private message bodies excluded.
- Human-readable transcript export is explicit and warns that it includes locations and public-key fingerprints.
- Export is plaintext by approved product choice, requires no extra export PIN, and is not restorable.

---

## 20. Stock client protection and recovery

- The single FriendMesh carrier slot remains present in ordinary channel configuration.
- Stock clients may read metadata needed for compatibility.
- Edit/delete attempts receive a clear protected-channel error.
- BLE synchronization and all unrelated Meshtastic configuration continue working.
- Advanced FriendMesh recovery screen can temporarily unlock a channel after PIN and two-step warning.
- Unlock displays that security guarantees and local metadata can break.
- Reconciliation flow detects stock-client changes and offers repair, rebind, or explicit destructive removal.

Test with current supported Meshtastic phone clients before every release. FriendMesh must not depend on modifying those apps.

---

## 21. Six-theme design system

### Universal layout contract

All six themes use identical information architecture, controls, state names, ordering, touch targets, keyboard paths, trackball focus order, and destructive confirmations. Themes may vary palette and approved geometry only.

No animations. State changes use immediate redraw, progress indicators, banners, and standard icons.

### Semantic tokens FriendMesh requires

- Screen/background/surface/card/border.
- Primary/secondary/muted/disabled text.
- Accent/focus/pressed/selected.
- Secure/rekey/sync/storage states.
- Incoming/outgoing/system message bubbles.
- Delivered/queued/failed.
- Map marker/route/stale/accuracy ring.
- Meetup/vote/attendance.
- Help Request.
- Fixed SOS red/white/black overlay.
- Destructive pending/undo/committed.

Marker identity colors are fixed across themes and paired with initials/shapes. SOS is fixed across themes. Alert meaning never relies on color alone.

### Per-theme intent

- Clean Modern: cyan focus, restrained cards, fastest scanning.
- Retro Terminal: square geometry and phosphor palette; SOS/error remains distinct.
- Neobrutalist: heavy borders and hard shadows without shrinking touch targets.
- Orbital Mission: telemetry framing without confusing normal orange/red with alerts.
- Alpine Daylight: outdoor readability and strong disabled/stale differentiation.
- Friendly Mesh: rounded approachable surfaces without reducing seriousness of security/emergency states.

### Mandatory regression matrix

Every FriendMesh screen in every theme must capture and verify:

- Normal.
- Empty.
- Loading/syncing.
- Focused.
- Pressed.
- Disabled.
- Warning.
- Error.
- Stale position.
- SOS.
- Help.
- Pending destructive action and undo.
- Touch, trackball, and keyboard navigation.

---

## 22. Implementation phases

Every phase includes architecture, T-Deck UX/design, all-six-theme acceptance, automated tests, physical tests, rollback/recovery behavior, and documentation updates. A checked implementation item without its phase gate is not complete.

### Phase 0 — Freeze contracts and warn the public

Build:

- Commit this plan, core handbook, roadmap links, and terminology glossary.
- Add prominent incomplete/unsafe README warning.
- Create requirements decision log and unresolved-risk register.
- Define release-one feature list and “not safe yet” status.
- Add upstream Meshtastic delta ledger template.

Design:

- Produce low-fidelity 320×240 wireframes for every screen/state.
- Define global navigation, focus order, dialog patterns, undo banner, status chips, and SOS overlay.
- Map every screen through all six themes before implementation.

Gate:

- No contradictory requirements across the three canonical documents.
- `git diff --check` and Markdown-link audit pass.
- README cannot be mistaken for a finished emergency/security product.

Working-tree evidence, 2026-07-12:

- Added `.github/copilot-instructions.md` as the canonical verified repository/FriendMesh agent contract.
- Added decision log, unresolved-risk register, canonical glossary, upstream-delta ledger, and pinned-baseline manifest under `docs/friendmesh/`.
- Froze 320x240 reusable patterns and traceable wireframes for all required product areas, plus six-theme and three-input state matrices.
- Reconciled the architecture handbook's obsolete channel-URL-only admission flow to the approved signed nearby invitation/admin approval/member grant flow.
- `git diff --check` and an 11-file local Markdown-link audit passed before Phase 1 coding.
- Phase remains unchecked because these working-tree artifacts are not yet committed; no commit was authorized in this session.

### Phase 1 — Meshtastic compatibility and observability harness

Build:

- Add read-only packet/event diagnostics for port, channel, destination type, ACK/NAK, hops, timestamps, queue state, and position freshness.
- Add upstream compatibility fixtures for public text, channel text, PKI DM, position, NodeInfo, telemetry, traceroute, BLE config, and protected-channel handling.
- Capture baseline packet/protobuf vectors from `v2.7.26`.
- Add T-Deck capability service for GPS, magnetometer, SD, maps, input, and LoRa state.

Design:

- Developer diagnostics screen uses semantic tables and never exposes secrets.
- Capability badges use identical placement in all themes.
- Missing capability states explain degradation, not failure.

Tests/gate:

- Native unit tests for hop/time/position helpers.
- Two-node stock interoperability passes.
- BLE phone sync passes.
- All six themes render diagnostics/missing-capability screens.

Current implementation evidence, 2026-07-12:

- Added a secret-free read-only observer for destination type, channel, decoded/encrypted state, port, ACK/NAK and error enum, existing Meshtastic hop calculation, receive age/clock skew, transport, queue state, and position/last-heard freshness.
- Added a bounded T-Deck capability-state service for GPS, magnetometer, SD, maps, touch, trackball, keyboard, and LoRa with independent unavailable/not-detected/degraded/ready states.
- Added native test cases for time, hops, packet metadata, ACK/NAK, queue/freshness, and capability degradation.
- Added ten secret-free fixed protobuf vectors covering public/channel text, position, NodeInfo, telemetry, traceroute, ACK/NAK, and BLE config request/completion boundaries, plus an independent host checker against the checked-in Nanopb bindings.
- Added a fixed-capacity, chronological diagnostic event store for packet, queue, and position metadata; overflow evicts the oldest record and never retains payload text, keys, or coordinates.
- Connected decoded packet and queue-status callbacks already delivered through the device UI controller to that store without changing send, routing, radio, or channel behavior.
- Added a T-Deck-only Tools → FriendMesh Diagnostics view with runtime capability states and the eight newest redacted packet/queue/position records; it is created dynamically to avoid modifying generated EEZ screen files and uses existing semantic panel/button styles and focus handling.
- Added one-second automatic D-01/D-02 refresh while either diagnostics view is open, plus a rollover-safe 60-second position-freshness probe that records only state/source/skew transitions unless a new position arrives.
- Added D-02 semantic event details for port, destination, payload state, transport, hops, ACK/NAK, exact routing error enum, queue state, position source, and freshness. Node IDs, message bodies, keys, and coordinates are never retained or displayed.
- Added a non-destructive Clear view boundary and a two-step SD-only plaintext export of every retained event, including records hidden by Clear view. The RAM store remains bounded to the newest 16 events. Exports are explicitly non-restorable and redact coordinates and node IDs while excluding message bodies and keys.
- Verified the official firmware endpoint `54e0d8d0ab2ff56b3a9ce967e53f79e49af560fb` and protobuf endpoint `6b1ded439633cd03d4af85b44231b91d1d106278`; five critical local schemas match the official protobuf commit byte-for-byte by SHA-256.
- `pio run -e t-deck-tft` passed twice after implementation; inherited warnings remain.
- Native execution is environment-blocked before the FriendMesh suite by missing Portduino host prerequisites (`pkg-config`/`argp.h`). PKI/encrypted-air vectors, BLE/multihop physical tests, and six-theme/three-input evidence remain open, so Phase 1 stays unchecked.
- Physical T-Deck testing confirmed an ordinary private Meshtastic channel can exchange text with a stock Meshtastic device. An initial direct PKI message reached the destination but returned `Routing_Error_NO_CHANNEL` (`6`), isolating stale/mismatched cached peer keys. After removing only the two peer NodeDB records and allowing fresh NodeInfo exchange, FriendMeshOS-to-stock PKI DM returned ACK/error `0`, stock-to-FriendMeshOS logged `PKI Decryption worked!`, rendered the text, and completed the reliable ACK exchange. This is a bidirectional PKI interoperability pass for the tested pair without factory reset or key regeneration.
- The first D-01 diagnostics screen opened physically, but its compact abbreviations were not self-explanatory to the operator. D-02 now supplies semantic detail and exact error names; this replacement build still requires physical D-01/D-02 input/theme/export verification.
- The stock map rendered, but synchronous Google fallback-tile retrieval caused unacceptable UI stalls. FriendMeshOS now compiles with online fallback tiles disabled while retaining SD/offline map rendering; later re-enablement requires asynchronous bounded fetching.
- The full FriendMeshOS splash is embedded as a compressed application asset so normal firmware upload updates branding without an `uploadfs` operation or saved-config changes. Physical verification of the replacement build remains pending.
- Operator verification passed for the application-embedded FriendMeshOS `v0.2.1` splash, nonblocking offline-only missing-tile map behavior, Signal Scanner/position request, and Trace Route on the tested FriendMeshOS/stock pair. The replacement build retained ordinary private-channel and bidirectional PKI interoperability.
- Operator verification passed for D-02 older/newer navigation, non-destructive Clear view with all retained events still exported, all six themes, and touch/trackball/keyboard navigation. The inspected SD export contained all seven retained events and no coordinates, node IDs, message bodies, or keys.
- BLE phone connection and configuration passed. That test exposed the startup splash being reused above the reboot chooser; the replacement build now hides every startup-only logo/version object before showing a separate dark FriendMeshOS device-controls screen with Restart, Pairing mode, Power off, and cancel guidance. Operator recheck confirmed the startup splash, chooser visibility, labels, focus/navigation, cancel, and device-control actions all work correctly.
- Public-channel text was received from a node reported 13 km away. This is useful range/interoperability evidence but is not classified as controlled multihop without observed hop metadata or a deliberately isolated relay path.

### Phase 2 — FriendMesh protobuf and signing identity

Build:

- [x] Define `friendmesh.proto` and Nanopb bounds for a single payload no larger than 233 bytes.
- [x] Implement magic/version dispatch on `PRIVATE_APP` without consuming unrelated private-app traffic.
- [x] Implement domain-separated canonical encoding and ESP32-S3 Ed25519 primitives with a fail-closed boot self-test.
- [x] Define and validate the signing-identity binding to the sender node and current Meshtastic X25519 public key.
- [x] Implement bounded RAM replay cache, sender sequences, event IDs, signature verification, timestamp bounds, unknown-version/type rejection, and deterministic host checks.
- [x] Implement bounded outbound canonical frame construction with injectable signing and fail-closed error results; production radio transmission remains disabled without a protected identity.
- [x] Define the authenticated-encrypted signing-identity storage interface and lifecycle states; reject plaintext/unauthenticated stores and wipe RAM key material on lock/destruction.
- [ ] Persist the signing seed and durable replay state through the reviewed Phase 3 encrypted-storage boundary.
- [ ] Connect the protected identity to the radio send path and prove two-FriendMesh-device exchange.

Design:

- Identity setup/verification screen.
- Fingerprint comparison optimized for in-person use.
- Key-change screen cannot be dismissed as ordinary warning.
- Theme-independent verified/pending/key-changed iconography.

Tests/gate:

- [x] Published RFC 8032 crypto vector passes independently; firmware embeds its own known-answer self-test.
- [x] Invalid-signature, replayed, stale-sequence, stale/future-time, wrong-sender, wrong-group, and wrong-epoch events are rejected by the host protocol checker.
- [x] Canonical frame round trips, unknown fields, unknown versions/types, maximum payload, and replay-capacity exhaustion are covered.
- [x] Run 150,000 deterministic random/mutated decoder cases under host undefined-behavior and bounds sanitizers; this found and fixed unsafe reads of unknown Nanopb enum values.
- [ ] Confirm the firmware crypto self-test and receiver-ready log on physical T-Deck hardware.
- [x] Confirm the previously working T-Deck/Meshtastic behavior remains unaffected after the Phase 2 application upload; operator reported it still works on 2026-07-12.

Implementation contract: [`docs/friendmesh/PHASE2_PROTOCOL_CONTRACT.md`](docs/friendmesh/PHASE2_PROTOCOL_CONTRACT.md). Phase 2 remains open until the identity UI/persistence, fuzzing, transmit exchange, physical regression, and theme/input gates pass.

### Phase 3 — Encrypted storage and transaction engine

Build:

- Device storage key, PIN protection, subkey derivation, AEAD records.
- Internal essential store and SD storage provider.
- Append-only journal, snapshots, index, compaction, schema migration, transaction recovery.
- Capacity accounting and fallback.
- Durable outbox and pending-event scheduler.

Design:

- Storage status/detail screen.
- Clear SD/no-SD feature comparison.
- Full/read-only/removed/corrupt states and cleanup actions.
- Security-sensitive PIN flows consistent across themes.

Tests/gate:

- Power cut at every transaction boundary.
- SD removal/full/corrupt/read-only.
- Nonce uniqueness across reboot.
- No plaintext secrets/history on disk.
- All six themes cover storage degraded/error/recovery states.

### Phase 4 — Group domain model and security status

Build:

- Group/member schemas, limits, alias rules, admin chain, epochs, status computation.
- Eight-group bounded domain store plus one-time shared-carrier allocator.
- Protected-channel ownership and MQTT enforcement.
- 90-day activity/expiration framework.
- Local blocking.

Design:

- Boot home, group chooser, empty/create/join tiles, dashboard, Members, Security.
- Clear `Secure`, `Rekey pending`, `Unsafe config`, and `Storage degraded` states.
- Group tiles validated at maximum name length and eight-group paged/scroll density.

Tests/gate:

- Invalid aliases, duplicate names, full group, full channel table, unknown members, stale epochs.
- No group keys in metadata/logs.
- Six-theme group shell matrix passes.

### Phase 5 — Nearby invitation, verification, and membership

Build:

- Invitation session and six-character code.
- Nearby request discovery/challenge.
- Candidate proof of signing/PKI key possession.
- Pending admin queue.
- In-person verification and approval.
- Member-specific initial key grants.
- Join and alias-change system events.

Design:

- Inviter live window.
- Candidate code/alias/privacy/history screens.
- Admin compare/approve/reject screen.
- Conflict, timeout, full-group, no-slot, and verification-failed states.

Tests/gate:

- Multiple simultaneous candidates.
- Invitation closes while requests pending.
- Spoofed code/request/key rejected.
- New member cannot receive group traffic before approval.
- All six themes and three inputs pass.

### Phase 6 — Rekey, leave, kick, replacement, succession, disband

Build:

- One-hour reversible leave/kick countdown.
- Muting during pending removal.
- Forward-only rekey journal.
- Sealed offline grants and receipt replication.
- Device replacement.
- Admin transfer and ten-day quorum succession.
- Retired-key audit vault/detector.
- One-hour disband and 90-day expiry.

Design:

- Timeline-based transaction screens.
- Undo always visible until irreversible boundary.
- Member status chips and `Resend key`.
- Retired-key observation uses cautious language.
- Admin succession voting and former-admin return state.

Tests/gate:

- Power loss at every rekey step.
- Offline members and changed keys.
- Admin disappears/returns.
- Network partition and competing succession attempts.
- Removed member cannot decrypt new epoch.
- Six-theme destructive/undo/error matrix passes.

### Phase 7 — Group chat and durable outbox

Build:

- Signed FriendMesh chat messages.
- Message bubbles, state machine, unread counts.
- Long-message fragmentation/reassembly.
- Thumbs up/down.
- Sender/admin tombstones.
- SD drafts.
- 24-hour queue and delayed timestamps.
- Mute behavior.

Design:

- Approved date/alias/time/bubble layout.
- Left/right alignment.
- Long-press diagnostics.
- Partial fragment, failed, delayed, deleted, blocked, and system rows.
- Keyboard composition and trackball message actions.

Tests/gate:

- Loss, duplicates, out-of-order fragments, reboot mid-send, queue full, storage failure.
- Stock clients do not display FriendMesh chat.
- Public Meshtastic remains functional.
- Six-theme full chat matrix passes physically.

### Phase 8 — Offline history synchronization

Build:

- Inventory/high-water/gap protocol.
- Priority scheduler and channel-utilization throttle.
- 40 internal/1,000 SD retention.
- Pre-join history disclosure.
- Removal epoch access control.
- Conflict quarantine and diagnostics.

Design:

- Sync status and `History incomplete` messaging.
- Nonblocking progress.
- No-history/no-replica states.
- Per-group storage usage.

Tests/gate:

- Devices off for hours/days.
- Multiple replicas with different subsets.
- New member prehistory.
- Removed member requests.
- Slow presets and busy channel.
- Live SOS/chat preempts sync.
- Six-theme sync states pass.

### Phase 9 — Map, position privacy, and group filters

Build:

- Map filters and multi-group badges.
- Stale/last-known position model.
- Precise/hidden policy.
- Member detail cards.
- No-tile fallback.
- Fixed marker identity palette.

Design:

- All-known default map and fast filter control.
- Tapped details and Navigate action.
- Stale fade and dismissible warnings.
- Theme-independent marker legibility; no theme-specific maps.

Tests/gate:

- Missing GPS/tiles/SD, stale/hidden/phone positions, multi-group member.
- Accuracy and age displayed honestly.
- Six themes maintain marker contrast.

### Phase 10 — Compass, navigation, and breadcrumbs

Build:

- Pure distance/bearing service.
- Heading-provider interface and fallback chain.
- Five-second updates, closer/farther trend, arrival radius.
- Local/destination breadcrumb sharing.
- Navigation-start notification.
- Optional magnetometer adapter left nonblocking until hardware exists.

Design:

- Member detail confirmation.
- Map and arrow-only modes.
- Large arrow, distance, heading source, age/hops/signal/alert details.
- Dismissible stale banner and manual completion.

Tests/gate:

- Geographic unit vectors, poles/date line, missing/stale GPS, moving GPS course.
- Simulator verifies MAG interface without requiring hardware.
- Original/Plus validation recorded separately.
- Six-theme navigation matrix passes.

### Phase 11 — Markers, meetups, and Emergency Rally Point

Build:

- Marker lifecycle and reconfirmation.
- Standard waypoint bridge for ordinary markers.
- FriendMesh-only sensitive markers.
- Meetup proposal/votes/quorum/attendance/arrival/expiry/cancel/move.
- Emergency Rally Point and private return-to-start.

Design:

- Chat voting card.
- Map/detail/navigation actions.
- Countdown and vote results.
- Move/cancel confirmations and marker manager.
- Fixed semantic marker symbols across themes.

Tests/gate:

- Offline votes, deadline crossing, conflicting votes, admin override threshold, clock skew.
- Arrival accuracy and expiry.
- Marker reconfirm/delete and stock waypoint visibility rules.
- Six-theme cards/map/details pass.

### Phase 12 — SOS

Build:

- Hold/countdown activation.
- Incident ID, recipient states, dedup, adaptive retry/beacon scheduler.
- Exact/last-known location policy.
- Group/emergency-contact and public fallback paths.
- Close reasons and SD incident log.
- Storage-bypass emergency outbox.

Design:

- Fixed red/white/black overlay.
- Max-intent hold with clear progress.
- Five-second privacy countdown.
- Recipient full-screen alert and action dashboard.
- Map/arrow launch from incident.

Tests/gate:

- Accidental input, duplicates, no GPS, stale GPS, no peers, congested mesh, duty-cycle limits, reboot, storage failure.
- No claim of guaranteed emergency service.
- All themes render identical emergency semantics.
- Physical multi-device drill passes.

### Phase 13 — Help Requests

Build:

- Categories/free text/silent option.
- Response states and location sharing.
- Two-responder/admin/requester escalation certificates.
- De-escalation/closure and three-hour handled state.
- Storage-bypass path.

Design:

- Urgent-but-below-SOS styling.
- Neutral discreet-extraction notification.
- Response buttons and responder list.
- Escalation confirmation explaining consequence.

Tests/gate:

- Multiple responders, unable, requester offline/incapable, conflicting escalation/de-escalation.
- Location stops on closure.
- Six-theme Help matrix and physical drill pass.

### Phase 14 — SD-only Contacts and notification center

Build:

- Verified contact registry.
- Emergency-alert consent.
- PKI DM/navigation integration.
- SD notification center and retention.
- Contact removal/identity change.

Design:

- Separate Contacts area clearly distinct from groups.
- SD-required explanation.
- Emergency permission during verification.
- Notification filters and incident priority.

Tests/gate:

- SD removal, contact in multiple/no groups, identity replacement, no consent.
- Group history never leaks into contact DM.
- Six-theme Contacts/notifications pass.

### Phase 15 — Interoperability, protected channels, and upstream update rehearsal

Build:

- Stock-client read/protected-write behavior.
- Advanced unlock/reconciliation.
- Public Meshtastic area regression.
- Upstream delta exercise against a selected later Meshtastic point without automatically merging it.

Design:

- Clear protection errors and recovery consequences.
- Normal Meshtastic area remains recognizable and complete.
- No FriendMesh group content appears in stock clients.

Tests/gate:

- Stable Meshtastic device, stock phone clients, public/private normal channels, PKI DMs, NodeDB, telemetry, GPS, traceroute, BLE reconnect.
- FriendMesh frames relay but do not render on stock clients.
- Critical public SOS fallback renders as text.

### Phase 16 — Full security, reliability, and six-theme qualification

Build:

- Threat-model closure.
- Fuzzing, static checks, power-loss campaign, storage endurance, memory/stack/PSRAM profiling.
- Airtime and battery measurements for eight members across eight configured groups.
- Key-audit expiration and secure cleanup.
- Accessibility/focus/contrast fixes.

Design:

- Capture every required screen/state in all six themes.
- Verify outdoor Alpine/Clean Modern readability, dark-room use, fixed marker colors, fixed SOS overlay, no animation dependency.
- Test longest names, full groups, full histories, and maximum badges.

Gate:

- Complete automated and physical matrices.
- No unresolved critical/high security issue.
- No unbounded queue/storage/memory behavior.
- No privacy or emergency claim exceeds measured behavior.

### Phase 17 — Release qualification

Build:

- Reproducible clean build, artifacts, checksums, changelog, migration/recovery instructions, supported-hardware matrix.
- Factory and update paths.
- Versioned schemas and backup/export warnings.
- Known limitations and risk acknowledgment.

Design:

- Final screenshots for all themes and major flows.
- First-run safety/privacy/verification education.
- About screen with FriendMeshOS/upstream versions and attribution.

Gate:

- User explicitly approves release.
- README warning changes from “incomplete” only after every required gate is evidenced.
- Tag only after physical multi-device regression and recovery drill.

---

## 23. Test topology

Minimum meaningful qualification fixture:

- Development FriendMeshOS T-Deck.
- At least two additional FriendMeshOS T-Deck-class devices for group/ACK/rekey/SOS.
- Stable untouched Meshtastic comparison node.
- Stock phone client over BLE.
- Optional relay/router to force multi-hop paths.
- SD present/absent/full/corrupt cases.
- GPS present/absent/stale cases.
- Optional Linux keeper only in later tests.

Eight-member scale can use a combination of physical devices and deterministic Portduino/simulator nodes, but final emergency, timing, radio, input, map, and storage behavior needs physical T-Deck evidence.

Required campaigns:

- Direct and multi-hop.
- Packet loss/duplication/reordering.
- Power loss at transaction boundaries.
- Devices offline across rekey/history/disband.
- Congested LongFast and slower presets.
- Eight groups and eight members per group.
- Public Meshtastic simultaneous traffic.
- BLE reconnect/config attempts.
- All six themes and all three input methods.

---

## 24. Feasibility ladder

### Tier 1 — Required and feasible on T-Deck

- Verified device identity and group aliases.
- Eight groups/eight members per group.
- FriendMesh-only signed group chat.
- Strong random group PSKs and MQTT-off policy.
- Admin/member roles and secure invitations.
- Leave/kick/rekey with offline pending state.
- Local encrypted history and bounded sync.
- Map filters, stale positions, distance/bearing.
- North-up and moving GPS-course navigation.
- Markers, meetups, voting, Emergency Rally Point.
- SOS/Help best-effort state machines.
- Theme-complete UI.

### Tier 2 — Feasible but inherently best effort

- Offline history recovery: only if a replica retained the records.
- Remote deletion and group erasure: only compliant devices that receive the tombstone.
- Admin succession: quorum reduces split brain but cannot make disconnected networks instantaneous.
- Retired-key traffic detection: observation/signature evidence, not physical attribution.
- Public SOS fallback: receipt not guaranteed and not emergency dispatch.
- Offline rekey: waits for target return and unchanged verified identity.

### Tier 3 — Requires SD

- 1,000 records per group.
- Durable drafts.
- Contacts.
- Notification center.
- Offline map tiles.
- Personal markers.
- Breadcrumb storage.
- Permanent SOS incident logs.

### Tier 4 — Requires optional hardware or infrastructure

- Device-relative stationary compass: external supported magnetometer.
- Guaranteed-ish long-term offline history availability: one or more always-on FriendMesh keepers.
- Larger groups: airtime and storage redesign/testing.
- Cross-device implementation: new UI/hardware adapters and full qualification.

### Tier 5 — Later research, not release-one dependencies

- Registered upstream FriendMesh port number.
- Linux history keeper.
- More than eight groups or eight tested members per group.
- Richer contact network.
- Additional heading sensors and calibration UI.
- Phone-side FriendMesh integration, only if a separate compatible app is ever built.

### Tier 6 — Cannot honestly be guaranteed

- Hiding the existence/metadata of LoRa transmissions.
- Preventing a person with the current group key from reading group content.
- Reliably identifying or physically tracing an old-key transmitter from Meshtastic headers.
- Erasing screenshots, exports, malicious firmware, or permanently offline copies.
- Recovering history when no device retained it.
- Guaranteed SOS delivery or emergency-service response.
- Accurate one-meter arrival from ordinary consumer GPS.
- Reliable device-relative stationary direction without a magnetometer.
- Secure private FriendMesh operation in licensed/Ham mode.
- Automatically calling emergency services from the standalone T-Deck.
- Absolute prevention of configuration tampering by someone with physical control and custom firmware.

These are documented limitations, not unfinished checkboxes.

---

## 25. Approved public warning

Until final release qualification, the README must prominently state:

> FriendMeshOS is incomplete development firmware. Group security, history synchronization, emergency alerts, key rotation, location sharing, and Meshtastic interoperability may be unfinished or unsafe. Do not rely on it for emergency communication, personal safety, secure messaging, or production use.

---

## 26. Definition of done

FriendMesh is not done because screens exist or a demo succeeds. It is done only when:

- Every required phase gate has recorded evidence.
- Every cryptographic/state-machine behavior has vectors and failure tests.
- Stock Meshtastic interoperability remains intact.
- Four-group/eight-member scale is measured.
- Offline/rekey/history limitations are reflected honestly in UI.
- Every screen and state passes all six themes and all T-Deck inputs.
- Original and Plus capability results are documented.
- Recovery from power loss, storage failure, missing GPS, and offline members is proven.
- Emergency language and behavior never imply guaranteed help.
- README, roadmap, core handbook, game plan, changelog, and release artifacts agree.
- The user explicitly approves the completed release.
