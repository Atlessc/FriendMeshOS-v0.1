#include "FriendMeshCrypto.h"

#include <cstring>

#if defined(ARCH_ESP32)
#include <sodium.h>
#endif

namespace friendmesh::security
{

bool FriendMeshCrypto::available()
{
#if defined(ARCH_ESP32)
    return sodium_init() >= 0;
#else
    return false;
#endif
}

bool FriendMeshCrypto::generateSeed(uint8_t seed[ED25519_SEED_SIZE])
{
#if defined(ARCH_ESP32)
    if (!seed || !available()) {
        return false;
    }
    randombytes_buf(seed, ED25519_SEED_SIZE);
    return true;
#else
    (void)seed;
    return false;
#endif
}

bool FriendMeshCrypto::derivePublicKey(const uint8_t seed[ED25519_SEED_SIZE],
                                       uint8_t publicKey[ED25519_PUBLIC_KEY_SIZE])
{
#if defined(ARCH_ESP32)
    if (!seed || !publicKey || !available()) {
        return false;
    }
    uint8_t secretKey[crypto_sign_SECRETKEYBYTES];
    const int result = crypto_sign_seed_keypair(publicKey, secretKey, seed);
    sodium_memzero(secretKey, sizeof(secretKey));
    return result == 0;
#else
    (void)seed;
    (void)publicKey;
    return false;
#endif
}

bool FriendMeshCrypto::sign(const uint8_t seed[ED25519_SEED_SIZE],
                            const uint8_t expectedPublicKey[ED25519_PUBLIC_KEY_SIZE], const uint8_t *message,
                            size_t messageSize, uint8_t signature[ED25519_SIGNATURE_SIZE])
{
#if defined(ARCH_ESP32)
    if (!seed || !expectedPublicKey || (!message && messageSize != 0) || !signature || !available()) {
        return false;
    }
    uint8_t derivedPublicKey[crypto_sign_PUBLICKEYBYTES];
    uint8_t secretKey[crypto_sign_SECRETKEYBYTES];
    if (crypto_sign_seed_keypair(derivedPublicKey, secretKey, seed) != 0 ||
        sodium_memcmp(derivedPublicKey, expectedPublicKey, sizeof(derivedPublicKey)) != 0) {
        sodium_memzero(secretKey, sizeof(secretKey));
        sodium_memzero(derivedPublicKey, sizeof(derivedPublicKey));
        return false;
    }
    unsigned long long signatureSize = 0;
    const int result = crypto_sign_detached(signature, &signatureSize, message, messageSize, secretKey);
    sodium_memzero(secretKey, sizeof(secretKey));
    sodium_memzero(derivedPublicKey, sizeof(derivedPublicKey));
    return result == 0 && signatureSize == ED25519_SIGNATURE_SIZE;
#else
    (void)seed;
    (void)expectedPublicKey;
    (void)message;
    (void)messageSize;
    (void)signature;
    return false;
#endif
}

bool FriendMeshCrypto::verify(const uint8_t publicKey[ED25519_PUBLIC_KEY_SIZE], const uint8_t *message,
                              size_t messageSize, const uint8_t signature[ED25519_SIGNATURE_SIZE])
{
#if defined(ARCH_ESP32)
    return publicKey && (!message ? messageSize == 0 : true) && signature && available() &&
           crypto_sign_verify_detached(signature, message, messageSize, publicKey) == 0;
#else
    (void)publicKey;
    (void)message;
    (void)messageSize;
    (void)signature;
    return false;
#endif
}

bool FriendMeshCrypto::selfTest()
{
#if defined(ARCH_ESP32)
    static constexpr uint8_t seed[ED25519_SEED_SIZE] = {
        0x9d, 0x61, 0xb1, 0x9d, 0xef, 0xfd, 0x5a, 0x60, 0xba, 0x84, 0x4a, 0xf4, 0x92, 0xec, 0x2c, 0xc4,
        0x44, 0x49, 0xc5, 0x69, 0x7b, 0x32, 0x69, 0x19, 0x70, 0x3b, 0xac, 0x03, 0x1c, 0xae, 0x7f, 0x60,
    };
    static constexpr uint8_t expectedPublicKey[ED25519_PUBLIC_KEY_SIZE] = {
        0xd7, 0x5a, 0x98, 0x01, 0x82, 0xb1, 0x0a, 0xb7, 0xd5, 0x4b, 0xfe, 0xd3, 0xc9, 0x64, 0x07, 0x3a,
        0x0e, 0xe1, 0x72, 0xf3, 0xda, 0xa6, 0x23, 0x25, 0xaf, 0x02, 0x1a, 0x68, 0xf7, 0x07, 0x51, 0x1a,
    };
    static constexpr uint8_t expectedSignature[ED25519_SIGNATURE_SIZE] = {
        0xe5, 0x56, 0x43, 0x00, 0xc3, 0x60, 0xac, 0x72, 0x90, 0x86, 0xe2, 0xcc, 0x80, 0x6e, 0x82, 0x8a,
        0x84, 0x87, 0x7f, 0x1e, 0xb8, 0xe5, 0xd9, 0x74, 0xd8, 0x73, 0xe0, 0x65, 0x22, 0x49, 0x01, 0x55,
        0x5f, 0xb8, 0x82, 0x15, 0x90, 0xa3, 0x3b, 0xac, 0xc6, 0x1e, 0x39, 0x70, 0x1c, 0xf9, 0xb4, 0x6b,
        0xd2, 0x5b, 0xf5, 0xf0, 0x59, 0x5b, 0xbe, 0x24, 0x65, 0x51, 0x41, 0x43, 0x8e, 0x7a, 0x10, 0x0b,
    };
    static constexpr uint8_t malleatedSignature[ED25519_SIGNATURE_SIZE] = {
        0xe5, 0x56, 0x43, 0x00, 0xc3, 0x60, 0xac, 0x72, 0x90, 0x86, 0xe2, 0xcc, 0x80, 0x6e, 0x82, 0x8a,
        0x84, 0x87, 0x7f, 0x1e, 0xb8, 0xe5, 0xd9, 0x74, 0xd8, 0x73, 0xe0, 0x65, 0x22, 0x49, 0x01, 0x55,
        0x4c, 0x8c, 0x78, 0x72, 0xaa, 0x06, 0x4e, 0x04, 0x9d, 0xbb, 0x30, 0x13, 0xfb, 0xf2, 0x93, 0x80,
        0xd2, 0x5b, 0xf5, 0xf0, 0x59, 0x5b, 0xbe, 0x24, 0x65, 0x51, 0x41, 0x43, 0x8e, 0x7a, 0x10, 0x1b,
    };

    uint8_t publicKey[ED25519_PUBLIC_KEY_SIZE];
    uint8_t signature[ED25519_SIGNATURE_SIZE];
    if (!derivePublicKey(seed, publicKey) ||
        sodium_memcmp(publicKey, expectedPublicKey, ED25519_PUBLIC_KEY_SIZE) != 0 ||
        !sign(seed, publicKey, nullptr, 0, signature) ||
        sodium_memcmp(signature, expectedSignature, ED25519_SIGNATURE_SIZE) != 0 ||
        !verify(publicKey, nullptr, 0, signature) || verify(publicKey, nullptr, 0, malleatedSignature)) {
        return false;
    }
    signature[0] ^= 0x01;
    return !verify(publicKey, nullptr, 0, signature);
#else
    return false;
#endif
}

} // namespace friendmesh::security
