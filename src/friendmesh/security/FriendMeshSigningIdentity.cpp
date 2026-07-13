#include "FriendMeshSigningIdentity.h"

#include <cstring>

namespace friendmesh::security
{

FriendMeshSigningIdentity::~FriendMeshSigningIdentity()
{
    clearRecord();
}

bool FriendMeshSigningIdentity::storeIsProtected(const SigningIdentityStore *store)
{
    return store && store->securityLevel() == IdentityStorageSecurity::ENCRYPTED_AUTHENTICATED;
}

bool FriendMeshSigningIdentity::recordIsValid(const SigningIdentityRecord &candidate) const
{
    if (candidate.keyGeneration == 0) {
        return false;
    }
    uint8_t derived[ED25519_PUBLIC_KEY_SIZE];
    const bool valid = FriendMeshCrypto::derivePublicKey(candidate.seed, derived) &&
                       std::memcmp(derived, candidate.publicKey, sizeof(derived)) == 0;
    FriendMeshCrypto::secureWipe(derived, sizeof(derived));
    return valid;
}

SigningIdentityStatus FriendMeshSigningIdentity::initialize(SigningIdentityStore *store)
{
    clearRecord();
    if (!FriendMeshCrypto::available()) {
        currentStatus = SigningIdentityStatus::CRYPTO_UNAVAILABLE;
        return currentStatus;
    }
    if (!store) {
        currentStatus = SigningIdentityStatus::STORAGE_UNAVAILABLE;
        return currentStatus;
    }
    if (!storeIsProtected(store)) {
        currentStatus = SigningIdentityStatus::STORAGE_UNSAFE;
        return currentStatus;
    }

    SigningIdentityRecord loaded{};
    const IdentityLoadResult result = store->load(loaded);
    if (result == IdentityLoadResult::NOT_FOUND) {
        currentStatus = SigningIdentityStatus::NOT_CONFIGURED;
    } else if (result == IdentityLoadResult::LOCKED) {
        currentStatus = SigningIdentityStatus::LOCKED;
    } else if (result != IdentityLoadResult::LOADED) {
        currentStatus = SigningIdentityStatus::CORRUPT;
    } else if (!recordIsValid(loaded)) {
        currentStatus = SigningIdentityStatus::CORRUPT;
    } else {
        record = loaded;
        currentStatus = SigningIdentityStatus::READY;
    }
    FriendMeshCrypto::secureWipe(&loaded, sizeof(loaded));
    return currentStatus;
}

SigningIdentityStatus FriendMeshSigningIdentity::create(SigningIdentityStore &store, uint32_t keyGeneration)
{
    if (currentStatus != SigningIdentityStatus::NOT_CONFIGURED) {
        return SigningIdentityStatus::CREATE_NOT_ALLOWED;
    }
    clearRecord();
    if (!FriendMeshCrypto::available()) {
        currentStatus = SigningIdentityStatus::CRYPTO_UNAVAILABLE;
        return currentStatus;
    }
    if (!storeIsProtected(&store)) {
        currentStatus = SigningIdentityStatus::STORAGE_UNSAFE;
        return currentStatus;
    }
    if (keyGeneration == 0 || !FriendMeshCrypto::generateSeed(record.seed) ||
        !FriendMeshCrypto::derivePublicKey(record.seed, record.publicKey)) {
        clearRecord();
        currentStatus = SigningIdentityStatus::CRYPTO_UNAVAILABLE;
        return currentStatus;
    }
    record.keyGeneration = keyGeneration;
    if (!store.save(record)) {
        clearRecord();
        currentStatus = SigningIdentityStatus::PERSIST_FAILED;
        return currentStatus;
    }
    currentStatus = SigningIdentityStatus::READY;
    return currentStatus;
}

void FriendMeshSigningIdentity::lock()
{
    clearRecord();
    currentStatus = SigningIdentityStatus::LOCKED;
}

bool FriendMeshSigningIdentity::sign(const uint8_t *message, size_t messageSize,
                                     uint8_t signature[ED25519_SIGNATURE_SIZE]) const
{
    return ready() && FriendMeshCrypto::sign(record.seed, record.publicKey, message, messageSize, signature);
}

bool FriendMeshSigningIdentity::createSignature(const uint8_t publicKey[ED25519_PUBLIC_KEY_SIZE],
                                                const uint8_t *message, size_t messageSize,
                                                uint8_t signature[ED25519_SIGNATURE_SIZE], void *context)
{
    const auto *identity = static_cast<const FriendMeshSigningIdentity *>(context);
    return identity && identity->ready() && identity->publicKey() &&
           std::memcmp(identity->publicKey(), publicKey, ED25519_PUBLIC_KEY_SIZE) == 0 &&
           identity->sign(message, messageSize, signature);
}

void FriendMeshSigningIdentity::clearRecord()
{
    FriendMeshCrypto::secureWipe(&record, sizeof(record));
}

const char *signingIdentityStatusName(SigningIdentityStatus status)
{
    switch (status) {
    case SigningIdentityStatus::STORAGE_UNAVAILABLE:
        return "STORAGE_UNAVAILABLE";
    case SigningIdentityStatus::STORAGE_UNSAFE:
        return "STORAGE_UNSAFE";
    case SigningIdentityStatus::NOT_CONFIGURED:
        return "NOT_CONFIGURED";
    case SigningIdentityStatus::LOCKED:
        return "LOCKED";
    case SigningIdentityStatus::READY:
        return "READY";
    case SigningIdentityStatus::CORRUPT:
        return "CORRUPT";
    case SigningIdentityStatus::CRYPTO_UNAVAILABLE:
        return "CRYPTO_UNAVAILABLE";
    case SigningIdentityStatus::CREATE_NOT_ALLOWED:
        return "CREATE_NOT_ALLOWED";
    default:
        return "PERSIST_FAILED";
    }
}

} // namespace friendmesh::security
