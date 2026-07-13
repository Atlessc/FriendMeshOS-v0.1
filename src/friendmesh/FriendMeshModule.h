#pragma once

#if defined(FRIENDMESHOS_TDECK)

#include "friendmesh/protocol/FriendMeshReplayWindow.h"
#include "mesh/SinglePortModule.h"

class FriendMeshModule : public SinglePortModule
{
  public:
    FriendMeshModule();

  protected:
    ProcessMessage handleReceived(const meshtastic_MeshPacket &packet) override;

  private:
    friendmesh::protocol::FriendMeshReplayWindow replayWindow;
    bool cryptoReady = false;
};

extern FriendMeshModule *friendMeshModule;

#endif
