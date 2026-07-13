#pragma once

#include "DiagnosticEventStore.h"
#include <cstddef>

namespace friendmesh::observability
{

const char *destinationName(DestinationType value);
const char *payloadName(PayloadType value);
const char *routingResultName(RoutingResult value);
const char *routingErrorName(meshtastic_Routing_Error value);
const char *queueStateName(QueueState value);
const char *freshnessName(Freshness value);
const char *portName(meshtastic_PortNum value);
const char *transportName(meshtastic_MeshPacket_TransportMechanism value);
const char *positionSourceName(meshtastic_Position_LocSource value);
bool formatDiagnosticEvent(const DiagnosticEvent &event, char *output, size_t outputSize);

} // namespace friendmesh::observability
