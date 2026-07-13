#include "DiagnosticFormatter.h"

#include <cstdarg>
#include <cstdio>

namespace friendmesh::observability
{

const char *destinationName(DestinationType value)
{
    switch (value) {
    case DestinationType::DIRECT:
        return "DIRECT";
    case DestinationType::BROADCAST:
        return "BROADCAST";
    case DestinationType::LOCAL_TRANSPORT_BROADCAST:
        return "LOCAL";
    default:
        return "INVALID";
    }
}

const char *payloadName(PayloadType value)
{
    switch (value) {
    case PayloadType::DECODED:
        return "DECODED";
    case PayloadType::ENCRYPTED:
        return "ENCRYPTED";
    default:
        return "UNKNOWN";
    }
}

const char *routingResultName(RoutingResult value)
{
    switch (value) {
    case RoutingResult::ACK:
        return "ACK";
    case RoutingResult::NAK:
        return "NAK";
    case RoutingResult::MALFORMED:
        return "MALFORMED";
    default:
        return "N/A";
    }
}

const char *routingErrorName(meshtastic_Routing_Error value)
{
    switch (value) {
    case meshtastic_Routing_Error_NONE:
        return "NONE";
    case meshtastic_Routing_Error_NO_ROUTE:
        return "NO_ROUTE";
    case meshtastic_Routing_Error_GOT_NAK:
        return "GOT_NAK";
    case meshtastic_Routing_Error_TIMEOUT:
        return "TIMEOUT";
    case meshtastic_Routing_Error_NO_INTERFACE:
        return "NO_INTERFACE";
    case meshtastic_Routing_Error_MAX_RETRANSMIT:
        return "MAX_RETRANSMIT";
    case meshtastic_Routing_Error_NO_CHANNEL:
        return "NO_CHANNEL";
    case meshtastic_Routing_Error_TOO_LARGE:
        return "TOO_LARGE";
    case meshtastic_Routing_Error_NO_RESPONSE:
        return "NO_RESPONSE";
    case meshtastic_Routing_Error_DUTY_CYCLE_LIMIT:
        return "DUTY_CYCLE_LIMIT";
    case meshtastic_Routing_Error_BAD_REQUEST:
        return "BAD_REQUEST";
    case meshtastic_Routing_Error_NOT_AUTHORIZED:
        return "NOT_AUTHORIZED";
    case meshtastic_Routing_Error_PKI_FAILED:
        return "PKI_FAILED";
    case meshtastic_Routing_Error_PKI_UNKNOWN_PUBKEY:
        return "PKI_UNKNOWN_PUBKEY";
    default:
        return "OTHER";
    }
}

const char *queueStateName(QueueState value)
{
    switch (value) {
    case QueueState::READY:
        return "READY";
    case QueueState::FULL:
        return "FULL";
    case QueueState::INVALID:
        return "INVALID";
    default:
        return "UNAVAILABLE";
    }
}

const char *freshnessName(Freshness value)
{
    switch (value) {
    case Freshness::FRESH:
        return "FRESH";
    case Freshness::STALE:
        return "STALE";
    default:
        return "UNKNOWN";
    }
}

const char *portName(meshtastic_PortNum value)
{
    switch (value) {
    case meshtastic_PortNum_TEXT_MESSAGE_APP:
        return "TEXT_MESSAGE";
    case meshtastic_PortNum_POSITION_APP:
        return "POSITION";
    case meshtastic_PortNum_NODEINFO_APP:
        return "NODEINFO";
    case meshtastic_PortNum_ROUTING_APP:
        return "ROUTING";
    case meshtastic_PortNum_ADMIN_APP:
        return "ADMIN";
    case meshtastic_PortNum_TELEMETRY_APP:
        return "TELEMETRY";
    case meshtastic_PortNum_TRACEROUTE_APP:
        return "TRACEROUTE";
    case meshtastic_PortNum_PRIVATE_APP:
        return "PRIVATE_APP";
    default:
        return "OTHER";
    }
}

const char *transportName(meshtastic_MeshPacket_TransportMechanism value)
{
    switch (value) {
    case meshtastic_MeshPacket_TransportMechanism_TRANSPORT_INTERNAL:
        return "INTERNAL";
    case meshtastic_MeshPacket_TransportMechanism_TRANSPORT_LORA:
        return "LORA";
    case meshtastic_MeshPacket_TransportMechanism_TRANSPORT_LORA_ALT1:
        return "LORA_ALT1";
    case meshtastic_MeshPacket_TransportMechanism_TRANSPORT_LORA_ALT2:
        return "LORA_ALT2";
    case meshtastic_MeshPacket_TransportMechanism_TRANSPORT_LORA_ALT3:
        return "LORA_ALT3";
    case meshtastic_MeshPacket_TransportMechanism_TRANSPORT_MQTT:
        return "MQTT";
    case meshtastic_MeshPacket_TransportMechanism_TRANSPORT_MULTICAST_UDP:
        return "MULTICAST_UDP";
    case meshtastic_MeshPacket_TransportMechanism_TRANSPORT_API:
        return "API";
    default:
        return "UNKNOWN";
    }
}

const char *positionSourceName(meshtastic_Position_LocSource value)
{
    switch (value) {
    case meshtastic_Position_LocSource_LOC_MANUAL:
        return "MANUAL";
    case meshtastic_Position_LocSource_LOC_INTERNAL:
        return "INTERNAL_GPS";
    case meshtastic_Position_LocSource_LOC_EXTERNAL:
        return "EXTERNAL";
    default:
        return "UNSET";
    }
}

class BufferWriter
{
  public:
    BufferWriter(char *output, size_t outputSize) : output(output), outputSize(outputSize)
    {
        if (output && outputSize > 0) {
            output[0] = '\0';
        }
    }

    void append(const char *format, ...)
    {
        if (!output || used >= outputSize) {
            return;
        }
        va_list args;
        va_start(args, format);
        const int written = std::vsnprintf(output + used, outputSize - used, format, args);
        va_end(args);
        if (written < 0) {
            valid = false;
            return;
        }
        if (static_cast<size_t>(written) >= outputSize - used) {
            used = outputSize;
            valid = false;
            return;
        }
        used += static_cast<size_t>(written);
    }

    bool ok() const { return valid; }

  private:
    char *output;
    size_t outputSize;
    size_t used = 0;
    bool valid = true;
};

static void appendAge(BufferWriter &writer, const char *label, const Age &age)
{
    if (!age.known) {
        writer.append("%s: UNKNOWN\n", label);
    } else if (age.clockSkew) {
        writer.append("%s: CLOCK_SKEW\n", label);
    } else {
        writer.append("%s: %lus\n", label, static_cast<unsigned long>(age.seconds));
    }
}

bool formatDiagnosticEvent(const DiagnosticEvent &event, char *output, size_t outputSize)
{
    BufferWriter writer(output, outputSize);
    writer.append("Event #%lu\nCaptured: %lu\n", static_cast<unsigned long>(event.sequence),
                  static_cast<unsigned long>(event.capturedAt));

    if (event.type == DiagnosticEventType::PACKET) {
        writer.append("Type: PACKET\nPort: %s (%u)\nChannel index: %u\nDestination: %s\nPayload: %s\nTransport: %s\n",
                      portName(event.packet.port), static_cast<unsigned>(event.packet.port),
                      static_cast<unsigned>(event.packet.channel), destinationName(event.packet.destination),
                      payloadName(event.packet.payload), transportName(event.packet.transport));
        if (event.packet.observedHops < 0) {
            writer.append("Observed hops: UNKNOWN\n");
        } else {
            writer.append("Observed hops: %d\n", event.packet.observedHops);
        }
        writer.append("Want ACK: %s\nPKI encrypted: %s\nRouting: %s\nError: %s (%u)\n",
                      event.packet.wantsAck ? "YES" : "NO", event.packet.pkiEncrypted ? "YES" : "NO",
                      routingResultName(event.packet.routing), routingErrorName(event.packet.routingError),
                      static_cast<unsigned>(event.packet.routingError));
        appendAge(writer, "Receive age", event.packet.receiveAge);
    } else if (event.type == DiagnosticEventType::QUEUE) {
        writer.append("Type: QUEUE\nState: %s\nSlots: %u free / %u\nResult code: %d\n",
                      queueStateName(event.queue.state), static_cast<unsigned>(event.queue.free),
                      static_cast<unsigned>(event.queue.capacity), event.queue.result);
    } else {
        writer.append("Type: POSITION\nSource: %s\nPosition: %s\nLast heard: %s\nCoordinates: REDACTED\n",
                      positionSourceName(event.position.source), freshnessName(event.position.position),
                      freshnessName(event.position.lastHeard));
        appendAge(writer, "Position age", event.position.positionAge);
        appendAge(writer, "Last-heard age", event.position.lastHeardAge);
    }

    writer.append("Redacted: IDs, bodies, keys, coordinates\n");
    return writer.ok();
}

} // namespace friendmesh::observability
