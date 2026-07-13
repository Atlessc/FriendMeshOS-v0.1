#include "FriendMeshRecordCodec.h"

#include <cstring>

namespace friendmesh::storage
{
namespace
{
constexpr uint8_t MAGIC[] = {'F', 'M', 'S', 'R'};
constexpr uint8_t FORMAT_VERSION = 1;

void writeU32(uint8_t *target, uint32_t value)
{
    for (size_t i = 0; i < 4; ++i) {
        target[i] = static_cast<uint8_t>(value >> (8 * i));
    }
}

void writeU64(uint8_t *target, uint64_t value)
{
    for (size_t i = 0; i < 8; ++i) {
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

uint64_t readU64(const uint8_t *source)
{
    uint64_t value = 0;
    for (size_t i = 0; i < 8; ++i) {
        value |= static_cast<uint64_t>(source[i]) << (8 * i);
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

bool validType(uint8_t value)
{
    return value >= static_cast<uint8_t>(StorageRecordType::SIGNING_IDENTITY) &&
           value <= static_cast<uint8_t>(StorageRecordType::NOTIFICATION);
}

bool validMetadata(const StorageRecordMetadata &metadata)
{
    return validType(static_cast<uint8_t>(metadata.type)) && metadata.schemaVersion != 0 && metadata.keyEpoch != 0 &&
           metadata.sequence != 0 && !allZero(metadata.recordId, sizeof(metadata.recordId));
}

void wipe(void *value, size_t size)
{
    volatile uint8_t *bytes = static_cast<volatile uint8_t *>(value);
    while (size-- > 0) {
        *bytes++ = 0;
    }
}
} // namespace

StorageCodecResult FriendMeshRecordCodec::encode(const StorageAead &aead, const uint8_t key[STORAGE_KEY_SIZE],
                                                 const uint8_t nonce[STORAGE_NONCE_SIZE],
                                                 const StorageRecordMetadata &metadata, const uint8_t *plaintext,
                                                 size_t plaintextSize, uint8_t *record, size_t recordCapacity,
                                                 size_t &recordSize)
{
    recordSize = 0;
    if (!key || !nonce || (!plaintext && plaintextSize != 0) || !record) {
        return StorageCodecResult::INVALID_ARGUMENT;
    }
    if (!validMetadata(metadata) || allZero(nonce, STORAGE_NONCE_SIZE)) {
        return StorageCodecResult::INVALID_METADATA;
    }
    if (plaintextSize > STORAGE_MAX_PLAINTEXT_SIZE) {
        return StorageCodecResult::RECORD_TOO_LARGE;
    }
    const size_t requiredSize = STORAGE_RECORD_HEADER_SIZE + plaintextSize + STORAGE_TAG_SIZE;
    if (recordCapacity < requiredSize) {
        return StorageCodecResult::OUTPUT_TOO_SMALL;
    }
    if (!aead.available()) {
        return StorageCodecResult::CRYPTO_UNAVAILABLE;
    }

    std::memcpy(record, MAGIC, sizeof(MAGIC));
    record[4] = FORMAT_VERSION;
    record[5] = static_cast<uint8_t>(metadata.type);
    record[6] = 0;
    record[7] = 0;
    writeU32(record + 8, metadata.schemaVersion);
    writeU32(record + 12, metadata.keyEpoch);
    writeU64(record + 16, metadata.sequence);
    std::memcpy(record + 24, metadata.scopeId, STORAGE_ID_SIZE);
    std::memcpy(record + 40, metadata.recordId, STORAGE_ID_SIZE);
    std::memcpy(record + 56, nonce, STORAGE_NONCE_SIZE);
    writeU32(record + 80, static_cast<uint32_t>(plaintextSize));

    size_t sealedSize = 0;
    if (!aead.seal(key, nonce, record, STORAGE_RECORD_HEADER_SIZE, plaintext, plaintextSize,
                   record + STORAGE_RECORD_HEADER_SIZE, sealedSize) || sealedSize != plaintextSize + STORAGE_TAG_SIZE) {
        wipe(record, requiredSize);
        return StorageCodecResult::ENCRYPT_FAILED;
    }
    recordSize = requiredSize;
    return StorageCodecResult::OK;
}

StorageCodecResult FriendMeshRecordCodec::decode(const StorageAead &aead, const uint8_t key[STORAGE_KEY_SIZE],
                                                 const StorageRecordExpectation &expectation, const uint8_t *record,
                                                 size_t recordSize, StorageRecordMetadata &metadata, uint8_t *plaintext,
                                                 size_t plaintextCapacity, size_t &plaintextSize)
{
    plaintextSize = 0;
    if (!key || !record || !plaintext) {
        return StorageCodecResult::INVALID_ARGUMENT;
    }
    if (recordSize < STORAGE_RECORD_HEADER_SIZE + STORAGE_TAG_SIZE) {
        return StorageCodecResult::RECORD_TOO_SMALL;
    }
    if (recordSize > STORAGE_MAX_RECORD_SIZE) {
        return StorageCodecResult::RECORD_TOO_LARGE;
    }
    if (std::memcmp(record, MAGIC, sizeof(MAGIC)) != 0) {
        return StorageCodecResult::BAD_MAGIC;
    }
    if (record[4] != FORMAT_VERSION) {
        return StorageCodecResult::UNSUPPORTED_VERSION;
    }
    if (!validType(record[5])) {
        return StorageCodecResult::UNSUPPORTED_TYPE;
    }
    if (record[6] != 0 || record[7] != 0) {
        return StorageCodecResult::UNSUPPORTED_FLAGS;
    }

    const uint32_t encodedPlaintextSize = readU32(record + 80);
    if (encodedPlaintextSize > STORAGE_MAX_PLAINTEXT_SIZE ||
        recordSize != STORAGE_RECORD_HEADER_SIZE + encodedPlaintextSize + STORAGE_TAG_SIZE) {
        return StorageCodecResult::LENGTH_MISMATCH;
    }
    if (plaintextCapacity < encodedPlaintextSize) {
        return StorageCodecResult::OUTPUT_TOO_SMALL;
    }

    StorageRecordMetadata parsed;
    parsed.type = static_cast<StorageRecordType>(record[5]);
    parsed.schemaVersion = readU32(record + 8);
    parsed.keyEpoch = readU32(record + 12);
    parsed.sequence = readU64(record + 16);
    std::memcpy(parsed.scopeId, record + 24, STORAGE_ID_SIZE);
    std::memcpy(parsed.recordId, record + 40, STORAGE_ID_SIZE);
    if (!validMetadata(parsed) || allZero(record + 56, STORAGE_NONCE_SIZE)) {
        return StorageCodecResult::INVALID_METADATA;
    }
    if ((expectation.type && parsed.type != *expectation.type) ||
        (expectation.scopeId && std::memcmp(parsed.scopeId, expectation.scopeId, STORAGE_ID_SIZE) != 0)) {
        return StorageCodecResult::CONTEXT_MISMATCH;
    }
    if (!aead.available()) {
        return StorageCodecResult::CRYPTO_UNAVAILABLE;
    }

    size_t openedSize = 0;
    if (!aead.open(key, record + 56, record, STORAGE_RECORD_HEADER_SIZE, record + STORAGE_RECORD_HEADER_SIZE,
                   recordSize - STORAGE_RECORD_HEADER_SIZE, plaintext, openedSize) || openedSize != encodedPlaintextSize) {
        wipe(plaintext, plaintextCapacity);
        return StorageCodecResult::AUTH_FAILED;
    }
    metadata = parsed;
    plaintextSize = openedSize;
    return StorageCodecResult::OK;
}

const char *FriendMeshRecordCodec::resultName(StorageCodecResult result)
{
    switch (result) {
    case StorageCodecResult::OK: return "OK";
    case StorageCodecResult::INVALID_ARGUMENT: return "INVALID_ARGUMENT";
    case StorageCodecResult::INVALID_METADATA: return "INVALID_METADATA";
    case StorageCodecResult::OUTPUT_TOO_SMALL: return "OUTPUT_TOO_SMALL";
    case StorageCodecResult::RECORD_TOO_SMALL: return "RECORD_TOO_SMALL";
    case StorageCodecResult::RECORD_TOO_LARGE: return "RECORD_TOO_LARGE";
    case StorageCodecResult::BAD_MAGIC: return "BAD_MAGIC";
    case StorageCodecResult::UNSUPPORTED_VERSION: return "UNSUPPORTED_VERSION";
    case StorageCodecResult::UNSUPPORTED_TYPE: return "UNSUPPORTED_TYPE";
    case StorageCodecResult::UNSUPPORTED_FLAGS: return "UNSUPPORTED_FLAGS";
    case StorageCodecResult::LENGTH_MISMATCH: return "LENGTH_MISMATCH";
    case StorageCodecResult::CONTEXT_MISMATCH: return "CONTEXT_MISMATCH";
    case StorageCodecResult::CRYPTO_UNAVAILABLE: return "CRYPTO_UNAVAILABLE";
    case StorageCodecResult::ENCRYPT_FAILED: return "ENCRYPT_FAILED";
    case StorageCodecResult::AUTH_FAILED: return "AUTH_FAILED";
    }
    return "UNKNOWN";
}

} // namespace friendmesh::storage
