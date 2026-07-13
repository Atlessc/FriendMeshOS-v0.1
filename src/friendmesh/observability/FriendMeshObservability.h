#pragma once

#include "mesh/generated/meshtastic/mesh.pb.h"
#include <cstdint>

namespace friendmesh::observability
{

constexpr uint32_t POSITION_STALE_SECONDS = 5U * 60U;

enum class DestinationType : uint8_t { INVALID, BROADCAST, LOCAL_TRANSPORT_BROADCAST, DIRECT };
enum class PayloadType : uint8_t { DECODED, ENCRYPTED, UNKNOWN };
enum class RoutingResult : uint8_t { NOT_ROUTING, ACK, NAK, MALFORMED };
enum class QueueState : uint8_t { UNAVAILABLE, READY, FULL, INVALID };
enum class Freshness : uint8_t { UNKNOWN, FRESH, STALE };

struct Age
{
    bool known = false;
    bool clockSkew = false;
    uint32_t seconds = 0;
};

struct PacketObservation
{
    meshtastic_PortNum port = meshtastic_PortNum_UNKNOWN_APP;
    DestinationType destination = DestinationType::INVALID;
    PayloadType payload = PayloadType::UNKNOWN;
    RoutingResult routing = RoutingResult::NOT_ROUTING;
    meshtastic_Routing_Error routingError = meshtastic_Routing_Error_NONE;
    uint8_t channel = 0;
    bool wantsAck = false;
    bool pkiEncrypted = false;
    int8_t observedHops = -1;
    Age receiveAge;
    meshtastic_MeshPacket_TransportMechanism transport =
        meshtastic_MeshPacket_TransportMechanism_TRANSPORT_INTERNAL;
};

struct QueueObservation
{
    QueueState state = QueueState::UNAVAILABLE;
    int8_t result = 0;
    uint8_t free = 0;
    uint8_t capacity = 0;
    uint32_t packetId = 0;
};

struct PositionObservation
{
    Freshness position = Freshness::UNKNOWN;
    Freshness lastHeard = Freshness::UNKNOWN;
    Age positionAge;
    Age lastHeardAge;
    meshtastic_Position_LocSource source = meshtastic_Position_LocSource_LOC_UNSET;
};

Age ageAt(uint32_t nowSeconds, uint32_t timestampSeconds);
int8_t observedHops(const meshtastic_MeshPacket &packet, int8_t unknownValue = -1);
PacketObservation inspectPacket(const meshtastic_MeshPacket &packet, uint32_t nowSeconds);
QueueObservation inspectQueue(const meshtastic_QueueStatus &queue);
PositionObservation inspectPosition(bool hasPosition, uint32_t positionTimestamp, uint32_t lastHeardTimestamp,
                                    uint32_t nowSeconds, uint32_t staleAfterSeconds = POSITION_STALE_SECONDS,
                                    meshtastic_Position_LocSource source = meshtastic_Position_LocSource_LOC_UNSET);

} // namespace friendmesh::observability
