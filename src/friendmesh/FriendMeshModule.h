#pragma once

#if defined(FRIENDMESHOS_TDECK)

#include "friendmesh/protocol/FriendMeshReplayWindow.h"
#include "friendmesh/security/FriendMeshSigningIdentity.h"
#include "mesh/SinglePortModule.h"

class FriendMeshModule : public SinglePortModule
{
  public:
    FriendMeshModule();
    friendmesh::security::SigningIdentityStatus signingIdentityStatus() const { return signingIdentity.status(); }

  protected:
    ProcessMessage handleReceived(const meshtastic_MeshPacket &packet) override;

  private:
    friendmesh::protocol::FriendMeshReplayWindow replayWindow;
    friendmesh::security::FriendMeshSigningIdentity signingIdentity;
    bool cryptoReady = false;
};

extern FriendMeshModule *friendMeshModule;

#endif
