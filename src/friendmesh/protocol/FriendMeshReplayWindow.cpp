#include "FriendMeshReplayWindow.h"

#include <cstring>

namespace friendmesh::protocol
{

FriendMeshReplayWindow::SenderWindow *FriendMeshReplayWindow::findSender(
    const friendmesh_FriendMeshSignedFields &fields)
{
    for (auto &sender : senders) {
        if (sender.used && sender.groupEpoch == fields.group_epoch && sender.senderNode == fields.sender_node &&
            std::memcmp(sender.groupId.data(), fields.group_id, sender.groupId.size()) == 0 &&
            std::memcmp(sender.signingPublicKey.data(), fields.signing_public_key, sender.signingPublicKey.size()) == 0) {
            return &sender;
        }
    }
    return nullptr;
}

FriendMeshReplayWindow::SenderWindow *FriendMeshReplayWindow::allocateSender(
    const friendmesh_FriendMeshSignedFields &fields)
{
    for (auto &sender : senders) {
        if (!sender.used) {
            sender.used = true;
            std::memcpy(sender.groupId.data(), fields.group_id, sender.groupId.size());
            std::memcpy(sender.signingPublicKey.data(), fields.signing_public_key, sender.signingPublicKey.size());
            sender.groupEpoch = fields.group_epoch;
            sender.senderNode = fields.sender_node;
            sender.highestSequence = fields.sender_sequence;
            sender.receivedBitmap = 1;
            return &sender;
        }
    }
    return nullptr;
}

bool FriendMeshReplayWindow::hasEvent(const uint8_t eventId[16]) const
{
    for (const auto &event : events) {
        if (event.used && std::memcmp(event.eventId.data(), eventId, event.eventId.size()) == 0) {
            return true;
        }
    }
    return false;
}

void FriendMeshReplayWindow::recordEvent(const uint8_t eventId[16])
{
    auto &event = events[nextEvent];
    event.used = true;
    std::memcpy(event.eventId.data(), eventId, event.eventId.size());
    nextEvent = (nextEvent + 1) % events.size();
}

ReplayResult FriendMeshReplayWindow::checkAndRecord(const friendmesh_FriendMeshSignedFields &fields)
{
    if (hasEvent(fields.event_id)) {
        return ReplayResult::DUPLICATE_EVENT;
    }

    SenderWindow *sender = findSender(fields);
    if (!sender) {
        sender = allocateSender(fields);
        if (!sender) {
            return ReplayResult::CAPACITY_FULL;
        }
        recordEvent(fields.event_id);
        return ReplayResult::ACCEPTED;
    }

    if (fields.sender_sequence > sender->highestSequence) {
        const uint64_t advance = fields.sender_sequence - sender->highestSequence;
        sender->receivedBitmap = advance >= REPLAY_SEQUENCE_WINDOW_BITS ? 1 : (sender->receivedBitmap << advance) | 1;
        sender->highestSequence = fields.sender_sequence;
    } else {
        const uint64_t age = sender->highestSequence - fields.sender_sequence;
        if (age >= REPLAY_SEQUENCE_WINDOW_BITS) {
            return ReplayResult::STALE_SEQUENCE;
        }
        const uint64_t mask = UINT64_C(1) << age;
        if ((sender->receivedBitmap & mask) != 0) {
            return ReplayResult::DUPLICATE_SEQUENCE;
        }
        sender->receivedBitmap |= mask;
    }

    recordEvent(fields.event_id);
    return ReplayResult::ACCEPTED;
}

void FriendMeshReplayWindow::clear()
{
    senders = {};
    events = {};
    nextEvent = 0;
}

} // namespace friendmesh::protocol
