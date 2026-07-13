#pragma once

#include "FriendMeshMasterKey.h"

namespace friendmesh::storage
{

class FriendMeshStorageCrypto final : public StorageAead, public StoragePasswordKdf, public StorageSubkeyKdf
{
  public:
    bool available() const override;
    bool seal(const uint8_t key[STORAGE_KEY_SIZE], const uint8_t nonce[STORAGE_NONCE_SIZE], const uint8_t *aad,
              size_t aadSize, const uint8_t *plaintext, size_t plaintextSize, uint8_t *ciphertextAndTag,
              size_t &ciphertextAndTagSize) const override;
    bool open(const uint8_t key[STORAGE_KEY_SIZE], const uint8_t nonce[STORAGE_NONCE_SIZE], const uint8_t *aad,
              size_t aadSize, const uint8_t *ciphertextAndTag, size_t ciphertextAndTagSize, uint8_t *plaintext,
              size_t &plaintextSize) const override;
    bool deriveWrappingKey(const uint8_t *credential, size_t credentialSize,
                           const uint8_t salt[STORAGE_KDF_SALT_SIZE], uint32_t operations, uint32_t memoryKiB,
                           uint8_t output[STORAGE_KEY_SIZE]) const override;
    bool deriveStorageSubkey(const uint8_t masterKey[STORAGE_KEY_SIZE], uint64_t subkeyId, const char context[8],
                             uint8_t output[STORAGE_KEY_SIZE]) const override;
    bool randomNonce(uint8_t nonce[STORAGE_NONCE_SIZE]) const;
    bool selfTest() const;
};

} // namespace friendmesh::storage
