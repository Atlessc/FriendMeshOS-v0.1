#pragma once

#include "FriendMeshWrappedKeyStore.h"

namespace friendmesh::storage
{

class FriendMeshInternalKeySlots final : public WrappedKeySlotBackend
{
  public:
    FriendMeshInternalKeySlots();
    FriendMeshInternalKeySlots(const char *slotAPath, const char *slotBPath, const char *slotDirectory);

    bool available() const override;
    WrappedKeySlotReadResult readSlot(uint8_t slot, uint8_t *record, size_t capacity,
                                      size_t &recordSize) const override;
    bool writeSlot(uint8_t slot, const uint8_t *record, size_t recordSize) override;
    bool clearSlot(uint8_t slot) override;
    bool clearSlots();

  private:
    const char *slotPaths[WRAPPED_KEY_SLOT_COUNT];
    const char *slotDirectory;
};

} // namespace friendmesh::storage
