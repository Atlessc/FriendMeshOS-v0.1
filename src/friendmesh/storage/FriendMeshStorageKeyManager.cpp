#include "FriendMeshStorageKeyManager.h"

#include <cstring>
#include <limits>

namespace friendmesh::storage
{
namespace
{
void wipe(void *value, size_t size)
{
    volatile uint8_t *bytes = static_cast<volatile uint8_t *>(value);
    while (size-- > 0) {
        *bytes++ = 0;
    }
}

bool equal(const uint8_t *left, const uint8_t *right, size_t size)
{
    uint8_t difference = 0;
    for (size_t index = 0; index < size; ++index) {
        difference |= left[index] ^ right[index];
    }
    return difference == 0;
}
} // namespace

DeviceBindingResult FriendMeshHardwareDeviceBinding::read(uint8_t binding[STORAGE_DEVICE_BINDING_SIZE]) const
{
    return hardwareDeviceBinding(binding);
}

FriendMeshStorageKeyManager::FriendMeshStorageKeyManager(
    FriendMeshWrappedKeyStore &store, WrappedKeySlotBackend &backend, const StoragePasswordKdf &passwordKdf,
    const StorageAead &aead, const StorageSubkeyKdf &subkeyKdf, const StorageRandomSource &random,
    const StorageDeviceBindingSource &bindingSource)
    : store(store), backend(backend), passwordKdf(passwordKdf), aead(aead), subkeyKdf(subkeyKdf), random(random),
      bindingSource(bindingSource)
{
}

FriendMeshStorageKeyManager::~FriendMeshStorageKeyManager()
{
    clearMasterKey();
}

bool FriendMeshStorageKeyManager::validPin(const uint8_t *pin, size_t pinSize)
{
    if (!pin || pinSize != STORAGE_PIN_SIZE) {
        return false;
    }
    bool nonzero = false;
    for (size_t index = 0; index < pinSize; ++index) {
        if (pin[index] < '0' || pin[index] > '9') {
            return false;
        }
        nonzero = nonzero || pin[index] != '0';
    }
    return nonzero;
}

StorageKeyStatus FriendMeshStorageKeyManager::probe()
{
    clearMasterKey();
    if (!passwordKdf.available() || !aead.available() || !subkeyKdf.available()) {
        currentStatus = StorageKeyStatus::CRYPTO_UNAVAILABLE;
        setResult(StorageKeyResult::CRYPTO_UNAVAILABLE);
        return currentStatus;
    }
    if (!backend.available()) {
        currentStatus = StorageKeyStatus::STORAGE_UNAVAILABLE;
        setResult(StorageKeyResult::STORAGE_UNAVAILABLE);
        return currentStatus;
    }
    uint8_t binding[STORAGE_DEVICE_BINDING_SIZE]{};
    const DeviceBindingResult bindingResult = bindingSource.read(binding);
    wipe(binding, sizeof(binding));
    if (bindingResult != DeviceBindingResult::OK) {
        currentStatus = StorageKeyStatus::BINDING_UNAVAILABLE;
        setResult(StorageKeyResult::BINDING_UNAVAILABLE);
        return currentStatus;
    }

    bool present = false;
    bool structurallyValid = false;
    uint8_t record[STORAGE_WRAPPED_KEY_SIZE]{};
    for (uint8_t slot = 0; slot < WRAPPED_KEY_SLOT_COUNT; ++slot) {
        size_t recordSize = 0;
        const WrappedKeySlotReadResult read = backend.readSlot(slot, record, sizeof(record), recordSize);
        wipe(record, sizeof(record));
        if (read == WrappedKeySlotReadResult::IO_ERROR) {
            currentStatus = StorageKeyStatus::CORRUPT;
            setResult(StorageKeyResult::CORRUPT);
            return currentStatus;
        }
        if (read == WrappedKeySlotReadResult::OK) {
            present = true;
            structurallyValid = structurallyValid || recordSize == STORAGE_WRAPPED_KEY_SIZE;
        }
    }
    currentStatus = !present                  ? StorageKeyStatus::NOT_CONFIGURED
                    : structurallyValid       ? StorageKeyStatus::LOCKED
                                              : StorageKeyStatus::CORRUPT;
    setResult(currentStatus == StorageKeyStatus::NOT_CONFIGURED ? StorageKeyResult::NOT_CONFIGURED
              : currentStatus == StorageKeyStatus::LOCKED       ? StorageKeyResult::LOCKED
                                                                 : StorageKeyResult::CORRUPT);
    return currentStatus;
}

StorageKeyResult FriendMeshStorageKeyManager::prepareWrapped(
    const uint8_t *pin, size_t pinSize, uint32_t generation, const uint8_t key[STORAGE_KEY_SIZE],
    uint8_t record[STORAGE_WRAPPED_KEY_SIZE])
{
    uint8_t binding[STORAGE_DEVICE_BINDING_SIZE]{};
    uint8_t salt[STORAGE_KDF_SALT_SIZE]{};
    uint8_t nonce[STORAGE_NONCE_SIZE]{};
    size_t recordSize = 0;
    StorageKeyResult result = StorageKeyResult::OK;
    if (bindingSource.read(binding) != DeviceBindingResult::OK) {
        result = StorageKeyResult::BINDING_UNAVAILABLE;
    } else if (!random.fill(salt, sizeof(salt)) || !random.fill(nonce, sizeof(nonce))) {
        result = StorageKeyResult::RANDOM_FAILED;
    } else {
        WrappedMasterKeyParameters parameters;
        parameters.generation = generation;
        if (FriendMeshMasterKey::wrap(passwordKdf, aead, pin, pinSize, binding, parameters, salt, nonce, key, record,
                                      STORAGE_WRAPPED_KEY_SIZE, recordSize) != MasterKeyResult::OK ||
            recordSize != STORAGE_WRAPPED_KEY_SIZE) {
            result = StorageKeyResult::WRAP_FAILED;
        }
    }
    wipe(binding, sizeof(binding));
    wipe(salt, sizeof(salt));
    wipe(nonce, sizeof(nonce));
    if (result != StorageKeyResult::OK) {
        wipe(record, STORAGE_WRAPPED_KEY_SIZE);
    }
    return result;
}

StorageKeyResult FriendMeshStorageKeyManager::loadWithPin(const uint8_t *pin, size_t pinSize,
                                                          WrappedKeyLoadInfo &info,
                                                          uint8_t key[STORAGE_KEY_SIZE]) const
{
    uint8_t binding[STORAGE_DEVICE_BINDING_SIZE]{};
    if (bindingSource.read(binding) != DeviceBindingResult::OK) {
        wipe(binding, sizeof(binding));
        return StorageKeyResult::BINDING_UNAVAILABLE;
    }
    const WrappedKeyStoreResult result = store.load(passwordKdf, aead, pin, pinSize, binding, info, key);
    wipe(binding, sizeof(binding));
    switch (result) {
    case WrappedKeyStoreResult::OK: return StorageKeyResult::OK;
    case WrappedKeyStoreResult::EMPTY: return StorageKeyResult::NOT_CONFIGURED;
    case WrappedKeyStoreResult::STORAGE_UNAVAILABLE: return StorageKeyResult::STORAGE_UNAVAILABLE;
    case WrappedKeyStoreResult::NO_AUTHENTICATED_KEY: return StorageKeyResult::NO_AUTHENTICATED_KEY;
    default: return StorageKeyResult::CORRUPT;
    }
}

StorageKeyResult FriendMeshStorageKeyManager::commitGeneration(
    const uint8_t *pin, size_t pinSize, uint32_t generation, const uint8_t key[STORAGE_KEY_SIZE],
    const WrappedKeyLoadInfo *current, WrappedKeyLoadInfo &verified)
{
    uint8_t record[STORAGE_WRAPPED_KEY_SIZE]{};
    StorageKeyResult result = prepareWrapped(pin, pinSize, generation, key, record);
    if (result == StorageKeyResult::OK) {
        uint8_t slotWritten = 0;
        if (store.commitPrepared(record, sizeof(record), current, slotWritten) != WrappedKeyStoreResult::OK) {
            result = StorageKeyResult::WRITE_FAILED;
        } else {
            uint8_t verifiedKey[STORAGE_KEY_SIZE]{};
            result = loadWithPin(pin, pinSize, verified, verifiedKey);
            if (result == StorageKeyResult::OK &&
                (verified.parameters.generation != generation || !equal(key, verifiedKey, sizeof(verifiedKey)))) {
                result = StorageKeyResult::VERIFY_FAILED;
            }
            wipe(verifiedKey, sizeof(verifiedKey));
        }
    }
    wipe(record, sizeof(record));
    return result;
}

StorageKeyResult FriendMeshStorageKeyManager::provision(const uint8_t *pin, size_t pinSize)
{
    if (!validPin(pin, pinSize)) {
        return setResult(StorageKeyResult::INVALID_PIN);
    }
    if (ready() || currentStatus == StorageKeyStatus::LOCKED) {
        return setResult(StorageKeyResult::ALREADY_CONFIGURED);
    }
    if (currentStatus != StorageKeyStatus::NOT_CONFIGURED && probe() != StorageKeyStatus::NOT_CONFIGURED) {
        return setResult(currentStatus == StorageKeyStatus::LOCKED ? StorageKeyResult::ALREADY_CONFIGURED
                                                                   : currentResult);
    }
    uint8_t candidate[STORAGE_KEY_SIZE]{};
    if (!random.fill(candidate, sizeof(candidate))) {
        wipe(candidate, sizeof(candidate));
        return setResult(StorageKeyResult::RANDOM_FAILED);
    }
    WrappedKeyLoadInfo first;
    StorageKeyResult result = commitGeneration(pin, pinSize, 1, candidate, nullptr, first);
    if (result == StorageKeyResult::OK) {
        WrappedKeyLoadInfo second;
        result = commitGeneration(pin, pinSize, 2, candidate, &first, second);
        if (result == StorageKeyResult::OK) {
            std::memcpy(masterKey, candidate, sizeof(masterKey));
            loadInfo = second;
            currentStatus = second.degraded ? StorageKeyStatus::DEGRADED : StorageKeyStatus::READY;
        }
    }
    wipe(candidate, sizeof(candidate));
    if (result != StorageKeyResult::OK) {
        clearMasterKey();
        currentStatus = StorageKeyStatus::CORRUPT;
    }
    return setResult(result);
}

StorageKeyResult FriendMeshStorageKeyManager::unlock(const uint8_t *pin, size_t pinSize)
{
    if (!validPin(pin, pinSize)) {
        return setResult(StorageKeyResult::INVALID_PIN);
    }
    clearMasterKey();
    WrappedKeyLoadInfo info;
    uint8_t candidate[STORAGE_KEY_SIZE]{};
    const StorageKeyResult result = loadWithPin(pin, pinSize, info, candidate);
    if (result == StorageKeyResult::OK) {
        std::memcpy(masterKey, candidate, sizeof(masterKey));
        loadInfo = info;
        currentStatus = info.degraded ? StorageKeyStatus::DEGRADED : StorageKeyStatus::READY;
    } else if (result == StorageKeyResult::NOT_CONFIGURED) {
        currentStatus = StorageKeyStatus::NOT_CONFIGURED;
    } else if (result == StorageKeyResult::NO_AUTHENTICATED_KEY) {
        currentStatus = StorageKeyStatus::LOCKED;
    } else if (result == StorageKeyResult::STORAGE_UNAVAILABLE) {
        currentStatus = StorageKeyStatus::STORAGE_UNAVAILABLE;
    } else if (result == StorageKeyResult::BINDING_UNAVAILABLE) {
        currentStatus = StorageKeyStatus::BINDING_UNAVAILABLE;
    } else {
        currentStatus = StorageKeyStatus::CORRUPT;
    }
    wipe(candidate, sizeof(candidate));
    return setResult(result);
}

StorageKeyResult FriendMeshStorageKeyManager::rewrap(const uint8_t *currentPin, size_t currentPinSize,
                                                     const uint8_t *newPin, size_t newPinSize)
{
    if (!validPin(currentPin, currentPinSize) || !validPin(newPin, newPinSize)) {
        return setResult(StorageKeyResult::INVALID_PIN);
    }
    if (!ready()) {
        return setResult(StorageKeyResult::LOCKED);
    }
    WrappedKeyLoadInfo current;
    uint8_t verifiedCurrent[STORAGE_KEY_SIZE]{};
    StorageKeyResult result = loadWithPin(currentPin, currentPinSize, current, verifiedCurrent);
    if (result != StorageKeyResult::OK || !equal(masterKey, verifiedCurrent, sizeof(masterKey))) {
        wipe(verifiedCurrent, sizeof(verifiedCurrent));
        return setResult(result == StorageKeyResult::NO_AUTHENTICATED_KEY ? result : StorageKeyResult::VERIFY_FAILED);
    }
    wipe(verifiedCurrent, sizeof(verifiedCurrent));
    if (current.parameters.generation > std::numeric_limits<uint32_t>::max() - 2U) {
        return setResult(StorageKeyResult::CORRUPT);
    }

    WrappedKeyLoadInfo firstNew;
    result = commitGeneration(newPin, newPinSize, current.parameters.generation + 1U, masterKey, &current, firstNew);
    if (result != StorageKeyResult::OK) {
        return setResult(result);
    }
    if (!backend.clearSlot(current.selectedSlot)) {
        currentStatus = StorageKeyStatus::DEGRADED;
        return setResult(StorageKeyResult::RETIRE_FAILED);
    }

    WrappedKeyLoadInfo retired;
    uint8_t verifiedNew[STORAGE_KEY_SIZE]{};
    result = loadWithPin(newPin, newPinSize, retired, verifiedNew);
    const bool retiredValid = result == StorageKeyResult::OK &&
                              retired.parameters.generation == firstNew.parameters.generation &&
                              equal(masterKey, verifiedNew, sizeof(masterKey));
    wipe(verifiedNew, sizeof(verifiedNew));
    if (!retiredValid) {
        currentStatus = StorageKeyStatus::CORRUPT;
        return setResult(StorageKeyResult::VERIFY_FAILED);
    }

    WrappedKeyLoadInfo secondNew;
    result = commitGeneration(newPin, newPinSize, retired.parameters.generation + 1U, masterKey, &retired, secondNew);
    if (result == StorageKeyResult::OK) {
        loadInfo = secondNew;
        currentStatus = secondNew.degraded ? StorageKeyStatus::DEGRADED : StorageKeyStatus::READY;
    } else {
        loadInfo = retired;
        currentStatus = StorageKeyStatus::DEGRADED;
    }
    return setResult(result);
}

void FriendMeshStorageKeyManager::lock()
{
    clearMasterKey();
    probe();
}

StorageKeyResult FriendMeshStorageKeyManager::deriveSubkey(StorageKeyDomain domain,
                                                           uint8_t output[STORAGE_KEY_SIZE]) const
{
    if (output) {
        wipe(output, STORAGE_KEY_SIZE);
    }
    if (!output) {
        return StorageKeyResult::INVALID_ARGUMENT;
    }
    if (!ready()) {
        return StorageKeyResult::LOCKED;
    }
    return FriendMeshMasterKey::deriveSubkey(subkeyKdf, masterKey, domain, output) == MasterKeyResult::OK
               ? StorageKeyResult::OK
               : StorageKeyResult::SUBKEY_FAILED;
}

void FriendMeshStorageKeyManager::clearMasterKey()
{
    wipe(masterKey, sizeof(masterKey));
    loadInfo = WrappedKeyLoadInfo{};
}

StorageKeyResult FriendMeshStorageKeyManager::setResult(StorageKeyResult result)
{
    currentResult = result;
    return result;
}

const char *FriendMeshStorageKeyManager::statusName(StorageKeyStatus status)
{
    switch (status) {
    case StorageKeyStatus::STORAGE_UNAVAILABLE: return "STORAGE_UNAVAILABLE";
    case StorageKeyStatus::CRYPTO_UNAVAILABLE: return "CRYPTO_UNAVAILABLE";
    case StorageKeyStatus::BINDING_UNAVAILABLE: return "BINDING_UNAVAILABLE";
    case StorageKeyStatus::NOT_CONFIGURED: return "NOT_CONFIGURED";
    case StorageKeyStatus::LOCKED: return "LOCKED";
    case StorageKeyStatus::READY: return "READY";
    case StorageKeyStatus::DEGRADED: return "DEGRADED";
    case StorageKeyStatus::CORRUPT: return "CORRUPT";
    }
    return "UNKNOWN";
}

const char *FriendMeshStorageKeyManager::resultName(StorageKeyResult result)
{
    switch (result) {
    case StorageKeyResult::OK: return "OK";
    case StorageKeyResult::INVALID_ARGUMENT: return "INVALID_ARGUMENT";
    case StorageKeyResult::INVALID_PIN: return "INVALID_PIN";
    case StorageKeyResult::NOT_CONFIGURED: return "NOT_CONFIGURED";
    case StorageKeyResult::ALREADY_CONFIGURED: return "ALREADY_CONFIGURED";
    case StorageKeyResult::LOCKED: return "LOCKED";
    case StorageKeyResult::STORAGE_UNAVAILABLE: return "STORAGE_UNAVAILABLE";
    case StorageKeyResult::CRYPTO_UNAVAILABLE: return "CRYPTO_UNAVAILABLE";
    case StorageKeyResult::BINDING_UNAVAILABLE: return "BINDING_UNAVAILABLE";
    case StorageKeyResult::NO_AUTHENTICATED_KEY: return "NO_AUTHENTICATED_KEY";
    case StorageKeyResult::CORRUPT: return "CORRUPT";
    case StorageKeyResult::RANDOM_FAILED: return "RANDOM_FAILED";
    case StorageKeyResult::WRAP_FAILED: return "WRAP_FAILED";
    case StorageKeyResult::WRITE_FAILED: return "WRITE_FAILED";
    case StorageKeyResult::VERIFY_FAILED: return "VERIFY_FAILED";
    case StorageKeyResult::RETIRE_FAILED: return "RETIRE_FAILED";
    case StorageKeyResult::SUBKEY_FAILED: return "SUBKEY_FAILED";
    }
    return "UNKNOWN";
}

} // namespace friendmesh::storage
