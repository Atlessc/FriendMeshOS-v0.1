#pragma once

#include <cstddef>
#include <cstdint>

namespace friendmesh::storage
{

constexpr size_t KDF_BENCHMARK_CASE_COUNT = 5;

enum class KdfBenchmarkState : uint8_t {
    IDLE,
    RUNNING,
    PASSED,
    FAILED,
    UNAVAILABLE,
};

struct KdfBenchmarkCaseResult {
    uint32_t memoryKiB = 0;
    uint32_t operations = 0;
    uint32_t durationMs = 0;
    bool passed = false;
};

struct KdfBenchmarkSnapshot {
    KdfBenchmarkState state = KdfBenchmarkState::IDLE;
    uint8_t completed = 0;
    uint32_t heapLowWater = 0;
    uint32_t psramLowWater = 0;
    uint32_t taskStackLowWater = 0;
    KdfBenchmarkCaseResult results[KDF_BENCHMARK_CASE_COUNT]{};
};

bool startKdfBenchmark();
KdfBenchmarkSnapshot kdfBenchmarkSnapshot();
const char *kdfBenchmarkStateName(KdfBenchmarkState state);

} // namespace friendmesh::storage
