#include "FriendMeshDeviceBindingTest.h"

#include "FSCommon.h"
#include "FriendMeshDeviceBinding.h"
#include "FriendMeshInternalKeySlots.h"
#include "FriendMeshStorageCrypto.h"
#include "SPILock.h"
#include "SafeFile.h"
#include "configuration.h"

#include <cstring>

#if defined(ARDUINO_ARCH_ESP32)
#include "Arduino.h"
#include "esp_ota_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

namespace friendmesh::storage
{
namespace
{
constexpr const char *DIAGNOSTIC_SLOT_A_PATH = "/friendmesh/diagnostics/device_binding_a.bin";
constexpr const char *DIAGNOSTIC_SLOT_B_PATH = "/friendmesh/diagnostics/device_binding_b.bin";
constexpr const char *DIAGNOSTIC_SLOT_DIRECTORY = "/friendmesh/diagnostics";
constexpr const char *DIAGNOSTIC_MARKER_PATH = "/friendmesh/diagnostics/device_binding.marker";
constexpr size_t APP_FINGERPRINT_HEX_SIZE = 64;
constexpr size_t MARKER_SIZE = 8 + APP_FINGERPRINT_HEX_SIZE;
constexpr uint8_t MARKER_MAGIC[] = {0x46, 0x4d, 0x44, 0x42};
constexpr uint8_t MARKER_VERSION = 1;
constexpr uint8_t MARKER_PREPARED = 1;
constexpr uint8_t MARKER_REBOOT_VERIFIED = 2;

DeviceBindingTestSnapshot current;

#if defined(ARDUINO_ARCH_ESP32)
portMUX_TYPE snapshotMux = portMUX_INITIALIZER_UNLOCKED;
#endif

constexpr uint8_t CREDENTIAL[] = {
    0x46, 0x4d, 0x2d, 0x42, 0x49, 0x4e, 0x44, 0x2d,
    0x54, 0x45, 0x53, 0x54, 0x2d, 0x30, 0x30, 0x31,
};
constexpr uint8_t MASTER_KEY[STORAGE_KEY_SIZE] = {
    0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab,
    0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0,
};
constexpr uint8_t SALT[STORAGE_KDF_SALT_SIZE] = {
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
};
constexpr uint8_t NONCE[STORAGE_NONCE_SIZE] = {
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c,
    0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
};

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

bool isLowerHex(char value)
{
    return (value >= '0' && value <= '9') || (value >= 'a' && value <= 'f');
}

#if defined(ARDUINO_ARCH_ESP32)
void publishSnapshot(const DeviceBindingTestSnapshot &snapshot)
{
    portENTER_CRITICAL(&snapshotMux);
    current = snapshot;
    portEXIT_CRITICAL(&snapshotMux);
}

bool currentAppFingerprint(char fingerprint[APP_FINGERPRINT_HEX_SIZE + 1])
{
    std::memset(fingerprint, 0, APP_FINGERPRINT_HEX_SIZE + 1);
    const esp_app_desc_t *description = esp_ota_get_app_description();
    if (!description || sizeof(description->app_elf_sha256) * 2 != APP_FINGERPRINT_HEX_SIZE ||
        allZero(description->app_elf_sha256, sizeof(description->app_elf_sha256))) {
        return false;
    }
    constexpr char HEX_DIGITS[] = "0123456789abcdef";
    for (size_t index = 0; index < sizeof(description->app_elf_sha256); ++index) {
        fingerprint[index * 2] = HEX_DIGITS[description->app_elf_sha256[index] >> 4];
        fingerprint[index * 2 + 1] = HEX_DIGITS[description->app_elf_sha256[index] & 0x0f];
    }
    return true;
}

bool markerExists()
{
#ifdef FSCom
    concurrency::LockGuard guard(spiLock);
    return FSCom.exists(DIAGNOSTIC_MARKER_PATH);
#else
    return false;
#endif
}

bool writeMarker(uint8_t stage, const char fingerprint[APP_FINGERPRINT_HEX_SIZE + 1])
{
#ifdef FSCom
    uint8_t marker[MARKER_SIZE]{};
    std::memcpy(marker, MARKER_MAGIC, sizeof(MARKER_MAGIC));
    marker[4] = MARKER_VERSION;
    marker[5] = stage;
    std::memcpy(marker + 8, fingerprint, APP_FINGERPRINT_HEX_SIZE);
    SafeFile file(DIAGNOSTIC_MARKER_PATH, true);
    const bool fullyWritten = file.write(marker, sizeof(marker)) == sizeof(marker);
    const bool closed = file.close();
    wipe(marker, sizeof(marker));
    return fullyWritten && closed;
#else
    (void)stage;
    (void)fingerprint;
    return false;
#endif
}

bool readMarker(uint8_t &stage, char fingerprint[APP_FINGERPRINT_HEX_SIZE + 1])
{
#ifdef FSCom
    uint8_t marker[MARKER_SIZE]{};
    {
        concurrency::LockGuard guard(spiLock);
        File file = FSCom.open(DIAGNOSTIC_MARKER_PATH, FILE_O_READ);
        if (!file || file.size() != sizeof(marker)) {
            if (file) {
                file.close();
            }
            return false;
        }
        const size_t read = file.read(marker, sizeof(marker));
        file.close();
        if (read != sizeof(marker)) {
            return false;
        }
    }
    if (std::memcmp(marker, MARKER_MAGIC, sizeof(MARKER_MAGIC)) != 0 || marker[4] != MARKER_VERSION ||
        (marker[5] != MARKER_PREPARED && marker[5] != MARKER_REBOOT_VERIFIED) || marker[6] != 0 || marker[7] != 0) {
        wipe(marker, sizeof(marker));
        return false;
    }
    for (size_t index = 0; index < APP_FINGERPRINT_HEX_SIZE; ++index) {
        if (!isLowerHex(static_cast<char>(marker[8 + index]))) {
            wipe(marker, sizeof(marker));
            return false;
        }
    }
    stage = marker[5];
    std::memcpy(fingerprint, marker + 8, APP_FINGERPRINT_HEX_SIZE);
    fingerprint[APP_FINGERPRINT_HEX_SIZE] = '\0';
    wipe(marker, sizeof(marker));
    return true;
#else
    (void)stage;
    (void)fingerprint;
    return false;
#endif
}

bool removeMarker()
{
#ifdef FSCom
    concurrency::LockGuard guard(spiLock);
    bool removed = true;
    if (FSCom.exists(DIAGNOSTIC_MARKER_PATH) && !FSCom.remove(DIAGNOSTIC_MARKER_PATH)) {
        removed = false;
    }
    String temporaryPath(DIAGNOSTIC_MARKER_PATH);
    temporaryPath += ".tmp";
    if (FSCom.exists(temporaryPath.c_str()) && !FSCom.remove(temporaryPath.c_str())) {
        removed = false;
    }
    return removed;
#else
    return false;
#endif
}

bool cleanup(FriendMeshInternalKeySlots &backend)
{
    const bool slotsCleared = backend.clearSlots();
    const bool markerRemoved = removeMarker();
    return slotsCleared && markerRemoved;
}

bool verifyStoredBinding(FriendMeshWrappedKeyStore &store, const FriendMeshStorageCrypto &crypto,
                         const uint8_t binding[STORAGE_DEVICE_BINDING_SIZE], DeviceBindingTestSnapshot &result)
{
    WrappedKeyLoadInfo loadInfo;
    uint8_t loadedKey[STORAGE_KEY_SIZE]{};
    const bool sameDevice = store.load(crypto, crypto, CREDENTIAL, sizeof(CREDENTIAL), binding, loadInfo,
                                       loadedKey) == WrappedKeyStoreResult::OK &&
                            loadInfo.parameters.generation == 1 && loadInfo.selectedSlot == 0 &&
                            std::memcmp(loadedKey, MASTER_KEY, sizeof(loadedKey)) == 0;
    result.sameDevicePassed = sameDevice;
    if (!sameDevice) {
        wipe(loadedKey, sizeof(loadedKey));
        return false;
    }

    uint8_t wrongBinding[STORAGE_DEVICE_BINDING_SIZE];
    std::memcpy(wrongBinding, binding, sizeof(wrongBinding));
    wrongBinding[0] ^= 1;
    std::memset(loadedKey, 0xa5, sizeof(loadedKey));
    result.wrongBindingRejected =
        store.load(crypto, crypto, CREDENTIAL, sizeof(CREDENTIAL), wrongBinding, loadInfo,
                   loadedKey) == WrappedKeyStoreResult::NO_AUTHENTICATED_KEY &&
        allZero(loadedKey, sizeof(loadedKey));
    wipe(wrongBinding, sizeof(wrongBinding));
    wipe(loadedKey, sizeof(loadedKey));
    return result.wrongBindingRejected;
}

DeviceBindingTestSnapshot executeTest()
{
    DeviceBindingTestSnapshot result;
    result.state = DeviceBindingTestState::RUNNING;
    const uint32_t started = millis();
    FriendMeshInternalKeySlots backend(DIAGNOSTIC_SLOT_A_PATH, DIAGNOSTIC_SLOT_B_PATH,
                                       DIAGNOSTIC_SLOT_DIRECTORY);
    FriendMeshWrappedKeyStore store(backend);
    FriendMeshStorageCrypto crypto;
    uint8_t binding[STORAGE_DEVICE_BINDING_SIZE]{};
    char currentFingerprint[APP_FINGERPRINT_HEX_SIZE + 1]{};
    char initialFingerprint[APP_FINGERPRINT_HEX_SIZE + 1]{};

    auto finish = [&](DeviceBindingTestState state) {
        wipe(binding, sizeof(binding));
        wipe(currentFingerprint, sizeof(currentFingerprint));
        wipe(initialFingerprint, sizeof(initialFingerprint));
        result.durationMs = millis() - started;
        result.state = state;
        return result;
    };
    auto fail = [&](uint8_t step) {
        result.failedStep = step;
        result.cleanupPassed = cleanup(backend);
        return finish(DeviceBindingTestState::FAILED);
    };

    if (!backend.available()) {
        return finish(DeviceBindingTestState::UNAVAILABLE);
    }
    if (!crypto.available()) {
        return fail(1);
    }
    if (hardwareDeviceBinding(binding) != DeviceBindingResult::OK) {
        return fail(2);
    }
    result.bindingAvailable = true;
    if (!currentAppFingerprint(currentFingerprint)) {
        return fail(3);
    }

    if (!markerExists()) {
        if (!backend.clearSlots() || !removeMarker()) {
            return fail(4);
        }
        WrappedMasterKeyParameters parameters;
        parameters.memoryKiB = STORAGE_KDF_MEMORY_MIN_KIB;
        parameters.operations = STORAGE_KDF_OPERATIONS_MIN;
        parameters.generation = 1;
        uint8_t record[STORAGE_WRAPPED_KEY_SIZE]{};
        size_t recordSize = 0;
        if (FriendMeshMasterKey::wrap(crypto, crypto, CREDENTIAL, sizeof(CREDENTIAL), binding, parameters,
                                      SALT, NONCE, MASTER_KEY, record, sizeof(record),
                                      recordSize) != MasterKeyResult::OK) {
            wipe(record, sizeof(record));
            return fail(5);
        }
        uint8_t slotWritten = 0;
        const bool committed =
            store.commitPrepared(record, recordSize, nullptr, slotWritten) == WrappedKeyStoreResult::OK &&
            slotWritten == 0;
        wipe(record, sizeof(record));
        if (!committed) {
            return fail(6);
        }
        if (!verifyStoredBinding(store, crypto, binding, result)) {
            return fail(7);
        }
        if (!writeMarker(MARKER_PREPARED, currentFingerprint)) {
            return fail(8);
        }
        return finish(DeviceBindingTestState::REBOOT_REQUIRED);
    }

    uint8_t markerStage = 0;
    if (!readMarker(markerStage, initialFingerprint)) {
        return fail(9);
    }
    if (markerStage == MARKER_PREPARED) {
        if (std::memcmp(initialFingerprint, currentFingerprint, APP_FINGERPRINT_HEX_SIZE) != 0) {
            return fail(10);
        }
        if (!verifyStoredBinding(store, crypto, binding, result)) {
            return fail(11);
        }
        result.rebootPassed = true;
        if (!writeMarker(MARKER_REBOOT_VERIFIED, initialFingerprint)) {
            return fail(12);
        }
        return finish(DeviceBindingTestState::UPDATE_REQUIRED);
    }

    result.rebootPassed = true;
    if (std::memcmp(initialFingerprint, currentFingerprint, APP_FINGERPRINT_HEX_SIZE) == 0) {
        return finish(DeviceBindingTestState::UPDATE_REQUIRED);
    }
    result.updateChanged = true;
    if (!verifyStoredBinding(store, crypto, binding, result)) {
        return fail(13);
    }
    result.cleanupPassed = cleanup(backend);
    if (!result.cleanupPassed) {
        result.failedStep = 14;
        return finish(DeviceBindingTestState::FAILED);
    }
    return finish(DeviceBindingTestState::PASSED);
}

void deviceBindingTestTask(void *)
{
    vTaskDelay(1);
    const DeviceBindingTestSnapshot result = executeTest();
    publishSnapshot(result);
    LOG_INFO("FriendMesh diagnostic device-binding test complete: %s duration=%lums",
             deviceBindingTestStateName(result.state), static_cast<unsigned long>(result.durationMs));
    vTaskDelete(nullptr);
}
#endif
} // namespace

bool runDeviceBindingTest()
{
#if defined(ARDUINO_ARCH_ESP32)
    portENTER_CRITICAL(&snapshotMux);
    if (current.state == DeviceBindingTestState::RUNNING ||
        current.state == DeviceBindingTestState::REBOOT_REQUIRED ||
        current.state == DeviceBindingTestState::UPDATE_REQUIRED) {
        portEXIT_CRITICAL(&snapshotMux);
        return false;
    }
    current = {};
    current.state = DeviceBindingTestState::RUNNING;
    portEXIT_CRITICAL(&snapshotMux);

    if (xTaskCreatePinnedToCore(deviceBindingTestTask, "fm-bind-test-b", 12288, nullptr, tskIDLE_PRIORITY,
                                nullptr, 1) != pdPASS) {
        DeviceBindingTestSnapshot failed;
        failed.state = DeviceBindingTestState::FAILED;
        failed.failedStep = 15;
        publishSnapshot(failed);
        return false;
    }
    return true;
#else
    current = {};
    current.state = DeviceBindingTestState::UNAVAILABLE;
    return false;
#endif
}

DeviceBindingTestSnapshot deviceBindingTestSnapshot()
{
#if defined(ARDUINO_ARCH_ESP32)
    portENTER_CRITICAL(&snapshotMux);
    const DeviceBindingTestSnapshot copy = current;
    portEXIT_CRITICAL(&snapshotMux);
    return copy;
#else
    return current;
#endif
}

const char *deviceBindingTestStateName(DeviceBindingTestState state)
{
    switch (state) {
    case DeviceBindingTestState::IDLE: return "IDLE";
    case DeviceBindingTestState::RUNNING: return "RUNNING";
    case DeviceBindingTestState::REBOOT_REQUIRED: return "REBOOT REQUIRED";
    case DeviceBindingTestState::UPDATE_REQUIRED: return "UPDATE REQUIRED";
    case DeviceBindingTestState::PASSED: return "PASS";
    case DeviceBindingTestState::FAILED: return "FAIL";
    case DeviceBindingTestState::UNAVAILABLE: return "UNAVAILABLE";
    }
    return "UNKNOWN";
}

const char *deviceBindingTestStepName(uint8_t step)
{
    switch (step) {
    case 0: return "NONE";
    case 1: return "CRYPTO";
    case 2: return "HARDWARE_ID";
    case 3: return "APP_FINGERPRINT";
    case 4: return "PRE_CLEAN";
    case 5: return "WRAP";
    case 6: return "COMMIT";
    case 7: return "PREP_VERIFY";
    case 8: return "PREP_MARKER";
    case 9: return "READ_MARKER";
    case 10: return "REBOOT_BUILD_CHANGED";
    case 11: return "REBOOT_VERIFY";
    case 12: return "REBOOT_MARKER";
    case 13: return "UPDATE_VERIFY";
    case 14: return "CLEANUP";
    case 15: return "TASK_START";
    default: return "UNKNOWN";
    }
}

} // namespace friendmesh::storage
