#pragma once

#include "friendmesh/generated/friendmesh.pb.h"
#include <array>
#include <cstddef>
#include <cstdint>

namespace friendmesh::protocol
{

constexpr size_t REPLAY_SENDER_CAPACITY = 64;
constexpr size_t REPLAY_EVENT_CAPACITY = 128;
constexpr uint8_t REPLAY_SEQUENCE_WINDOW_BITS = 64;

enum class ReplayResult : uint8_t {
    ACCEPTED,
    DUPLICATE_EVENT,
    DUPLICATE_SEQUENCE,
    STALE_SEQUENCE,
    CAPACITY_FULL,
};

class FriendMeshReplayWindow
{
  public:
    ReplayResult checkAndRecord(const friendmesh_FriendMeshSignedFields &fields);
    void clear();

  private:
    struct SenderWindow
    {
        bool used = false;
        std::array<uint8_t, 16> groupId{};
        std::array<uint8_t, 32> signingPublicKey{};
        uint32_t groupEpoch = 0;
        uint32_t senderNode = 0;
        uint64_t highestSequence = 0;
        uint64_t receivedBitmap = 0;
    };

    struct EventRecord
    {
        bool used = false;
        std::array<uint8_t, 16> eventId{};
    };

    SenderWindow *findSender(const friendmesh_FriendMeshSignedFields &fields);
    SenderWindow *allocateSender(const friendmesh_FriendMeshSignedFields &fields);
    bool hasEvent(const uint8_t eventId[16]) const;
    void recordEvent(const uint8_t eventId[16]);

    std::array<SenderWindow, REPLAY_SENDER_CAPACITY> senders{};
    std::array<EventRecord, REPLAY_EVENT_CAPACITY> events{};
    size_t nextEvent = 0;
};

} // namespace friendmesh::protocol
