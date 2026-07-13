#pragma once

#include "FriendMeshReplayWindow.h"
#include "friendmesh/generated/friendmesh.pb.h"
#include <cstddef>
#include <cstdint>

namespace friendmesh::protocol
{

constexpr uint8_t FRIENDMESH_PROTOCOL_VERSION = 1;
constexpr size_t FRIENDMESH_FRAME_PREFIX_SIZE = 5;
constexpr size_t FRIENDMESH_MAX_FRAME_SIZE = 233;
constexpr size_t FRIENDMESH_MAX_CANONICAL_SIZE = 192;

enum class DecodeResult : uint8_t {
    OK,
    NOT_FRIENDMESH,
    UNSUPPORTED_VERSION,
    TOO_LARGE,
    MALFORMED,
    NON_CANONICAL,
};

enum class ValidationResult : uint8_t {
    ACCEPTED,
    MALFORMED,
    UNSUPPORTED_VERSION,
    UNKNOWN_EVENT_TYPE,
    INVALID_SENDER,
    WRONG_GROUP,
    WRONG_EPOCH,
    STALE_TIMESTAMP,
    FUTURE_TIMESTAMP,
    INVALID_SIGNATURE,
    REPLAYED_EVENT,
    REPLAYED_SEQUENCE,
    STALE_SEQUENCE,
    REPLAY_CAPACITY_FULL,
};

struct ValidationContext
{
    bool requireSender = false;
    uint32_t senderNode = 0;
    bool requireGroup = false;
    uint8_t groupId[16]{};
    bool requireEpoch = false;
    uint32_t groupEpoch = 0;
    bool requireTimestamp = false;
    uint64_t currentTime = 0;
    uint32_t maxPastSeconds = 0;
    uint32_t maxFutureSeconds = 0;
};

using SignatureVerifyFn = bool (*)(const uint8_t publicKey[32], const uint8_t *message, size_t messageSize,
                                   const uint8_t signature[64]);

bool encodeCanonicalSignedFields(const friendmesh_FriendMeshSignedFields &fields, uint8_t *output, size_t outputCapacity,
                                 size_t &outputSize);
bool encodeFrame(const friendmesh_FriendMeshEnvelope &envelope, uint8_t *output, size_t outputCapacity,
                 size_t &outputSize);
DecodeResult decodeFrame(const uint8_t *frame, size_t frameSize, friendmesh_FriendMeshEnvelope &envelope);
ValidationResult validateEnvelope(const friendmesh_FriendMeshEnvelope &envelope, const ValidationContext &context,
                                  SignatureVerifyFn verifySignature, FriendMeshReplayWindow &replayWindow);
const char *validationResultName(ValidationResult result);

} // namespace friendmesh::protocol
