#include "../test/fixtures/MeshtasticV2726Vectors.h"

#include "mesh/generated/meshtastic/mesh.pb.h"
#include "mesh/generated/meshtastic/telemetry.pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include <cstdio>
#include <cstring>

using namespace friendmesh::compatibility::v2726;

template <typename Message, size_t N>
static bool roundTrips(const char *name, const uint8_t (&bytes)[N], const pb_msgdesc_t &descriptor)
{
    Message message{};
    uint8_t encoded[256] = {};
    pb_istream_t input = pb_istream_from_buffer(bytes, N);
    if (!pb_decode(&input, &descriptor, &message)) {
        std::fprintf(stderr, "%s decode failed\n", name);
        return false;
    }
    pb_ostream_t output = pb_ostream_from_buffer(encoded, sizeof(encoded));
    if (!pb_encode(&output, &descriptor, &message)) {
        std::fprintf(stderr, "%s encode failed\n", name);
        return false;
    }
    const size_t size = output.bytes_written;
    if (size != N || std::memcmp(bytes, encoded, N) != 0) {
        std::fprintf(stderr, "%s round-trip mismatch\n", name);
        return false;
    }
    return true;
}

int main()
{
    bool valid = true;
    valid &= roundTrips<meshtastic_MeshPacket>("public text", PUBLIC_TEXT, meshtastic_MeshPacket_msg);
    valid &= roundTrips<meshtastic_MeshPacket>("channel text", CHANNEL_TEXT, meshtastic_MeshPacket_msg);
    valid &= roundTrips<meshtastic_Position>("position", POSITION, meshtastic_Position_msg);
    valid &= roundTrips<meshtastic_NodeInfo>("node info", NODE_INFO, meshtastic_NodeInfo_msg);
    valid &= roundTrips<meshtastic_Telemetry>("telemetry", TELEMETRY, meshtastic_Telemetry_msg);
    valid &= roundTrips<meshtastic_RouteDiscovery>("traceroute", TRACEROUTE, meshtastic_RouteDiscovery_msg);
    valid &= roundTrips<meshtastic_Routing>("routing ACK", ROUTING_ACK, meshtastic_Routing_msg);
    valid &= roundTrips<meshtastic_Routing>("routing NAK", ROUTING_NAK, meshtastic_Routing_msg);
    valid &= roundTrips<meshtastic_ToRadio>("BLE config request", BLE_TO_RADIO_CONFIG, meshtastic_ToRadio_msg);
    valid &= roundTrips<meshtastic_FromRadio>("BLE config complete", BLE_FROM_RADIO_COMPLETE, meshtastic_FromRadio_msg);
    if (valid) {
        std::puts("Meshtastic v2.7.26 protobuf vectors passed");
    }
    return valid ? 0 : 1;
}
