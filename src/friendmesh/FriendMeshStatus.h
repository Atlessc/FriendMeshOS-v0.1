#pragma once

#include "friendmesh/security/FriendMeshSigningIdentity.h"
#include "friendmesh/storage/FriendMeshStorageKeyManager.h"

#if defined(FRIENDMESHOS_TDECK)
friendmesh::security::SigningIdentityStatus friendMeshSigningIdentityStatus();
bool friendMeshStorageCryptoReady();
friendmesh::storage::StorageKeyStatus friendMeshStorageKeyStatus();
friendmesh::storage::StorageKeyResult friendMeshStorageKeyResult();
#endif
