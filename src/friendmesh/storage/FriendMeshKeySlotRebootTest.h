#pragma once

#include <cstdint>

namespace friendmesh::storage
{

enum class KeySlotRebootTestState : uint8_t {
    IDLE = 0,
    RUNNING,
    REBOOT_REQUIRED,
    PASSED,
    FAILED,
    UNAVAILABLE,
};

struct KeySlotRebootTestSnapshot {
    KeySlotRebootTestState state = KeySlotRebootTestState::IDLE;
    uint8_t failedStep = 0;
    uint8_t validMask = 0;
    uint32_t selectedGeneration = 0;
    uint32_t durationMs = 0;
    bool interruptedWriteRecovered = false;
    bool credentialRewrapPassed = false;
    bool oldCredentialRejected = false;
    bool cleanupPassed = false;
};

bool runKeySlotRebootTest();
KeySlotRebootTestSnapshot keySlotRebootTestSnapshot();
const char *keySlotRebootTestStateName(KeySlotRebootTestState state);
const char *keySlotRebootTestStepName(uint8_t step);

} // namespace friendmesh::storage
