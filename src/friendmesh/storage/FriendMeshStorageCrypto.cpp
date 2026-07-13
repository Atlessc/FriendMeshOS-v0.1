#include "FriendMeshStorageCrypto.h"

#include <cstring>

#if defined(ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32)
#include <sodium.h>
#endif

namespace friendmesh::storage
{

bool FriendMeshStorageCrypto::available() const
{
#if defined(ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32)
    return sodium_init() >= 0;
#else
    return false;
#endif
}

bool FriendMeshStorageCrypto::seal(const uint8_t key[STORAGE_KEY_SIZE], const uint8_t nonce[STORAGE_NONCE_SIZE],
                                   const uint8_t *aad, size_t aadSize, const uint8_t *plaintext,
                                   size_t plaintextSize, uint8_t *ciphertextAndTag,
                                   size_t &ciphertextAndTagSize) const
{
    ciphertextAndTagSize = 0;
#if defined(ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32)
    if (!key || !nonce || (!aad && aadSize != 0) || (!plaintext && plaintextSize != 0) || !ciphertextAndTag ||
        !available()) {
        return false;
    }
    unsigned long long resultSize = 0;
    const int result = crypto_aead_xchacha20poly1305_ietf_encrypt(ciphertextAndTag, &resultSize, plaintext,
                                                                 plaintextSize, aad, aadSize, nullptr, nonce, key);
    ciphertextAndTagSize = static_cast<size_t>(resultSize);
    return result == 0;
#else
    (void)key;
    (void)nonce;
    (void)aad;
    (void)aadSize;
    (void)plaintext;
    (void)plaintextSize;
    (void)ciphertextAndTag;
    return false;
#endif
}

bool FriendMeshStorageCrypto::open(const uint8_t key[STORAGE_KEY_SIZE], const uint8_t nonce[STORAGE_NONCE_SIZE],
                                   const uint8_t *aad, size_t aadSize, const uint8_t *ciphertextAndTag,
                                   size_t ciphertextAndTagSize, uint8_t *plaintext, size_t &plaintextSize) const
{
    plaintextSize = 0;
#if defined(ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32)
    if (!key || !nonce || (!aad && aadSize != 0) || !ciphertextAndTag || ciphertextAndTagSize < STORAGE_TAG_SIZE ||
        !plaintext || !available()) {
        return false;
    }
    unsigned long long resultSize = 0;
    const int result = crypto_aead_xchacha20poly1305_ietf_decrypt(plaintext, &resultSize, nullptr, ciphertextAndTag,
                                                                 ciphertextAndTagSize, aad, aadSize, nonce, key);
    plaintextSize = static_cast<size_t>(resultSize);
    return result == 0;
#else
    (void)key;
    (void)nonce;
    (void)aad;
    (void)aadSize;
    (void)ciphertextAndTag;
    (void)ciphertextAndTagSize;
    (void)plaintext;
    return false;
#endif
}

bool FriendMeshStorageCrypto::randomNonce(uint8_t nonce[STORAGE_NONCE_SIZE]) const
{
    return fill(nonce, STORAGE_NONCE_SIZE);
}

bool FriendMeshStorageCrypto::fill(uint8_t *output, size_t outputSize) const
{
#if defined(ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32)
    if (!output || outputSize == 0 || !available()) {
        return false;
    }
    do {
        randombytes_buf(output, outputSize);
        uint8_t combined = 0;
        for (size_t i = 0; i < outputSize; ++i) {
            combined |= output[i];
        }
        if (combined != 0) {
            return true;
        }
    } while (true);
#else
    (void)output;
    (void)outputSize;
    return false;
#endif
}

bool FriendMeshStorageCrypto::deriveWrappingKey(const uint8_t *credential, size_t credentialSize,
                                                const uint8_t salt[STORAGE_KDF_SALT_SIZE], uint32_t operations,
                                                uint32_t memoryKiB, uint8_t output[STORAGE_KEY_SIZE]) const
{
#if defined(ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32)
    if (!credential || credentialSize == 0 || credentialSize > STORAGE_CREDENTIAL_MAX_SIZE || !salt || !output ||
        operations < STORAGE_KDF_OPERATIONS_MIN || operations > STORAGE_KDF_OPERATIONS_MAX ||
        memoryKiB < STORAGE_KDF_MEMORY_MIN_KIB || memoryKiB > STORAGE_KDF_MEMORY_MAX_KIB || !available()) {
        return false;
    }
    const int result = crypto_pwhash(output, STORAGE_KEY_SIZE, reinterpret_cast<const char *>(credential),
                                     credentialSize, salt, operations, static_cast<size_t>(memoryKiB) * 1024U,
                                     crypto_pwhash_ALG_ARGON2ID13);
    if (result != 0) {
        sodium_memzero(output, STORAGE_KEY_SIZE);
    }
    return result == 0;
#else
    (void)credential;
    (void)credentialSize;
    (void)salt;
    (void)operations;
    (void)memoryKiB;
    (void)output;
    return false;
#endif
}

bool FriendMeshStorageCrypto::deriveStorageSubkey(const uint8_t masterKey[STORAGE_KEY_SIZE], uint64_t subkeyId,
                                                  const char context[8], uint8_t output[STORAGE_KEY_SIZE]) const
{
#if defined(ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32)
    if (!masterKey || !context || !output || !available()) {
        return false;
    }
    const int result = crypto_kdf_derive_from_key(output, STORAGE_KEY_SIZE, subkeyId, context, masterKey);
    if (result != 0) {
        sodium_memzero(output, STORAGE_KEY_SIZE);
    }
    return result == 0;
#else
    (void)masterKey;
    (void)subkeyId;
    (void)context;
    (void)output;
    return false;
#endif
}

bool FriendMeshStorageCrypto::selfTest() const
{
#if defined(ARCH_ESP32) || defined(ARDUINO_ARCH_ESP32)
    static constexpr uint8_t key[STORAGE_KEY_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    };
    static constexpr uint8_t nonce[STORAGE_NONCE_SIZE] = {
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab,
        0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
    };
    static constexpr uint8_t plaintext[] = "FriendMesh storage vector v1";
    static constexpr uint8_t expectedSealed[] = {
        0x03, 0x9e, 0x63, 0x78, 0xd9, 0x39, 0xe9, 0xc9, 0x12, 0x07, 0x0d, 0xfa, 0x05, 0x5f, 0xd1,
        0xa1, 0x5c, 0xcf, 0xbf, 0x5a, 0xcf, 0x65, 0xbe, 0xb6, 0x6f, 0x60, 0xfd, 0xd6, 0x9d, 0xe9,
        0xe6, 0x67, 0x65, 0x51, 0x8b, 0xcc, 0xe9, 0x59, 0x1c, 0xa4, 0x80, 0x55, 0xeb, 0xb9,
    };
    StorageRecordMetadata metadata;
    metadata.schemaVersion = 1;
    metadata.keyEpoch = 7;
    metadata.sequence = 42;
    for (size_t i = 0; i < STORAGE_ID_SIZE; ++i) {
        metadata.scopeId[i] = static_cast<uint8_t>(0x10 + i);
        metadata.recordId[i] = static_cast<uint8_t>(0x30 + i);
    }
    uint8_t record[STORAGE_RECORD_HEADER_SIZE + sizeof(expectedSealed)];
    size_t recordSize = 0;
    if (FriendMeshRecordCodec::encode(*this, key, nonce, metadata, plaintext, sizeof(plaintext) - 1, record,
                                      sizeof(record), recordSize) != StorageCodecResult::OK ||
        recordSize != sizeof(record) ||
        std::memcmp(record + STORAGE_RECORD_HEADER_SIZE, expectedSealed, sizeof(expectedSealed)) != 0) {
        return false;
    }
    uint8_t opened[sizeof(plaintext) - 1];
    size_t openedSize = 0;
    StorageRecordMetadata decoded;
    StorageRecordExpectation expectation;
    return FriendMeshRecordCodec::decode(*this, key, expectation, record, recordSize, decoded, opened, sizeof(opened),
                                         openedSize) == StorageCodecResult::OK &&
           openedSize == sizeof(opened) && std::memcmp(opened, plaintext, sizeof(opened)) == 0;
#else
    return false;
#endif
}

} // namespace friendmesh::storage
