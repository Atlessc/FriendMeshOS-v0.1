#pragma once

#if defined(FRIENDMESHOS_TDECK)

#include "friendmesh/protocol/FriendMeshReplayWindow.h"
#include "friendmesh/security/FriendMeshSigningIdentity.h"
#include "friendmesh/storage/FriendMeshInternalKeySlots.h"
#include "friendmesh/storage/FriendMeshStorageKeyManager.h"
#include "mesh/SinglePortModule.h"

class FriendMeshModule : public SinglePortModule
{
  public:
    FriendMeshModule();
    friendmesh::security::SigningIdentityStatus signingIdentityStatus() const { return signingIdentity.status(); }
    bool storageCryptoSelfTestPassed() const { return storageCryptoReady; }
    friendmesh::storage::StorageKeyStatus storageKeyStatus() const { return storageKeyManager.status(); }
    friendmesh::storage::StorageKeyResult storageKeyResult() const { return storageKeyManager.lastResult(); }

  protected:
    ProcessMessage handleReceived(const meshtastic_MeshPacket &packet) override;

  private:
    friendmesh::protocol::FriendMeshReplayWindow replayWindow;
    friendmesh::security::FriendMeshSigningIdentity signingIdentity;
    friendmesh::storage::FriendMeshStorageCrypto storageCrypto;
    friendmesh::storage::FriendMeshInternalKeySlots internalKeySlots;
    friendmesh::storage::FriendMeshWrappedKeyStore wrappedKeyStore;
    friendmesh::storage::FriendMeshHardwareDeviceBinding deviceBinding;
    friendmesh::storage::FriendMeshStorageKeyManager storageKeyManager;
    bool cryptoReady = false;
    bool storageCryptoReady = false;
};

extern FriendMeshModule *friendMeshModule;

#endif
