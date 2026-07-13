#include "FriendMeshObservability.h"

#include "mesh/MeshTypes.h"
#include "mesh/NodeDB.h"
#include "pb_decode.h"

namespace friendmesh::observability
{

Age ageAt(uint32_t nowSeconds, uint32_t timestampSeconds)
{
    if (timestampSeconds == 0) {
        return {};
    }
    if (timestampSeconds > nowSeconds) {
        return {true, true, 0};
    }
    return {true, false, nowSeconds - timestampSeconds};
}

int8_t observedHops(const meshtastic_MeshPacket &packet, int8_t unknownValue)
{
    return getHopsAway(packet, unknownValue);
}

static DestinationType destinationType(uint32_t destination)
{
    if (destination == 0) {
        return DestinationType::INVALID;
    }
    if (destination == NODENUM_BROADCAST) {
        return DestinationType::BROADCAST;
    }
    if (destination == NODENUM_BROADCAST_NO_LORA) {
        return DestinationType::LOCAL_TRANSPORT_BROADCAST;
    }
    return DestinationType::DIRECT;
}

static void inspectRouting(const meshtastic_MeshPacket &packet, PacketObservation &result)
{
    if (result.payload != PayloadType::DECODED || result.port != meshtastic_PortNum_ROUTING_APP) {
        return;
    }

    meshtastic_Routing routing = meshtastic_Routing_init_default;
    if (!pb_decode_from_bytes(packet.decoded.payload.bytes, packet.decoded.payload.size, &meshtastic_Routing_msg, &routing) ||
        routing.which_variant != meshtastic_Routing_error_reason_tag) {
        result.routing = RoutingResult::MALFORMED;
        return;
    }

    result.routingError = routing.error_reason;
    result.routing = routing.error_reason == meshtastic_Routing_Error_NONE ? RoutingResult::ACK : RoutingResult::NAK;
}

PacketObservation inspectPacket(const meshtastic_MeshPacket &packet, uint32_t nowSeconds)
{
    PacketObservation result;
    result.destination = destinationType(packet.to);
    result.channel = packet.channel;
    result.wantsAck = packet.want_ack;
    result.pkiEncrypted = packet.pki_encrypted;
    result.observedHops = observedHops(packet);
    result.receiveAge = ageAt(nowSeconds, packet.rx_time);
    result.transport = packet.transport_mechanism;

    if (packet.which_payload_variant == meshtastic_MeshPacket_decoded_tag) {
        result.payload = PayloadType::DECODED;
        result.port = packet.decoded.portnum;
    } else if (packet.which_payload_variant == meshtastic_MeshPacket_encrypted_tag) {
        result.payload = PayloadType::ENCRYPTED;
    }

    inspectRouting(packet, result);
    return result;
}

QueueObservation inspectQueue(const meshtastic_QueueStatus &queue)
{
    QueueObservation result;
    result.result = queue.res;
    result.free = queue.free;
    result.capacity = queue.maxlen;
    result.packetId = queue.mesh_packet_id;

    if (queue.maxlen == 0) {
        result.state = QueueState::UNAVAILABLE;
    } else if (queue.free > queue.maxlen) {
        result.state = QueueState::INVALID;
    } else if (queue.free == 0) {
        result.state = QueueState::FULL;
    } else {
        result.state = QueueState::READY;
    }
    return result;
}

static Freshness freshness(const Age &age, uint32_t staleAfterSeconds)
{
    if (!age.known || age.clockSkew) {
        return Freshness::UNKNOWN;
    }
    return age.seconds > staleAfterSeconds ? Freshness::STALE : Freshness::FRESH;
}

PositionObservation inspectPosition(bool hasPosition, uint32_t positionTimestamp, uint32_t lastHeardTimestamp,
                                    uint32_t nowSeconds, uint32_t staleAfterSeconds,
                                    meshtastic_Position_LocSource source)
{
    PositionObservation result;
    result.source = source;
    if (hasPosition) {
        result.positionAge = ageAt(nowSeconds, positionTimestamp);
        result.position = freshness(result.positionAge, staleAfterSeconds);
    }
    result.lastHeardAge = ageAt(nowSeconds, lastHeardTimestamp);
    result.lastHeard = freshness(result.lastHeardAge, staleAfterSeconds);
    return result;
}

} // namespace friendmesh::observability
