#pragma once

#include "FriendMeshDeviceBinding.h"
#include "FriendMeshStorageCrypto.h"
#include "FriendMeshWrappedKeyStore.h"

namespace friendmesh::storage
{

constexpr size_t STORAGE_PIN_SIZE = 6;

class StorageDeviceBindingSource
{
  public:
    virtual ~StorageDeviceBindingSource() = default;
    virtual DeviceBindingResult read(uint8_t binding[STORAGE_DEVICE_BINDING_SIZE]) const = 0;
};

class FriendMeshHardwareDeviceBinding final : public StorageDeviceBindingSource
{
  public:
    DeviceBindingResult read(uint8_t binding[STORAGE_DEVICE_BINDING_SIZE]) const override;
};

enum class StorageKeyStatus : uint8_t {
    STORAGE_UNAVAILABLE = 0,
    CRYPTO_UNAVAILABLE,
    BINDING_UNAVAILABLE,
    NOT_CONFIGURED,
    LOCKED,
    READY,
    DEGRADED,
    CORRUPT,
};

enum class StorageKeyResult : uint8_t {
    OK = 0,
    INVALID_ARGUMENT,
    INVALID_PIN,
    NOT_CONFIGURED,
    ALREADY_CONFIGURED,
    LOCKED,
    STORAGE_UNAVAILABLE,
    CRYPTO_UNAVAILABLE,
    BINDING_UNAVAILABLE,
    NO_AUTHENTICATED_KEY,
    CORRUPT,
    RANDOM_FAILED,
    WRAP_FAILED,
    WRITE_FAILED,
    VERIFY_FAILED,
    RETIRE_FAILED,
    SUBKEY_FAILED,
};

class FriendMeshStorageKeyManager
{
  public:
    FriendMeshStorageKeyManager(FriendMeshWrappedKeyStore &store, WrappedKeySlotBackend &backend,
                                const StoragePasswordKdf &passwordKdf, const StorageAead &aead,
                                const StorageSubkeyKdf &subkeyKdf, const StorageRandomSource &random,
                                const StorageDeviceBindingSource &bindingSource);
    ~FriendMeshStorageKeyManager();

    StorageKeyStatus probe();
    StorageKeyResult provision(const uint8_t *pin, size_t pinSize);
    StorageKeyResult unlock(const uint8_t *pin, size_t pinSize);
    StorageKeyResult rewrap(const uint8_t *currentPin, size_t currentPinSize, const uint8_t *newPin,
                            size_t newPinSize);
    void lock();
    StorageKeyResult deriveSubkey(StorageKeyDomain domain, uint8_t output[STORAGE_KEY_SIZE]) const;

    StorageKeyStatus status() const { return currentStatus; }
    StorageKeyResult lastResult() const { return currentResult; }
    uint32_t generation() const { return ready() ? loadInfo.parameters.generation : 0; }
    bool ready() const
    {
        return currentStatus == StorageKeyStatus::READY || currentStatus == StorageKeyStatus::DEGRADED;
    }

    static bool validPin(const uint8_t *pin, size_t pinSize);
    static const char *statusName(StorageKeyStatus status);
    static const char *resultName(StorageKeyResult result);

  private:
    StorageKeyResult prepareWrapped(const uint8_t *pin, size_t pinSize, uint32_t generation,
                                    const uint8_t key[STORAGE_KEY_SIZE], uint8_t record[STORAGE_WRAPPED_KEY_SIZE]);
    StorageKeyResult loadWithPin(const uint8_t *pin, size_t pinSize, WrappedKeyLoadInfo &info,
                                 uint8_t key[STORAGE_KEY_SIZE]) const;
    StorageKeyResult commitGeneration(const uint8_t *pin, size_t pinSize, uint32_t generation,
                                      const uint8_t key[STORAGE_KEY_SIZE], const WrappedKeyLoadInfo *current,
                                      WrappedKeyLoadInfo &verified);
    void clearMasterKey();
    StorageKeyResult setResult(StorageKeyResult result);

    FriendMeshWrappedKeyStore &store;
    WrappedKeySlotBackend &backend;
    const StoragePasswordKdf &passwordKdf;
    const StorageAead &aead;
    const StorageSubkeyKdf &subkeyKdf;
    const StorageRandomSource &random;
    const StorageDeviceBindingSource &bindingSource;
    uint8_t masterKey[STORAGE_KEY_SIZE]{};
    WrappedKeyLoadInfo loadInfo{};
    StorageKeyStatus currentStatus = StorageKeyStatus::STORAGE_UNAVAILABLE;
    StorageKeyResult currentResult = StorageKeyResult::STORAGE_UNAVAILABLE;
};

} // namespace friendmesh::storage
