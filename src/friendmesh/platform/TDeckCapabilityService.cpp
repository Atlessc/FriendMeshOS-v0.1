#include "TDeckCapabilityService.h"

namespace friendmesh::platform
{

CapabilityState TDeckCapabilityService::evaluate(const CapabilityProbe &probe)
{
    if (!probe.supported) {
        return CapabilityState::UNAVAILABLE;
    }
    if (!probe.detected) {
        return CapabilityState::NOT_DETECTED;
    }
    return probe.operational ? CapabilityState::READY : CapabilityState::DEGRADED;
}

TDeckCapabilities TDeckCapabilityService::snapshot(const TDeckCapabilityInputs &inputs)
{
    TDeckCapabilities result;
    result.gps = evaluate(inputs.gps);
    result.magnetometer = evaluate(inputs.magnetometer);
    result.sd = evaluate(inputs.sd);
    result.maps = result.sd == CapabilityState::READY ? evaluate(inputs.maps) : CapabilityState::UNAVAILABLE;
    result.touch = evaluate(inputs.touch);
    result.trackball = evaluate(inputs.trackball);
    result.keyboard = evaluate(inputs.keyboard);
    result.lora = evaluate(inputs.lora);
    return result;
}

} // namespace friendmesh::platform
