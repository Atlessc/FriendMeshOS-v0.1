#pragma once

#include <cstddef>
#include <cstdint>

namespace friendmesh::storage
{

constexpr size_t STORAGE_KEY_SIZE = 32;
constexpr size_t STORAGE_NONCE_SIZE = 24;
constexpr size_t STORAGE_TAG_SIZE = 16;
constexpr size_t STORAGE_ID_SIZE = 16;
constexpr size_t STORAGE_RECORD_HEADER_SIZE = 84;
constexpr size_t STORAGE_MAX_PLAINTEXT_SIZE = 4096;
constexpr size_t STORAGE_MAX_RECORD_SIZE = STORAGE_RECORD_HEADER_SIZE + STORAGE_MAX_PLAINTEXT_SIZE + STORAGE_TAG_SIZE;

enum class StorageRecordType : uint8_t {
    SIGNING_IDENTITY = 1,
    REPLAY_STATE = 2,
    TRANSACTION = 3,
    GROUP_STATE = 4,
    HISTORY = 5,
    OUTBOX = 6,
    CONTACT = 7,
    NOTIFICATION = 8,
};

enum class StorageCodecResult : uint8_t {
    OK = 0,
    INVALID_ARGUMENT,
    INVALID_METADATA,
    OUTPUT_TOO_SMALL,
    RECORD_TOO_SMALL,
    RECORD_TOO_LARGE,
    BAD_MAGIC,
    UNSUPPORTED_VERSION,
    UNSUPPORTED_TYPE,
    UNSUPPORTED_FLAGS,
    LENGTH_MISMATCH,
    CONTEXT_MISMATCH,
    CRYPTO_UNAVAILABLE,
    ENCRYPT_FAILED,
    AUTH_FAILED,
};

struct StorageRecordMetadata {
    StorageRecordType type = StorageRecordType::SIGNING_IDENTITY;
    uint32_t schemaVersion = 0;
    uint32_t keyEpoch = 0;
    uint64_t sequence = 0;
    uint8_t scopeId[STORAGE_ID_SIZE] = {};
    uint8_t recordId[STORAGE_ID_SIZE] = {};
};

struct StorageRecordExpectation {
    const StorageRecordType *type = nullptr;
    const uint8_t *scopeId = nullptr;
};

class StorageAead
{
  public:
    virtual ~StorageAead() = default;
    virtual bool available() const = 0;
    virtual bool seal(const uint8_t key[STORAGE_KEY_SIZE], const uint8_t nonce[STORAGE_NONCE_SIZE],
                      const uint8_t *aad, size_t aadSize, const uint8_t *plaintext, size_t plaintextSize,
                      uint8_t *ciphertextAndTag, size_t &ciphertextAndTagSize) const = 0;
    virtual bool open(const uint8_t key[STORAGE_KEY_SIZE], const uint8_t nonce[STORAGE_NONCE_SIZE],
                      const uint8_t *aad, size_t aadSize, const uint8_t *ciphertextAndTag,
                      size_t ciphertextAndTagSize, uint8_t *plaintext, size_t &plaintextSize) const = 0;
};

class FriendMeshRecordCodec
{
  public:
    static StorageCodecResult encode(const StorageAead &aead, const uint8_t key[STORAGE_KEY_SIZE],
                                     const uint8_t nonce[STORAGE_NONCE_SIZE], const StorageRecordMetadata &metadata,
                                     const uint8_t *plaintext, size_t plaintextSize, uint8_t *record,
                                     size_t recordCapacity, size_t &recordSize);
    static StorageCodecResult decode(const StorageAead &aead, const uint8_t key[STORAGE_KEY_SIZE],
                                     const StorageRecordExpectation &expectation, const uint8_t *record,
                                     size_t recordSize, StorageRecordMetadata &metadata, uint8_t *plaintext,
                                     size_t plaintextCapacity, size_t &plaintextSize);
    static const char *resultName(StorageCodecResult result);
};

} // namespace friendmesh::storage
