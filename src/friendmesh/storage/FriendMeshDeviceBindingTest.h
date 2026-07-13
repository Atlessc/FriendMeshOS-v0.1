#pragma once

#include <cstdint>

namespace friendmesh::storage
{

enum class DeviceBindingTestState : uint8_t {
    IDLE = 0,
    RUNNING,
    REBOOT_REQUIRED,
    UPDATE_REQUIRED,
    PASSED,
    FAILED,
    UNAVAILABLE,
};

struct DeviceBindingTestSnapshot {
    DeviceBindingTestState state = DeviceBindingTestState::IDLE;
    uint8_t failedStep = 0;
    uint32_t durationMs = 0;
    bool bindingAvailable = false;
    bool sameDevicePassed = false;
    bool rebootPassed = false;
    bool updateChanged = false;
    bool wrongBindingRejected = false;
    bool cleanupPassed = false;
};

bool runDeviceBindingTest();
DeviceBindingTestSnapshot deviceBindingTestSnapshot();
const char *deviceBindingTestStateName(DeviceBindingTestState state);
const char *deviceBindingTestStepName(uint8_t step);

} // namespace friendmesh::storage
