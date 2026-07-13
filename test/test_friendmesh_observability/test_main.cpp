#include "TestUtil.h"
#include "friendmesh/observability/DiagnosticEventStore.h"
#include "friendmesh/observability/DiagnosticFormatter.h"
#include "friendmesh/observability/FriendMeshObservability.h"
#include "friendmesh/platform/TDeckCapabilityService.h"
#include "mesh/MeshTypes.h"
#include "pb_encode.h"
#include <cstring>
#include <unity.h>

using namespace friendmesh::observability;
using namespace friendmesh::platform;

static meshtastic_MeshPacket decodedPacket(meshtastic_PortNum port)
{
    meshtastic_MeshPacket packet = meshtastic_MeshPacket_init_default;
    packet.which_payload_variant = meshtastic_MeshPacket_decoded_tag;
    packet.decoded.portnum = port;
    packet.to = NODENUM_BROADCAST;
    return packet;
}

static void test_age_handles_unknown_skew_and_elapsed()
{
    TEST_ASSERT_FALSE(ageAt(100, 0).known);

    const Age skew = ageAt(100, 101);
    TEST_ASSERT_TRUE(skew.known);
    TEST_ASSERT_TRUE(skew.clockSkew);
    TEST_ASSERT_EQUAL_UINT32(0, skew.seconds);

    const Age elapsed = ageAt(100, 40);
    TEST_ASSERT_TRUE(elapsed.known);
    TEST_ASSERT_FALSE(elapsed.clockSkew);
    TEST_ASSERT_EQUAL_UINT32(60, elapsed.seconds);
}

static void test_hops_match_meshtastic_rules()
{
    meshtastic_MeshPacket packet = meshtastic_MeshPacket_init_default;
    TEST_ASSERT_EQUAL_INT8(-1, observedHops(packet));

    packet.which_payload_variant = meshtastic_MeshPacket_decoded_tag;
    packet.decoded.has_bitfield = true;
    TEST_ASSERT_EQUAL_INT8(0, observedHops(packet));

    packet.hop_start = 3;
    packet.hop_limit = 1;
    TEST_ASSERT_EQUAL_INT8(2, observedHops(packet));

    packet.hop_start = 1;
    packet.hop_limit = 3;
    TEST_ASSERT_EQUAL_INT8(-1, observedHops(packet));
}

static void test_packet_observation_exposes_metadata_not_payload()
{
    meshtastic_MeshPacket packet = decodedPacket(meshtastic_PortNum_TEXT_MESSAGE_APP);
    packet.channel = 2;
    packet.want_ack = true;
    packet.hop_start = 3;
    packet.hop_limit = 2;
    packet.rx_time = 90;
    packet.transport_mechanism = meshtastic_MeshPacket_TransportMechanism_TRANSPORT_LORA;

    const PacketObservation result = inspectPacket(packet, 100);
    TEST_ASSERT_EQUAL(meshtastic_PortNum_TEXT_MESSAGE_APP, result.port);
    TEST_ASSERT_EQUAL(static_cast<int>(DestinationType::BROADCAST), static_cast<int>(result.destination));
    TEST_ASSERT_EQUAL(static_cast<int>(PayloadType::DECODED), static_cast<int>(result.payload));
    TEST_ASSERT_EQUAL_UINT8(2, result.channel);
    TEST_ASSERT_TRUE(result.wantsAck);
    TEST_ASSERT_EQUAL_INT8(1, result.observedHops);
    TEST_ASSERT_EQUAL_UINT32(10, result.receiveAge.seconds);
}

static void test_routing_ack_and_nak_are_decoded()
{
    meshtastic_MeshPacket packet = decodedPacket(meshtastic_PortNum_ROUTING_APP);
    meshtastic_Routing routing = meshtastic_Routing_init_default;
    routing.which_variant = meshtastic_Routing_error_reason_tag;
    routing.error_reason = meshtastic_Routing_Error_NONE;
    packet.decoded.payload.size = pb_encode_to_bytes(packet.decoded.payload.bytes, sizeof(packet.decoded.payload.bytes),
                                                     &meshtastic_Routing_msg, &routing);

    TEST_ASSERT_EQUAL(static_cast<int>(RoutingResult::ACK), static_cast<int>(inspectPacket(packet, 1).routing));

    routing.error_reason = meshtastic_Routing_Error_NO_ROUTE;
    packet.decoded.payload.size = pb_encode_to_bytes(packet.decoded.payload.bytes, sizeof(packet.decoded.payload.bytes),
                                                     &meshtastic_Routing_msg, &routing);
    const PacketObservation nak = inspectPacket(packet, 1);
    TEST_ASSERT_EQUAL(static_cast<int>(RoutingResult::NAK), static_cast<int>(nak.routing));
    TEST_ASSERT_EQUAL(meshtastic_Routing_Error_NO_ROUTE, nak.routingError);
}

static void test_queue_and_position_states_are_explicit()
{
    meshtastic_QueueStatus queue = meshtastic_QueueStatus_init_default;
    TEST_ASSERT_EQUAL(static_cast<int>(QueueState::UNAVAILABLE), static_cast<int>(inspectQueue(queue).state));
    queue.maxlen = 16;
    TEST_ASSERT_EQUAL(static_cast<int>(QueueState::FULL), static_cast<int>(inspectQueue(queue).state));
    queue.free = 4;
    TEST_ASSERT_EQUAL(static_cast<int>(QueueState::READY), static_cast<int>(inspectQueue(queue).state));
    queue.free = 17;
    TEST_ASSERT_EQUAL(static_cast<int>(QueueState::INVALID), static_cast<int>(inspectQueue(queue).state));

    const PositionObservation position = inspectPosition(true, 600, 950, 1000, POSITION_STALE_SECONDS,
                                                         meshtastic_Position_LocSource_LOC_INTERNAL);
    TEST_ASSERT_EQUAL(static_cast<int>(Freshness::STALE), static_cast<int>(position.position));
    TEST_ASSERT_EQUAL(static_cast<int>(Freshness::FRESH), static_cast<int>(position.lastHeard));
    TEST_ASSERT_EQUAL(meshtastic_Position_LocSource_LOC_INTERNAL, position.source);
}

static void test_diagnostic_formatter_names_errors_and_redaction()
{
    DiagnosticEvent event;
    event.sequence = 9;
    event.capturedAt = 100;
    event.type = DiagnosticEventType::PACKET;
    event.packet.port = meshtastic_PortNum_ROUTING_APP;
    event.packet.destination = DestinationType::DIRECT;
    event.packet.payload = PayloadType::DECODED;
    event.packet.routing = RoutingResult::NAK;
    event.packet.routingError = meshtastic_Routing_Error_NO_CHANNEL;
    event.packet.transport = meshtastic_MeshPacket_TransportMechanism_TRANSPORT_LORA;

    char output[768];
    TEST_ASSERT_TRUE(formatDiagnosticEvent(event, output, sizeof(output)));
    TEST_ASSERT_NOT_NULL(strstr(output, "NO_CHANNEL (6)"));
    TEST_ASSERT_NOT_NULL(strstr(output, "Redacted: IDs, bodies, keys, coordinates"));
    TEST_ASSERT_EQUAL_STRING("PKI_UNKNOWN_PUBKEY", routingErrorName(meshtastic_Routing_Error_PKI_UNKNOWN_PUBKEY));

    char tooSmall[8];
    TEST_ASSERT_FALSE(formatDiagnosticEvent(event, tooSmall, sizeof(tooSmall)));
}

static void test_capabilities_degrade_independently()
{
    TDeckCapabilityInputs inputs;
    inputs.gps = {true, true, true};
    inputs.magnetometer = {true, false, false};
    inputs.sd = {true, true, false};
    inputs.maps = {true, true, true};
    inputs.touch = {true, true, true};
    inputs.trackball = {true, true, true};
    inputs.keyboard = {true, true, true};
    inputs.lora = {true, true, true};

    const TDeckCapabilities result = TDeckCapabilityService::snapshot(inputs);
    TEST_ASSERT_TRUE(result.canNavigate());
    TEST_ASSERT_FALSE(result.hasBodyRelativeHeading());
    TEST_ASSERT_FALSE(result.hasExpandedStorage());
    TEST_ASSERT_FALSE(result.canUseMap());
    TEST_ASSERT_EQUAL(static_cast<int>(CapabilityState::NOT_DETECTED), static_cast<int>(result.magnetometer));
    TEST_ASSERT_EQUAL(static_cast<int>(CapabilityState::DEGRADED), static_cast<int>(result.sd));
}

static void test_diagnostic_store_is_bounded_and_ordered()
{
    DiagnosticEventStore store;
    PacketObservation packet;
    packet.channel = 1;
    store.recordPacket(packet, 10);

    QueueObservation queue;
    queue.capacity = 16;
    store.recordQueue(queue, 11);
    TEST_ASSERT_EQUAL_UINT32(2, store.size());
    TEST_ASSERT_EQUAL(static_cast<int>(DiagnosticEventType::PACKET), static_cast<int>(store.at(0)->type));
    TEST_ASSERT_EQUAL(static_cast<int>(DiagnosticEventType::QUEUE), static_cast<int>(store.at(1)->type));

    for (size_t i = 0; i < store.capacity(); i++) {
        store.recordPosition({}, 100 + i);
    }
    TEST_ASSERT_EQUAL_UINT32(store.capacity(), store.size());
    TEST_ASSERT_EQUAL_UINT32(3, store.at(0)->sequence);
    TEST_ASSERT_EQUAL_UINT32(100, store.at(0)->capturedAt);
    TEST_ASSERT_NULL(store.at(store.size()));

    store.clear();
    TEST_ASSERT_EQUAL_UINT32(0, store.size());
    TEST_ASSERT_NULL(store.at(0));
}

void setUp(void) {}
void tearDown(void) {}

void setup()
{
    delay(10);
    delay(2000);
    initializeTestEnvironment();

    UNITY_BEGIN();
    RUN_TEST(test_age_handles_unknown_skew_and_elapsed);
    RUN_TEST(test_hops_match_meshtastic_rules);
    RUN_TEST(test_packet_observation_exposes_metadata_not_payload);
    RUN_TEST(test_routing_ack_and_nak_are_decoded);
    RUN_TEST(test_queue_and_position_states_are_explicit);
    RUN_TEST(test_diagnostic_formatter_names_errors_and_redaction);
    RUN_TEST(test_capabilities_degrade_independently);
    RUN_TEST(test_diagnostic_store_is_bounded_and_ordered);
    exit(UNITY_END());
}

void loop() {}
