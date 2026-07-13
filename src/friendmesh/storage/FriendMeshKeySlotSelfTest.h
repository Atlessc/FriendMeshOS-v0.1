#pragma once

#include <cstdint>

namespace friendmesh::storage
{

enum class KeySlotSelfTestState : uint8_t {
    IDLE = 0,
    RUNNING,
    PASSED,
    FAILED,
    UNAVAILABLE,
};

struct KeySlotSelfTestSnapshot {
    KeySlotSelfTestState state = KeySlotSelfTestState::IDLE;
    uint8_t failedStep = 0;
    uint8_t validMask = 0;
    uint32_t selectedGeneration = 0;
    uint32_t durationMs = 0;
    bool degradedRecovery = false;
    bool cleanupPassed = false;
};

bool runKeySlotSelfTest();
KeySlotSelfTestSnapshot keySlotSelfTestSnapshot();
const char *keySlotSelfTestStateName(KeySlotSelfTestState state);
const char *keySlotSelfTestStepName(uint8_t step);

} // namespace friendmesh::storage
