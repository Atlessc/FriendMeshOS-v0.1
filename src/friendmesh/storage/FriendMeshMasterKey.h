#pragma once

#include "FriendMeshRecordCodec.h"

#include <cstddef>
#include <cstdint>

namespace friendmesh::storage
{

constexpr size_t STORAGE_KDF_SALT_SIZE = 16;
constexpr size_t STORAGE_DEVICE_BINDING_SIZE = 16;
constexpr size_t STORAGE_WRAPPED_KEY_HEADER_SIZE = 60;
constexpr size_t STORAGE_WRAPPED_KEY_SIZE = STORAGE_WRAPPED_KEY_HEADER_SIZE + STORAGE_KEY_SIZE + STORAGE_TAG_SIZE;
constexpr size_t STORAGE_WRAP_AAD_SIZE = STORAGE_WRAPPED_KEY_HEADER_SIZE + STORAGE_DEVICE_BINDING_SIZE;
constexpr size_t STORAGE_CREDENTIAL_MAX_SIZE = 64;
constexpr uint32_t STORAGE_KDF_MEMORY_MIN_KIB = 8;
constexpr uint32_t STORAGE_KDF_MEMORY_MAX_KIB = 2048;
constexpr uint32_t STORAGE_KDF_OPERATIONS_MIN = 1;
constexpr uint32_t STORAGE_KDF_OPERATIONS_MAX = 6;
constexpr uint32_t STORAGE_KDF_V1_MEMORY_KIB = 1024;
constexpr uint32_t STORAGE_KDF_V1_OPERATIONS = 3;

struct WrappedMasterKeyParameters {
    uint32_t memoryKiB = STORAGE_KDF_V1_MEMORY_KIB;
    uint32_t operations = STORAGE_KDF_V1_OPERATIONS;
    uint32_t generation = 1;
};

enum class MasterKeyResult : uint8_t {
    OK = 0,
    INVALID_ARGUMENT,
    INVALID_PARAMETERS,
    OUTPUT_TOO_SMALL,
    BAD_MAGIC,
    UNSUPPORTED_VERSION,
    UNSUPPORTED_KDF,
    UNSUPPORTED_AEAD,
    UNSUPPORTED_FLAGS,
    LENGTH_MISMATCH,
    CRYPTO_UNAVAILABLE,
    KDF_FAILED,
    WRAP_FAILED,
    AUTH_FAILED,
    UNSUPPORTED_DOMAIN,
    SUBKEY_FAILED,
};

class StoragePasswordKdf
{
  public:
    virtual ~StoragePasswordKdf() = default;
    virtual bool available() const = 0;
    virtual bool deriveWrappingKey(const uint8_t *credential, size_t credentialSize,
                                   const uint8_t salt[STORAGE_KDF_SALT_SIZE], uint32_t operations,
                                   uint32_t memoryKiB, uint8_t output[STORAGE_KEY_SIZE]) const = 0;
};

class StorageSubkeyKdf
{
  public:
    virtual ~StorageSubkeyKdf() = default;
    virtual bool available() const = 0;
    virtual bool deriveStorageSubkey(const uint8_t masterKey[STORAGE_KEY_SIZE], uint64_t subkeyId,
                                     const char context[8], uint8_t output[STORAGE_KEY_SIZE]) const = 0;
};

enum class StorageKeyDomain : uint8_t {
    SIGNING_IDENTITY = 1,
    REPLAY_STATE = 2,
    TRANSACTION = 3,
    GROUP_STATE = 4,
    HISTORY = 5,
    OUTBOX = 6,
    CONTACT = 7,
    NOTIFICATION = 8,
    KEY_AUDIT = 9,
    DRAFT = 10,
};

class FriendMeshMasterKey
{
  public:
    static MasterKeyResult inspect(const uint8_t *record, size_t recordSize,
                                   WrappedMasterKeyParameters &parameters);
    static MasterKeyResult wrap(const StoragePasswordKdf &kdf, const StorageAead &aead, const uint8_t *credential,
                                size_t credentialSize, const uint8_t deviceBinding[STORAGE_DEVICE_BINDING_SIZE],
                                const WrappedMasterKeyParameters &parameters,
                                const uint8_t salt[STORAGE_KDF_SALT_SIZE],
                                const uint8_t nonce[STORAGE_NONCE_SIZE],
                                const uint8_t masterKey[STORAGE_KEY_SIZE], uint8_t *record, size_t recordCapacity,
                                size_t &recordSize);
    static MasterKeyResult unwrap(const StoragePasswordKdf &kdf, const StorageAead &aead, const uint8_t *credential,
                                  size_t credentialSize, const uint8_t deviceBinding[STORAGE_DEVICE_BINDING_SIZE],
                                  const uint8_t *record, size_t recordSize, WrappedMasterKeyParameters &parameters,
                                  uint8_t masterKey[STORAGE_KEY_SIZE]);
    static MasterKeyResult deriveSubkey(const StorageSubkeyKdf &kdf, const uint8_t masterKey[STORAGE_KEY_SIZE],
                                        StorageKeyDomain domain, uint8_t subkey[STORAGE_KEY_SIZE]);
    static const char *resultName(MasterKeyResult result);
};

} // namespace friendmesh::storage
