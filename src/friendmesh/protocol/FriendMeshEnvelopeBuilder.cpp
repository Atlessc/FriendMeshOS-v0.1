#include "FriendMeshEnvelopeBuilder.h"

#include <cstring>

namespace friendmesh::protocol
{

static bool allZero(const uint8_t *value, size_t size)
{
    if (!value) {
        return true;
    }
    uint8_t combined = 0;
    for (size_t index = 0; index < size; index++) {
        combined |= value[index];
    }
    return combined == 0;
}

EnvelopeBuildResult buildSignedFrame(const EnvelopeBuildRequest &request, const uint8_t signingPublicKey[32],
                                     SignatureCreateFn createSignature, void *signatureContext,
                                     friendmesh_FriendMeshEnvelope &envelope, uint8_t *frame, size_t frameCapacity,
                                     size_t &frameSize)
{
    envelope = friendmesh_FriendMeshEnvelope_init_zero;
    frameSize = 0;
    if (!request.groupId || !request.eventId || request.groupEpoch == 0 || request.senderNode == 0 ||
        request.senderSequence == 0 || request.createdAt == 0 || allZero(request.groupId, 16) ||
        allZero(request.eventId, 16) || allZero(signingPublicKey, 32) || !createSignature || !frame) {
        return EnvelopeBuildResult::INVALID_ARGUMENT;
    }
    if (request.payloadSize > sizeof(envelope.signed_fields.payload.bytes) ||
        (request.payloadSize > 0 && !request.payload)) {
        return EnvelopeBuildResult::PAYLOAD_TOO_LARGE;
    }
    if (request.eventType <= friendmesh_FriendMeshEventType_FRIENDMESH_EVENT_UNSPECIFIED ||
        request.eventType > friendmesh_FriendMeshEventType_FRIENDMESH_EVENT_PROTOCOL_PROBE) {
        return EnvelopeBuildResult::UNKNOWN_EVENT_TYPE;
    }

    envelope.has_signed_fields = true;
    auto &fields = envelope.signed_fields;
    fields.protocol_version = FRIENDMESH_PROTOCOL_VERSION;
    std::memcpy(fields.group_id, request.groupId, sizeof(fields.group_id));
    fields.group_epoch = request.groupEpoch;
    fields.sender_node = request.senderNode;
    fields.sender_sequence = request.senderSequence;
    std::memcpy(fields.event_id, request.eventId, sizeof(fields.event_id));
    fields.created_at = request.createdAt;
    fields.event_type = request.eventType;
    std::memcpy(fields.signing_public_key, signingPublicKey, sizeof(fields.signing_public_key));
    fields.payload.size = request.payloadSize;
    if (request.payloadSize > 0) {
        std::memcpy(fields.payload.bytes, request.payload, request.payloadSize);
    }

    uint8_t canonical[FRIENDMESH_MAX_CANONICAL_SIZE];
    size_t canonicalSize = 0;
    if (!encodeCanonicalSignedFields(fields, canonical, sizeof(canonical), canonicalSize)) {
        return EnvelopeBuildResult::CANONICAL_ENCODING_FAILED;
    }
    if (!createSignature(signingPublicKey, canonical, canonicalSize, envelope.signature, signatureContext)) {
        return EnvelopeBuildResult::SIGNING_FAILED;
    }
    if (!encodeFrame(envelope, frame, frameCapacity, frameSize)) {
        return EnvelopeBuildResult::FRAME_ENCODING_FAILED;
    }
    return EnvelopeBuildResult::BUILT;
}

const char *envelopeBuildResultName(EnvelopeBuildResult result)
{
    switch (result) {
    case EnvelopeBuildResult::BUILT:
        return "BUILT";
    case EnvelopeBuildResult::INVALID_ARGUMENT:
        return "INVALID_ARGUMENT";
    case EnvelopeBuildResult::PAYLOAD_TOO_LARGE:
        return "PAYLOAD_TOO_LARGE";
    case EnvelopeBuildResult::UNKNOWN_EVENT_TYPE:
        return "UNKNOWN_EVENT_TYPE";
    case EnvelopeBuildResult::CANONICAL_ENCODING_FAILED:
        return "CANONICAL_ENCODING_FAILED";
    case EnvelopeBuildResult::SIGNING_FAILED:
        return "SIGNING_FAILED";
    default:
        return "FRAME_ENCODING_FAILED";
    }
}

} // namespace friendmesh::protocol
