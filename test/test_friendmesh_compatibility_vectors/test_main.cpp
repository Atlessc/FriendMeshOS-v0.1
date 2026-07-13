#include "../fixtures/MeshtasticV2726Vectors.h"

#include "TestUtil.h"
#include "mesh/generated/meshtastic/mesh.pb.h"
#include "mesh/generated/meshtastic/telemetry.pb.h"
#include "mesh/mesh-pb-constants.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include <cstring>
#include <unity.h>

using namespace friendmesh::compatibility::v2726;

template <typename Message, size_t N>
static Message decodeAndRequireRoundTrip(const uint8_t (&bytes)[N], const pb_msgdesc_t &descriptor)
{
    Message message{};
    TEST_ASSERT_TRUE(pb_decode_from_bytes(bytes, N, &descriptor, &message));

    uint8_t encoded[256] = {};
    const size_t encodedSize = pb_encode_to_bytes(encoded, sizeof(encoded), &descriptor, &message);
    TEST_ASSERT_EQUAL_UINT32(N, encodedSize);
    TEST_ASSERT_EQUAL_MEMORY(bytes, encoded, N);
    return message;
}

static void test_public_and_private_channel_text_vectors()
{
    const auto publicPacket =
        decodeAndRequireRoundTrip<meshtastic_MeshPacket>(PUBLIC_TEXT, meshtastic_MeshPacket_msg);
    TEST_ASSERT_EQUAL_UINT32(SYNTHETIC_NODE, publicPacket.from);
    TEST_ASSERT_EQUAL_UINT8(0, publicPacket.channel);
    TEST_ASSERT_EQUAL(meshtastic_PortNum_TEXT_MESSAGE_APP, publicPacket.decoded.portnum);
    TEST_ASSERT_EQUAL_STRING_LEN("public-vector", publicPacket.decoded.payload.bytes, publicPacket.decoded.payload.size);

    const auto privatePacket =
        decodeAndRequireRoundTrip<meshtastic_MeshPacket>(CHANNEL_TEXT, meshtastic_MeshPacket_msg);
    TEST_ASSERT_EQUAL_UINT8(2, privatePacket.channel);
    TEST_ASSERT_EQUAL_STRING_LEN("private-vector", privatePacket.decoded.payload.bytes, privatePacket.decoded.payload.size);
}

static void test_position_node_info_and_telemetry_vectors()
{
    const auto position = decodeAndRequireRoundTrip<meshtastic_Position>(POSITION, meshtastic_Position_msg);
    TEST_ASSERT_EQUAL_INT32(377749000, position.latitude_i);
    TEST_ASSERT_EQUAL_INT32(-1224194000, position.longitude_i);
    TEST_ASSERT_EQUAL_UINT32(32, position.precision_bits);

    const auto nodeInfo = decodeAndRequireRoundTrip<meshtastic_NodeInfo>(NODE_INFO, meshtastic_NodeInfo_msg);
    TEST_ASSERT_TRUE(nodeInfo.has_user);
    TEST_ASSERT_EQUAL_STRING("Vector Node", nodeInfo.user.long_name);
    TEST_ASSERT_TRUE(nodeInfo.is_key_manually_verified);
    TEST_ASSERT_EQUAL_UINT8(1, nodeInfo.hops_away);

    const auto telemetry = decodeAndRequireRoundTrip<meshtastic_Telemetry>(TELEMETRY, meshtastic_Telemetry_msg);
    TEST_ASSERT_EQUAL(meshtastic_Telemetry_device_metrics_tag, telemetry.which_variant);
    TEST_ASSERT_EQUAL_UINT32(76, telemetry.variant.device_metrics.battery_level);
    TEST_ASSERT_FLOAT_WITHIN(0.001F, 4.125F, telemetry.variant.device_metrics.voltage);
}

static void test_traceroute_and_routing_vectors()
{
    const auto route = decodeAndRequireRoundTrip<meshtastic_RouteDiscovery>(TRACEROUTE, meshtastic_RouteDiscovery_msg);
    TEST_ASSERT_EQUAL_UINT32(2, route.route_count);
    TEST_ASSERT_EQUAL_UINT32(0x11111111U, route.route[0]);
    TEST_ASSERT_EQUAL_INT8(-8, route.snr_towards[0]);
    TEST_ASSERT_EQUAL_UINT32(1, route.route_back_count);

    const auto ack = decodeAndRequireRoundTrip<meshtastic_Routing>(ROUTING_ACK, meshtastic_Routing_msg);
    TEST_ASSERT_EQUAL(meshtastic_Routing_Error_NONE, ack.error_reason);
    const auto nak = decodeAndRequireRoundTrip<meshtastic_Routing>(ROUTING_NAK, meshtastic_Routing_msg);
    TEST_ASSERT_EQUAL(meshtastic_Routing_Error_NO_ROUTE, nak.error_reason);
}

static void test_ble_config_boundary_vectors()
{
    const auto request = decodeAndRequireRoundTrip<meshtastic_ToRadio>(BLE_TO_RADIO_CONFIG, meshtastic_ToRadio_msg);
    TEST_ASSERT_EQUAL(meshtastic_ToRadio_want_config_id_tag, request.which_payload_variant);
    TEST_ASSERT_EQUAL_UINT32(SYNTHETIC_CONFIG_ID, request.want_config_id);

    const auto complete =
        decodeAndRequireRoundTrip<meshtastic_FromRadio>(BLE_FROM_RADIO_COMPLETE, meshtastic_FromRadio_msg);
    TEST_ASSERT_EQUAL(meshtastic_FromRadio_config_complete_id_tag, complete.which_payload_variant);
    TEST_ASSERT_EQUAL_UINT32(SYNTHETIC_CONFIG_ID, complete.config_complete_id);
}

void setUp(void) {}
void tearDown(void) {}

void setup()
{
    delay(10);
    delay(2000);
    initializeTestEnvironment();

    UNITY_BEGIN();
    RUN_TEST(test_public_and_private_channel_text_vectors);
    RUN_TEST(test_position_node_info_and_telemetry_vectors);
    RUN_TEST(test_traceroute_and_routing_vectors);
    RUN_TEST(test_ble_config_boundary_vectors);
    exit(UNITY_END());
}

void loop() {}
