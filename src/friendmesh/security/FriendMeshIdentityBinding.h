#pragma once

#include "friendmesh/generated/friendmesh.pb.h"
#include <cstddef>
#include <cstdint>

namespace friendmesh::security
{

constexpr size_t IDENTITY_FINGERPRINT_SIZE = 32;

bool encodeIdentityBinding(const friendmesh_FriendMeshIdentityBinding &binding, uint8_t *output, size_t outputCapacity,
                           size_t &outputSize);
bool decodeIdentityBinding(const uint8_t *payload, size_t payloadSize, friendmesh_FriendMeshIdentityBinding &binding);
void identityFingerprint(uint32_t senderNode, const uint8_t signingPublicKey[32],
                         const friendmesh_FriendMeshIdentityBinding &binding,
                         uint8_t fingerprint[IDENTITY_FINGERPRINT_SIZE]);
uint32_t identitySecurityNumber(const uint8_t fingerprint[IDENTITY_FINGERPRINT_SIZE]);
bool formatIdentityFingerprint(const uint8_t fingerprint[IDENTITY_FINGERPRINT_SIZE], char *output, size_t outputSize);

} // namespace friendmesh::security
