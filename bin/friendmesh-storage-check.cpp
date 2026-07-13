#include "friendmesh/storage/FriendMeshDeviceBinding.h"
#include "friendmesh/storage/FriendMeshStorageKeyManager.h"
#include "friendmesh/storage/FriendMeshWrappedKeyStore.h"

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

class TestKdf final : public StoragePasswordKdf, public StorageSubkeyKdf
{
  public:
    bool available() const override { return true; }

    bool deriveWrappingKey(const uint8_t *credential, size_t credentialSize,
                           const uint8_t salt[STORAGE_KDF_SALT_SIZE], uint32_t operations, uint32_t memoryKiB,
                           uint8_t output[STORAGE_KEY_SIZE]) const override
    {
        ++wrappingCalls;
        for (size_t i = 0; i < STORAGE_KEY_SIZE; ++i) {
            output[i] = credential[i % credentialSize] ^ salt[i % STORAGE_KDF_SALT_SIZE] ^
                        static_cast<uint8_t>(operations * 17U + i) ^
                        static_cast<uint8_t>(memoryKiB >> ((i % 4U) * 8U));
        }
        return true;
    }

    bool deriveStorageSubkey(const uint8_t masterKey[STORAGE_KEY_SIZE], uint64_t subkeyId, const char context[8],
                             uint8_t output[STORAGE_KEY_SIZE]) const override
    {
        for (size_t i = 0; i < STORAGE_KEY_SIZE; ++i) {
            output[i] = masterKey[i] ^ static_cast<uint8_t>(context[i % 8]) ^
                        static_cast<uint8_t>(subkeyId >> ((i % 8U) * 8U)) ^ static_cast<uint8_t>(i * 13U);
        }
        return true;
    }

    mutable size_t wrappingCalls = 0;
};

class MemorySlotBackend final : public WrappedKeySlotBackend
{
  public:
    bool available() const override { return isAvailable; }

    WrappedKeySlotReadResult readSlot(uint8_t slot, uint8_t *record, size_t capacity,
                                      size_t &recordSize) const override
    {
        recordSize = 0;
        if (slot >= WRAPPED_KEY_SLOT_COUNT || ioError[slot]) return WrappedKeySlotReadResult::IO_ERROR;
        if (!present[slot]) return WrappedKeySlotReadResult::NOT_FOUND;
        if (sizes[slot] > capacity) return WrappedKeySlotReadResult::IO_ERROR;
        std::memcpy(record, slots[slot].data(), sizes[slot]);
        recordSize = sizes[slot];
        return WrappedKeySlotReadResult::OK;
    }

    bool writeSlot(uint8_t slot, const uint8_t *record, size_t recordSize) override
    {
        ++writeCalls;
        if (slot >= WRAPPED_KEY_SLOT_COUNT || !record || recordSize > slots[slot].size()) return false;
        const size_t bytes = failAfterBytes < recordSize ? failAfterBytes : recordSize;
        std::memcpy(slots[slot].data(), record, bytes);
        sizes[slot] = bytes;
        present[slot] = true;
        if (bytes != recordSize) return false;
        if (corruptStoredWrite) {
            slots[slot][STORAGE_WRAPPED_KEY_HEADER_SIZE] ^= 1;
            corruptStoredWrite = false;
        }
        return true;
    }

    bool clearSlot(uint8_t slot) override
    {
        if (slot >= WRAPPED_KEY_SLOT_COUNT || failClear) return false;
        slots[slot].fill(0);
        sizes[slot] = 0;
        present[slot] = false;
        return true;
    }

    void setSlot(uint8_t slot, const uint8_t *record, size_t recordSize)
    {
        std::memcpy(slots[slot].data(), record, recordSize);
        sizes[slot] = recordSize;
        present[slot] = true;
    }

    bool isAvailable = true;
    bool present[WRAPPED_KEY_SLOT_COUNT]{};
    bool ioError[WRAPPED_KEY_SLOT_COUNT]{};
    bool corruptStoredWrite = false;
    bool failClear = false;
    size_t failAfterBytes = STORAGE_WRAPPED_KEY_SIZE;
    size_t writeCalls = 0;
    std::array<uint8_t, STORAGE_WRAPPED_KEY_SIZE> slots[WRAPPED_KEY_SLOT_COUNT]{};
    size_t sizes[WRAPPED_KEY_SLOT_COUNT]{};
};

class TestRandom final : public StorageRandomSource
{
  public:
    bool fill(uint8_t *output, size_t outputSize) const override
    {
        if (!available || !output || outputSize == 0) return false;
        for (size_t index = 0; index < outputSize; ++index) {
            output[index] = next++;
        }
        return true;
    }

    bool available = true;
    mutable uint8_t next = 1;
};

class TestBinding final : public StorageDeviceBindingSource
{
  public:
    DeviceBindingResult read(uint8_t output[STORAGE_DEVICE_BINDING_SIZE]) const override
    {
        if (!available || !output) return DeviceBindingResult::HARDWARE_UNAVAILABLE;
        std::memcpy(output, binding.data(), binding.size());
        return DeviceBindingResult::OK;
    }

    bool available = true;
    std::array<uint8_t, STORAGE_DEVICE_BINDING_SIZE> binding{
        0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
        0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0,
    };
};

int fail(const char *message)
{
    std::fprintf(stderr, "%s\n", message);
    return 1;
}
} // namespace

int main()
{
    std::array<uint8_t, STORAGE_HARDWARE_ID_SIZE> hardwareIdA{};
    std::array<uint8_t, STORAGE_HARDWARE_ID_SIZE> hardwareIdB{};
    for (size_t index = 0; index < hardwareIdA.size(); ++index) {
        hardwareIdA[index] = static_cast<uint8_t>(0x11U + index);
        hardwareIdB[index] = static_cast<uint8_t>(0x80U + index);
    }
    std::array<uint8_t, STORAGE_DEVICE_BINDING_SIZE> derivedBindingA{};
    std::array<uint8_t, STORAGE_DEVICE_BINDING_SIZE> repeatedBindingA{};
    std::array<uint8_t, STORAGE_DEVICE_BINDING_SIZE> derivedBindingB{};
    if (deriveDeviceBinding(hardwareIdA.data(), hardwareIdA.size(), derivedBindingA.data()) !=
            DeviceBindingResult::OK ||
        deriveDeviceBinding(hardwareIdA.data(), hardwareIdA.size(), repeatedBindingA.data()) !=
            DeviceBindingResult::OK ||
        deriveDeviceBinding(hardwareIdB.data(), hardwareIdB.size(), derivedBindingB.data()) !=
            DeviceBindingResult::OK ||
        derivedBindingA != repeatedBindingA || derivedBindingA == derivedBindingB) {
        return fail("device binding derivation was not stable and distinct");
    }
    std::array<uint8_t, STORAGE_HARDWARE_ID_SIZE> zeroHardwareId{};
    repeatedBindingA.fill(0xa5);
    if (deriveDeviceBinding(zeroHardwareId.data(), zeroHardwareId.size(), repeatedBindingA.data()) !=
            DeviceBindingResult::INVALID_HARDWARE_ID ||
        repeatedBindingA != std::array<uint8_t, STORAGE_DEVICE_BINDING_SIZE>{}) {
        return fail("invalid hardware id did not fail closed");
    }
    repeatedBindingA.fill(0xa5);
    if (deriveDeviceBinding(hardwareIdA.data(), hardwareIdA.size() - 1, repeatedBindingA.data()) !=
            DeviceBindingResult::INVALID_HARDWARE_ID ||
        repeatedBindingA != std::array<uint8_t, STORAGE_DEVICE_BINDING_SIZE>{}) {
        return fail("short hardware id did not fail closed");
    }

    TestAead aead;
    TestKdf kdf;
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

    static constexpr uint8_t credential[] = "739204";
    const auto binding = derivedBindingA;
    std::array<uint8_t, STORAGE_KDF_SALT_SIZE> salt{};
    std::array<uint8_t, STORAGE_NONCE_SIZE> wrapNonce{};
    std::array<uint8_t, STORAGE_KEY_SIZE> masterKey{};
    for (size_t i = 0; i < salt.size(); ++i) salt[i] = static_cast<uint8_t>(0x31 + i);
    for (size_t i = 0; i < wrapNonce.size(); ++i) wrapNonce[i] = static_cast<uint8_t>(0x51 + i);
    for (size_t i = 0; i < masterKey.size(); ++i) masterKey[i] = static_cast<uint8_t>(0x71 + i);
    WrappedMasterKeyParameters wrapParameters;
    wrapParameters.memoryKiB = 1024;
    wrapParameters.operations = 3;
    wrapParameters.generation = 7;
    std::array<uint8_t, STORAGE_WRAPPED_KEY_SIZE> wrapped{};
    size_t wrappedSize = 0;
    if (FriendMeshMasterKey::wrap(kdf, aead, credential, sizeof(credential) - 1, binding.data(), wrapParameters,
                                  salt.data(), wrapNonce.data(), masterKey.data(), wrapped.data(), wrapped.size(),
                                  wrappedSize) != MasterKeyResult::OK ||
        wrappedSize != wrapped.size()) {
        return fail("master key wrap failed");
    }
    static constexpr uint8_t expectedWrapped[STORAGE_WRAPPED_KEY_SIZE] = {
        0x46, 0x4d, 0x57, 0x4b, 0x01, 0x01, 0x01, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x00, 0x04, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33, 0x34,
        0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40,
        0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c,
        0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
        0x15, 0x11, 0x1f, 0x10, 0x12, 0x1e, 0x19, 0x11, 0x1b, 0x10, 0x16, 0x16,
        0x15, 0x69, 0x67, 0xd0, 0xa2, 0xa6, 0xa1, 0xa1, 0xab, 0xa8, 0xae, 0xa6,
        0x9d, 0x99, 0x97, 0x98, 0x9a, 0x86, 0x81, 0xe9, 0x1f, 0x03, 0x4c, 0xb8,
        0xaf, 0xc6, 0x69, 0xbe, 0x96, 0x99, 0xb7, 0x27, 0xb6, 0x6b, 0xdd, 0x45,
    };
    if (std::memcmp(wrapped.data(), expectedWrapped, wrapped.size()) != 0) {
        return fail("wrapped master key vector changed");
    }

    std::array<uint8_t, STORAGE_KEY_SIZE> unwrapped{};
    WrappedMasterKeyParameters decodedWrapParameters;
    if (FriendMeshMasterKey::unwrap(kdf, aead, credential, sizeof(credential) - 1, binding.data(), wrapped.data(),
                                    wrapped.size(), decodedWrapParameters, unwrapped.data()) != MasterKeyResult::OK ||
        std::memcmp(unwrapped.data(), masterKey.data(), masterKey.size()) != 0 ||
        decodedWrapParameters.memoryKiB != wrapParameters.memoryKiB ||
        decodedWrapParameters.operations != wrapParameters.operations ||
        decodedWrapParameters.generation != wrapParameters.generation) {
        return fail("master key unwrap failed");
    }

    static constexpr uint8_t wrongCredential[] = "739205";
    unwrapped.fill(0xa5);
    if (FriendMeshMasterKey::unwrap(kdf, aead, wrongCredential, sizeof(wrongCredential) - 1, binding.data(),
                                    wrapped.data(), wrapped.size(), decodedWrapParameters,
                                    unwrapped.data()) != MasterKeyResult::AUTH_FAILED) {
        return fail("wrong credential was not rejected");
    }
    for (uint8_t value : unwrapped) {
        if (value != 0) return fail("failed unwrap output was not wiped");
    }
    auto wrongBinding = binding;
    wrongBinding[0] ^= 1;
    if (FriendMeshMasterKey::unwrap(kdf, aead, credential, sizeof(credential) - 1, wrongBinding.data(),
                                    wrapped.data(), wrapped.size(), decodedWrapParameters,
                                    unwrapped.data()) != MasterKeyResult::AUTH_FAILED) {
        return fail("wrong device binding was not rejected");
    }
    for (size_t i = 0; i < wrapped.size(); ++i) {
        auto mutated = wrapped;
        mutated[i] ^= 1;
        if (FriendMeshMasterKey::unwrap(kdf, aead, credential, sizeof(credential) - 1, binding.data(), mutated.data(),
                                        mutated.size(), decodedWrapParameters, unwrapped.data()) ==
            MasterKeyResult::OK) {
            return fail("tampered wrapped key was accepted");
        }
    }
    for (size_t size = 0; size < wrapped.size(); ++size) {
        if (FriendMeshMasterKey::unwrap(kdf, aead, credential, sizeof(credential) - 1, binding.data(), wrapped.data(),
                                        size, decodedWrapParameters, unwrapped.data()) == MasterKeyResult::OK) {
            return fail("truncated wrapped key was accepted");
        }
    }

    auto excessiveCost = wrapped;
    excessiveCost[12] = 0x01;
    excessiveCost[13] = 0x08;
    excessiveCost[14] = 0x00;
    excessiveCost[15] = 0x00;
    const size_t callsBeforeCostCheck = kdf.wrappingCalls;
    if (FriendMeshMasterKey::unwrap(kdf, aead, credential, sizeof(credential) - 1, binding.data(),
                                    excessiveCost.data(), excessiveCost.size(), decodedWrapParameters,
                                    unwrapped.data()) != MasterKeyResult::INVALID_PARAMETERS ||
        kdf.wrappingCalls != callsBeforeCostCheck) {
        return fail("untrusted KDF cost was executed");
    }

    std::array<std::array<uint8_t, STORAGE_KEY_SIZE>, 10> subkeys{};
    for (size_t index = 0; index < subkeys.size(); ++index) {
        if (FriendMeshMasterKey::deriveSubkey(kdf, masterKey.data(),
                                              static_cast<StorageKeyDomain>(index + 1), subkeys[index].data()) !=
            MasterKeyResult::OK) {
            return fail("subkey derivation failed");
        }
        std::array<uint8_t, STORAGE_KEY_SIZE> repeated{};
        if (FriendMeshMasterKey::deriveSubkey(kdf, masterKey.data(),
                                              static_cast<StorageKeyDomain>(index + 1), repeated.data()) !=
                MasterKeyResult::OK ||
            repeated != subkeys[index]) {
            return fail("subkey derivation was not stable");
        }
        for (size_t prior = 0; prior < index; ++prior) {
            if (subkeys[prior] == subkeys[index]) return fail("storage domains reused a subkey");
        }
    }
    std::array<uint8_t, STORAGE_KEY_SIZE> invalidSubkey;
    invalidSubkey.fill(0xa5);
    if (FriendMeshMasterKey::deriveSubkey(kdf, masterKey.data(), static_cast<StorageKeyDomain>(255),
                                          invalidSubkey.data()) != MasterKeyResult::UNSUPPORTED_DOMAIN) {
        return fail("unknown storage domain was accepted");
    }
    for (uint8_t value : invalidSubkey) {
        if (value != 0) return fail("unknown-domain output was not wiped");
    }

    auto makeWrapped = [&](uint32_t generation, const std::array<uint8_t, STORAGE_KEY_SIZE> &keyToWrap,
                           std::array<uint8_t, STORAGE_WRAPPED_KEY_SIZE> &output) {
        WrappedMasterKeyParameters parameters;
        parameters.generation = generation;
        std::array<uint8_t, STORAGE_KDF_SALT_SIZE> generationSalt{};
        std::array<uint8_t, STORAGE_NONCE_SIZE> generationNonce{};
        for (size_t i = 0; i < generationSalt.size(); ++i) {
            generationSalt[i] = static_cast<uint8_t>(0x20U + generation + i);
        }
        for (size_t i = 0; i < generationNonce.size(); ++i) {
            generationNonce[i] = static_cast<uint8_t>(0x60U + generation + i);
        }
        size_t outputSize = 0;
        return FriendMeshMasterKey::wrap(kdf, aead, credential, sizeof(credential) - 1, binding.data(), parameters,
                                         generationSalt.data(), generationNonce.data(), keyToWrap.data(),
                                         output.data(), output.size(), outputSize) == MasterKeyResult::OK &&
               outputSize == output.size();
    };

    MemorySlotBackend slotBackend;
    FriendMeshWrappedKeyStore keyStore(slotBackend);
    WrappedKeyLoadInfo loadInfo;
    std::array<uint8_t, STORAGE_KEY_SIZE> recoveredKey;
    recoveredKey.fill(0xa5);
    if (keyStore.load(kdf, aead, credential, sizeof(credential) - 1, binding.data(), loadInfo,
                      recoveredKey.data()) != WrappedKeyStoreResult::EMPTY) {
        return fail("empty wrapped-key store was not detected");
    }
    for (uint8_t value : recoveredKey) {
        if (value != 0) return fail("empty-store output was not wiped");
    }

    std::array<uint8_t, STORAGE_WRAPPED_KEY_SIZE> generation1{};
    std::array<uint8_t, STORAGE_WRAPPED_KEY_SIZE> generation2{};
    std::array<uint8_t, STORAGE_WRAPPED_KEY_SIZE> generation3{};
    std::array<uint8_t, STORAGE_WRAPPED_KEY_SIZE> generation4{};
    if (!makeWrapped(1, masterKey, generation1) || !makeWrapped(2, masterKey, generation2) ||
        !makeWrapped(3, masterKey, generation3) || !makeWrapped(4, masterKey, generation4)) {
        return fail("transaction test wrapped-key generation failed");
    }

    uint8_t slotWritten = 99;
    if (keyStore.commitPrepared(generation1.data(), generation1.size(), nullptr, slotWritten) !=
            WrappedKeyStoreResult::OK ||
        slotWritten != 0 ||
        keyStore.load(kdf, aead, credential, sizeof(credential) - 1, binding.data(), loadInfo,
                      recoveredKey.data()) != WrappedKeyStoreResult::OK ||
        loadInfo.parameters.generation != 1 || loadInfo.selectedSlot != 0 || !loadInfo.degraded ||
        recoveredKey != masterKey) {
        return fail("initial wrapped-key slot commit failed");
    }

    slotBackend.failAfterBytes = 37;
    if (keyStore.commitPrepared(generation2.data(), generation2.size(), &loadInfo, slotWritten) !=
            WrappedKeyStoreResult::WRITE_FAILED ||
        keyStore.load(kdf, aead, credential, sizeof(credential) - 1, binding.data(), loadInfo,
                      recoveredKey.data()) != WrappedKeyStoreResult::OK ||
        loadInfo.parameters.generation != 1 || loadInfo.selectedSlot != 0 || recoveredKey != masterKey) {
        return fail("partial inactive-slot write did not preserve the prior generation");
    }

    slotBackend.failAfterBytes = STORAGE_WRAPPED_KEY_SIZE;
    if (keyStore.commitPrepared(generation2.data(), generation2.size(), &loadInfo, slotWritten) !=
            WrappedKeyStoreResult::OK ||
        slotWritten != 1 ||
        keyStore.load(kdf, aead, credential, sizeof(credential) - 1, binding.data(), loadInfo,
                      recoveredKey.data()) != WrappedKeyStoreResult::OK ||
        loadInfo.parameters.generation != 2 || loadInfo.selectedSlot != 1 || loadInfo.degraded) {
        return fail("second wrapped-key generation was not selected");
    }

    slotBackend.failAfterBytes = 59;
    if (keyStore.commitPrepared(generation3.data(), generation3.size(), &loadInfo, slotWritten) !=
            WrappedKeyStoreResult::WRITE_FAILED ||
        keyStore.load(kdf, aead, credential, sizeof(credential) - 1, binding.data(), loadInfo,
                      recoveredKey.data()) != WrappedKeyStoreResult::OK ||
        loadInfo.parameters.generation != 2 || loadInfo.selectedSlot != 1) {
        return fail("interrupted slot rotation did not recover the last-known-good generation");
    }

    slotBackend.failAfterBytes = STORAGE_WRAPPED_KEY_SIZE;
    if (keyStore.commitPrepared(generation3.data(), generation3.size(), &loadInfo, slotWritten) !=
            WrappedKeyStoreResult::OK ||
        keyStore.load(kdf, aead, credential, sizeof(credential) - 1, binding.data(), loadInfo,
                      recoveredKey.data()) != WrappedKeyStoreResult::OK ||
        loadInfo.parameters.generation != 3 || loadInfo.selectedSlot != 0) {
        return fail("third wrapped-key generation was not recovered");
    }

    slotBackend.corruptStoredWrite = true;
    if (keyStore.commitPrepared(generation4.data(), generation4.size(), &loadInfo, slotWritten) !=
            WrappedKeyStoreResult::READBACK_FAILED ||
        keyStore.load(kdf, aead, credential, sizeof(credential) - 1, binding.data(), loadInfo,
                      recoveredKey.data()) != WrappedKeyStoreResult::OK ||
        loadInfo.parameters.generation != 3 || loadInfo.selectedSlot != 0) {
        return fail("corrupt readback did not preserve the last-known-good generation");
    }

    const size_t writesBeforeStale = slotBackend.writeCalls;
    if (keyStore.commitPrepared(generation3.data(), generation3.size(), &loadInfo, slotWritten) !=
            WrappedKeyStoreResult::STALE_GENERATION ||
        slotBackend.writeCalls != writesBeforeStale) {
        return fail("stale generation reached slot storage");
    }

    recoveredKey.fill(0xa5);
    if (keyStore.load(kdf, aead, wrongCredential, sizeof(wrongCredential) - 1, binding.data(), loadInfo,
                      recoveredKey.data()) != WrappedKeyStoreResult::NO_AUTHENTICATED_KEY) {
        return fail("wrong credential unlocked a wrapped-key slot");
    }
    for (uint8_t value : recoveredKey) {
        if (value != 0) return fail("failed slot unlock output was not wiped");
    }

    MemorySlotBackend degradedBackend;
    degradedBackend.setSlot(0, generation1.data(), generation1.size());
    degradedBackend.ioError[1] = true;
    FriendMeshWrappedKeyStore degradedStore(degradedBackend);
    if (degradedStore.load(kdf, aead, credential, sizeof(credential) - 1, binding.data(), loadInfo,
                           recoveredKey.data()) != WrappedKeyStoreResult::OK ||
        !loadInfo.degraded || loadInfo.ioErrorMask != 0x02 || recoveredKey != masterKey) {
        return fail("valid slot did not survive peer-slot I/O failure");
    }

    auto conflictingKey = masterKey;
    conflictingKey[0] ^= 0x80;
    std::array<uint8_t, STORAGE_WRAPPED_KEY_SIZE> conflictingGeneration1{};
    if (!makeWrapped(1, conflictingKey, conflictingGeneration1)) {
        return fail("conflict test wrapped-key generation failed");
    }
    MemorySlotBackend conflictBackend;
    conflictBackend.setSlot(0, generation1.data(), generation1.size());
    conflictBackend.setSlot(1, conflictingGeneration1.data(), conflictingGeneration1.size());
    FriendMeshWrappedKeyStore conflictStore(conflictBackend);
    if (conflictStore.load(kdf, aead, credential, sizeof(credential) - 1, binding.data(), loadInfo,
                           recoveredKey.data()) != WrappedKeyStoreResult::GENERATION_CONFLICT) {
        return fail("same-generation divergent master keys were not quarantined");
    }

    static constexpr uint8_t productionPin[] = "381904";
    static constexpr uint8_t replacementPin[] = "604217";
    static constexpr uint8_t invalidZeroPin[] = "000000";
    static constexpr uint8_t invalidShortPin[] = "12345";
    static constexpr uint8_t invalidTextPin[] = "12A456";
    if (!FriendMeshStorageKeyManager::validPin(productionPin, sizeof(productionPin) - 1) ||
        FriendMeshStorageKeyManager::validPin(invalidZeroPin, sizeof(invalidZeroPin) - 1) ||
        FriendMeshStorageKeyManager::validPin(invalidShortPin, sizeof(invalidShortPin) - 1) ||
        FriendMeshStorageKeyManager::validPin(invalidTextPin, sizeof(invalidTextPin) - 1)) {
        return fail("production PIN policy was not enforced");
    }

    MemorySlotBackend productionBackend;
    FriendMeshWrappedKeyStore productionStore(productionBackend);
    TestRandom productionRandom;
    TestBinding productionBinding;
    FriendMeshStorageKeyManager keyManager(productionStore, productionBackend, kdf, aead, kdf, productionRandom,
                                           productionBinding);
    if (keyManager.probe() != StorageKeyStatus::NOT_CONFIGURED ||
        keyManager.provision(invalidZeroPin, sizeof(invalidZeroPin) - 1) != StorageKeyResult::INVALID_PIN ||
        productionBackend.writeCalls != 0) {
        return fail("invalid PIN reached production key storage");
    }
    productionBackend.present[0] = true;
    productionBackend.sizes[0] = 7;
    if (keyManager.probe() != StorageKeyStatus::CORRUPT) {
        return fail("structurally invalid production slot was reported as locked");
    }
    productionBackend.clearSlot(0);
    productionBinding.available = false;
    if (keyManager.probe() != StorageKeyStatus::BINDING_UNAVAILABLE) {
        return fail("missing hardware binding did not fail closed");
    }
    productionBinding.available = true;
    productionRandom.available = false;
    if (keyManager.provision(productionPin, sizeof(productionPin) - 1) != StorageKeyResult::RANDOM_FAILED ||
        productionBackend.writeCalls != 0) {
        return fail("random-source failure reached production key storage");
    }
    productionRandom.available = true;
    if (keyManager.provision(productionPin, sizeof(productionPin) - 1) != StorageKeyResult::OK ||
        keyManager.status() != StorageKeyStatus::READY || keyManager.generation() != 2 ||
        !productionBackend.present[0] || !productionBackend.present[1]) {
        return fail("explicit production key provisioning failed");
    }
    std::array<uint8_t, STORAGE_KEY_SIZE> managerSubkey{};
    if (keyManager.deriveSubkey(StorageKeyDomain::SIGNING_IDENTITY, managerSubkey.data()) != StorageKeyResult::OK ||
        managerSubkey == std::array<uint8_t, STORAGE_KEY_SIZE>{}) {
        return fail("ready production manager did not derive a domain subkey");
    }
    if (keyManager.provision(replacementPin, sizeof(replacementPin) - 1) !=
        StorageKeyResult::ALREADY_CONFIGURED) {
        return fail("production manager silently reprovisioned an existing key");
    }

    keyManager.lock();
    managerSubkey.fill(0xa5);
    if (keyManager.status() != StorageKeyStatus::LOCKED ||
        keyManager.deriveSubkey(StorageKeyDomain::SIGNING_IDENTITY, managerSubkey.data()) !=
            StorageKeyResult::LOCKED ||
        managerSubkey != std::array<uint8_t, STORAGE_KEY_SIZE>{}) {
        return fail("locking did not revoke production subkey access");
    }
    if (keyManager.unlock(wrongCredential, sizeof(wrongCredential) - 1) !=
            StorageKeyResult::NO_AUTHENTICATED_KEY ||
        keyManager.status() != StorageKeyStatus::LOCKED ||
        keyManager.unlock(productionPin, sizeof(productionPin) - 1) != StorageKeyResult::OK ||
        keyManager.status() != StorageKeyStatus::READY || keyManager.generation() != 2) {
        return fail("production manager PIN unlock lifecycle failed");
    }
    if (keyManager.rewrap(wrongCredential, sizeof(wrongCredential) - 1, replacementPin,
                          sizeof(replacementPin) - 1) != StorageKeyResult::NO_AUTHENTICATED_KEY ||
        keyManager.status() != StorageKeyStatus::READY) {
        return fail("wrong current PIN changed the unlocked production state");
    }
    if (keyManager.rewrap(productionPin, sizeof(productionPin) - 1, replacementPin,
                          sizeof(replacementPin) - 1) != StorageKeyResult::OK ||
        keyManager.status() != StorageKeyStatus::READY || keyManager.generation() != 4) {
        return fail("transactional production PIN rewrap failed");
    }
    keyManager.lock();
    if (keyManager.unlock(productionPin, sizeof(productionPin) - 1) !=
            StorageKeyResult::NO_AUTHENTICATED_KEY ||
        keyManager.unlock(replacementPin, sizeof(replacementPin) - 1) != StorageKeyResult::OK ||
        keyManager.generation() != 4) {
        return fail("retired production PIN still unlocked storage");
    }
    keyManager.lock();
    productionBinding.binding[0] ^= 1;
    if (keyManager.unlock(replacementPin, sizeof(replacementPin) - 1) !=
            StorageKeyResult::NO_AUTHENTICATED_KEY ||
        keyManager.status() != StorageKeyStatus::LOCKED) {
        return fail("wrong production device binding unlocked storage");
    }

    MemorySlotBackend failedProvisionBackend;
    failedProvisionBackend.failAfterBytes = 23;
    FriendMeshWrappedKeyStore failedProvisionStore(failedProvisionBackend);
    TestRandom failedProvisionRandom;
    TestBinding failedProvisionBinding;
    FriendMeshStorageKeyManager failedProvisionManager(failedProvisionStore, failedProvisionBackend, kdf, aead, kdf,
                                                       failedProvisionRandom, failedProvisionBinding);
    if (failedProvisionManager.provision(productionPin, sizeof(productionPin) - 1) !=
            StorageKeyResult::WRITE_FAILED ||
        failedProvisionManager.status() != StorageKeyStatus::CORRUPT) {
        return fail("failed production commit did not fail closed");
    }
    managerSubkey.fill(0xa5);
    if (failedProvisionManager.deriveSubkey(StorageKeyDomain::SIGNING_IDENTITY, managerSubkey.data()) !=
            StorageKeyResult::LOCKED ||
        managerSubkey != std::array<uint8_t, STORAGE_KEY_SIZE>{}) {
        return fail("failed production commit retained usable key material");
    }

    std::puts("FriendMesh storage framing checks passed");
    return 0;
}
