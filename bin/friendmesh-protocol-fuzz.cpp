#include "friendmesh/protocol/FriendMeshProtocol.h"

#include "pb_decode.h"
#include "pb_encode.h"
#include <array>
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

uint32_t randomState = 0x46534d48;

uint32_t nextRandom()
{
    randomState ^= randomState << 13;
    randomState ^= randomState >> 17;
    randomState ^= randomState << 5;
    return randomState;
}

bool decodeResultIsKnown(friendmesh::protocol::DecodeResult result)
{
    using friendmesh::protocol::DecodeResult;
    return result == DecodeResult::OK || result == DecodeResult::NOT_FRIENDMESH ||
           result == DecodeResult::UNSUPPORTED_VERSION || result == DecodeResult::TOO_LARGE ||
           result == DecodeResult::MALFORMED || result == DecodeResult::NON_CANONICAL;
}

bool rejectSignature(const uint8_t[32], const uint8_t *, size_t, const uint8_t[64])
{
    return false;
}

bool exercise(const uint8_t *data, size_t size)
{
    friendmesh_FriendMeshEnvelope envelope = friendmesh_FriendMeshEnvelope_init_zero;
    const auto decoded = friendmesh::protocol::decodeFrame(data, size, envelope);
    if (!decodeResultIsKnown(decoded)) {
        return false;
    }
    if (decoded == friendmesh::protocol::DecodeResult::OK) {
        friendmesh::protocol::ValidationContext context;
        friendmesh::protocol::FriendMeshReplayWindow replay;
        const auto validated = friendmesh::protocol::validateEnvelope(envelope, context, rejectSignature, replay);
        if (validated != friendmesh::protocol::ValidationResult::INVALID_SIGNATURE &&
            validated != friendmesh::protocol::ValidationResult::MALFORMED &&
            validated != friendmesh::protocol::ValidationResult::UNSUPPORTED_VERSION &&
            validated != friendmesh::protocol::ValidationResult::UNKNOWN_EVENT_TYPE) {
            return false;
        }
    }
    return true;
}

bool buildSeedFrame(std::array<uint8_t, friendmesh::protocol::FRIENDMESH_MAX_FRAME_SIZE> &frame, size_t &frameSize)
{
    friendmesh_FriendMeshEnvelope envelope = friendmesh_FriendMeshEnvelope_init_zero;
    envelope.has_signed_fields = true;
    auto &fields = envelope.signed_fields;
    fields.protocol_version = friendmesh::protocol::FRIENDMESH_PROTOCOL_VERSION;
    fields.group_epoch = 1;
    fields.sender_node = 0x12345678;
    fields.sender_sequence = 1;
    fields.created_at = 1783896000;
    fields.event_type = friendmesh_FriendMeshEventType_FRIENDMESH_EVENT_PROTOCOL_PROBE;
    for (size_t index = 0; index < sizeof(fields.group_id); index++) {
        fields.group_id[index] = static_cast<uint8_t>(index + 1);
        fields.event_id[index] = static_cast<uint8_t>(index + 0x20);
    }
    for (size_t index = 0; index < sizeof(fields.signing_public_key); index++) {
        fields.signing_public_key[index] = static_cast<uint8_t>(index + 0x40);
    }
    std::memset(envelope.signature, 0xa5, sizeof(envelope.signature));
    return friendmesh::protocol::encodeFrame(envelope, frame.data(), frame.size(), frameSize);
}

} // namespace

int main()
{
    std::array<uint8_t, 260> input{};
    for (size_t iteration = 0; iteration < 100000; iteration++) {
        const size_t size = nextRandom() % (input.size() + 1);
        for (size_t index = 0; index < size; index++) {
            input[index] = static_cast<uint8_t>(nextRandom());
        }
        if (!exercise(input.data(), size)) {
            std::fprintf(stderr, "Random decode invariant failed at iteration %zu\n", iteration);
            return 1;
        }
    }

    std::array<uint8_t, friendmesh::protocol::FRIENDMESH_MAX_FRAME_SIZE> seed{};
    size_t seedSize = 0;
    if (!buildSeedFrame(seed, seedSize) || !exercise(seed.data(), seedSize)) {
        std::fputs("Could not construct canonical seed frame\n", stderr);
        return 1;
    }
    for (size_t iteration = 0; iteration < 50000; iteration++) {
        input.fill(0);
        std::memcpy(input.data(), seed.data(), seedSize);
        const size_t mutations = 1 + nextRandom() % 8;
        for (size_t mutation = 0; mutation < mutations; mutation++) {
            const size_t index = nextRandom() % seedSize;
            input[index] ^= static_cast<uint8_t>(1U << (nextRandom() % 8));
        }
        if (!exercise(input.data(), seedSize)) {
            std::fprintf(stderr, "Mutation decode invariant failed at iteration %zu\n", iteration);
            return 1;
        }
    }

    std::puts("FriendMesh deterministic decoder fuzz checks passed (150000 cases)");
    return 0;
}
