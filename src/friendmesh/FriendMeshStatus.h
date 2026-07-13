#pragma once

#include "friendmesh/security/FriendMeshSigningIdentity.h"

#if defined(FRIENDMESHOS_TDECK)
friendmesh::security::SigningIdentityStatus friendMeshSigningIdentityStatus();
bool friendMeshStorageCryptoReady();
#endif
