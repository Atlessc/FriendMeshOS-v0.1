#pragma once

#include <cstdint>

namespace friendmesh::platform
{

enum class CapabilityState : uint8_t { UNAVAILABLE, NOT_DETECTED, DEGRADED, READY };

struct CapabilityProbe
{
    bool supported = false;
    bool detected = false;
    bool operational = false;
};

struct TDeckCapabilityInputs
{
    CapabilityProbe gps;
    CapabilityProbe magnetometer;
    CapabilityProbe sd;
    CapabilityProbe maps;
    CapabilityProbe touch;
    CapabilityProbe trackball;
    CapabilityProbe keyboard;
    CapabilityProbe lora;
};

struct TDeckCapabilities
{
    CapabilityState gps = CapabilityState::UNAVAILABLE;
    CapabilityState magnetometer = CapabilityState::UNAVAILABLE;
    CapabilityState sd = CapabilityState::UNAVAILABLE;
    CapabilityState maps = CapabilityState::UNAVAILABLE;
    CapabilityState touch = CapabilityState::UNAVAILABLE;
    CapabilityState trackball = CapabilityState::UNAVAILABLE;
    CapabilityState keyboard = CapabilityState::UNAVAILABLE;
    CapabilityState lora = CapabilityState::UNAVAILABLE;

    bool canUseMap() const { return maps == CapabilityState::READY; }
    bool canNavigate() const { return gps == CapabilityState::READY; }
    bool hasBodyRelativeHeading() const { return magnetometer == CapabilityState::READY; }
    bool hasExpandedStorage() const { return sd == CapabilityState::READY; }
};

class TDeckCapabilityService
{
  public:
    static CapabilityState evaluate(const CapabilityProbe &probe);
    static TDeckCapabilities snapshot(const TDeckCapabilityInputs &inputs);
};

} // namespace friendmesh::platform
