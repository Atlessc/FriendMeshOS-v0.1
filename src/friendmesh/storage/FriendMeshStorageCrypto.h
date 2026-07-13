#pragma once

#include "FriendMeshRecordCodec.h"

namespace friendmesh::storage
{

class FriendMeshStorageCrypto final : public StorageAead
{
  public:
    bool available() const override;
    bool seal(const uint8_t key[STORAGE_KEY_SIZE], const uint8_t nonce[STORAGE_NONCE_SIZE], const uint8_t *aad,
              size_t aadSize, const uint8_t *plaintext, size_t plaintextSize, uint8_t *ciphertextAndTag,
              size_t &ciphertextAndTagSize) const override;
    bool open(const uint8_t key[STORAGE_KEY_SIZE], const uint8_t nonce[STORAGE_NONCE_SIZE], const uint8_t *aad,
              size_t aadSize, const uint8_t *ciphertextAndTag, size_t ciphertextAndTagSize, uint8_t *plaintext,
              size_t &plaintextSize) const override;
    bool randomNonce(uint8_t nonce[STORAGE_NONCE_SIZE]) const;
    bool selfTest() const;
};

} // namespace friendmesh::storage
