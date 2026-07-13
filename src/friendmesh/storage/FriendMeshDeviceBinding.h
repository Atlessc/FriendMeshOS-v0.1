#pragma once

#include "FriendMeshMasterKey.h"

#include <cstddef>
#include <cstdint>

namespace friendmesh::storage
{

constexpr size_t STORAGE_HARDWARE_ID_SIZE = 16;

enum class DeviceBindingResult : uint8_t {
    OK = 0,
    INVALID_ARGUMENT,
    INVALID_HARDWARE_ID,
    HARDWARE_UNAVAILABLE,
    READ_FAILED,
};

DeviceBindingResult deriveDeviceBinding(const uint8_t *hardwareId, size_t hardwareIdSize,
                                        uint8_t binding[STORAGE_DEVICE_BINDING_SIZE]);
DeviceBindingResult hardwareDeviceBinding(uint8_t binding[STORAGE_DEVICE_BINDING_SIZE]);
const char *deviceBindingResultName(DeviceBindingResult result);

} // namespace friendmesh::storage
