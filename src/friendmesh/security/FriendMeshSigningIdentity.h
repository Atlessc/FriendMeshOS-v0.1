#pragma once

#include "FriendMeshCrypto.h"
#include <cstddef>
#include <cstdint>

namespace friendmesh::security
{

enum class IdentityStorageSecurity : uint8_t {
    NONE,
    PLAINTEXT,
    ENCRYPTED_UNAUTHENTICATED,
    ENCRYPTED_AUTHENTICATED,
};

enum class IdentityLoadResult : uint8_t {
    LOADED,
    NOT_FOUND,
    LOCKED,
    ERROR,
};

struct SigningIdentityRecord
{
    uint8_t seed[ED25519_SEED_SIZE]{};
    uint8_t publicKey[ED25519_PUBLIC_KEY_SIZE]{};
    uint32_t keyGeneration = 0;
};

class SigningIdentityStore
{
  public:
    virtual ~SigningIdentityStore() = default;
    virtual IdentityStorageSecurity securityLevel() const = 0;
    virtual IdentityLoadResult load(SigningIdentityRecord &record) = 0;
    virtual bool save(const SigningIdentityRecord &record) = 0;
};

enum class SigningIdentityStatus : uint8_t {
    STORAGE_UNAVAILABLE,
    STORAGE_UNSAFE,
    NOT_CONFIGURED,
    LOCKED,
    READY,
    CORRUPT,
    CRYPTO_UNAVAILABLE,
    PERSIST_FAILED,
    CREATE_NOT_ALLOWED,
};

class FriendMeshSigningIdentity
{
  public:
    ~FriendMeshSigningIdentity();

    SigningIdentityStatus initialize(SigningIdentityStore *store);
    SigningIdentityStatus create(SigningIdentityStore &store, uint32_t keyGeneration = 1);
    void lock();

    SigningIdentityStatus status() const { return currentStatus; }
    bool ready() const { return currentStatus == SigningIdentityStatus::READY; }
    const uint8_t *publicKey() const { return ready() ? record.publicKey : nullptr; }
    uint32_t keyGeneration() const { return ready() ? record.keyGeneration : 0; }
    bool sign(const uint8_t *message, size_t messageSize, uint8_t signature[ED25519_SIGNATURE_SIZE]) const;

    static bool createSignature(const uint8_t publicKey[ED25519_PUBLIC_KEY_SIZE], const uint8_t *message,
                                size_t messageSize, uint8_t signature[ED25519_SIGNATURE_SIZE], void *context);

  private:
    static bool storeIsProtected(const SigningIdentityStore *store);
    bool recordIsValid(const SigningIdentityRecord &candidate) const;
    void clearRecord();

    SigningIdentityRecord record{};
    SigningIdentityStatus currentStatus = SigningIdentityStatus::STORAGE_UNAVAILABLE;
};

const char *signingIdentityStatusName(SigningIdentityStatus status);

} // namespace friendmesh::security
