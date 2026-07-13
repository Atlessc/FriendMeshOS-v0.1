#include "FriendMeshProtocol.h"

#include "mesh/mesh-pb-constants.h"
#include <cstring>

namespace friendmesh::protocol
{

static constexpr uint8_t FRAME_MAGIC[4] = {'F', 'M', 'S', 'H'};
static constexpr char SIGNING_DOMAIN[] = "FriendMeshOS Signed Envelope v1";

static bool allZero(const uint8_t *value, size_t size)
{
    uint8_t combined = 0;
    for (size_t index = 0; index < size; index++) {
        combined |= value[index];
    }
    return combined == 0;
}

bool encodeCanonicalSignedFields(const friendmesh_FriendMeshSignedFields &fields, uint8_t *output, size_t outputCapacity,
                                 size_t &outputSize)
{
    constexpr size_t domainSize = sizeof(SIGNING_DOMAIN) - 1;
    outputSize = 0;
    if (!output || outputCapacity < domainSize) {
        return false;
    }
    std::memcpy(output, SIGNING_DOMAIN, domainSize);
    const size_t encoded = pb_encode_to_bytes(output + domainSize, outputCapacity - domainSize,
                                              &friendmesh_FriendMeshSignedFields_msg, &fields);
    if (encoded == 0) {
        return false;
    }
    outputSize = domainSize + encoded;
    return outputSize <= FRIENDMESH_MAX_CANONICAL_SIZE;
}

bool encodeFrame(const friendmesh_FriendMeshEnvelope &envelope, uint8_t *output, size_t outputCapacity,
                 size_t &outputSize)
{
    outputSize = 0;
    if (!output || outputCapacity < FRIENDMESH_FRAME_PREFIX_SIZE) {
        return false;
    }
    std::memcpy(output, FRAME_MAGIC, sizeof(FRAME_MAGIC));
    output[sizeof(FRAME_MAGIC)] = FRIENDMESH_PROTOCOL_VERSION;
    const size_t encoded = pb_encode_to_bytes(output + FRIENDMESH_FRAME_PREFIX_SIZE,
                                              outputCapacity - FRIENDMESH_FRAME_PREFIX_SIZE,
                                              &friendmesh_FriendMeshEnvelope_msg, &envelope);
    if (encoded == 0) {
        return false;
    }
    outputSize = FRIENDMESH_FRAME_PREFIX_SIZE + encoded;
    return outputSize <= FRIENDMESH_MAX_FRAME_SIZE;
}

DecodeResult decodeFrame(const uint8_t *frame, size_t frameSize, friendmesh_FriendMeshEnvelope &envelope)
{
    envelope = friendmesh_FriendMeshEnvelope_init_zero;
    if (!frame || frameSize < FRIENDMESH_FRAME_PREFIX_SIZE ||
        std::memcmp(frame, FRAME_MAGIC, sizeof(FRAME_MAGIC)) != 0) {
        return DecodeResult::NOT_FRIENDMESH;
    }
    if (frameSize > FRIENDMESH_MAX_FRAME_SIZE) {
        return DecodeResult::TOO_LARGE;
    }
    if (frame[sizeof(FRAME_MAGIC)] != FRIENDMESH_PROTOCOL_VERSION) {
        return DecodeResult::UNSUPPORTED_VERSION;
    }
    if (!pb_decode_from_bytes(frame + FRIENDMESH_FRAME_PREFIX_SIZE, frameSize - FRIENDMESH_FRAME_PREFIX_SIZE,
                              &friendmesh_FriendMeshEnvelope_msg, &envelope)) {
        return DecodeResult::MALFORMED;
    }

    uint8_t canonicalFrame[FRIENDMESH_MAX_FRAME_SIZE];
    size_t canonicalSize = 0;
    if (!encodeFrame(envelope, canonicalFrame, sizeof(canonicalFrame), canonicalSize) || canonicalSize != frameSize ||
        std::memcmp(frame, canonicalFrame, frameSize) != 0) {
        return DecodeResult::NON_CANONICAL;
    }
    return DecodeResult::OK;
}

static ValidationResult replayResult(ReplayResult result)
{
    switch (result) {
    case ReplayResult::ACCEPTED:
        return ValidationResult::ACCEPTED;
    case ReplayResult::DUPLICATE_EVENT:
        return ValidationResult::REPLAYED_EVENT;
    case ReplayResult::DUPLICATE_SEQUENCE:
        return ValidationResult::REPLAYED_SEQUENCE;
    case ReplayResult::STALE_SEQUENCE:
        return ValidationResult::STALE_SEQUENCE;
    default:
        return ValidationResult::REPLAY_CAPACITY_FULL;
    }
}

ValidationResult validateEnvelope(const friendmesh_FriendMeshEnvelope &envelope, const ValidationContext &context,
                                  SignatureVerifyFn verifySignature, FriendMeshReplayWindow &replayWindow)
{
    const auto &fields = envelope.signed_fields;
    if (!envelope.has_signed_fields || fields.sender_node == 0 || fields.sender_sequence == 0 || fields.created_at == 0 ||
        allZero(fields.event_id, sizeof(fields.event_id)) || allZero(fields.signing_public_key, sizeof(fields.signing_public_key)) ||
        !verifySignature) {
        return ValidationResult::MALFORMED;
    }
    if (fields.protocol_version != FRIENDMESH_PROTOCOL_VERSION) {
        return ValidationResult::UNSUPPORTED_VERSION;
    }
    if (fields.event_type <= friendmesh_FriendMeshEventType_FRIENDMESH_EVENT_UNSPECIFIED ||
        fields.event_type > friendmesh_FriendMeshEventType_FRIENDMESH_EVENT_PROTOCOL_PROBE) {
        return ValidationResult::UNKNOWN_EVENT_TYPE;
    }
    if (context.requireSender && fields.sender_node != context.senderNode) {
        return ValidationResult::INVALID_SENDER;
    }
    if (context.requireGroup && std::memcmp(fields.group_id, context.groupId, sizeof(fields.group_id)) != 0) {
        return ValidationResult::WRONG_GROUP;
    }
    if (context.requireEpoch && fields.group_epoch != context.groupEpoch) {
        return ValidationResult::WRONG_EPOCH;
    }
    if (context.requireTimestamp) {
        if (fields.created_at < context.currentTime &&
            context.currentTime - fields.created_at > context.maxPastSeconds) {
            return ValidationResult::STALE_TIMESTAMP;
        }
        if (fields.created_at > context.currentTime &&
            fields.created_at - context.currentTime > context.maxFutureSeconds) {
            return ValidationResult::FUTURE_TIMESTAMP;
        }
    }

    uint8_t canonical[FRIENDMESH_MAX_CANONICAL_SIZE];
    size_t canonicalSize = 0;
    if (!encodeCanonicalSignedFields(fields, canonical, sizeof(canonical), canonicalSize)) {
        return ValidationResult::MALFORMED;
    }
    if (!verifySignature(fields.signing_public_key, canonical, canonicalSize, envelope.signature)) {
        return ValidationResult::INVALID_SIGNATURE;
    }
    return replayResult(replayWindow.checkAndRecord(fields));
}

const char *validationResultName(ValidationResult result)
{
    switch (result) {
    case ValidationResult::ACCEPTED:
        return "ACCEPTED";
    case ValidationResult::MALFORMED:
        return "MALFORMED";
    case ValidationResult::UNSUPPORTED_VERSION:
        return "UNSUPPORTED_VERSION";
    case ValidationResult::UNKNOWN_EVENT_TYPE:
        return "UNKNOWN_EVENT_TYPE";
    case ValidationResult::INVALID_SENDER:
        return "INVALID_SENDER";
    case ValidationResult::WRONG_GROUP:
        return "WRONG_GROUP";
    case ValidationResult::WRONG_EPOCH:
        return "WRONG_EPOCH";
    case ValidationResult::STALE_TIMESTAMP:
        return "STALE_TIMESTAMP";
    case ValidationResult::FUTURE_TIMESTAMP:
        return "FUTURE_TIMESTAMP";
    case ValidationResult::INVALID_SIGNATURE:
        return "INVALID_SIGNATURE";
    case ValidationResult::REPLAYED_EVENT:
        return "REPLAYED_EVENT";
    case ValidationResult::REPLAYED_SEQUENCE:
        return "REPLAYED_SEQUENCE";
    case ValidationResult::STALE_SEQUENCE:
        return "STALE_SEQUENCE";
    default:
        return "REPLAY_CAPACITY_FULL";
    }
}

} // namespace friendmesh::protocol
