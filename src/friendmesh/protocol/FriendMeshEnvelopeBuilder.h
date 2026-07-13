#pragma once

#include "FriendMeshProtocol.h"
#include <cstddef>
#include <cstdint>

namespace friendmesh::protocol
{

enum class EnvelopeBuildResult : uint8_t {
    BUILT,
    INVALID_ARGUMENT,
    PAYLOAD_TOO_LARGE,
    UNKNOWN_EVENT_TYPE,
    CANONICAL_ENCODING_FAILED,
    SIGNING_FAILED,
    FRAME_ENCODING_FAILED,
};

struct EnvelopeBuildRequest
{
    const uint8_t *groupId = nullptr;
    uint32_t groupEpoch = 0;
    uint32_t senderNode = 0;
    uint64_t senderSequence = 0;
    const uint8_t *eventId = nullptr;
    uint64_t createdAt = 0;
    friendmesh_FriendMeshEventType eventType = friendmesh_FriendMeshEventType_FRIENDMESH_EVENT_UNSPECIFIED;
    const uint8_t *payload = nullptr;
    size_t payloadSize = 0;
};

using SignatureCreateFn = bool (*)(const uint8_t publicKey[32], const uint8_t *message, size_t messageSize,
                                   uint8_t signature[64], void *context);

EnvelopeBuildResult buildSignedFrame(const EnvelopeBuildRequest &request, const uint8_t signingPublicKey[32],
                                     SignatureCreateFn createSignature, void *signatureContext,
                                     friendmesh_FriendMeshEnvelope &envelope, uint8_t *frame, size_t frameCapacity,
                                     size_t &frameSize);
const char *envelopeBuildResultName(EnvelopeBuildResult result);

} // namespace friendmesh::protocol
