#include "FriendMeshInternalKeySlots.h"

#include "FSCommon.h"
#include "SPILock.h"
#include "SafeFile.h"

namespace friendmesh::storage
{
namespace
{
constexpr const char *PRODUCTION_SLOT_A_PATH = "/friendmesh/keys/master_a.bin";
constexpr const char *PRODUCTION_SLOT_B_PATH = "/friendmesh/keys/master_b.bin";
constexpr const char *PRODUCTION_SLOT_DIRECTORY = "/friendmesh/keys";
} // namespace

FriendMeshInternalKeySlots::FriendMeshInternalKeySlots()
    : FriendMeshInternalKeySlots(PRODUCTION_SLOT_A_PATH, PRODUCTION_SLOT_B_PATH, PRODUCTION_SLOT_DIRECTORY)
{
}

FriendMeshInternalKeySlots::FriendMeshInternalKeySlots(const char *slotAPath, const char *slotBPath,
                                                       const char *slotDirectory)
    : slotPaths{slotAPath, slotBPath}, slotDirectory(slotDirectory)
{
}

bool FriendMeshInternalKeySlots::available() const
{
#ifdef FSCom
    return slotPaths[0] && slotPaths[1] && slotDirectory;
#else
    return false;
#endif
}

WrappedKeySlotReadResult FriendMeshInternalKeySlots::readSlot(uint8_t slot, uint8_t *record, size_t capacity,
                                                              size_t &recordSize) const
{
    recordSize = 0;
    if (!available() || slot >= WRAPPED_KEY_SLOT_COUNT || !record) {
        return WrappedKeySlotReadResult::IO_ERROR;
    }
#ifdef FSCom
    concurrency::LockGuard guard(spiLock);
    if (!FSCom.exists(slotPaths[slot])) {
        return WrappedKeySlotReadResult::NOT_FOUND;
    }
    File file = FSCom.open(slotPaths[slot], FILE_O_READ);
    if (!file) {
        return WrappedKeySlotReadResult::IO_ERROR;
    }
    const size_t size = file.size();
    if (size > capacity) {
        file.close();
        return WrappedKeySlotReadResult::IO_ERROR;
    }
    const size_t read = file.read(record, size);
    file.close();
    if (read != size) {
        return WrappedKeySlotReadResult::IO_ERROR;
    }
    recordSize = read;
    return WrappedKeySlotReadResult::OK;
#else
    (void)capacity;
    return WrappedKeySlotReadResult::IO_ERROR;
#endif
}

bool FriendMeshInternalKeySlots::writeSlot(uint8_t slot, const uint8_t *record, size_t recordSize)
{
    if (!available() || slot >= WRAPPED_KEY_SLOT_COUNT || !record || recordSize != STORAGE_WRAPPED_KEY_SIZE) {
        return false;
    }
#ifdef FSCom
    {
        concurrency::LockGuard guard(spiLock);
        if ((!FSCom.exists("/friendmesh") && !FSCom.mkdir("/friendmesh")) ||
            (!FSCom.exists(slotDirectory) && !FSCom.mkdir(slotDirectory))) {
            return false;
        }
    }
    SafeFile file(slotPaths[slot], true);
    const size_t written = file.write(record, recordSize);
    const bool closed = file.close();
    return written == recordSize && closed;
#else
    return false;
#endif
}

bool FriendMeshInternalKeySlots::clearSlot(uint8_t slot)
{
#ifdef FSCom
    if (!available() || slot >= WRAPPED_KEY_SLOT_COUNT) {
        return false;
    }
    concurrency::LockGuard guard(spiLock);
    bool cleared = true;
    const char *path = slotPaths[slot];
    if (FSCom.exists(path) && !FSCom.remove(path)) {
        cleared = false;
    }
    String temporaryPath(path);
    temporaryPath += ".tmp";
    if (FSCom.exists(temporaryPath.c_str()) && !FSCom.remove(temporaryPath.c_str())) {
        cleared = false;
    }
    return cleared;
#else
    (void)slot;
    return false;
#endif
}

bool FriendMeshInternalKeySlots::clearSlots()
{
    bool cleared = true;
    for (uint8_t slot = 0; slot < WRAPPED_KEY_SLOT_COUNT; ++slot) {
        if (!clearSlot(slot)) {
            cleared = false;
        }
    }
    return cleared;
}

} // namespace friendmesh::storage
