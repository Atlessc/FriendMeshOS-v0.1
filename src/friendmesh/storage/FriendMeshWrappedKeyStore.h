#pragma once

#include "FriendMeshMasterKey.h"

#include <cstddef>
#include <cstdint>

namespace friendmesh::storage
{

constexpr uint8_t WRAPPED_KEY_SLOT_COUNT = 2;

enum class WrappedKeySlotReadResult : uint8_t {
    OK = 0,
    NOT_FOUND,
    IO_ERROR,
};

class WrappedKeySlotBackend
{
  public:
    virtual ~WrappedKeySlotBackend() = default;
    virtual bool available() const = 0;
    virtual WrappedKeySlotReadResult readSlot(uint8_t slot, uint8_t *record, size_t capacity,
                                              size_t &recordSize) const = 0;
    virtual bool writeSlot(uint8_t slot, const uint8_t *record, size_t recordSize) = 0;
};

enum class WrappedKeyStoreResult : uint8_t {
    OK = 0,
    EMPTY,
    INVALID_ARGUMENT,
    STORAGE_UNAVAILABLE,
    READ_FAILED,
    NO_AUTHENTICATED_KEY,
    GENERATION_CONFLICT,
    STALE_GENERATION,
    WRITE_FAILED,
    READBACK_FAILED,
};

struct WrappedKeyLoadInfo {
    WrappedMasterKeyParameters parameters{};
    uint8_t selectedSlot = 0;
    uint8_t presentMask = 0;
    uint8_t validMask = 0;
    uint8_t ioErrorMask = 0;
    bool degraded = false;
};

class FriendMeshWrappedKeyStore
{
  public:
    explicit FriendMeshWrappedKeyStore(WrappedKeySlotBackend &backend) : backend(backend) {}

    WrappedKeyStoreResult load(const StoragePasswordKdf &kdf, const StorageAead &aead, const uint8_t *credential,
                               size_t credentialSize,
                               const uint8_t deviceBinding[STORAGE_DEVICE_BINDING_SIZE], WrappedKeyLoadInfo &info,
                               uint8_t masterKey[STORAGE_KEY_SIZE]) const;
    WrappedKeyStoreResult commitPrepared(const uint8_t *record, size_t recordSize,
                                         const WrappedKeyLoadInfo *current, uint8_t &slotWritten);
    static const char *resultName(WrappedKeyStoreResult result);

  private:
    WrappedKeySlotBackend &backend;
};

} // namespace friendmesh::storage
