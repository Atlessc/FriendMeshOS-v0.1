#include "FriendMeshKdfBenchmark.h"
#include "configuration.h"

#if defined(ARDUINO_ARCH_ESP32)
#include "Arduino.h"
#include <sodium.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

namespace friendmesh::storage
{
namespace
{
KdfBenchmarkSnapshot current;

#if defined(ARDUINO_ARCH_ESP32)
portMUX_TYPE snapshotMux = portMUX_INITIALIZER_UNLOCKED;

struct KdfCandidate {
    uint32_t memoryKiB;
    uint32_t operations;
};

constexpr KdfCandidate CANDIDATES[KDF_BENCHMARK_CASE_COUNT] = {
    {256, 1},
    {512, 1},
    {1024, 1},
    {1024, 2},
    {1024, 3},
};

void setState(KdfBenchmarkState state)
{
    portENTER_CRITICAL(&snapshotMux);
    current.state = state;
    portEXIT_CRITICAL(&snapshotMux);
}

void benchmarkTask(void *)
{
    static constexpr char password[] = "FriendMesh Argon2id physical benchmark v1";
    static constexpr uint8_t salt[crypto_pwhash_SALTBYTES] = {
        0x46, 0x4d, 0x4b, 0x44, 0x46, 0x2d, 0x42, 0x45,
        0x4e, 0x43, 0x48, 0x2d, 0x56, 0x30, 0x30, 0x31,
    };
    bool passed = sodium_init() >= 0;

    for (size_t index = 0; passed && index < KDF_BENCHMARK_CASE_COUNT; ++index) {
        uint8_t output[32]{};
        LOG_INFO("FriendMesh Argon2id benchmark starting: case=%u memory=%luKiB ops=%lu", unsigned(index + 1),
                 static_cast<unsigned long>(CANDIDATES[index].memoryKiB),
                 static_cast<unsigned long>(CANDIDATES[index].operations));
        const uint32_t started = millis();
        const int result = crypto_pwhash(output, sizeof(output), password, sizeof(password) - 1, salt,
                                         CANDIDATES[index].operations, CANDIDATES[index].memoryKiB * 1024U,
                                         crypto_pwhash_ALG_ARGON2ID13);
        const uint32_t duration = millis() - started;
        sodium_memzero(output, sizeof(output));

        portENTER_CRITICAL(&snapshotMux);
        current.results[index] = {CANDIDATES[index].memoryKiB, CANDIDATES[index].operations, duration, result == 0};
        current.completed = static_cast<uint8_t>(index + 1);
        current.heapLowWater = ESP.getMinFreeHeap();
        current.psramLowWater = ESP.getMinFreePsram();
        current.taskStackLowWater = uxTaskGetStackHighWaterMark(nullptr);
        portEXIT_CRITICAL(&snapshotMux);

        LOG_INFO("FriendMesh Argon2id benchmark: memory=%luKiB ops=%lu result=%s duration=%lums",
                 static_cast<unsigned long>(CANDIDATES[index].memoryKiB),
                 static_cast<unsigned long>(CANDIDATES[index].operations), result == 0 ? "PASS" : "FAIL",
                 static_cast<unsigned long>(duration));
        passed = result == 0;
        vTaskDelay(pdMS_TO_TICKS(25));
    }

    setState(passed ? KdfBenchmarkState::PASSED : KdfBenchmarkState::FAILED);
    LOG_INFO("FriendMesh Argon2id benchmark complete: %s", passed ? "PASS" : "FAIL");
    vTaskDelete(nullptr);
}
#endif
} // namespace

bool startKdfBenchmark()
{
#if defined(ARDUINO_ARCH_ESP32)
    portENTER_CRITICAL(&snapshotMux);
    if (current.state == KdfBenchmarkState::RUNNING) {
        portEXIT_CRITICAL(&snapshotMux);
        return false;
    }
    current = {};
    current.state = KdfBenchmarkState::RUNNING;
    portEXIT_CRITICAL(&snapshotMux);

    // CPU 0's idle task is watchdog-monitored; Argon2 is a monolithic call that cannot yield internally.
    if (xTaskCreatePinnedToCore(benchmarkTask, "fm-kdf-bench", 12288, nullptr, tskIDLE_PRIORITY, nullptr, 1) != pdPASS) {
        setState(KdfBenchmarkState::FAILED);
        return false;
    }
    return true;
#else
    current.state = KdfBenchmarkState::UNAVAILABLE;
    return false;
#endif
}

KdfBenchmarkSnapshot kdfBenchmarkSnapshot()
{
#if defined(ARDUINO_ARCH_ESP32)
    portENTER_CRITICAL(&snapshotMux);
    const KdfBenchmarkSnapshot copy = current;
    portEXIT_CRITICAL(&snapshotMux);
    return copy;
#else
    return current;
#endif
}

const char *kdfBenchmarkStateName(KdfBenchmarkState state)
{
    switch (state) {
    case KdfBenchmarkState::IDLE: return "IDLE";
    case KdfBenchmarkState::RUNNING: return "RUNNING";
    case KdfBenchmarkState::PASSED: return "PASS";
    case KdfBenchmarkState::FAILED: return "FAIL";
    case KdfBenchmarkState::UNAVAILABLE: return "UNAVAILABLE";
    }
    return "UNKNOWN";
}

} // namespace friendmesh::storage
