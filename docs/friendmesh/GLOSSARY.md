# FriendMesh Canonical Glossary

Status: active Phase 0 terminology contract  
Last reviewed: 2026-07-12  
Applies to: firmware, protobuf names, storage schemas, UI copy, diagnostics, tests, manuals, and release notes

Use these terms consistently with the [`FriendMesh game plan`](../../FRIENDMESH_FULL_IMPLEMENTATION_GAME_PLAN.md) and [`Meshtastic/FriendMesh architecture handbook`](../../MESHTASTIC_CORE_AND_FRIENDMESH_ARCHITECTURE.md). New wire or persisted names must be added here before release; an implementation may use a shorter symbol internally only when its mapping is unambiguous.

Term status:

- `FROZEN`: approved meaning; changing it requires a decision-log entry and compatibility review.
- `BOUNDARY`: existing Meshtastic meaning that FriendMesh must preserve.
- `UI`: approved user-facing label or wording rule.
- `RESEARCH`: concept is approved, but exact primitive/bound is not selected.
- `RETIRED`: retained only to explain an obsolete term; do not use in new work.

## A-F

| Term | Canonical meaning | Do not confuse with / wording constraint | Status | Authority / evidence |
| --- | --- | --- | --- | --- |
| ACK | Meshtastic routing acknowledgment correlated to a packet request. For direct traffic it indicates protocol handling; for broadcast, hearing a relay can serve as an implicit ACK. | Never claim an ACK means a human read the message or every member received it. | BOUNDARY | Handbook: Application transport and Broadcast behavior |
| Active incident | An SOS or Help Request whose signed state has not reached a valid closed/resolved state. | A delivered or arrived response does not automatically close the incident. | FROZEN | Game plan: SOS states; Help Request system |
| Admin | The one active administrative device identity for a group membership epoch. | Not a human account spanning devices; not automatically restored after succession. | FROZEN | Game plan: Admin model |
| Administrative epoch | The generation of administrative authority, advanced by a valid succession or other explicitly designed authority transition. | Distinct from key epoch and membership epoch, even if an implementation stores related counters together. | FROZEN | Game plan: Admin succession; Administrative event chain |
| Administrative event chain | Strictly ordered admin-authorized events containing admin sequence, prior-event hash, membership epoch, and admin signature or quorum certificate. | Ordinary concurrent chat is not globally ordered on this chain. | FROZEN | Game plan: Administrative event chain |
| Advanced channel recovery unlock | PIN-gated, two-step temporary ability to alter a protected FriendMesh-owned channel through recovery tooling. | Not routine channel editing and not a promise that metadata/security can always be repaired. | UI | Game plan: Stock client protection and recovery |
| AEAD | Authenticated encryption with associated data used for FriendMesh stored records and any selected sealed-grant construction. | AES-CTR alone is not AEAD and is insufficient for authenticated storage. Exact construction is still a reviewed implementation choice. | RESEARCH | Game plan: Per-device storage encryption |
| Alias | Group-specific display name for one device member, normalized deterministically, case-insensitively unique, and capped at 32 display characters plus safe UTF-8 byte bounds. | Not Meshtastic `long_name`, `short_name`, a global username, or proof of identity. | FROZEN | Game plan: Required identity fields |
| APPROVED | Membership status created by a valid signed admin approval for the current membership context. | A device setting its own flag, possessing a display name, or appearing in NodeDB is not approval. | FROZEN | Game plan: Identity fields; Invitation flow |
| ARRIVED | A responder's explicit manual confirmation that they arrived at an SOS/Help/meetup destination. | Not inferred solely from GPS and does not automatically close an SOS. | FROZEN | Game plan: SOS states; Meetup attendance |
| Arrival radius | `max(10 m, reported GPS accuracy)` used to offer an `I'm here` action. | Not a claim of one-meter precision and not automatic navigation completion. | FROZEN | Game plan: Navigation arrival |
| Best effort | Behavior attempted within radio, storage, compliant-device, and reachability limits, without guaranteed completion. | Do not shorten to language implying reliable delivery, remote deletion, or emergency dispatch. | UI | Game plan: Feasibility ladder |
| BLOCKED_LOCAL | Local-only member state that hides ordinary future content/presence according to personal preferences. | Does not revoke group membership or rotate a PSK; emergency handling remains explicit. | FROZEN | Game plan: Personal blocking |
| Breadcrumb | Time-ordered navigation positions retained privately by the navigator and optionally shared with the destination only during an active trip. | Not indefinite group location history or a public route. | FROZEN | Game plan: Breadcrumb sharing |
| Broadcast | Meshtastic destination semantics where one packet is addressed to the channel audience and may be relayed through managed flooding. | Does not imply every node received it; should not create ACK storms. | BOUNDARY | Handbook: Broadcast versus direct behavior |
| Capability detection | Runtime discovery of GPS, magnetometer, SD/maps, inputs, and LoRa state used to select an explicit fallback. | Missing optional hardware is degradation, not generic firmware failure. | FROZEN | Game plan: T-Deck capability contract |
| Channel hash | One-byte XOR-derived hint in the visible radio header used to narrow candidate channel keys. | Not authentication, identity proof, or a secure channel identifier. | BOUNDARY | Handbook: Channel encryption |
| Channel index / slot | Local Meshtastic channel-table position used by a primary or secondary channel. FriendMesh consumes one managed carrier slot total for up to eight application groups. | Not a FriendMesh group ID or group security boundary; never overwrite automatically when full. | FROZEN | Game plan: Group limits |
| FriendMesh carrier | One managed Meshtastic secondary channel that transports versioned FriendMesh envelopes for every local group. | Its outer PSK is not sufficient group confidentiality or authorization; inner group AEAD and signatures are mandatory. | FROZEN | Game plan: Group limits; architecture handbook: Group data model |
| Channel PSK | Meshtastic symmetric channel key. A secure FriendMesh epoch uses 32 CSPRNG bytes. | Possession permits decrypt/inject at the outer AES-CTR layer; it is not signed membership proof. | BOUNDARY | Game plan: Group key requirements; handbook: Channel encryption |
| Client transport | BLE, serial, TCP, or other local stream carrying `ToRadio`/`FromRadio` protobuf messages between device and client. | Not the LoRa radio frame or FriendMesh over-air envelope. | BOUNDARY | Handbook: Client/device transport |
| CLOSED | Terminal incident state with an authorized close reason such as false alarm, safe, or help arrived. | `DELIVERED`, `RESPONDING`, and `ARRIVED` are nonterminal recipient states. | FROZEN | Game plan: SOS states |
| Compliant device | Firmware that enforces the approved FriendMesh signatures, epochs, erasure/disband tombstones, storage, and UI rules. | Cannot include malicious/custom firmware merely because it can receive Meshtastic traffic. | FROZEN | Game plan: Erasure and feasibility limits |
| Contact | Later SD-only verified device identity independent of group membership, capable of PKI DM/navigation and optional emergency alerts. | Not automatically a group member and not automatically an emergency contact. | FROZEN | Game plan: Later SD-only Contacts |
| CSPRNG | Cryptographically secure random-number generator used for group PSKs, signing/storage keys as applicable, nonces, and unpredictable IDs. | Not `rand()`, timestamps, invitation codes, PINs, names, or hashes of human input. | FROZEN | Game plan: Group key requirements |
| Current membership | The fixed set of nonremoved members authorized for the event/vote's referenced membership epoch. | Do not change a vote denominator retroactively as devices become reachable/unreachable. | FROZEN | Game plan: Succession, meetup, erasure |
| DELIVERED | Per-recipient application acknowledgment that a device received an incident. | Does not mean the person read it, is responding, arrived, or called emergency services. | UI | Game plan: SOS states |
| Delayed message | Ordinary message composed earlier and transmitted later from a durable outbox; UI shows original composition time and `Delayed X`. | Not silently rewritten to the transmit/receive time. | UI | Game plan: Clock behavior; Queue policy |
| Deletion tombstone | Signed best-effort event hiding an eligible ordinary message while preserving ordering and displaying `Message deleted`. | Not guaranteed removal from exports/screenshots/malicious clients; not permitted for system records while group exists. | FROZEN | Game plan: Chat operations |
| Device identity | Binding of full node number, Meshtastic X25519 fingerprint, and FriendMesh Ed25519 signing fingerprint verified in person. | A name, one-byte next-hop hint, MAC alone, or NodeDB row is not sufficient. | FROZEN | Game plan: Identity and trust architecture |
| Device replacement | Admin-approved transition to a newly verified device identity while preserving logical alias, ordinal, role, and history and revoking the old identity. | Not automatic acceptance of a new key because a name matches. | FROZEN | Game plan: Device replacement |
| Direct packet | Meshtastic packet with a nonbroadcast destination node number; it may still use relays/next-hop routing. | Not necessarily one physical hop. | BOUNDARY | Handbook: Direct packets and next-hop routing |
| Disband | Irreversible signed group destruction committed after a two-step, one-hour undoable countdown. | Not the same as one member leaving, local group deletion, expiration warning, or history erase vote. | FROZEN | Game plan: Disbanding |
| Disband tombstone | Replicated control record causing compliant offline members to erase group keys/state when they return. | Does not guarantee deletion from malicious or never-returning devices. | FROZEN | Game plan: Disbanding |
| Emergency alerts permission | Explicit Contact-level consent to receive SOS alerts that may bypass mute/quiet hours. | Ordinary contact verification or shared-group membership does not implicitly grant it. | FROZEN | Game plan: Contacts; SOS activation |
| Emergency Rally Point | One persistent admin-controlled group destination that does not expire until changed. | Not the one active impromptu meetup or private return-to-start point. | FROZEN | Game plan: Meetup types |
| Epoch | Versioned authorization/key context carried by signed events. The qualified term must identify which epoch is meant. | Avoid bare `epoch` in UI, logs, tests, and schema documentation when key, membership, and administrative epochs can differ. | FROZEN | Game plan: Group model and wire envelope |
| Erase commit | Signed/quorum-certified control event emitted after the approved history-erasure reachability and 75% yes thresholds. | Not proof that every copy everywhere was deleted. | FROZEN | Game plan: Best-effort erasure |
| Event | Canonically encoded, signed FriendMesh state/message unit with type, ID, group/epoch context, sender sequence, time metadata, payload hash, and optional TTL/fragment fields. | Not the outer Meshtastic packet; one logical event may span fragments or retransmissions. | FROZEN | Game plan: Envelope header |
| Event ID | Stable replay/deduplication identifier for one logical FriendMesh event. | Not Meshtastic packet ID; retransmitted packets/fragments remain associated with the logical event. | FROZEN | Game plan: Event identity and deduplication |
| Exact location | Full available coordinates delivered under approved private-group/contact policy or deliberate public opt-in. | Never call truncated, hidden, stale, or absent coordinates exact/current. | UI | Game plan: Map privacy; SOS activation |
| Expiration | Safe lifecycle transaction after 90 days without authenticated group activity, with visible warnings and clock-correction safeguards. | Not silent local deletion and not the same as disband. | FROZEN | Game plan: Group inactivity |
| FriendMesh | The T-Deck-only private group, navigation, coordination, SOS, Help, and later Contacts product layer above preserved Meshtastic services. | Not a new LoRa mesh protocol and not a generic contact skin. | FROZEN | Game plan: Product definition |
| FriendMeshEnvelope | Compact versioned Nanopb application envelope with magic bytes and signed canonical FriendMesh content carried initially on `PRIVATE_APP`. | Not a modified radio header, standard text payload, or client-link `MeshPacket`. | FROZEN | Game plan: Wire protocol |
| FriendMesh-owned channel | Secondary Meshtastic channel allocated to one FriendMesh group, using random 32-byte PSK, MQTT off, and protected configuration semantics. | Not a separate frequency/RF network; all enabled channels share primary RF settings. | FROZEN | Game plan: Group limits; handbook: Multiple-group receive/send |

## G-P

| Term | Canonical meaning | Do not confuse with / wording constraint | Status | Authority / evidence |
| --- | --- | --- | --- | --- |
| Group | Random-ID FriendMesh coordination domain sharing the carrier while retaining its own active admin, ordered device membership, AEAD epoch key, signed event state, and local storage. | Not a Meshtastic channel name/PSK or a NodeDB filter. | FROZEN | Game plan: Group model |
| Group ID | Random 128-bit identifier independent of group name, PSK, or channel index. | Never derive it from display metadata or use the channel hash as substitute. | FROZEN | Game plan: Group fields |
| Group key epoch | Version/generation of the active random group PSK used for FriendMesh-owned channel traffic. | Distinct from membership and administrative authority epochs. Retired epochs are receive-audit only. | FROZEN | Game plan: Key epochs and rekey |
| HAM MODE | User-facing security state when licensed amateur-radio mode disables encrypted private FriendMesh groups. | Never display `SECURE` for a private group in this mode. | UI | Game plan: Security states |
| Heading source | Explicit navigation orientation provider label: `MAG`, `GPS`, or `NORTH-UP`. | GPS ground track is not stationary compass heading. | UI | Game plan: Navigation fallback chain |
| Help Request | Signed urgent, non-emergency incident with category/text, response states, temporary responder location sharing, and controlled escalation. | Visually and semantically below SOS; not ordinary chat. | FROZEN | Game plan: Help Request system |
| Hidden location | Group policy or channel precision state in which precise member coordinates are not available to FriendMesh. | Do not infer, reconstruct, or restore stripped precision. | FROZEN | Game plan: Map privacy; handbook: Position |
| History incomplete | Honest state indicating missing retained events and/or no reachable replica, without treating the group as generally failed. | Not `sync failed` when records simply no longer exist or no replica is reachable. | UI | Game plan: History synchronization |
| High-water mark | Highest contiguous accepted sender sequence known for a member, combined with bounded missing ranges for efficient sync inventory. | Not proof that all earlier events exist unless gaps are empty and validated. | FROZEN | Game plan: Event identity and sync |
| Hop limit | Remaining forwarding budget in a received Meshtastic packet. | Not stable distance, member identity, or universally exact physical hops traveled. | BOUNDARY | Handbook: Hop meanings |
| Hop start | Origin's initial forwarding budget encoded in packet flags. | `hop_start - hop_limit` is an observation subject to routing helpers/optimizations. | BOUNDARY | Handbook: Fixed header; Hop meanings |
| Implicit ACK | Sender hearing another node relay its broadcast packet, proving the flood started. | Does not prove all group members or any human received/read it. | BOUNDARY | Handbook: Broadcast versus direct behavior |
| Impromptu meetup | One active member-proposed current-position destination with timed full-membership voting, attendance, navigation, arrival, and expiry. | Not the Emergency Rally Point or personal return-to-start. | FROZEN | Game plan: Meetup system |
| In-person verification | Human comparison of a security-number/fingerprint transcript binding node number, Meshtastic PKI key, and FriendMesh signing key before approval. | A six-character invitation code alone is discovery, not identity verification. | FROZEN | Game plan: Candidate/admin flow |
| Invitation code | Six-character alphanumeric nearby discovery token valid only while inviter keeps the invitation screen open. | Not a group PSK, membership credential, long-term secret, or approval. | FROZEN | Game plan: Open invitation |
| Join event ID | Stable signed reference to the admin approval that established a member record. | Not an invitation request or current packet ID. | FROZEN | Game plan: Identity fields |
| Join ordinal | Persistent member ordering established on approval and preserved through device replacement; used to identify default succession candidate. | Not a mutable alias sort or last-seen order. | FROZEN | Game plan: Identity fields; succession |
| Key change | Security state in which a member's verified identity fingerprint changed and must be reverified. | Never auto-approve based on node number or name. | UI | Game plan: Security states |
| Key epoch | Use `group key epoch`; bare form is discouraged when membership/admin contexts are present. | Not automatically equivalent to membership epoch. | RETIRED | This glossary: Epoch |
| Key grant | Member-specific encrypted delivery of a group key after signed approval or during rekey, bound to group/epoch/identity. | Never a public group broadcast or plaintext channel URL in the approval flow. | FROZEN | Game plan: Invitation; Offline rekey envelopes |
| KICK_PENDING | One-hour reversible membership-removal state initiated by admin; compliant target is immediately muted. | Not cryptographic revocation until rekey commits. | FROZEN | Game plan: Leave and kick state machine |
| Last heard | Age since the latest valid observation of a node. | Not the age of its position and not a guarantee it remains reachable now. | UI | Game plan: Member details; handbook: Presence |
| Last-known location | Newest retained valid position when no current position exists, always labeled with source, age, precision, and accuracy. | Never render as live/current exact location. | UI | Game plan: Map and SOS missing GPS |
| LEAVING / LEAVE_PENDING | One-hour reversible voluntary-departure state waiting for authorized coordinator/admin processing; compliant device is muted. | Closing the UI or deleting local data is not leaving; key possession continues until rekey. | FROZEN | Game plan: Leave and kick state machine |
| Local receive time | Receiver's local timestamp for packet/event arrival, stored alongside sender and sync times. | Do not overwrite original sender time. | FROZEN | Game plan: Clock behavior |
| Managed flooding | Meshtastic relay behavior using duplicate history, randomized contention, cancellation, and decreasing hop budget. | FriendMesh must not create a separate router/scheduler that ignores this behavior. | BOUNDARY | Handbook: Managed flooding |
| Marker | Signed group or SD-only personal point with a typed lifecycle, details, and navigation action. | Sensitive Meetup/Help/SOS/control markers stay FriendMesh-only; not every marker is a standard waypoint. | FROZEN | Game plan: Marker system |
| Member | One verified device identity occupying one group slot, with role, ordinal, alias, status, fingerprints, and membership context. | Not an abstract person, NodeDB entry, display name, or contact. | FROZEN | Game plan: One device is one member |
| Membership epoch | Versioned membership authorization context referenced by signed events and administrative chain validation. | Distinct from the active group PSK version and administrative authority generation. | FROZEN | Game plan: Identity fields; Administrative chain |
| Meshtastic area | Separate on-device area for ordinary upstream-compatible text, nodes, configuration, and supported Meshtastic features. | FriendMesh group chat must not replace or appear as standard public text. | UI | Game plan: Compatibility boundary; Boot home |
| MQTT off | Required FriendMesh-owned channel policy with both uplink and downlink disabled. | A configured PSK alone is not enough for `SECURE`. | FROZEN | Game plan: Privacy boundary |
| NAK | Meshtastic routing response indicating a typed failure such as no route, timeout, malformed request, or queue/duty condition in the pinned enum. | Do not collapse every future/unknown error to `not sent`. | BOUNDARY | Handbook: Application transport |
| Node number | Full 32-bit Meshtastic node identifier. | Short name and low-byte next-hop/relay hints are not stable identity substitutes. | BOUNDARY | Handbook: Node identity |
| NodeDB | Bounded eventually consistent local cache of node identity, position, radio, and route observations. | Never membership authority or guaranteed synchronized presence. | BOUNDARY | Handbook: NodeDB, discovery, and presence |
| Nonce | Unique value required by encryption modes; Meshtastic channel/PKI layouts are compatibility boundaries, while FriendMesh storage nonce allocation needs separate crash-safe design. | Never reuse under one key; do not change Meshtastic packet nonce construction for MVP. | BOUNDARY | Handbook: Encryption; game plan: Storage encryption |
| NORTH-UP | Heading mode where destination bearing is drawn relative to geographic north without requiring device orientation. | Not body-relative direction. It is the guaranteed no-magnetometer fallback. | UI | Game plan: Navigation fallback chain |
| Offline rekey envelope | Replicable ciphertext containing one signed member-specific current-epoch grant for an offline remaining member. | Not a broadcast PSK and not valid after target fingerprint changes. | FROZEN | Game plan: Offline rekey envelopes |
| Ordinary message | Non-system FriendMesh chat authored by a member, eligible for reactions and owner/admin tombstone deletion. | Not an undeletable join/rekey/admin/SOS/meetup security record. | FROZEN | Game plan: Chat operations |
| Outside help | User action that, after a second explicit warning, sends a standard public Meshtastic SOS fallback and may include explicitly approved coordinates. | Does not place a phone call or dispatch emergency services. | UI | Game plan: SOS recipient experience |
| Packet ID | Meshtastic per-sender identifier used for duplicate/retry/ACK state and channel nonce construction. | Not globally permanent and not the FriendMesh logical event ID. | BOUNDARY | Handbook: Packet IDs and duplicate suppression |
| Pending transaction | Durable reversible or forward-recovery state for membership, rekey, erase, disband, or expiration operations. | UI pending state is not equivalent to committed cryptographic state. | FROZEN | Game plan: Group fields; Transaction recovery |
| Personal marker | SD-required marker local to one device and absent from group history. | Not replicated signed group marker. | FROZEN | Game plan: Marker rules |
| PKI | Meshtastic per-peer X25519 key agreement followed by SHA-256 and AES-256-CCM for direct encrypted payloads. | X25519 is not a signing algorithm; FriendMesh uses separate Ed25519 signatures. | BOUNDARY | Handbook: Per-peer PKI |
| PKI fingerprint | Human-comparable fingerprint of a device's Meshtastic X25519 public key included in FriendMesh identity binding. | Not the Ed25519 signing fingerprint and not a private key. | FROZEN | Game plan: Identity fields |
| Position age | Time since the referenced coordinate fix/observation according to preserved time-quality metadata. | Not the same as last-heard age. | UI | Game plan: Member details/navigation |
| Position precision | Meshtastic per-channel privacy control that can suppress or truncate location coordinates. | FriendMesh must never infer or restore removed precision. | BOUNDARY | Handbook: Position, map, distance, heading |
| `PRIVATE_APP` | Port number 256 private application space in the pinned Meshtastic schema, used initially with FriendMesh magic/version dispatch. | Does not make an unversioned payload globally unique; a registered port remains later research. | BOUNDARY | Game plan: Application transport; handbook: Port numbers |
| Protected channel | FriendMesh-owned channel whose normal stock-client edit/delete actions are denied while metadata remains readable for compatibility. | Not immutable against physical control/custom firmware; advanced recovery can deliberately unlock it. | FROZEN | Game plan: Stock client protection |
| Public fallback | Standard readable Meshtastic SOS text sent only after deliberate outside-help confirmation, with coordinates only under explicit approval. | Not normal FriendMesh group traffic and not emergency-service dispatch. | FROZEN | Game plan: SOS architecture |

## Q-Z

| Term | Canonical meaning | Do not confuse with / wording constraint | Status | Authority / evidence |
| --- | --- | --- | --- | --- |
| Quorum certificate | Signed collection/summary proving the required fixed-membership threshold approved an administrative transition, escalation, or erasure. | Reachability alone is not approval; denominators follow the event's approved rules. | FROZEN | Game plan: Succession and erasure |
| Reachable | Device/member observed sufficiently recently under a defined protocol window for an operation that explicitly has a reachability prerequisite. | Not synonymous with current membership, delivered, or guaranteed future contact. Exact measurement window must be specified by implementation decision/test. | RESEARCH | Game plan: Erasure vote; Sync |
| Reaction | Signed thumbs-up or thumbs-down state; one current reaction per member per ordinary message. | Not free-form emoji, a reply thread, or message edit. | FROZEN | Game plan: Chat operations |
| REKEY_COMMITTED | Irreversible state where durable new group record/grants define the active key epoch and removed identity has no grant. | Not reached merely because preparation or one grant began. | FROZEN | Game plan: Rekey state machine |
| REKEY_PENDING | Security/member state where an approved remaining member lacks the current group key epoch and cannot read new group traffic until valid grant receipt. | Not a removed member and not generic history sync incompleteness. | UI | Game plan: Offline rekey; Security states |
| Retired key | Previous group PSK retained, when enabled, for at most 30 days inside encrypted audit storage and never selected for transmission. | Detection does not prove physical attacker identity or current membership. | FROZEN | Game plan: Retired-key audit |
| Routing priority | Meshtastic queue class controlling relative transmission treatment. FriendMesh ordinary/background work uses existing compatible priorities; SOS/Help/key/control may be elevated as approved. | Do not invent a scheduler that bypasses queue, airtime, or duty-cycle logic. | BOUNDARY | Handbook: Queue priority; game plan: Queue policy |
| `SECURE` | Group security state requiring random 32-byte PSK, MQTT off, current valid epoch, verified members, and no transaction fault. | Does not promise traffic-analysis resistance, malicious-device protection, guaranteed delivery, or hardware-backed at-rest security. | UI | Game plan: Security states |
| Sender sequence | Monotonic per-sender event sequence used with event IDs/high-water marks for replay checks and sync. | Not the Meshtastic rolling packet ID or a global total order. | FROZEN | Game plan: Envelope header; Event identity |
| Sender time | Timestamp reported by the event originator with quality flags. | Preserve it, but do not trust it alone for destructive countdowns, succession, or silent history rewriting. | FROZEN | Game plan: Clock behavior |
| Signing fingerprint | Human-comparable fingerprint of the FriendMesh Ed25519 public signing key. | Distinct from Meshtastic X25519 PKI fingerprint. | FROZEN | Game plan: Identity fields |
| Signing identity | FriendMesh Ed25519 keypair used to sign attributable application events and bound to device identity during verification. | Never reuse a group PSK or X25519 private key as signing material. | FROZEN | Game plan: Why FriendMesh needs a signing key |
| SOS | Highest-severity signed FriendMesh peer-assistance incident activated through intentional hold/countdown and delivered best effort across groups and consented Contacts. | Not guaranteed delivery, emergency-service dispatch, or a telephone call. | FROZEN | Game plan: SOS state machine |
| Stale position | Retained position older than the approved freshness threshold; the game plan requires visual fading after five minutes. | A stale position can still be last-known, but must not be rendered as current. | UI | Game plan: Map marker display |
| Standard waypoint | Meshtastic `WAYPOINT_APP` representation usable for approved ordinary nonsensitive marker interoperability. | Meetup, Help, SOS, and control markers remain FriendMesh-only. | BOUNDARY | Game plan: Marker types; handbook: Port numbers |
| Storage degraded | Security/status condition where persistence capacity or medium falls below requested tier but explicitly supported fallback remains. | Not a silent success state. Some control/chat classes must stop; SOS/Help may bypass with warning. | UI | Game plan: Storage failure policy; Security states |
| Storage master key | Per-device secret from which domain-separated storage subkeys are derived and which is protected through screen PIN plus reviewed wrapping/KDF design. | Not the screen PIN itself, group PSK, or signing key. | RESEARCH | Game plan: Per-device storage encryption |
| Storage tier | Capability contract distinguishing essential internal retention from SD-expanded features/capacity. | Do not advertise SD-only Contacts, durable drafts, maps, or 1,000-record retention without mounted healthy SD. | FROZEN | Game plan: Storage tiers |
| Succession | Quorum-authorized admin replacement made available after ten days without authenticated admin activity and advancing administrative authority. | Not automatic clock-only takeover or restoration of returning former admin. | FROZEN | Game plan: Admin succession |
| Sync receipt time | Local time an event was received through history synchronization. | Distinct from original sender and first live receive times. | FROZEN | Game plan: Clock behavior |
| System record | Signed join, leave, kick, rekey, admin, SOS, meetup, or security/control history item undeletable while group exists except through valid total erasure/disband semantics. | Not an ordinary message eligible for sender/admin tombstone. | FROZEN | Game plan: Chat operations; System records |
| T-Deck Plus result | Separate physical capability/qualification record for Plus hardware using the common runtime-detected build. | Do not infer it from original T-Deck results or vice versa. | FROZEN | Game plan: T-Deck capability contract |
| Tombstone | Signed replicated event that records deletion/disband/erase intent for compliant devices, including those returning later. Use a qualified term in user-facing text. | Does not guarantee removal from malicious, exported, screenshot, or never-returning copies. | FROZEN | Game plan: Chat deletion; Erasure; Disbanding |
| Transaction journal | Durable authenticated write-ahead state used to recover multi-step security and membership operations after reboot/power loss. | Not ordinary event history and not optional before transmitting security-critical state. | FROZEN | Game plan: Forward-only recovery; Files and journals |
| `UNSAFE CONFIG` | Group state caused by protected-channel drift, unlock, MQTT enablement, or another violated security invariant; group traffic is blocked until reconciled. | Never show `SECURE` concurrently. | UI | Game plan: Security states |
| Verified | Identity state reached only after approved proof-of-possession and in-person comparison binds node number, PKI key, and signing key. | Invitation-code possession, NodeDB presence, matching alias, or PSK possession alone is unverified. | FROZEN | Game plan: Identity and Invitation flow |
| Verified Contact | SD-stored, separately verified device contact whose identity remains valid only while its public-key binding is unchanged. | Does not imply group membership or emergency-alert permission. | FROZEN | Game plan: Later SD-only Contacts |
| Wire compatibility | Ability to preserve expected Meshtastic on-air and client-link field numbers, framing, crypto selection, routing, port semantics, and unknown-value behavior. | A successful local UI demo is not compatibility evidence. | BOUNDARY | Architecture handbook: Governing contract and Protobuf architecture |

## Normative wording rules

Use these rules in implementation notes, UI, and release claims:

- `Delivered` means device acknowledgment only; use `Read` nowhere unless a separate future human-read protocol is approved.
- Qualify every `epoch` as `group key`, `membership`, or `administrative` in technical surfaces.
- Qualify every `key` as PSK, X25519 private/public key, Ed25519 signing key, storage master/subkey, sealed-grant key, or retired group key.
- Use `best effort` with the specific limitation, not as a vague disclaimer detached from the action.
- Say `last heard: N hops, X ago`, not timeless `N hops away`.
- Say `last-known location, X old` when current GPS is unavailable.
- Say `public Meshtastic fallback`; never say `call emergency services` or imply the device placed a call.
- Say `rekey pending` only for current-key delivery state; use `sync incomplete` for missing history.
- Say `device member` when person/device ambiguity affects slots, votes, Contacts, or replacement.
- Never display PSK bytes, private keys, decrypted diagnostic bodies, or precise coordinates by default.

## Term change checklist

- [ ] Add a new decision record or reference an existing one.
- [ ] Identify protobuf numeric values and persisted-schema compatibility.
- [ ] Define the user-facing label and fallback for unknown future values.
- [ ] Update every affected screen, diagnostic, test fixture, and manual.
- [ ] Add migration/alias handling when a shipped term changes.
- [ ] Update risk and upstream-delta records when the term crosses a security or Meshtastic boundary.
- [ ] Link committed evidence and reviewer approval.

## Review history

| Date | Scope | Reviewer | Result | Evidence |
| --- | --- | --- | --- | --- |
| 2026-07-12 | Initial canonical vocabulary extracted from approved product and Meshtastic architecture contracts | Phase 0 documentation pass | Created; implementation-specific crypto/bound terms remain marked `RESEARCH` | Canonical documents linked above |
