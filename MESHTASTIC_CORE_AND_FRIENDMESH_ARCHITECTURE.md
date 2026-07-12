# Meshtastic Core and FriendMesh Architecture Handbook

This document is the protocol and implementation source of truth for FriendMeshOS work built on the vendored Meshtastic `v2.7.26.54e0d8d` foundation. It explains what makes the current firmware Meshtastic, identifies the exact source anchors, and defines how FriendMesh groups and the Totem-inspired Friend Compass must reuse the protocol without breaking interoperability.

It is intentionally deeper than a feature overview. Before changing radio, routing, channel, encryption, NodeDB, position, or protobuf behavior, trace the named source path and update this document when behavior changes.

## Governing product contract

FriendMeshOS is a T-Deck product fork with a Meshtastic compatibility boundary, not a reskinned upstream UI and not a new incompatible mesh protocol.

The project may redesign or replace the LilyGO T-Deck's complete on-device experience: navigation, layouts, buttons, menus, workflows, information architecture, visual system, storage UI, map/compass experience, group management, survey tools, and future T-Deck-specific applications. Upstream screen structure is not a compatibility requirement.

The project must preserve or deliberately version the behavior that makes the device a Meshtastic node:

- Compatible LoRa physical settings and regional constraints.
- The Meshtastic 16-byte radio header and packet-size limits.
- Node-number identity, packet IDs, duplicate detection, routing, hop, relay, and next-hop behavior.
- Broadcast and direct-message semantics.
- ACK, NAK, retry, priority, airtime, and congestion behavior.
- Channel PSKs, PKI identity, nonces, encryption selection, and key lifecycle.
- Standard port numbers and protobuf wire compatibility.
- NodeInfo, Position, text, telemetry, traceroute, admin, client API, and phone interoperability where supported by the T-Deck build.

Meshtastic `2.7.26` is the pinned base, not a permanent ban on upstream updates. Upstream fixes and improvements can be brought into FriendMeshOS when relevant. Every update must be understood at the behavioral boundary and classified before it is merged.

### Current platform scope

All firmware implementation, UI design, memory budgeting, hardware integration, testing, and release qualification currently target only:

```text
LilyGO T-Deck
ESP32-S3
PlatformIO environment: t-deck-tft
320x240 TFT + touch + trackball + keyboard + SX1262 + microSD
```

No other Meshtastic board is a FriendMeshOS-supported target today. Shared upstream code may remain in the repository because the T-Deck depends on it, but FriendMeshOS must not spend implementation or qualification effort making new features portable to other boards during this phase.

The architecture must still be documented in a way that lets another developer repeat the approach on another device later. Documentation must distinguish:

- Protocol-independent behavior that every compatible implementation must preserve.
- T-Deck-specific hardware/UI code.
- Reusable service interfaces and data models.
- Assumptions that would need replacement on another display, input system, MCU, radio, or storage layout.

Portability is a documentation requirement, not a current build-target requirement.

## Upstream Meshtastic update policy

FriendMeshOS must make upstream changes understandable and selectively adoptable rather than periodically replacing the fork wholesale.

For every candidate upstream update:

1. Record the current FriendMeshOS base commit, upstream source commit/tag, and exact comparison range.
2. Review protobufs before firmware code. Identify field additions, enum additions, removed/deprecated values, generated-binding changes, maximum sizes, and client compatibility implications.
3. Review radio and routing changes: header flags, hop calculations, flood/next-hop behavior, retry timing, queue priorities, airtime limits, region tables, and modem presets.
4. Review security changes: nonce construction, PSK handling, PKI selection, key generation/storage, admin authorization, and reset behavior.
5. Review NodeDB, position, telemetry, text, traceroute, client API, and storage migrations.
6. Classify every relevant upstream change as:
   - **Required compatibility update** — needed to communicate safely/correctly with current Meshtastic nodes or clients.
   - **T-Deck reliability/security update** — applicable bug, safety, regulatory, or hardware fix.
   - **Optional capability update** — useful but not required for compatibility.
   - **Not applicable** — another board/UI/platform or disabled feature with no shared dependency impact.
   - **Conflict requiring adaptation** — overlaps FriendMeshOS code and must be reimplemented against the new upstream contract.
7. Port the smallest coherent change set. Do not overwrite FriendMeshOS UI or feature architecture merely because upstream reorganized its screens.
8. Regenerate protobufs only from a reviewed schema commit and record generator/tool versions.
9. Run compile, static, unit, protocol-vector, two-node, stock-client, and physical T-Deck regression appropriate to the changed boundary.
10. Update this handbook, the roadmap milestone log, and an upstream-delta ledger before declaring the update complete.

An upstream-delta ledger should eventually record, per imported range, the upstream commits, local adaptations, intentionally skipped changes, tests, and remaining risks. This is the primary mechanism for keeping FriendMeshOS maintainable as Meshtastic evolves.

## FriendMeshOS application and feature boundary

New product features should be organized above the preserved Meshtastic services:

```text
FriendMeshOS T-Deck UI and workflows
  group chat | compass/map | SOS | storage | diagnostics | survey tools
                         |
FriendMeshOS application services and local metadata
                         |
Existing Meshtastic modules, NodeDB, channels, positions, routing and crypto
                         |
Meshtastic packet/protobuf compatibility boundary
                         |
T-Deck SX1262 LoRa + ESP32-S3 Wi-Fi/BLE + shared SPI hardware
```

UI code must not reach directly into radio queues, encryption buffers, private keys, or protobuf storage files. It should consume stable FriendMeshOS service interfaces that wrap existing Meshtastic APIs. This makes core updates auditable and lets a future device implementation replace only its UI/hardware adapters.

### Future survey and Deflock-style capabilities

The architecture reserves first-class navigation and service boundaries for authorized field diagnostics, including:

- Passive Wi-Fi discovery, channel occupancy, signal history, and authorized survey export.
- Passive BLE advertisement discovery, service UUID inventory, signal history, and authorized export.
- Local correlation of observations with time and user-authorized location.
- User-owned RF/environmental detection and mapping workflows inspired by the defensive transparency goals of projects such as Deflock.
- Future hardware-assisted tools behind explicit capability checks.

These features must not be spliced into Meshtastic routing or encryption. The ESP32-S3 Wi-Fi/BLE radios require an RF mode broker that coordinates coexistence, power, memory, UI redraw, and storage while preserving or explicitly pausing normal connectivity. The separate SX1262 LoRa path may remain available only where measured behavior proves it reliable.

“Bluetooth interrogation” must be decomposed into explicit operations. Passive advertisement scanning and service metadata already broadcast by devices can belong in the normal authorized survey workflow. Active connections, service enumeration, pairing, writes, spoofing, disruption, credential capture, or bypass behavior require separate threat, consent, legal, coexistence, and safety review. Disruptive or deceptive functionality stays outside the normal FriendMeshOS field build and, if ever authorized for controlled research, must use a separate conspicuously labeled lab build.

Survey data can expose device identities, homes, routines, precise locations, and infrastructure. It must be treated as sensitive: bounded retention, explicit capture state, local-first storage, redacted exports, no automatic public upload, and clear deletion controls.

## Authority and version boundary

The strongest authority for this checkout is the code that is compiled:

- Wire format and radio behavior: `src/mesh/RadioInterface.h`, `src/mesh/RadioInterface.cpp`, and `src/mesh/RadioLibInterface.cpp`.
- Packet lifecycle: `src/mesh/Router.cpp`, `FloodingRouter.cpp`, `ReliableRouter.cpp`, and `NextHopRouter.cpp`.
- Cryptography and channels: `src/mesh/CryptoEngine.cpp`, `Channels.cpp`, and `protobufs/meshtastic/channel.proto`.
- Application payloads: `protobufs/meshtastic/*.proto`, especially `mesh.proto`, `portnums.proto`, `config.proto`, `telemetry.proto`, and `deviceonly.proto`.
- Node state and position: `src/mesh/NodeDB.cpp`, `src/modules/NodeInfoModule.cpp`, `src/modules/PositionModule.cpp`, and `src/mesh/PositionPrecision.cpp`.
- Client transport: `src/mesh/PhoneAPI.cpp`, `StreamAPI.cpp`, `MeshService.cpp`, and `protobufs/meshtastic/mesh.proto` (`ToRadio` / `FromRadio`).

The public upstream references are the [official firmware](https://github.com/meshtastic/firmware), [official protobuf repository](https://github.com/meshtastic/protobufs), and [official Meshtastic documentation](https://meshtastic.org/docs/). Upstream `master` can differ from this pinned firmware, so never copy a current upstream field or behavior into FriendMeshOS without checking compatibility with the vendored schema and generated bindings.

## The packet lifecycle in one view

```text
Application/module
  -> Router::allocForSending()
  -> decoded Data protobuf (portnum + payload + request/reply metadata)
  -> Router::sendLocal()
  -> routing/priority/reliability policy
  -> perhapsEncode()
       -> channel AES-CTR OR per-peer PKI AES-256-CCM
  -> 16-byte Meshtastic radio header + encrypted payload
  -> contention delay / TX queue / LoRa radio
  -> receiving radio parses header and records RSSI/SNR
  -> duplicate detection and relay decision
  -> channel-hash candidate selection or PKI key selection
  -> perhapsDecode()
  -> module dispatch by PortNum
  -> NodeDB/UI/phone update and optional ACK/NAK/response
```

The LoRa radio never transmits the full `MeshPacket` protobuf. The radio frame has a fixed 16-byte unencrypted header followed by an encrypted or clear encoded `Data` payload. Several `MeshPacket` fields, including receive time, RSSI, SNR, transport mechanism, and transmit scheduling time, exist only inside a node or on the client link.

## Layer 1: physical radio

Meshtastic uses Semtech LoRa modulation directly; it is not LoRaWAN. There is no LoRaWAN join server, DevAddr, network session, application session, or gateway requirement.

### Physical parameters

The important LoRa parameters are:

- Region and legal frequency range.
- Frequency slot or explicit frequency override.
- Bandwidth (`bw`).
- Spreading factor (`sf`, normally 7 through 12).
- Coding-rate denominator (`cr`, such as 5 for 4/5).
- Preamble length, currently initialized to 16 symbols.
- Transmit power, bounded by hardware and regional limits.
- CRC and explicit LoRa packet behavior configured in the chipset driver.

`Config.LoRaConfig.ModemPreset` provides `LONG_FAST`, `MEDIUM_SLOW`, `MEDIUM_FAST`, `SHORT_SLOW`, `SHORT_FAST`, `LONG_MODERATE`, `SHORT_TURBO`, `LONG_TURBO`, `LITE_FAST`, `LITE_SLOW`, `NARROW_FAST`, and `NARROW_SLOW`. Deprecated slow presets remain in the schema for wire compatibility. The exact preset-to-BW/SF/CR mapping is implemented in `src/mesh/MeshRadio.cpp`; the protobuf enum alone is not sufficient.

All members of one RF mesh must agree on compatible region, frequency/slot, bandwidth, spreading factor, and coding rate. A secondary channel does not create another RF network: secondary channels share the primary channel's physical radio settings.

### Airtime and congestion

The radio calculates time-on-air from modem parameters and frame length. Airtime matters because:

- Slow presets keep the channel occupied longer.
- More hops multiply network airtime.
- Reliable retries repeat airtime.
- Position and NodeInfo broadcasts compete with chat.
- Legal duty-cycle and power limits vary by region.

`airtime.cpp` tracks channel utilization. Position and range-test modules throttle transmissions at utilization thresholds. FriendMeshOS must not add a second independent scheduler that ignores channel utilization.

### Listen-before-talk and collision avoidance

The radio layer uses channel activity detection and contention windows rather than centralized timeslots. Current constants include two CAD symbols for sub-GHz, four for 2.4 GHz, `CWmin=3`, and `CWmax=8`. Delay is randomized in slot-sized windows and can be weighted using received SNR so likely useful relays tend to transmit earlier. Router-like roles receive special timing treatment.

The retransmission delay includes twice the packet airtime, a congestion-dependent contention allowance, and an empirically selected 4500 ms processing allowance. Exact values are computed at runtime by `RadioInterface::getRetransmissionMsec()`; UI code must not promise a fixed delivery time.

## Layer 2: radio frame, identity, and MAC behavior

### Fixed 16-byte header

`PacketHeader` is the on-air header:

| Field | Size | Meaning |
| --- | ---: | --- |
| `to` | 4 | Destination node number or broadcast |
| `from` | 4 | Originating node number |
| `id` | 4 | Packet identifier; also participates in encryption nonce construction |
| `flags` | 1 | Hop limit, ACK request, MQTT traversal, and starting hop count |
| `channel` | 1 | Weak channel hash used to narrow decryption candidates |
| `next_hop` | 1 | Low byte of preferred next-hop node number |
| `relay_node` | 1 | Low byte of the latest relay node number |

Flag allocation:

- Bits 0-2: current hop limit (`0..7`).
- Bit 3: `want_ack`.
- Bit 4: `via_mqtt`.
- Bits 5-7: `hop_start` (`0..7`).

The maximum raw LoRa frame is 255 bytes. After the 16-byte header, protobuf overhead, and possibly 12 bytes of PKI overhead, applications have substantially less than 255 bytes. `Constants.DATA_PAYLOAD_LEN` advertises 233 bytes inside `Data`, but code must still use encoder results and actual packet limits.

### Node identity

A node number is a 32-bit identifier. `User.id` is normally the familiar `!` plus hexadecimal identity, while `User.long_name`, `short_name`, role, hardware model, public key, and messagability are advertised through `NODEINFO_APP`.

Node-number collision handling compares device identity/MAC information: one node retains the number and the other selects a new random number and announces itself again. FriendMesh membership must therefore use full node numbers plus public-key identity where available, never only `short_name`, display name, or the one-byte relay hint.

### Packet IDs and duplicate suppression

Packet IDs contain a rolling low portion and randomized high portion. They need to be unique per sender for the lifetime of a flood and its ACK/retry state, not globally forever.

`PacketHistory` keys observations by `(sender, packet id)`, records recent relayers, highest observed hop limit, requested next hop, and this node's original transmit hop limit. This suppresses duplicate floods, cancels redundant relays when another node transmits first, recognizes implicit broadcast ACKs, and detects upgraded copies with a better hop limit.

### Queue priority

The transmit queue is bounded (`MAX_TX_QUEUE=16`). Packets are ordered by late-window state and priority. Routing ACK/NAK packets receive ACK priority; text and admin traffic receive high priority; responses and reliable packets outrank ordinary traffic. When full, a higher-priority packet can evict a lower-priority non-late packet.

FriendMesh background updates must use ordinary/low priority and must never compete with text, ACK, admin, or SOS traffic at equal priority.

## Layer 3: routing and hops

### Managed flooding

Meshtastic fundamentally uses managed flooding:

1. A new packet is recorded in packet history.
2. A receiver decides whether it is a duplicate and whether it is eligible to rebroadcast.
3. Eligible relays schedule a randomized, often SNR-weighted delay.
4. If a node hears another relay first, it can cancel its pending copy.
5. Each ordinary relay decreases `hop_limit` until no more forwarding is allowed.

The default reliable hop limit is 3 and the hard wire maximum is 7. A hop limit of zero still permits reception by direct neighbors but no forwarding.

Current firmware includes a router-chain optimization: after the first hop, a router, router-late, or client-base node can preserve hop limit when the previous relay is a favorite node with a compatible router role. This is a specialized behavior and another reason not to equate `hop_start - hop_limit` with a universally perfect physical path length in every case.

### Direct packets and next-hop routing

`to != NODENUM_BROADCAST` makes a packet logically direct. It can still traverse relays. `NextHopRouter` learns a preferred next-hop low byte from successful traffic and NodeDB state. If a preferred path fails or is unavailable, delivery can fall back to flooding.

Only the low byte is carried in `next_hop`, so it is a routing hint, not a globally unique identity. Application code must never treat it as the target node.

### Hop meanings for the UI

There are three different concepts:

- `hop_limit`: remaining forwarding budget on this received packet.
- `hop_start`: budget at origin.
- Observed hops used: normally `hop_start - hop_limit`, subject to firmware helpers and routing optimizations.

NodeDB can retain a recent `hops_away` observation, but this is not a stable route guarantee. The Friend Compass UI should display `last heard: N hops` with age, not `N hops away` as timeless truth. For an explicit live path, invoke standard `TRACEROUTE_APP` and show forward/return routes, unknown gaps, SNR samples, timestamp, and failure state.

### Broadcast versus direct behavior

- Channel group chat is normally a broadcast destination on a selected channel.
- A direct message uses a node-number destination and may use per-peer PKI.
- Broadcasts must not generate ACK storms. A sender requesting reliability listens for another node rebroadcasting its packet; that overheard relay is an implicit ACK that the flood started.
- Direct reliable messages expect a routing ACK/NAK correlated by request packet ID.

## Layer 4: application transport, reliability, and streams

The `Data` protobuf contains:

- `portnum`: application protocol discriminator.
- `payload`: opaque bytes interpreted according to the port number.
- `want_response`: asks the destination module to respond.
- `dest` and `source`: legacy/deprecated addressing fields.
- `request_id`: correlates a response or ACK to an earlier packet ID.
- `reply_id`: correlates conversational replies where supported.
- emoji and bitfield metadata where supported by the pinned schema.

Reliability is message-level best effort, not TCP:

- No end-to-end byte stream exists over LoRa.
- A reliable direct packet can be retried until ACK, NAK, or retry exhaustion.
- A reliable broadcast receives only the implicit flood-start observation.
- An ACK proves protocol handling for a packet, not that a human read a message.
- Duplicate suppression makes retries mostly idempotent at the mesh layer, but custom application actions still need their own operation IDs if replay would be harmful.

`Routing.Error` carries positive success (`NONE`) and failure states such as no route, no interface, timeout, malformed request, duty-cycle or queue conditions depending on the pinned enum. FriendMeshOS must render the actual enum and preserve unknown future values rather than collapsing every failure into “not sent.”

### Core port numbers relevant to FriendMeshOS

| Port | Value | Payload |
| --- | ---: | --- |
| `TEXT_MESSAGE_APP` | 1 | UTF-8 text |
| `POSITION_APP` | 3 | `Position` protobuf |
| `NODEINFO_APP` | 4 | `User` protobuf |
| `ROUTING_APP` | 5 | `Routing` ACK/NAK protobuf |
| `ADMIN_APP` | 6 | `AdminMessage` protobuf |
| `WAYPOINT_APP` | 8 | `Waypoint` protobuf |
| `ALERT_APP` | 11 | Critical-alert text semantics |
| `KEY_VERIFICATION_APP` | 12 | PKI verification flow |
| `TELEMETRY_APP` | 67 | `Telemetry` protobuf |
| `TRACEROUTE_APP` | 70 | `RouteDiscovery` protobuf |
| `NEIGHBORINFO_APP` | 71 | Neighbor graph protobuf |
| `PRIVATE_APP` | 256 | Private application space in this schema |

Early FriendMesh functionality should reuse standard ports. A private FriendMesh payload is justified only for metadata that cannot be represented locally or through existing Meshtastic messages. Standard text and position must remain the interoperability path.

## Encryption and trust model

### Channel encryption

Each enabled channel has a name and PSK. Valid explicit PSKs are 16 bytes (AES-128) or 32 bytes (AES-256). A zero-length primary PSK means encryption is disabled. A secondary channel with no PSK inherits the primary key. One-byte keys are shorthand indexes into publicly known default 128-bit keys and are not private.

Channel payload encryption uses AES-CTR. Its 16-byte nonce is formed from packet ID and originating node number, with zero-filled remaining bytes. CTR encryption and decryption are the same operation.

Security consequences:

- Anyone with the channel PSK can decrypt and create channel traffic.
- The public/default key prevents no meaningful eavesdropping because it is published in source.
- AES-CTR provides confidentiality but not cryptographic authentication or integrity.
- The one-byte channel hash is only an XOR-derived decoder hint, not an authenticator.
- Node names, addressing, packet IDs, flags, channel hash, and relay hints remain visible in the radio header.
- A closed FriendMesh group therefore requires a random 32-byte PSK and careful key distribution, but it still should not be described as a strongly authenticated membership protocol.

### Per-peer PKI

Nodes can advertise a 32-byte X25519 public key in `User.public_key`. For direct messages:

1. Sender combines its private key with recipient public key using X25519.
2. The 32-byte shared secret is hashed with SHA-256.
3. Payload is encrypted using AES-256-CCM.
4. Nonce uses sender node number, packet ID, and a fresh 32-bit extra nonce.
5. Wire overhead is 12 bytes: an 8-byte CCM authentication tag plus the 4-byte extra nonce.

PKI provides authenticated encryption of the payload relative to the derived shared key. Identity still depends on receiving the correct public key. `KEY_VERIFICATION_APP` provides a human-verifiable security-number flow and should be surfaced for trusted-friend verification.

### Key lifecycle hazards

The private key is persistent device identity. A full factory reset that erases BLE bonds also removes it and generates a new pair. Peers retaining the old public key can no longer decrypt new direct messages until NodeInfo is exchanged. A partial configuration reset preserves it. FriendMeshOS must warn before key rotation and must never rotate keys as part of leaving a group.

Licensed amateur-radio mode disables channel encryption and PKI behavior as required by the firmware's operating rules. FriendMeshOS must not offer “closed group” claims while licensed mode is active.

### Key-distribution rule for groups

Group PSKs must be generated with a cryptographically secure RNG and transferred through the existing channel URL/QR/manual import mechanisms or another authenticated out-of-band path. Never derive a group PSK from a group name, PIN, human password, member names, or public channel key. Never write PSKs into FriendMesh group metadata, screenshots, logs, exports, or Git.

## Protobuf architecture

Protobuf field numbers are the wire contract. Names and generated-language APIs can change while field numbers remain compatible. Never reuse a removed field number, change a field's wire type, or renumber enums. Unknown fields should be allowed to pass through compatible clients where the runtime supports it.

Important schema families:

- `mesh.proto`: `Position`, `User`, `Routing`, `RouteDiscovery`, `Data`, `MeshPacket`, `NodeInfo`, `ToRadio`, `FromRadio`, neighbor information, waypoint data, priorities, and transport mechanisms.
- `channel.proto`: primary/secondary roles, PSKs, names, channel IDs, MQTT uplink/downlink flags, per-channel position precision and muting.
- `config.proto`: device roles, rebroadcast modes, position flags, regions, modem presets, hop limit, radio configuration, security keys, and transport configuration.
- `portnums.proto`: mapping from application number to payload encoding.
- `telemetry.proto`: device, environment, air-quality, power, health, and local statistics variants.
- `deviceonly.proto`: compact on-device forms such as `NodeInfoLite`, `UserLite`, and `PositionLite`.
- `admin.proto`: configuration transactions, channel mutation, owner/security operations, and remote administration.

Nanopb generates fixed/limited C structures for embedded use. Maximum field sizes, `has_` presence flags, `oneof` tags, and repeated-array capacities matter. Do not reason from a desktop protobuf implementation alone.

### Relevant enum discipline

Before adding a FriendMesh enum:

1. Search the `.proto` source and generated C headers.
2. Identify the zero/default meaning.
3. Preserve numeric values forever once shipped.
4. Add an explicit unknown/fallback UI state.
5. Check whether the field is on-air, client-only, or device-only.
6. Regenerate with `bin/regen-protos.sh` if a vendored `.proto` changes.
7. Test older clients and nodes before depending on the new value.

Core enum families that affect FriendMesh behavior include `PortNum`, `MeshPacket.Priority`, `MeshPacket.Delayed`, `TransportMechanism`, `Routing.Error`, `Channel.Role`, `DeviceConfig.Role`, `RebroadcastMode`, `PositionFlags`, `Position.LocSource`, `Position.AltSource`, `RegionCode`, `ModemPreset`, `HardwareModel`, and telemetry payload variants.

## NodeDB, discovery, and presence

`NodeDB` is the local, bounded, eventually consistent view of other nodes. A node record can contain:

- Full node number.
- `UserLite` identity, names, role, public key, and hardware model.
- Latest position and position time.
- Last-heard time.
- SNR and hops-away observations.
- Channel index on which the node was learned.
- Device metrics and favorite/ignored flags.
- Next-hop and relay observations.

NodeDB is not a membership authority and not a synchronized database. Records can be stale, evicted, incomplete, learned on another channel, or lack a public key/position. FriendMesh membership must persist independently as references to node identities and must gracefully resolve those references against current NodeDB state.

NodeInfo broadcasts announce identity and public keys. On channel changes the firmware can request fresh NodeInfo. FriendMeshOS should reuse that mechanism rather than inventing a parallel identity broadcast.

## Position, map, distance, and heading

`Position` uses signed integers where latitude/longitude are degrees multiplied by `1e7`. It can include MSL/HAE altitude, solution timestamp, acquisition source, DOP/accuracy, ground speed, ground track, fix quality/type, satellite count, expected next update, sequence number, and transmitted precision bits.

Position is normally broadcast through `POSITION_APP`. Each channel controls `position_precision`; zero suppresses position on that channel and lower precision intentionally truncates coordinates. FriendMesh group UI must make this privacy setting visible and must never infer or restore stripped precision.

### Distance

For the Friend Compass MVP:

- Convert `latitude_i` and `longitude_i` to radians.
- Use haversine or another well-tested great-circle formula.
- Display kilometers with precision appropriate to GPS and channel precision.
- For short distances, optionally show meters, but never imply centimeter accuracy.
- Show position age and source beside distance.
- Treat missing, zeroed, suppressed, or stale coordinates as unavailable.

Distance is geometric straight-line surface distance, not route length and not radio range.

### Bearing and heading

Calculate initial great-circle bearing from local position to target. A north-up arrow needs no device heading. A body-relative arrow requires a valid heading provider:

- `NORTH`: screen is north-up; guaranteed fallback.
- `GPS`: ground track while moving; not reliable stationary orientation.
- `MAG`: calibrated magnetometer plus declination handling.

Never label GPS course as compass heading while stationary.

### Presence freshness

Use the newest valid position timestamp, receive time, sequence information, and advertised `next_update` when available. Define UI freshness bands, but keep their thresholds configurable and distinguish:

- Node recently heard, position fresh.
- Node recently heard, position stale.
- Node not recently heard, last position retained.
- Identity known, no position shared.

## Client/device transport

Phones and local applications communicate with the radio using protobuf messages, usually `ToRadio` and `FromRadio`, over BLE, serial, TCP, or another stream transport. `PhoneAPI` and `StreamAPI` frame and dispatch these messages. The client link is not the LoRa wire format.

Configuration download is stateful: clients request config and receive owner, channels, configs, modules, and NodeDB records, ending with a completion marker. Configuration edits can use admin transactions. FriendMeshOS UI code running on-device should call existing firmware services and configuration paths rather than manually serializing `ToRadio` messages to itself.

## FriendMesh group architecture

### Required model

Use standard Meshtastic constructs:

- One ordinary primary channel defines the RF parameters.
- Each closed FriendMesh group maps to one enabled secondary channel.
- Each group uses a random 32-byte AES-256 PSK.
- Group chat uses broadcast `TEXT_MESSAGE_APP` on that channel.
- Group positions use `POSITION_APP` on that channel, respecting its precision.
- One person can join multiple groups by enabling multiple secondary channels, subject to the firmware's channel-table capacity.
- Per-person private messages use standard PKI direct messaging when verified keys are available.

FriendMesh metadata should be a versioned local record containing group ID, display name, color/icon, channel index/reference, ordered member references, favorite targets, UI preferences, and schema version. It must not duplicate PSKs.

### Membership semantics

A channel PSK grants the technical ability to read and inject channel traffic. A local member list is a UX roster, not cryptographic access control. Therefore:

- Removing someone from the local roster does not revoke their key.
- Real revocation requires rotating the group channel PSK and securely redistributing it to remaining members.
- Receiving a packet with a known group PSK does not prove it came from the person named in an editable NodeInfo record.
- Verified per-peer public keys can strengthen direct-message identity but do not authenticate AES-CTR group broadcasts.

The UI must explain these boundaries honestly.

### Multiple-group receive and send

All enabled secondary channels share the same RF traffic. On receive, the one-byte hash narrows candidate channel keys and successful decoding maps the packet to a local channel index. On send, the user explicitly selects a group/channel. Do not scan separate frequencies per group.

If channel-table capacity is exhausted, FriendMeshOS must refuse creation/import with a clear explanation. It must not silently overwrite another channel.

### Group discovery and joining

Recommended flow:

1. Group owner creates a secondary channel with secure random PSK and position precision.
2. Existing Meshtastic channel URL encodes shareable channel configuration.
3. Recipient previews name, channel slot impact, privacy settings, and whether an existing slot will change.
4. Recipient explicitly confirms import.
5. FriendMesh local metadata is created referencing the resulting channel.
6. Nodes exchange standard NodeInfo and Position messages.
7. Members optionally perform per-peer key verification for trusted DMs.

No FriendMesh-specific over-air join protocol is needed for the MVP.

### Compass and map data model

For each group member resolve:

- Stable full node number and known public-key fingerprint.
- Current NodeDB identity.
- Latest position and its source/precision/age.
- Last-heard age.
- SNR/RSSI from the latest direct reception where available.
- Last observed hop count and observation age.
- Optional last traceroute result, forward/return path, and age.

Map markers must be grouped/filterable by FriendMesh group, but a person in several groups should remain one node identity with multiple group badges—not duplicated physical people.

### SOS architecture

The interoperable MVP sends a standard text alert on the selected private group channel with `want_ack` as appropriate. Include sender, timestamp, position availability, and coordinates only when sharing policy permits. `ALERT_APP` can be evaluated for compatible critical-alert presentation, but no custom payload may be the only alert path.

SOS delivery is best effort. Broadcast implicit ACK means a relay was heard, not that every member or emergency service received it. Cancellation and acknowledgment require explicit application state and operation IDs.

## What FriendMeshOS must not change for the MVP

- The 16-byte radio header.
- Packet ID or nonce construction.
- Flooding, retry, or ACK semantics.
- Standard `TEXT_MESSAGE_APP`, `POSITION_APP`, `NODEINFO_APP`, or traceroute schemas.
- Channel PSK storage or channel URL format.
- Node-number allocation/collision behavior.
- LoRa modem presets or region rules.
- Full-reset key-rotation behavior.

These are compatibility boundaries. Build the group and compass experience above them first.

## Implementation phases

### Phase A: protocol observability

- Add read-only diagnostic views/logging for packet channel, port, destination type, hop start/remaining, observed hops, ACK/NAK, position age/source, and public-key availability.
- Add tests for hop calculations, stale positions, missing keys, invalid channel references, and unknown enum rendering.
- Never log PSKs, private keys, decrypted message bodies, precise positions, or channel URLs by default.

### Phase B: local FriendMesh model

- Implement versioned local group/member metadata without keys.
- Resolve members through NodeDB by full node number and key fingerprint.
- Support one node in multiple groups.
- Handle deleted/evicted NodeDB entries without deleting membership.

### Phase C: standard group chat and joining

- Create/import secondary channels through existing config APIs.
- Bind group chat to standard channel text messages.
- Surface channel capacity and PSK-rotation implications.
- Test with unmodified Meshtastic nodes and phone clients.

### Phase D: map and north-up compass

- Consume standard position state.
- Implement distance/bearing in a pure tested service.
- Add freshness/privacy states.
- Display last observed hop count with age and optional traceroute.

### Phase E: verified friends and PKI DMs

- Surface public-key fingerprints and key-verification state.
- Use standard PKI direct messages.
- Detect key changes and require re-verification.
- Never auto-accept a rotated identity because the display name matches.

### Phase F: SOS and richer metadata

- Ship interoperable standard-text SOS first.
- Add explicit ACK/cancel state with replay-safe IDs only after the base path is proven.
- Consider a registered/private protobuf only for additive metadata.

## Verification matrix

Every phase must test:

- Direct neighbor and multi-hop delivery.
- Broadcast group chat and PKI direct chat.
- Reliable success, explicit NAK, timeout, retry, queue pressure, and duplicate reception.
- Multiple enabled secondary groups.
- Node in multiple FriendMesh groups.
- Missing/stale NodeDB, position, public key, or route.
- Invalid and future enum values.
- Stock Meshtastic sender/receiver interoperability.
- Phone configuration round trips.
- Reboot persistence without factory reset.
- Group removal without key rotation and explicit PSK rotation/rejoin.
- Public/default channel versus secure random group channel.
- Licensed/Ham mode with encryption unavailable.
- Channel utilization and battery impact under periodic position traffic.

## Research checklist before protocol changes

Before modifying core behavior, answer and record:

1. Is this value transmitted over LoRa, over the client link, or local only?
2. Is it in the fixed header, encrypted `Data`, or another protobuf envelope?
3. Which router class observes or mutates it?
4. Does it affect nonce uniqueness, duplicate identity, ACK correlation, or retry cancellation?
5. Which older clients/nodes will see it?
6. What happens when the enum/field is unknown?
7. Does it increase airtime or contention?
8. Does it reveal identity, membership, keys, or location?
9. Can standard Meshtastic behavior already express the feature?
10. What physical multi-node test proves it safely?
