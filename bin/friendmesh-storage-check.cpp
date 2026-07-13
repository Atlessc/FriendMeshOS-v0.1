#include "friendmesh/storage/FriendMeshRecordCodec.h"

#include <array>
#include <cstdio>
#include <cstring>

using namespace friendmesh::storage;

namespace
{
class TestAead final : public StorageAead
{
  public:
    bool available() const override { return true; }

    bool seal(const uint8_t key[STORAGE_KEY_SIZE], const uint8_t nonce[STORAGE_NONCE_SIZE], const uint8_t *aad,
              size_t aadSize, const uint8_t *plaintext, size_t plaintextSize, uint8_t *ciphertextAndTag,
              size_t &ciphertextAndTagSize) const override
    {
        for (size_t i = 0; i < plaintextSize; ++i) {
            ciphertextAndTag[i] = plaintext[i] ^ key[i % STORAGE_KEY_SIZE] ^ nonce[i % STORAGE_NONCE_SIZE];
        }
        tag(key, nonce, aad, aadSize, ciphertextAndTag, plaintextSize, ciphertextAndTag + plaintextSize);
        ciphertextAndTagSize = plaintextSize + STORAGE_TAG_SIZE;
        return true;
    }

    bool open(const uint8_t key[STORAGE_KEY_SIZE], const uint8_t nonce[STORAGE_NONCE_SIZE], const uint8_t *aad,
              size_t aadSize, const uint8_t *ciphertextAndTag, size_t ciphertextAndTagSize, uint8_t *plaintext,
              size_t &plaintextSize) const override
    {
        if (ciphertextAndTagSize < STORAGE_TAG_SIZE) {
            return false;
        }
        const size_t cipherSize = ciphertextAndTagSize - STORAGE_TAG_SIZE;
        uint8_t expected[STORAGE_TAG_SIZE];
        tag(key, nonce, aad, aadSize, ciphertextAndTag, cipherSize, expected);
        uint8_t difference = 0;
        for (size_t i = 0; i < STORAGE_TAG_SIZE; ++i) {
            difference |= expected[i] ^ ciphertextAndTag[cipherSize + i];
        }
        if (difference != 0) {
            return false;
        }
        for (size_t i = 0; i < cipherSize; ++i) {
            plaintext[i] = ciphertextAndTag[i] ^ key[i % STORAGE_KEY_SIZE] ^ nonce[i % STORAGE_NONCE_SIZE];
        }
        plaintextSize = cipherSize;
        return true;
    }

  private:
    static void mix(uint64_t &state, const uint8_t *bytes, size_t size)
    {
        for (size_t i = 0; i < size; ++i) {
            state = (state ^ bytes[i]) * UINT64_C(1099511628211);
        }
    }

    static void tag(const uint8_t *key, const uint8_t *nonce, const uint8_t *aad, size_t aadSize,
                    const uint8_t *ciphertext, size_t ciphertextSize, uint8_t output[STORAGE_TAG_SIZE])
    {
        uint64_t a = UINT64_C(1469598103934665603);
        uint64_t b = UINT64_C(7809847782465536322);
        mix(a, key, STORAGE_KEY_SIZE);
        mix(a, nonce, STORAGE_NONCE_SIZE);
        mix(a, aad, aadSize);
        mix(a, ciphertext, ciphertextSize);
        mix(b, ciphertext, ciphertextSize);
        mix(b, aad, aadSize);
        mix(b, nonce, STORAGE_NONCE_SIZE);
        mix(b, key, STORAGE_KEY_SIZE);
        std::memcpy(output, &a, sizeof(a));
        std::memcpy(output + sizeof(a), &b, sizeof(b));
    }
};

int fail(const char *message)
{
    std::fprintf(stderr, "%s\n", message);
    return 1;
}
} // namespace

int main()
{
    TestAead aead;
    std::array<uint8_t, STORAGE_KEY_SIZE> key{};
    std::array<uint8_t, STORAGE_NONCE_SIZE> nonce{};
    StorageRecordMetadata metadata;
    metadata.schemaVersion = 3;
    metadata.keyEpoch = 9;
    metadata.sequence = 17;
    for (size_t i = 0; i < key.size(); ++i) key[i] = static_cast<uint8_t>(i + 1);
    for (size_t i = 0; i < nonce.size(); ++i) nonce[i] = static_cast<uint8_t>(0x80 + i);
    for (size_t i = 0; i < STORAGE_ID_SIZE; ++i) {
        metadata.scopeId[i] = static_cast<uint8_t>(0x20 + i);
        metadata.recordId[i] = static_cast<uint8_t>(0x40 + i);
    }
    static constexpr uint8_t message[] = "authenticated storage";
    std::array<uint8_t, STORAGE_MAX_RECORD_SIZE> record{};
    size_t recordSize = 0;
    if (FriendMeshRecordCodec::encode(aead, key.data(), nonce.data(), metadata, message, sizeof(message) - 1,
                                      record.data(), record.size(), recordSize) != StorageCodecResult::OK) {
        return fail("encode failed");
    }

    StorageRecordMetadata decoded;
    std::array<uint8_t, STORAGE_MAX_PLAINTEXT_SIZE> opened{};
    size_t openedSize = 0;
    StorageRecordExpectation expectation{&metadata.type, metadata.scopeId};
    if (FriendMeshRecordCodec::decode(aead, key.data(), expectation, record.data(), recordSize, decoded,
                                      opened.data(), opened.size(), openedSize) != StorageCodecResult::OK ||
        openedSize != sizeof(message) - 1 || std::memcmp(opened.data(), message, openedSize) != 0 ||
        decoded.sequence != metadata.sequence) {
        return fail("round trip failed");
    }

    auto wrongKey = key;
    wrongKey[0] ^= 1;
    if (FriendMeshRecordCodec::decode(aead, wrongKey.data(), expectation, record.data(), recordSize, decoded,
                                      opened.data(), opened.size(), openedSize) != StorageCodecResult::AUTH_FAILED) {
        return fail("wrong key was not rejected");
    }
    for (size_t i = 0; i < recordSize; ++i) {
        auto mutated = record;
        mutated[i] ^= 1;
        const auto result = FriendMeshRecordCodec::decode(aead, key.data(), expectation, mutated.data(), recordSize,
                                                          decoded, opened.data(), opened.size(), openedSize);
        if (result == StorageCodecResult::OK) {
            return fail("tampered record was accepted");
        }
    }
    for (size_t size = 0; size < recordSize; ++size) {
        if (FriendMeshRecordCodec::decode(aead, key.data(), expectation, record.data(), size, decoded, opened.data(),
                                          opened.size(), openedSize) == StorageCodecResult::OK) {
            return fail("truncated record was accepted");
        }
    }
    StorageRecordType wrongType = StorageRecordType::HISTORY;
    StorageRecordExpectation wrongExpectation{&wrongType, metadata.scopeId};
    if (FriendMeshRecordCodec::decode(aead, key.data(), wrongExpectation, record.data(), recordSize, decoded,
                                      opened.data(), opened.size(), openedSize) != StorageCodecResult::CONTEXT_MISMATCH) {
        return fail("wrong record context was accepted");
    }
    std::puts("FriendMesh storage framing checks passed");
    return 0;
}
