#include "FriendMeshDeviceBinding.h"

#include <cstring>

#if defined(ARDUINO_ARCH_ESP32)
#include <esp_efuse.h>
#include <esp_efuse_table.h>
#endif

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

bool allZero(const uint8_t *value, size_t size)
{
    uint8_t combined = 0;
    for (size_t index = 0; index < size; ++index) {
        combined |= value[index];
    }
    return combined == 0;
}
} // namespace

DeviceBindingResult deriveDeviceBinding(const uint8_t *hardwareId, size_t hardwareIdSize,
                                        uint8_t binding[STORAGE_DEVICE_BINDING_SIZE])
{
    if (binding) {
        wipe(binding, STORAGE_DEVICE_BINDING_SIZE);
    }
    if (!hardwareId || !binding) {
        return DeviceBindingResult::INVALID_ARGUMENT;
    }
    if (hardwareIdSize != STORAGE_HARDWARE_ID_SIZE || allZero(hardwareId, hardwareIdSize)) {
        return DeviceBindingResult::INVALID_HARDWARE_ID;
    }
    static_assert(STORAGE_HARDWARE_ID_SIZE == STORAGE_DEVICE_BINDING_SIZE);
    std::memcpy(binding, hardwareId, STORAGE_DEVICE_BINDING_SIZE);
    return DeviceBindingResult::OK;
}

DeviceBindingResult hardwareDeviceBinding(uint8_t binding[STORAGE_DEVICE_BINDING_SIZE])
{
    if (!binding) {
        return DeviceBindingResult::INVALID_ARGUMENT;
    }
    wipe(binding, STORAGE_DEVICE_BINDING_SIZE);
#if defined(ARDUINO_ARCH_ESP32) &&                                                                          \
    (defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C6))
    uint8_t hardwareId[STORAGE_HARDWARE_ID_SIZE]{};
    if (esp_efuse_read_field_blob(ESP_EFUSE_OPTIONAL_UNIQUE_ID, hardwareId, sizeof(hardwareId) * 8U) != ESP_OK) {
        wipe(hardwareId, sizeof(hardwareId));
        return DeviceBindingResult::READ_FAILED;
    }
    const DeviceBindingResult result = deriveDeviceBinding(hardwareId, sizeof(hardwareId), binding);
    wipe(hardwareId, sizeof(hardwareId));
    return result;
#else
    return DeviceBindingResult::HARDWARE_UNAVAILABLE;
#endif
}

const char *deviceBindingResultName(DeviceBindingResult result)
{
    switch (result) {
    case DeviceBindingResult::OK: return "OK";
    case DeviceBindingResult::INVALID_ARGUMENT: return "INVALID_ARGUMENT";
    case DeviceBindingResult::INVALID_HARDWARE_ID: return "INVALID_HARDWARE_ID";
    case DeviceBindingResult::HARDWARE_UNAVAILABLE: return "HARDWARE_UNAVAILABLE";
    case DeviceBindingResult::READ_FAILED: return "READ_FAILED";
    }
    return "UNKNOWN";
}

} // namespace friendmesh::storage
