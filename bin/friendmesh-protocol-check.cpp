#include "friendmesh/protocol/FriendMeshProtocol.h"

#include "pb_decode.h"
#include "pb_encode.h"
#include <cstdio>
#include <cstring>

size_t pb_encode_to_bytes(uint8_t *output, size_t capacity, const pb_msgdesc_t *descriptor, const void *message)
{
    pb_ostream_t stream = pb_ostream_from_buffer(output, capacity);
    return pb_encode(&stream, descriptor, message) ? stream.bytes_written : 0;
}

bool pb_decode_from_bytes(const uint8_t *input, size_t size, const pb_msgdesc_t *descriptor, void *message)
{
    pb_istream_t stream = pb_istream_from_buffer(input, size);
    return pb_decode(&stream, descriptor, message);
}

namespace
{

using friendmesh::protocol::DecodeResult;
using friendmesh::protocol::FriendMeshReplayWindow;
using friendmesh::protocol::ValidationContext;
using friendmesh::protocol::ValidationResult;

int failures = 0;

void require(bool condition, const char *name)
{
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", name);
        failures++;
    }
}

uint8_t checksum(const uint8_t *message, size_t size)
{
    uint8_t value = 0x5a;
    for (size_t index = 0; index < size; index++) {
        value = static_cast<uint8_t>((value << 1) | (value >> 7));
        value ^= message[index];
    }
    return value;
}

bool fakeVerify(const uint8_t[32], const uint8_t *message, size_t size, const uint8_t signature[64])
{
    if (signature[0] != checksum(message, size)) {
        return false;
    }
    for (size_t index = 1; index < 64; index++) {
        if (signature[index] != static_cast<uint8_t>(index)) {
            return false;
        }
    }
    return true;
}

void fakeSign(friendmesh_FriendMeshEnvelope &envelope)
{
    uint8_t canonical[friendmesh::protocol::FRIENDMESH_MAX_CANONICAL_SIZE]{};
    size_t canonicalSize = 0;
    require(friendmesh::protocol::encodeCanonicalSignedFields(envelope.signed_fields, canonical, sizeof(canonical),
                                                              canonicalSize),
            "canonical signed fields encode");
    envelope.signature[0] = checksum(canonical, canonicalSize);
    for (size_t index = 1; index < sizeof(envelope.signature); index++) {
        envelope.signature[index] = static_cast<uint8_t>(index);
    }
}

friendmesh_FriendMeshEnvelope makeEnvelope(uint32_t sender = 0x12345678, uint64_t sequence = 100,
                                           uint8_t eventSeed = 1)
{
    friendmesh_FriendMeshEnvelope envelope = friendmesh_FriendMeshEnvelope_init_zero;
    envelope.has_signed_fields = true;
    auto &fields = envelope.signed_fields;
    fields.protocol_version = friendmesh::protocol::FRIENDMESH_PROTOCOL_VERSION;
    fields.group_epoch = 7;
    fields.sender_node = sender;
    fields.sender_sequence = sequence;
    fields.created_at = 1783896000;
    fields.event_type = friendmesh_FriendMeshEventType_FRIENDMESH_EVENT_PROTOCOL_PROBE;
    for (size_t index = 0; index < sizeof(fields.group_id); index++) {
        fields.group_id[index] = static_cast<uint8_t>(0xa0 + index);
        fields.event_id[index] = static_cast<uint8_t>(eventSeed + index);
    }
    for (size_t index = 0; index < sizeof(fields.signing_public_key); index++) {
        fields.signing_public_key[index] = static_cast<uint8_t>(0x20 + index);
    }
    fakeSign(envelope);
    return envelope;
}

ValidationContext matchingContext(const friendmesh_FriendMeshEnvelope &envelope)
{
    ValidationContext context;
    context.requireSender = true;
    context.senderNode = envelope.signed_fields.sender_node;
    context.requireGroup = true;
    std::memcpy(context.groupId, envelope.signed_fields.group_id, sizeof(context.groupId));
    context.requireEpoch = true;
    context.groupEpoch = envelope.signed_fields.group_epoch;
    context.requireTimestamp = true;
    context.currentTime = envelope.signed_fields.created_at;
    context.maxPastSeconds = 24U * 60U * 60U;
    context.maxFutureSeconds = 10U * 60U;
    return context;
}

ValidationResult validateFresh(const friendmesh_FriendMeshEnvelope &envelope, const ValidationContext &context)
{
    FriendMeshReplayWindow replay;
    return friendmesh::protocol::validateEnvelope(envelope, context, fakeVerify, replay);
}

void testFrameBoundaries()
{
    auto envelope = makeEnvelope();
    uint8_t frame[friendmesh::protocol::FRIENDMESH_MAX_FRAME_SIZE]{};
    size_t frameSize = 0;
    require(friendmesh::protocol::encodeFrame(envelope, frame, sizeof(frame), frameSize), "frame encode");
    require(frameSize <= friendmesh::protocol::FRIENDMESH_MAX_FRAME_SIZE, "frame fits Meshtastic payload");

    friendmesh_FriendMeshEnvelope decoded = friendmesh_FriendMeshEnvelope_init_zero;
    require(friendmesh::protocol::decodeFrame(frame, frameSize, decoded) == DecodeResult::OK, "canonical frame decode");
    require(std::memcmp(&envelope, &decoded, sizeof(envelope)) == 0, "canonical frame round trip");

    uint8_t wrongVersion[sizeof(frame)]{};
    std::memcpy(wrongVersion, frame, frameSize);
    wrongVersion[4]++;
    require(friendmesh::protocol::decodeFrame(wrongVersion, frameSize, decoded) == DecodeResult::UNSUPPORTED_VERSION,
            "frame prefix version rejection");

    uint8_t nonCanonical[sizeof(frame)]{};
    std::memcpy(nonCanonical, frame, frameSize);
    nonCanonical[frameSize] = 0x78;
    nonCanonical[frameSize + 1] = 0;
    require(friendmesh::protocol::decodeFrame(nonCanonical, frameSize + 2, decoded) == DecodeResult::NON_CANONICAL,
            "unknown protobuf field rejection");

    envelope.signed_fields.payload.size = sizeof(envelope.signed_fields.payload.bytes);
    std::memset(envelope.signed_fields.payload.bytes, 0xa5, envelope.signed_fields.payload.size);
    fakeSign(envelope);
    require(friendmesh::protocol::encodeFrame(envelope, frame, sizeof(frame), frameSize), "maximum payload encode");
    require(frameSize <= friendmesh::protocol::FRIENDMESH_MAX_FRAME_SIZE, "maximum envelope fits 233 bytes");
}

void testValidationFailures()
{
    auto envelope = makeEnvelope();
    auto context = matchingContext(envelope);
    require(validateFresh(envelope, context) == ValidationResult::ACCEPTED, "valid envelope accepted");

    auto changed = context;
    changed.senderNode++;
    require(validateFresh(envelope, changed) == ValidationResult::INVALID_SENDER, "sender binding rejection");
    changed = context;
    changed.groupId[0] ^= 1;
    require(validateFresh(envelope, changed) == ValidationResult::WRONG_GROUP, "group binding rejection");
    changed = context;
    changed.groupEpoch++;
    require(validateFresh(envelope, changed) == ValidationResult::WRONG_EPOCH, "epoch binding rejection");
    changed = context;
    changed.currentTime += changed.maxPastSeconds + 1;
    require(validateFresh(envelope, changed) == ValidationResult::STALE_TIMESTAMP, "stale timestamp rejection");
    changed = context;
    changed.currentTime -= changed.maxFutureSeconds + 1;
    require(validateFresh(envelope, changed) == ValidationResult::FUTURE_TIMESTAMP, "future timestamp rejection");

    auto invalid = envelope;
    invalid.signature[7] ^= 1;
    require(validateFresh(invalid, context) == ValidationResult::INVALID_SIGNATURE, "signature rejection");
    invalid = envelope;
    invalid.signed_fields.protocol_version++;
    fakeSign(invalid);
    require(validateFresh(invalid, context) == ValidationResult::UNSUPPORTED_VERSION, "signed version rejection");
    invalid = envelope;
    invalid.signed_fields.event_type = friendmesh_FriendMeshEventType_FRIENDMESH_EVENT_UNSPECIFIED;
    fakeSign(invalid);
    require(validateFresh(invalid, context) == ValidationResult::UNKNOWN_EVENT_TYPE, "unknown event rejection");
    invalid = envelope;
    invalid.signed_fields.created_at = 0;
    fakeSign(invalid);
    require(validateFresh(invalid, context) == ValidationResult::MALFORMED, "zero timestamp rejection");
}

void testReplayWindow()
{
    auto first = makeEnvelope();
    auto context = matchingContext(first);
    FriendMeshReplayWindow replay;
    require(friendmesh::protocol::validateEnvelope(first, context, fakeVerify, replay) == ValidationResult::ACCEPTED,
            "first sequence accepted");
    require(friendmesh::protocol::validateEnvelope(first, context, fakeVerify, replay) == ValidationResult::REPLAYED_EVENT,
            "duplicate event rejected");

    auto duplicateSequence = makeEnvelope(first.signed_fields.sender_node, first.signed_fields.sender_sequence, 2);
    require(friendmesh::protocol::validateEnvelope(duplicateSequence, context, fakeVerify, replay) ==
                ValidationResult::REPLAYED_SEQUENCE,
            "duplicate sequence rejected");
    auto inWindow = makeEnvelope(first.signed_fields.sender_node, first.signed_fields.sender_sequence - 1, 3);
    require(friendmesh::protocol::validateEnvelope(inWindow, context, fakeVerify, replay) == ValidationResult::ACCEPTED,
            "out-of-order sequence accepted within window");
    auto stale = makeEnvelope(first.signed_fields.sender_node, first.signed_fields.sender_sequence - 65, 4);
    require(friendmesh::protocol::validateEnvelope(stale, context, fakeVerify, replay) == ValidationResult::STALE_SEQUENCE,
            "sequence outside replay window rejected");

    FriendMeshReplayWindow capacityReplay;
    ValidationContext noBindings;
    for (size_t index = 0; index < friendmesh::protocol::REPLAY_SENDER_CAPACITY; index++) {
        auto sender = makeEnvelope(static_cast<uint32_t>(index + 1), 1, static_cast<uint8_t>(index + 1));
        require(friendmesh::protocol::validateEnvelope(sender, noBindings, fakeVerify, capacityReplay) ==
                    ValidationResult::ACCEPTED,
                "replay sender capacity entry accepted");
    }
    auto overflow = makeEnvelope(0xf0000000, 1, 0xf0);
    require(friendmesh::protocol::validateEnvelope(overflow, noBindings, fakeVerify, capacityReplay) ==
                ValidationResult::REPLAY_CAPACITY_FULL,
            "replay sender capacity fails closed");
}

} // namespace

int main()
{
    testFrameBoundaries();
    testValidationFailures();
    testReplayWindow();
    if (failures == 0) {
        std::puts("FriendMesh protocol validation checks passed");
    }
    return failures == 0 ? 0 : 1;
}
