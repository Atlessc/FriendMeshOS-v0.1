#pragma once

#include <cstddef>
#include <cstdint>

namespace friendmesh::security
{

constexpr size_t ED25519_SEED_SIZE = 32;
constexpr size_t ED25519_PUBLIC_KEY_SIZE = 32;
constexpr size_t ED25519_SIGNATURE_SIZE = 64;

class FriendMeshCrypto
{
  public:
    static bool available();
    static bool selfTest();
    static bool generateSeed(uint8_t seed[ED25519_SEED_SIZE]);
    static bool derivePublicKey(const uint8_t seed[ED25519_SEED_SIZE],
                                uint8_t publicKey[ED25519_PUBLIC_KEY_SIZE]);
    static bool sign(const uint8_t seed[ED25519_SEED_SIZE], const uint8_t expectedPublicKey[ED25519_PUBLIC_KEY_SIZE],
                     const uint8_t *message, size_t messageSize, uint8_t signature[ED25519_SIGNATURE_SIZE]);
    static bool verify(const uint8_t publicKey[ED25519_PUBLIC_KEY_SIZE], const uint8_t *message, size_t messageSize,
                       const uint8_t signature[ED25519_SIGNATURE_SIZE]);
};

} // namespace friendmesh::security
