#include "FriendMeshKeySlotRebootTest.h"

#include "FSCommon.h"
#include "FriendMeshInternalKeySlots.h"
#include "FriendMeshStorageCrypto.h"
#include "SPILock.h"
#include "SafeFile.h"
#include "configuration.h"

#include <cstring>

#if defined(ARDUINO_ARCH_ESP32)
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

namespace friendmesh::storage
{
namespace
{
constexpr const char *DIAGNOSTIC_SLOT_A_PATH = "/friendmesh/diagnostics/keyslot_reboot_a.bin";
constexpr const char *DIAGNOSTIC_SLOT_B_PATH = "/friendmesh/diagnostics/keyslot_reboot_b.bin";
constexpr const char *DIAGNOSTIC_SLOT_DIRECTORY = "/friendmesh/diagnostics";
constexpr const char *DIAGNOSTIC_MARKER_PATH = "/friendmesh/diagnostics/keyslot_reboot.marker";
constexpr size_t INTERRUPTED_WRITE_SIZE = 37;
constexpr uint8_t MARKER[] = {0x46, 0x4d, 0x52, 0x42, 0x01, 0x01, 0x00, 0x00};

KeySlotRebootTestSnapshot current;

#if defined(ARDUINO_ARCH_ESP32)
portMUX_TYPE snapshotMux = portMUX_INITIALIZER_UNLOCKED;
#endif

constexpr uint8_t OLD_CREDENTIAL[] = {
    0x46, 0x4d, 0x2d, 0x52, 0x45, 0x42, 0x4f, 0x4f,
    0x54, 0x2d, 0x4f, 0x4c, 0x44, 0x2d, 0x30, 0x31,
};
constexpr uint8_t NEW_CREDENTIAL[] = {
    0x46, 0x4d, 0x2d, 0x52, 0x45, 0x42, 0x4f, 0x4f,
    0x54, 0x2d, 0x4e, 0x45, 0x57, 0x2d, 0x30, 0x32,
};
constexpr uint8_t DEVICE_BINDING[STORAGE_DEVICE_BINDING_SIZE] = {
    0x44, 0x2d, 0x30, 0x31, 0x2d, 0x52, 0x45, 0x42,
    0x4f, 0x4f, 0x54, 0x2d, 0x30, 0x30, 0x30, 0x31,
};
constexpr uint8_t MASTER_KEY[STORAGE_KEY_SIZE] = {
    0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b,
    0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0,
};
constexpr uint8_t SALT_1[STORAGE_KDF_SALT_SIZE] = {
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
};
constexpr uint8_t SALT_2[STORAGE_KDF_SALT_SIZE] = {
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
};
constexpr uint8_t NONCE_1[STORAGE_NONCE_SIZE] = {
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c,
    0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
};
constexpr uint8_t NONCE_2[STORAGE_NONCE_SIZE] = {
    0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54,
    0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60,
};

bool wrapGeneration(const FriendMeshStorageCrypto &crypto, const uint8_t *credential, size_t credentialSize,
                    uint32_t generation, const uint8_t *salt, const uint8_t *nonce,
                    uint8_t record[STORAGE_WRAPPED_KEY_SIZE], size_t &recordSize)
{
    WrappedMasterKeyParameters parameters;
    parameters.memoryKiB = STORAGE_KDF_MEMORY_MIN_KIB;
    parameters.operations = STORAGE_KDF_OPERATIONS_MIN;
    parameters.generation = generation;
    return FriendMeshMasterKey::wrap(crypto, crypto, credential, credentialSize, DEVICE_BINDING, parameters, salt,
                                     nonce, MASTER_KEY, record, STORAGE_WRAPPED_KEY_SIZE,
                                     recordSize) == MasterKeyResult::OK;
}

#if defined(ARDUINO_ARCH_ESP32)
void publishSnapshot(const KeySlotRebootTestSnapshot &snapshot)
{
    portENTER_CRITICAL(&snapshotMux);
    current = snapshot;
    portEXIT_CRITICAL(&snapshotMux);
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

bool writeMarker()
{
#ifdef FSCom
    SafeFile file(DIAGNOSTIC_MARKER_PATH, true);
    return file.write(MARKER, sizeof(MARKER)) == sizeof(MARKER) && file.close();
#else
    return false;
#endif
}

bool markerValid()
{
#ifdef FSCom
    concurrency::LockGuard guard(spiLock);
    File file = FSCom.open(DIAGNOSTIC_MARKER_PATH, FILE_O_READ);
    if (!file || file.size() != sizeof(MARKER)) {
        if (file) {
            file.close();
        }
        return false;
    }
    uint8_t marker[sizeof(MARKER)]{};
    const size_t read = file.read(marker, sizeof(marker));
    file.close();
    return read == sizeof(marker) && std::memcmp(marker, MARKER, sizeof(marker)) == 0;
#else
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

bool writeInterruptedTemporary(const uint8_t *record)
{
#ifdef FSCom
    concurrency::LockGuard guard(spiLock);
    String temporaryPath(DIAGNOSTIC_SLOT_B_PATH);
    temporaryPath += ".tmp";
    File file = FSCom.open(temporaryPath.c_str(), FILE_O_WRITE);
    if (!file) {
        return false;
    }
    const size_t written = file.write(record, INTERRUPTED_WRITE_SIZE);
    file.close();
    return written == INTERRUPTED_WRITE_SIZE;
#else
    (void)record;
    return false;
#endif
}

bool interruptedTemporaryPresent()
{
#ifdef FSCom
    concurrency::LockGuard guard(spiLock);
    String temporaryPath(DIAGNOSTIC_SLOT_B_PATH);
    temporaryPath += ".tmp";
    if (!FSCom.exists(temporaryPath.c_str()) || FSCom.exists(DIAGNOSTIC_SLOT_B_PATH)) {
        return false;
    }
    File file = FSCom.open(temporaryPath.c_str(), FILE_O_READ);
    if (!file) {
        return false;
    }
    const size_t size = file.size();
    file.close();
    return size == INTERRUPTED_WRITE_SIZE;
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

KeySlotRebootTestSnapshot executePrepare()
{
    KeySlotRebootTestSnapshot result;
    result.state = KeySlotRebootTestState::RUNNING;
    const uint32_t started = millis();
    FriendMeshInternalKeySlots backend(DIAGNOSTIC_SLOT_A_PATH, DIAGNOSTIC_SLOT_B_PATH,
                                       DIAGNOSTIC_SLOT_DIRECTORY);
    FriendMeshWrappedKeyStore store(backend);
    FriendMeshStorageCrypto crypto;

    auto fail = [&](uint8_t step) {
        result.failedStep = step;
        result.cleanupPassed = cleanup(backend);
        result.durationMs = millis() - started;
        result.state = KeySlotRebootTestState::FAILED;
        return result;
    };

    if (!backend.available()) {
        result.state = KeySlotRebootTestState::UNAVAILABLE;
        result.durationMs = millis() - started;
        return result;
    }
    if (!backend.clearSlots() || !removeMarker()) {
        return fail(1);
    }
    if (!crypto.available()) {
        return fail(2);
    }

    uint8_t record[STORAGE_WRAPPED_KEY_SIZE]{};
    size_t recordSize = 0;
    if (!wrapGeneration(crypto, OLD_CREDENTIAL, sizeof(OLD_CREDENTIAL), 1, SALT_1, NONCE_1, record,
                        recordSize)) {
        return fail(3);
    }
    uint8_t slotWritten = 0;
    if (store.commitPrepared(record, recordSize, nullptr, slotWritten) != WrappedKeyStoreResult::OK ||
        slotWritten != 0) {
        return fail(4);
    }

    WrappedKeyLoadInfo generationOne;
    uint8_t loadedKey[STORAGE_KEY_SIZE]{};
    if (store.load(crypto, crypto, OLD_CREDENTIAL, sizeof(OLD_CREDENTIAL), DEVICE_BINDING, generationOne,
                   loadedKey) != WrappedKeyStoreResult::OK ||
        generationOne.parameters.generation != 1 || generationOne.selectedSlot != 0 ||
        std::memcmp(loadedKey, MASTER_KEY, sizeof(loadedKey)) != 0) {
        return fail(5);
    }

    if (!wrapGeneration(crypto, OLD_CREDENTIAL, sizeof(OLD_CREDENTIAL), 2, SALT_2, NONCE_2, record,
                        recordSize) ||
        !writeInterruptedTemporary(record)) {
        return fail(6);
    }
    if (!writeMarker()) {
        return fail(7);
    }

    result.selectedGeneration = generationOne.parameters.generation;
    result.validMask = generationOne.validMask;
    result.durationMs = millis() - started;
    result.state = KeySlotRebootTestState::REBOOT_REQUIRED;
    return result;
}

KeySlotRebootTestSnapshot executeVerify()
{
    KeySlotRebootTestSnapshot result;
    result.state = KeySlotRebootTestState::RUNNING;
    const uint32_t started = millis();
    FriendMeshInternalKeySlots backend(DIAGNOSTIC_SLOT_A_PATH, DIAGNOSTIC_SLOT_B_PATH,
                                       DIAGNOSTIC_SLOT_DIRECTORY);
    FriendMeshWrappedKeyStore store(backend);
    FriendMeshStorageCrypto crypto;

    auto fail = [&](uint8_t step) {
        result.failedStep = step;
        result.cleanupPassed = cleanup(backend);
        result.durationMs = millis() - started;
        result.state = KeySlotRebootTestState::FAILED;
        return result;
    };

    if (!backend.available()) {
        result.state = KeySlotRebootTestState::UNAVAILABLE;
        result.durationMs = millis() - started;
        return result;
    }
    if (!crypto.available()) {
        return fail(8);
    }
    if (!markerValid()) {
        return fail(9);
    }

    WrappedKeyLoadInfo generationOne;
    uint8_t loadedKey[STORAGE_KEY_SIZE]{};
    if (!interruptedTemporaryPresent() ||
        store.load(crypto, crypto, OLD_CREDENTIAL, sizeof(OLD_CREDENTIAL), DEVICE_BINDING, generationOne,
                   loadedKey) != WrappedKeyStoreResult::OK ||
        generationOne.parameters.generation != 1 || generationOne.selectedSlot != 0 ||
        generationOne.validMask != 0x01 || std::memcmp(loadedKey, MASTER_KEY, sizeof(loadedKey)) != 0) {
        return fail(10);
    }
    result.interruptedWriteRecovered = true;

    if (!backend.clearSlot(1)) {
        return fail(11);
    }
    uint8_t record[STORAGE_WRAPPED_KEY_SIZE]{};
    size_t recordSize = 0;
    if (!wrapGeneration(crypto, NEW_CREDENTIAL, sizeof(NEW_CREDENTIAL), 2, SALT_2, NONCE_2, record,
                        recordSize)) {
        return fail(12);
    }
    uint8_t slotWritten = 0;
    if (store.commitPrepared(record, recordSize, &generationOne, slotWritten) != WrappedKeyStoreResult::OK ||
        slotWritten != 1) {
        return fail(13);
    }

    WrappedKeyLoadInfo generationTwo;
    if (store.load(crypto, crypto, NEW_CREDENTIAL, sizeof(NEW_CREDENTIAL), DEVICE_BINDING, generationTwo,
                   loadedKey) != WrappedKeyStoreResult::OK ||
        generationTwo.parameters.generation != 2 || generationTwo.selectedSlot != 1 ||
        generationTwo.validMask != 0x02 || std::memcmp(loadedKey, MASTER_KEY, sizeof(loadedKey)) != 0) {
        return fail(14);
    }
    if (!backend.clearSlot(0)) {
        return fail(15);
    }

    WrappedKeyLoadInfo oldCredentialInfo;
    if (store.load(crypto, crypto, OLD_CREDENTIAL, sizeof(OLD_CREDENTIAL), DEVICE_BINDING, oldCredentialInfo,
                   loadedKey) != WrappedKeyStoreResult::NO_AUTHENTICATED_KEY) {
        return fail(16);
    }
    result.oldCredentialRejected = true;

    if (store.load(crypto, crypto, NEW_CREDENTIAL, sizeof(NEW_CREDENTIAL), DEVICE_BINDING, generationTwo,
                   loadedKey) != WrappedKeyStoreResult::OK ||
        generationTwo.parameters.generation != 2 || generationTwo.selectedSlot != 1 ||
        generationTwo.validMask != 0x02 || std::memcmp(loadedKey, MASTER_KEY, sizeof(loadedKey)) != 0) {
        return fail(17);
    }
    result.credentialRewrapPassed = true;
    result.selectedGeneration = generationTwo.parameters.generation;
    result.validMask = generationTwo.validMask;
    result.cleanupPassed = cleanup(backend);
    result.durationMs = millis() - started;
    result.state = result.cleanupPassed ? KeySlotRebootTestState::PASSED : KeySlotRebootTestState::FAILED;
    result.failedStep = result.cleanupPassed ? 0 : 18;
    return result;
}

void keySlotRebootTestTask(void *)
{
    vTaskDelay(1);
    const KeySlotRebootTestSnapshot result = markerExists() ? executeVerify() : executePrepare();
    publishSnapshot(result);
    LOG_INFO("FriendMesh diagnostic reboot test complete: %s duration=%lums",
             keySlotRebootTestStateName(result.state), static_cast<unsigned long>(result.durationMs));
    vTaskDelete(nullptr);
}
#endif
} // namespace

bool runKeySlotRebootTest()
{
#if defined(ARDUINO_ARCH_ESP32)
    portENTER_CRITICAL(&snapshotMux);
    if (current.state == KeySlotRebootTestState::RUNNING ||
        current.state == KeySlotRebootTestState::REBOOT_REQUIRED) {
        portEXIT_CRITICAL(&snapshotMux);
        return false;
    }
    current = {};
    current.state = KeySlotRebootTestState::RUNNING;
    portEXIT_CRITICAL(&snapshotMux);

    if (xTaskCreatePinnedToCore(keySlotRebootTestTask, "fm-reboot-test", 12288, nullptr, tskIDLE_PRIORITY, nullptr,
                                1) != pdPASS) {
        KeySlotRebootTestSnapshot failed;
        failed.state = KeySlotRebootTestState::FAILED;
        failed.failedStep = 19;
        publishSnapshot(failed);
        return false;
    }
    return true;
#else
    current = {};
    current.state = KeySlotRebootTestState::UNAVAILABLE;
    return false;
#endif
}

KeySlotRebootTestSnapshot keySlotRebootTestSnapshot()
{
#if defined(ARDUINO_ARCH_ESP32)
    portENTER_CRITICAL(&snapshotMux);
    const KeySlotRebootTestSnapshot copy = current;
    portEXIT_CRITICAL(&snapshotMux);
    return copy;
#else
    return current;
#endif
}

const char *keySlotRebootTestStateName(KeySlotRebootTestState state)
{
    switch (state) {
    case KeySlotRebootTestState::IDLE: return "IDLE";
    case KeySlotRebootTestState::RUNNING: return "RUNNING";
    case KeySlotRebootTestState::REBOOT_REQUIRED: return "REBOOT REQUIRED";
    case KeySlotRebootTestState::PASSED: return "PASS";
    case KeySlotRebootTestState::FAILED: return "FAIL";
    case KeySlotRebootTestState::UNAVAILABLE: return "UNAVAILABLE";
    }
    return "UNKNOWN";
}

const char *keySlotRebootTestStepName(uint8_t step)
{
    switch (step) {
    case 0: return "NONE";
    case 1: return "PRE_CLEAN";
    case 2: return "PREP_CRYPTO";
    case 3: return "WRAP_GEN1";
    case 4: return "COMMIT_GEN1";
    case 5: return "LOAD_GEN1";
    case 6: return "WRITE_INTERRUPTED";
    case 7: return "WRITE_MARKER";
    case 8: return "VERIFY_CRYPTO";
    case 9: return "VERIFY_MARKER";
    case 10: return "REBOOT_RECOVERY";
    case 11: return "CLEAR_TEMP";
    case 12: return "REWRAP_RECORD";
    case 13: return "COMMIT_REWRAP";
    case 14: return "LOAD_NEW";
    case 15: return "CLEAR_OLD";
    case 16: return "REJECT_OLD";
    case 17: return "RELOAD_NEW";
    case 18: return "CLEANUP";
    case 19: return "TASK_START";
    default: return "UNKNOWN";
    }
}

} // namespace friendmesh::storage
