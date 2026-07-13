#include "FriendMeshMasterKey.h"

#include <cstring>

namespace friendmesh::storage
{
namespace
{
constexpr uint8_t MAGIC[] = {'F', 'M', 'W', 'K'};
constexpr uint8_t FORMAT_VERSION = 1;
constexpr uint8_t KDF_ARGON2ID13 = 1;
constexpr uint8_t AEAD_XCHACHA20_POLY1305 = 1;

struct DomainParameters {
    StorageKeyDomain domain;
    uint64_t subkeyId;
    char context[8];
};

constexpr DomainParameters DOMAINS[] = {
    {StorageKeyDomain::SIGNING_IDENTITY, 1, {'F', 'M', 'I', 'D', 'E', 'N', '0', '1'}},
    {StorageKeyDomain::REPLAY_STATE, 2, {'F', 'M', 'R', 'E', 'P', 'L', '0', '1'}},
    {StorageKeyDomain::TRANSACTION, 3, {'F', 'M', 'T', 'X', 'N', 'J', '0', '1'}},
    {StorageKeyDomain::GROUP_STATE, 4, {'F', 'M', 'G', 'R', 'U', 'P', '0', '1'}},
    {StorageKeyDomain::HISTORY, 5, {'F', 'M', 'H', 'I', 'S', 'T', '0', '1'}},
    {StorageKeyDomain::OUTBOX, 6, {'F', 'M', 'O', 'U', 'T', 'B', '0', '1'}},
    {StorageKeyDomain::CONTACT, 7, {'F', 'M', 'C', 'O', 'N', 'T', '0', '1'}},
    {StorageKeyDomain::NOTIFICATION, 8, {'F', 'M', 'N', 'O', 'T', 'I', '0', '1'}},
    {StorageKeyDomain::KEY_AUDIT, 9, {'F', 'M', 'K', 'A', 'U', 'D', '0', '1'}},
    {StorageKeyDomain::DRAFT, 10, {'F', 'M', 'D', 'R', 'A', 'F', '0', '1'}},
};

void writeU32(uint8_t *target, uint32_t value)
{
    for (size_t i = 0; i < 4; ++i) {
        target[i] = static_cast<uint8_t>(value >> (8 * i));
    }
}

uint32_t readU32(const uint8_t *source)
{
    uint32_t value = 0;
    for (size_t i = 0; i < 4; ++i) {
        value |= static_cast<uint32_t>(source[i]) << (8 * i);
    }
    return value;
}

bool allZero(const uint8_t *value, size_t size)
{
    uint8_t combined = 0;
    for (size_t i = 0; i < size; ++i) {
        combined |= value[i];
    }
    return combined == 0;
}

bool validParameters(const WrappedMasterKeyParameters &parameters)
{
    return parameters.memoryKiB >= STORAGE_KDF_MEMORY_MIN_KIB &&
           parameters.memoryKiB <= STORAGE_KDF_MEMORY_MAX_KIB &&
           parameters.operations >= STORAGE_KDF_OPERATIONS_MIN &&
           parameters.operations <= STORAGE_KDF_OPERATIONS_MAX && parameters.generation != 0;
}

bool validCredential(const uint8_t *credential, size_t credentialSize)
{
    return credential && credentialSize > 0 && credentialSize <= STORAGE_CREDENTIAL_MAX_SIZE;
}

void wipe(void *value, size_t size)
{
    volatile uint8_t *bytes = static_cast<volatile uint8_t *>(value);
    while (size-- > 0) {
        *bytes++ = 0;
    }
}

const DomainParameters *findDomain(StorageKeyDomain domain)
{
    for (const auto &parameters : DOMAINS) {
        if (parameters.domain == domain) {
            return &parameters;
        }
    }
    return nullptr;
}
} // namespace

MasterKeyResult FriendMeshMasterKey::inspect(const uint8_t *record, size_t recordSize,
                                             WrappedMasterKeyParameters &parameters)
{
    if (!record) {
        return MasterKeyResult::INVALID_ARGUMENT;
    }
    if (recordSize != STORAGE_WRAPPED_KEY_SIZE) {
        return MasterKeyResult::LENGTH_MISMATCH;
    }
    if (std::memcmp(record, MAGIC, sizeof(MAGIC)) != 0) {
        return MasterKeyResult::BAD_MAGIC;
    }
    if (record[4] != FORMAT_VERSION) {
        return MasterKeyResult::UNSUPPORTED_VERSION;
    }
    if (record[5] != KDF_ARGON2ID13) {
        return MasterKeyResult::UNSUPPORTED_KDF;
    }
    if (record[6] != AEAD_XCHACHA20_POLY1305) {
        return MasterKeyResult::UNSUPPORTED_AEAD;
    }
    if (record[7] != 0) {
        return MasterKeyResult::UNSUPPORTED_FLAGS;
    }

    WrappedMasterKeyParameters parsed;
    parsed.operations = readU32(record + 8);
    parsed.memoryKiB = readU32(record + 12);
    parsed.generation = readU32(record + 16);
    if (!validParameters(parsed) || allZero(record + 20, STORAGE_KDF_SALT_SIZE) ||
        allZero(record + 36, STORAGE_NONCE_SIZE)) {
        return MasterKeyResult::INVALID_PARAMETERS;
    }
    parameters = parsed;
    return MasterKeyResult::OK;
}

MasterKeyResult FriendMeshMasterKey::wrap(const StoragePasswordKdf &kdf, const StorageAead &aead,
                                          const uint8_t *credential, size_t credentialSize,
                                          const uint8_t deviceBinding[STORAGE_DEVICE_BINDING_SIZE],
                                          const WrappedMasterKeyParameters &parameters,
                                          const uint8_t salt[STORAGE_KDF_SALT_SIZE],
                                          const uint8_t nonce[STORAGE_NONCE_SIZE],
                                          const uint8_t masterKey[STORAGE_KEY_SIZE], uint8_t *record,
                                          size_t recordCapacity, size_t &recordSize)
{
    recordSize = 0;
    if (!validCredential(credential, credentialSize) || !deviceBinding || !salt || !nonce || !masterKey || !record) {
        return MasterKeyResult::INVALID_ARGUMENT;
    }
    if (!validParameters(parameters) || allZero(deviceBinding, STORAGE_DEVICE_BINDING_SIZE) ||
        allZero(salt, STORAGE_KDF_SALT_SIZE) || allZero(nonce, STORAGE_NONCE_SIZE)) {
        return MasterKeyResult::INVALID_PARAMETERS;
    }
    if (recordCapacity < STORAGE_WRAPPED_KEY_SIZE) {
        return MasterKeyResult::OUTPUT_TOO_SMALL;
    }
    if (!kdf.available() || !aead.available()) {
        return MasterKeyResult::CRYPTO_UNAVAILABLE;
    }

    std::memcpy(record, MAGIC, sizeof(MAGIC));
    record[4] = FORMAT_VERSION;
    record[5] = KDF_ARGON2ID13;
    record[6] = AEAD_XCHACHA20_POLY1305;
    record[7] = 0;
    writeU32(record + 8, parameters.operations);
    writeU32(record + 12, parameters.memoryKiB);
    writeU32(record + 16, parameters.generation);
    std::memcpy(record + 20, salt, STORAGE_KDF_SALT_SIZE);
    std::memcpy(record + 36, nonce, STORAGE_NONCE_SIZE);

    uint8_t wrappingKey[STORAGE_KEY_SIZE]{};
    if (!kdf.deriveWrappingKey(credential, credentialSize, salt, parameters.operations, parameters.memoryKiB,
                               wrappingKey)) {
        wipe(wrappingKey, sizeof(wrappingKey));
        wipe(record, STORAGE_WRAPPED_KEY_SIZE);
        return MasterKeyResult::KDF_FAILED;
    }
    uint8_t aad[STORAGE_WRAP_AAD_SIZE];
    std::memcpy(aad, record, STORAGE_WRAPPED_KEY_HEADER_SIZE);
    std::memcpy(aad + STORAGE_WRAPPED_KEY_HEADER_SIZE, deviceBinding, STORAGE_DEVICE_BINDING_SIZE);
    size_t sealedSize = 0;
    const bool sealed = aead.seal(wrappingKey, nonce, aad, sizeof(aad), masterKey, STORAGE_KEY_SIZE,
                                  record + STORAGE_WRAPPED_KEY_HEADER_SIZE, sealedSize);
    wipe(wrappingKey, sizeof(wrappingKey));
    wipe(aad, sizeof(aad));
    if (!sealed || sealedSize != STORAGE_KEY_SIZE + STORAGE_TAG_SIZE) {
        wipe(record, STORAGE_WRAPPED_KEY_SIZE);
        return MasterKeyResult::WRAP_FAILED;
    }
    recordSize = STORAGE_WRAPPED_KEY_SIZE;
    return MasterKeyResult::OK;
}

MasterKeyResult FriendMeshMasterKey::unwrap(const StoragePasswordKdf &kdf, const StorageAead &aead,
                                            const uint8_t *credential, size_t credentialSize,
                                            const uint8_t deviceBinding[STORAGE_DEVICE_BINDING_SIZE],
                                            const uint8_t *record, size_t recordSize,
                                            WrappedMasterKeyParameters &parameters,
                                            uint8_t masterKey[STORAGE_KEY_SIZE])
{
    if (masterKey) {
        wipe(masterKey, STORAGE_KEY_SIZE);
    }
    if (!validCredential(credential, credentialSize) || !deviceBinding || !record || !masterKey) {
        return MasterKeyResult::INVALID_ARGUMENT;
    }
    WrappedMasterKeyParameters parsed;
    const MasterKeyResult inspection = inspect(record, recordSize, parsed);
    if (inspection != MasterKeyResult::OK) {
        return inspection;
    }
    if (allZero(deviceBinding, STORAGE_DEVICE_BINDING_SIZE)) {
        return MasterKeyResult::INVALID_PARAMETERS;
    }
    if (!kdf.available() || !aead.available()) {
        return MasterKeyResult::CRYPTO_UNAVAILABLE;
    }

    uint8_t wrappingKey[STORAGE_KEY_SIZE]{};
    if (!kdf.deriveWrappingKey(credential, credentialSize, record + 20, parsed.operations, parsed.memoryKiB,
                               wrappingKey)) {
        wipe(wrappingKey, sizeof(wrappingKey));
        return MasterKeyResult::KDF_FAILED;
    }
    uint8_t aad[STORAGE_WRAP_AAD_SIZE];
    std::memcpy(aad, record, STORAGE_WRAPPED_KEY_HEADER_SIZE);
    std::memcpy(aad + STORAGE_WRAPPED_KEY_HEADER_SIZE, deviceBinding, STORAGE_DEVICE_BINDING_SIZE);
    size_t openedSize = 0;
    const bool opened = aead.open(wrappingKey, record + 36, aad, sizeof(aad),
                                  record + STORAGE_WRAPPED_KEY_HEADER_SIZE,
                                  STORAGE_WRAPPED_KEY_SIZE - STORAGE_WRAPPED_KEY_HEADER_SIZE, masterKey, openedSize);
    wipe(wrappingKey, sizeof(wrappingKey));
    wipe(aad, sizeof(aad));
    if (!opened || openedSize != STORAGE_KEY_SIZE) {
        wipe(masterKey, STORAGE_KEY_SIZE);
        return MasterKeyResult::AUTH_FAILED;
    }
    parameters = parsed;
    return MasterKeyResult::OK;
}

MasterKeyResult FriendMeshMasterKey::deriveSubkey(const StorageSubkeyKdf &kdf,
                                                  const uint8_t masterKey[STORAGE_KEY_SIZE], StorageKeyDomain domain,
                                                  uint8_t subkey[STORAGE_KEY_SIZE])
{
    if (subkey) {
        wipe(subkey, STORAGE_KEY_SIZE);
    }
    if (!masterKey || !subkey) {
        return MasterKeyResult::INVALID_ARGUMENT;
    }
    const DomainParameters *parameters = findDomain(domain);
    if (!parameters) {
        return MasterKeyResult::UNSUPPORTED_DOMAIN;
    }
    if (!kdf.available()) {
        return MasterKeyResult::CRYPTO_UNAVAILABLE;
    }
    if (!kdf.deriveStorageSubkey(masterKey, parameters->subkeyId, parameters->context, subkey)) {
        wipe(subkey, STORAGE_KEY_SIZE);
        return MasterKeyResult::SUBKEY_FAILED;
    }
    return MasterKeyResult::OK;
}

const char *FriendMeshMasterKey::resultName(MasterKeyResult result)
{
    switch (result) {
    case MasterKeyResult::OK: return "OK";
    case MasterKeyResult::INVALID_ARGUMENT: return "INVALID_ARGUMENT";
    case MasterKeyResult::INVALID_PARAMETERS: return "INVALID_PARAMETERS";
    case MasterKeyResult::OUTPUT_TOO_SMALL: return "OUTPUT_TOO_SMALL";
    case MasterKeyResult::BAD_MAGIC: return "BAD_MAGIC";
    case MasterKeyResult::UNSUPPORTED_VERSION: return "UNSUPPORTED_VERSION";
    case MasterKeyResult::UNSUPPORTED_KDF: return "UNSUPPORTED_KDF";
    case MasterKeyResult::UNSUPPORTED_AEAD: return "UNSUPPORTED_AEAD";
    case MasterKeyResult::UNSUPPORTED_FLAGS: return "UNSUPPORTED_FLAGS";
    case MasterKeyResult::LENGTH_MISMATCH: return "LENGTH_MISMATCH";
    case MasterKeyResult::CRYPTO_UNAVAILABLE: return "CRYPTO_UNAVAILABLE";
    case MasterKeyResult::KDF_FAILED: return "KDF_FAILED";
    case MasterKeyResult::WRAP_FAILED: return "WRAP_FAILED";
    case MasterKeyResult::AUTH_FAILED: return "AUTH_FAILED";
    case MasterKeyResult::UNSUPPORTED_DOMAIN: return "UNSUPPORTED_DOMAIN";
    case MasterKeyResult::SUBKEY_FAILED: return "SUBKEY_FAILED";
    }
    return "UNKNOWN";
}

} // namespace friendmesh::storage
