#pragma once

#include "FriendMeshObservability.h"
#include <array>
#include <cstddef>
#include <cstdint>

namespace friendmesh::observability
{

constexpr size_t DIAGNOSTIC_EVENT_CAPACITY = 16;

enum class DiagnosticEventType : uint8_t { PACKET, QUEUE, POSITION };

struct DiagnosticEvent
{
    DiagnosticEventType type = DiagnosticEventType::PACKET;
    uint32_t sequence = 0;
    uint32_t capturedAt = 0;
    PacketObservation packet;
    QueueObservation queue;
    PositionObservation position;
};

class DiagnosticEventStore
{
  public:
    void recordPacket(const PacketObservation &packet, uint32_t capturedAt);
    void recordQueue(const QueueObservation &queue, uint32_t capturedAt);
    void recordPosition(const PositionObservation &position, uint32_t capturedAt);
    void clear();

    size_t size() const { return count; }
    constexpr size_t capacity() const { return DIAGNOSTIC_EVENT_CAPACITY; }
    const DiagnosticEvent *at(size_t chronologicalIndex) const;

  private:
    void append(DiagnosticEvent event);

    std::array<DiagnosticEvent, DIAGNOSTIC_EVENT_CAPACITY> events{};
    size_t oldest = 0;
    size_t count = 0;
    uint32_t nextSequence = 1;
};

} // namespace friendmesh::observability
