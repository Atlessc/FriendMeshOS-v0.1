#include "FriendMeshIdentityBinding.h"

#include "mesh/mesh-pb-constants.h"
#include <SHA256.h>
#include <cstdio>
#include <cstring>

namespace friendmesh::security
{

static constexpr char IDENTITY_DOMAIN[] = "FriendMeshOS Identity Binding v1";

bool encodeIdentityBinding(const friendmesh_FriendMeshIdentityBinding &binding, uint8_t *output, size_t outputCapacity,
                           size_t &outputSize)
{
    outputSize = pb_encode_to_bytes(output, outputCapacity, &friendmesh_FriendMeshIdentityBinding_msg, &binding);
    return outputSize > 0;
}

bool decodeIdentityBinding(const uint8_t *payload, size_t payloadSize, friendmesh_FriendMeshIdentityBinding &binding)
{
    binding = friendmesh_FriendMeshIdentityBinding_init_zero;
    if (!payload || payloadSize == 0 ||
        !pb_decode_from_bytes(payload, payloadSize, &friendmesh_FriendMeshIdentityBinding_msg, &binding)) {
        return false;
    }
    uint8_t canonical[friendmesh_FriendMeshIdentityBinding_size];
    size_t canonicalSize = 0;
    return encodeIdentityBinding(binding, canonical, sizeof(canonical), canonicalSize) && canonicalSize == payloadSize &&
           std::memcmp(payload, canonical, payloadSize) == 0;
}

void identityFingerprint(uint32_t senderNode, const uint8_t signingPublicKey[32],
                         const friendmesh_FriendMeshIdentityBinding &binding,
                         uint8_t fingerprint[IDENTITY_FINGERPRINT_SIZE])
{
    const uint8_t nodeBytes[4] = {
        static_cast<uint8_t>(senderNode), static_cast<uint8_t>(senderNode >> 8),
        static_cast<uint8_t>(senderNode >> 16), static_cast<uint8_t>(senderNode >> 24),
    };
    const uint8_t generationBytes[4] = {
        static_cast<uint8_t>(binding.key_generation), static_cast<uint8_t>(binding.key_generation >> 8),
        static_cast<uint8_t>(binding.key_generation >> 16), static_cast<uint8_t>(binding.key_generation >> 24),
    };
    SHA256 hash;
    hash.update(IDENTITY_DOMAIN, sizeof(IDENTITY_DOMAIN) - 1);
    hash.update(nodeBytes, sizeof(nodeBytes));
    hash.update(binding.meshtastic_public_key, sizeof(binding.meshtastic_public_key));
    hash.update(signingPublicKey, 32);
    hash.update(generationBytes, sizeof(generationBytes));
    hash.finalize(fingerprint, IDENTITY_FINGERPRINT_SIZE);
}

uint32_t identitySecurityNumber(const uint8_t fingerprint[IDENTITY_FINGERPRINT_SIZE])
{
    const uint32_t value = (static_cast<uint32_t>(fingerprint[0]) << 24) |
                           (static_cast<uint32_t>(fingerprint[1]) << 16) |
                           (static_cast<uint32_t>(fingerprint[2]) << 8) | fingerprint[3];
    return value % 1000000U;
}

bool formatIdentityFingerprint(const uint8_t fingerprint[IDENTITY_FINGERPRINT_SIZE], char *output, size_t outputSize)
{
    if (!fingerprint || !output || outputSize < 96) {
        return false;
    }
    size_t used = 0;
    for (size_t index = 0; index < IDENTITY_FINGERPRINT_SIZE; index++) {
        const int written = std::snprintf(output + used, outputSize - used, index == 0 ? "%02X" :
                                          (index % 4 == 0 ? "-%02X" : "%02X"), fingerprint[index]);
        if (written <= 0 || static_cast<size_t>(written) >= outputSize - used) {
            return false;
        }
        used += static_cast<size_t>(written);
    }
    return true;
}

} // namespace friendmesh::security
