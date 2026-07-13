#include "DiagnosticEventStore.h"

namespace friendmesh::observability
{

void DiagnosticEventStore::append(DiagnosticEvent event)
{
    event.sequence = nextSequence++;
    size_t slot = (oldest + count) % events.size();
    if (count == events.size()) {
        slot = oldest;
        oldest = (oldest + 1) % events.size();
    } else {
        count++;
    }
    events[slot] = event;
}

void DiagnosticEventStore::recordPacket(const PacketObservation &packet, uint32_t capturedAt)
{
    DiagnosticEvent event;
    event.type = DiagnosticEventType::PACKET;
    event.capturedAt = capturedAt;
    event.packet = packet;
    append(event);
}

void DiagnosticEventStore::recordQueue(const QueueObservation &queue, uint32_t capturedAt)
{
    DiagnosticEvent event;
    event.type = DiagnosticEventType::QUEUE;
    event.capturedAt = capturedAt;
    event.queue = queue;
    append(event);
}

void DiagnosticEventStore::recordPosition(const PositionObservation &position, uint32_t capturedAt)
{
    DiagnosticEvent event;
    event.type = DiagnosticEventType::POSITION;
    event.capturedAt = capturedAt;
    event.position = position;
    append(event);
}

const DiagnosticEvent *DiagnosticEventStore::at(size_t chronologicalIndex) const
{
    if (chronologicalIndex >= count) {
        return nullptr;
    }
    return &events[(oldest + chronologicalIndex) % events.size()];
}

void DiagnosticEventStore::clear()
{
    events = {};
    oldest = 0;
    count = 0;
    nextSequence = 1;
}

} // namespace friendmesh::observability
