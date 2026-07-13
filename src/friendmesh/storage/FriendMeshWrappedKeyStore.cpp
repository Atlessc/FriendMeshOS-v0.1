#include "FriendMeshWrappedKeyStore.h"

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

bool equalParameters(const WrappedMasterKeyParameters &left, const WrappedMasterKeyParameters &right)
{
    return left.memoryKiB == right.memoryKiB && left.operations == right.operations &&
           left.generation == right.generation;
}
} // namespace

WrappedKeyStoreResult FriendMeshWrappedKeyStore::load(
    const StoragePasswordKdf &kdf, const StorageAead &aead, const uint8_t *credential, size_t credentialSize,
    const uint8_t deviceBinding[STORAGE_DEVICE_BINDING_SIZE], WrappedKeyLoadInfo &info,
    uint8_t masterKey[STORAGE_KEY_SIZE]) const
{
    info = WrappedKeyLoadInfo{};
    if (masterKey) {
        wipe(masterKey, STORAGE_KEY_SIZE);
    }
    if (!credential || credentialSize == 0 || !deviceBinding || !masterKey) {
        return WrappedKeyStoreResult::INVALID_ARGUMENT;
    }
    if (!backend.available()) {
        return WrappedKeyStoreResult::STORAGE_UNAVAILABLE;
    }

    uint8_t records[WRAPPED_KEY_SLOT_COUNT][STORAGE_WRAPPED_KEY_SIZE]{};
    uint8_t candidateKeys[WRAPPED_KEY_SLOT_COUNT][STORAGE_KEY_SIZE]{};
    WrappedMasterKeyParameters candidateParameters[WRAPPED_KEY_SLOT_COUNT]{};
    for (uint8_t slot = 0; slot < WRAPPED_KEY_SLOT_COUNT; ++slot) {
        size_t size = 0;
        const WrappedKeySlotReadResult read = backend.readSlot(slot, records[slot], sizeof(records[slot]), size);
        if (read == WrappedKeySlotReadResult::NOT_FOUND) {
            continue;
        }
        if (read == WrappedKeySlotReadResult::IO_ERROR) {
            info.ioErrorMask |= static_cast<uint8_t>(1U << slot);
            continue;
        }
        info.presentMask |= static_cast<uint8_t>(1U << slot);
        if (size != STORAGE_WRAPPED_KEY_SIZE) {
            continue;
        }
        if (FriendMeshMasterKey::unwrap(kdf, aead, credential, credentialSize, deviceBinding, records[slot], size,
                                        candidateParameters[slot], candidateKeys[slot]) == MasterKeyResult::OK) {
            info.validMask |= static_cast<uint8_t>(1U << slot);
        }
    }

    const uint8_t validCount = static_cast<uint8_t>((info.validMask & 1U) + ((info.validMask >> 1U) & 1U));
    if (validCount == 0) {
        wipe(candidateKeys, sizeof(candidateKeys));
        if (info.ioErrorMask != 0) {
            return WrappedKeyStoreResult::READ_FAILED;
        }
        return info.presentMask == 0 ? WrappedKeyStoreResult::EMPTY : WrappedKeyStoreResult::NO_AUTHENTICATED_KEY;
    }

    uint8_t selected = (info.validMask & 1U) != 0 ? 0 : 1;
    if (validCount == 2) {
        if (candidateParameters[0].generation == candidateParameters[1].generation) {
            if (!equalParameters(candidateParameters[0], candidateParameters[1]) ||
                std::memcmp(candidateKeys[0], candidateKeys[1], STORAGE_KEY_SIZE) != 0) {
                wipe(candidateKeys, sizeof(candidateKeys));
                return WrappedKeyStoreResult::GENERATION_CONFLICT;
            }
        } else if (candidateParameters[1].generation > candidateParameters[0].generation) {
            selected = 1;
        }
    }

    info.selectedSlot = selected;
    info.parameters = candidateParameters[selected];
    info.degraded = info.validMask != 0x03 || info.ioErrorMask != 0;
    std::memcpy(masterKey, candidateKeys[selected], STORAGE_KEY_SIZE);
    wipe(candidateKeys, sizeof(candidateKeys));
    return WrappedKeyStoreResult::OK;
}

WrappedKeyStoreResult FriendMeshWrappedKeyStore::commitPrepared(const uint8_t *record, size_t recordSize,
                                                                const WrappedKeyLoadInfo *current,
                                                                uint8_t &slotWritten)
{
    slotWritten = 0;
    if (!record) {
        return WrappedKeyStoreResult::INVALID_ARGUMENT;
    }
    if (!backend.available()) {
        return WrappedKeyStoreResult::STORAGE_UNAVAILABLE;
    }
    WrappedMasterKeyParameters prepared;
    if (FriendMeshMasterKey::inspect(record, recordSize, prepared) != MasterKeyResult::OK) {
        return WrappedKeyStoreResult::INVALID_ARGUMENT;
    }

    uint8_t targetSlot = 0;
    if (current) {
        if (current->selectedSlot >= WRAPPED_KEY_SLOT_COUNT || current->parameters.generation == 0 ||
            current->parameters.generation == std::numeric_limits<uint32_t>::max() ||
            prepared.generation != current->parameters.generation + 1U) {
            return WrappedKeyStoreResult::STALE_GENERATION;
        }
        targetSlot = static_cast<uint8_t>(current->selectedSlot ^ 1U);
    } else if (prepared.generation != 1) {
        return WrappedKeyStoreResult::STALE_GENERATION;
    }

    if (!backend.writeSlot(targetSlot, record, recordSize)) {
        return WrappedKeyStoreResult::WRITE_FAILED;
    }
    uint8_t readback[STORAGE_WRAPPED_KEY_SIZE]{};
    size_t readbackSize = 0;
    const WrappedKeySlotReadResult read = backend.readSlot(targetSlot, readback, sizeof(readback), readbackSize);
    const bool matches = read == WrappedKeySlotReadResult::OK && readbackSize == recordSize &&
                         std::memcmp(readback, record, recordSize) == 0;
    wipe(readback, sizeof(readback));
    if (!matches) {
        return WrappedKeyStoreResult::READBACK_FAILED;
    }
    slotWritten = targetSlot;
    return WrappedKeyStoreResult::OK;
}

const char *FriendMeshWrappedKeyStore::resultName(WrappedKeyStoreResult result)
{
    switch (result) {
    case WrappedKeyStoreResult::OK: return "OK";
    case WrappedKeyStoreResult::EMPTY: return "EMPTY";
    case WrappedKeyStoreResult::INVALID_ARGUMENT: return "INVALID_ARGUMENT";
    case WrappedKeyStoreResult::STORAGE_UNAVAILABLE: return "STORAGE_UNAVAILABLE";
    case WrappedKeyStoreResult::READ_FAILED: return "READ_FAILED";
    case WrappedKeyStoreResult::NO_AUTHENTICATED_KEY: return "NO_AUTHENTICATED_KEY";
    case WrappedKeyStoreResult::GENERATION_CONFLICT: return "GENERATION_CONFLICT";
    case WrappedKeyStoreResult::STALE_GENERATION: return "STALE_GENERATION";
    case WrappedKeyStoreResult::WRITE_FAILED: return "WRITE_FAILED";
    case WrappedKeyStoreResult::READBACK_FAILED: return "READBACK_FAILED";
    }
    return "UNKNOWN";
}

} // namespace friendmesh::storage
