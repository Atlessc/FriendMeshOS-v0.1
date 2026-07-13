#include "FriendMeshKeySlotSelfTest.h"

#include "FriendMeshInternalKeySlots.h"
#include "FriendMeshStorageCrypto.h"
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
constexpr const char *DIAGNOSTIC_SLOT_A_PATH = "/friendmesh/diagnostics/keyslot_test_a.bin";
constexpr const char *DIAGNOSTIC_SLOT_B_PATH = "/friendmesh/diagnostics/keyslot_test_b.bin";
constexpr const char *DIAGNOSTIC_SLOT_DIRECTORY = "/friendmesh/diagnostics";

KeySlotSelfTestSnapshot current;

#if defined(ARDUINO_ARCH_ESP32)
portMUX_TYPE snapshotMux = portMUX_INITIALIZER_UNLOCKED;
#endif

constexpr uint8_t CREDENTIAL[] = {
    0x46, 0x4d, 0x2d, 0x53, 0x4c, 0x4f, 0x54, 0x2d,
    0x54, 0x45, 0x53, 0x54, 0x2d, 0x30, 0x30, 0x31,
};
constexpr uint8_t DEVICE_BINDING[STORAGE_DEVICE_BINDING_SIZE] = {
    0x44, 0x2d, 0x30, 0x31, 0x2d, 0x4e, 0x4f, 0x4e,
    0x50, 0x52, 0x4f, 0x44, 0x2d, 0x30, 0x30, 0x31,
};
constexpr uint8_t MASTER_KEY[STORAGE_KEY_SIZE] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
    0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
};
constexpr uint8_t SALT_1[STORAGE_KDF_SALT_SIZE] = {
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40,
};
constexpr uint8_t SALT_2[STORAGE_KDF_SALT_SIZE] = {
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
};
constexpr uint8_t NONCE_1[STORAGE_NONCE_SIZE] = {
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c,
    0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
};
constexpr uint8_t NONCE_2[STORAGE_NONCE_SIZE] = {
    0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80,
};

bool wrapGeneration(const FriendMeshStorageCrypto &crypto, uint32_t generation, const uint8_t *salt,
                    const uint8_t *nonce, uint8_t record[STORAGE_WRAPPED_KEY_SIZE], size_t &recordSize)
{
    WrappedMasterKeyParameters parameters;
    parameters.memoryKiB = STORAGE_KDF_MEMORY_MIN_KIB;
    parameters.operations = STORAGE_KDF_OPERATIONS_MIN;
    parameters.generation = generation;
    return FriendMeshMasterKey::wrap(crypto, crypto, CREDENTIAL, sizeof(CREDENTIAL), DEVICE_BINDING, parameters,
                                     salt, nonce, MASTER_KEY, record, STORAGE_WRAPPED_KEY_SIZE,
                                     recordSize) == MasterKeyResult::OK;
}

#if defined(ARDUINO_ARCH_ESP32)
void publishSnapshot(const KeySlotSelfTestSnapshot &snapshot)
{
    portENTER_CRITICAL(&snapshotMux);
    current = snapshot;
    portEXIT_CRITICAL(&snapshotMux);
}

KeySlotSelfTestSnapshot executeKeySlotSelfTest()
{
    KeySlotSelfTestSnapshot result;
    result.state = KeySlotSelfTestState::RUNNING;
    const uint32_t started = millis();
    FriendMeshInternalKeySlots backend(DIAGNOSTIC_SLOT_A_PATH, DIAGNOSTIC_SLOT_B_PATH,
                                       DIAGNOSTIC_SLOT_DIRECTORY);
    FriendMeshWrappedKeyStore store(backend);
    FriendMeshStorageCrypto crypto;

    auto fail = [&](uint8_t step) {
        result.failedStep = step;
        result.cleanupPassed = backend.clearSlots();
        result.durationMs = millis() - started;
        result.state = KeySlotSelfTestState::FAILED;
        LOG_ERROR("FriendMesh diagnostic key-slot self-test failed at step %u", static_cast<unsigned>(step));
        return result;
    };

    if (!backend.available()) {
        result.state = KeySlotSelfTestState::UNAVAILABLE;
        result.durationMs = millis() - started;
        return result;
    }
    if (!backend.clearSlots()) {
        return fail(1);
    }
    if (!crypto.available()) {
        return fail(2);
    }

    uint8_t record[STORAGE_WRAPPED_KEY_SIZE]{};
    size_t recordSize = 0;
    if (!wrapGeneration(crypto, 1, SALT_1, NONCE_1, record, recordSize)) {
        return fail(3);
    }
    uint8_t slotWritten = 0;
    if (store.commitPrepared(record, recordSize, nullptr, slotWritten) != WrappedKeyStoreResult::OK ||
        slotWritten != 0) {
        return fail(4);
    }

    WrappedKeyLoadInfo generationOne;
    uint8_t loadedKey[STORAGE_KEY_SIZE]{};
    if (store.load(crypto, crypto, CREDENTIAL, sizeof(CREDENTIAL), DEVICE_BINDING, generationOne, loadedKey) !=
            WrappedKeyStoreResult::OK ||
        generationOne.parameters.generation != 1 || std::memcmp(loadedKey, MASTER_KEY, sizeof(loadedKey)) != 0) {
        return fail(5);
    }

    if (!wrapGeneration(crypto, 2, SALT_2, NONCE_2, record, recordSize)) {
        return fail(6);
    }
    if (store.commitPrepared(record, recordSize, &generationOne, slotWritten) != WrappedKeyStoreResult::OK ||
        slotWritten != 1) {
        return fail(7);
    }

    WrappedKeyLoadInfo generationTwo;
    if (store.load(crypto, crypto, CREDENTIAL, sizeof(CREDENTIAL), DEVICE_BINDING, generationTwo, loadedKey) !=
            WrappedKeyStoreResult::OK ||
        generationTwo.parameters.generation != 2 || generationTwo.selectedSlot != 1 ||
        generationTwo.validMask != 0x03 || std::memcmp(loadedKey, MASTER_KEY, sizeof(loadedKey)) != 0) {
        return fail(8);
    }

    std::memset(record, 0xa5, sizeof(record));
    if (!backend.writeSlot(0, record, sizeof(record))) {
        return fail(9);
    }
    WrappedKeyLoadInfo recovered;
    if (store.load(crypto, crypto, CREDENTIAL, sizeof(CREDENTIAL), DEVICE_BINDING, recovered, loadedKey) !=
            WrappedKeyStoreResult::OK ||
        recovered.parameters.generation != 2 || recovered.selectedSlot != 1 || recovered.validMask != 0x02 ||
        !recovered.degraded || std::memcmp(loadedKey, MASTER_KEY, sizeof(loadedKey)) != 0) {
        return fail(10);
    }

    result.validMask = recovered.validMask;
    result.selectedGeneration = recovered.parameters.generation;
    result.degradedRecovery = recovered.degraded;
    result.cleanupPassed = backend.clearSlots();
    result.durationMs = millis() - started;
    result.state = result.cleanupPassed ? KeySlotSelfTestState::PASSED : KeySlotSelfTestState::FAILED;
    result.failedStep = result.cleanupPassed ? 0 : 11;
    return result;
}

void keySlotSelfTestTask(void *)
{
    vTaskDelay(1);
    LOG_INFO("FriendMesh diagnostic key-slot self-test starting");
    const KeySlotSelfTestSnapshot result = executeKeySlotSelfTest();
    publishSnapshot(result);
    LOG_INFO("FriendMesh diagnostic key-slot self-test complete: %s duration=%lums",
             result.state == KeySlotSelfTestState::PASSED ? "PASS" : "FAIL",
             static_cast<unsigned long>(result.durationMs));
    vTaskDelete(nullptr);
}
#endif
} // namespace

bool runKeySlotSelfTest()
{
#if defined(ARDUINO_ARCH_ESP32)
    portENTER_CRITICAL(&snapshotMux);
    if (current.state == KeySlotSelfTestState::RUNNING) {
        portEXIT_CRITICAL(&snapshotMux);
        return false;
    }
    current = {};
    current.state = KeySlotSelfTestState::RUNNING;
    portEXIT_CRITICAL(&snapshotMux);

    if (xTaskCreatePinnedToCore(keySlotSelfTestTask, "fm-slot-test", 12288, nullptr, tskIDLE_PRIORITY, nullptr, 1) !=
        pdPASS) {
        KeySlotSelfTestSnapshot failed;
        failed.state = KeySlotSelfTestState::FAILED;
        failed.failedStep = 12;
        publishSnapshot(failed);
        LOG_ERROR("FriendMesh diagnostic key-slot self-test task creation failed");
        return false;
    }
    return true;
#else
    current = {};
    current.state = KeySlotSelfTestState::UNAVAILABLE;
    return false;
#endif
}

KeySlotSelfTestSnapshot keySlotSelfTestSnapshot()
{
#if defined(ARDUINO_ARCH_ESP32)
    portENTER_CRITICAL(&snapshotMux);
    const KeySlotSelfTestSnapshot copy = current;
    portEXIT_CRITICAL(&snapshotMux);
    return copy;
#else
    return current;
#endif
}

const char *keySlotSelfTestStateName(KeySlotSelfTestState state)
{
    switch (state) {
    case KeySlotSelfTestState::IDLE: return "IDLE";
    case KeySlotSelfTestState::RUNNING: return "RUNNING";
    case KeySlotSelfTestState::PASSED: return "PASS";
    case KeySlotSelfTestState::FAILED: return "FAIL";
    case KeySlotSelfTestState::UNAVAILABLE: return "UNAVAILABLE";
    }
    return "UNKNOWN";
}

const char *keySlotSelfTestStepName(uint8_t step)
{
    switch (step) {
    case 0: return "NONE";
    case 1: return "PRE_CLEAN";
    case 2: return "CRYPTO";
    case 3: return "WRAP_GEN1";
    case 4: return "COMMIT_GEN1";
    case 5: return "LOAD_GEN1";
    case 6: return "WRAP_GEN2";
    case 7: return "COMMIT_GEN2";
    case 8: return "LOAD_GEN2";
    case 9: return "CORRUPT_OLD";
    case 10: return "RECOVER";
    case 11: return "CLEANUP";
    case 12: return "TASK_START";
    default: return "UNKNOWN";
    }
}

} // namespace friendmesh::storage
