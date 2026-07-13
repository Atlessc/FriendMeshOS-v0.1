#include "FriendMeshModule.h"
#include "FriendMeshStatus.h"

#if defined(FRIENDMESHOS_TDECK)

#include "friendmesh/protocol/FriendMeshProtocol.h"
#include "friendmesh/security/FriendMeshCrypto.h"
#include "friendmesh/security/FriendMeshIdentityBinding.h"
#include "friendmesh/storage/FriendMeshStorageCrypto.h"
#include "gps/RTC.h"
#include "mesh/NodeDB.h"
#include <cstring>

FriendMeshModule *friendMeshModule;

friendmesh::security::SigningIdentityStatus friendMeshSigningIdentityStatus()
{
    return friendMeshModule ? friendMeshModule->signingIdentityStatus()
                            : friendmesh::security::SigningIdentityStatus::STORAGE_UNAVAILABLE;
}

bool friendMeshStorageCryptoReady()
{
    return friendMeshModule && friendMeshModule->storageCryptoSelfTestPassed();
}

FriendMeshModule::FriendMeshModule() : SinglePortModule("FriendMesh", meshtastic_PortNum_PRIVATE_APP)
{
    cryptoReady = friendmesh::security::FriendMeshCrypto::selfTest();
    if (!cryptoReady) {
        LOG_ERROR("FriendMesh Ed25519 self-test failed; signed envelopes disabled");
    } else {
        LOG_INFO("FriendMesh Ed25519 self-test passed; signed envelope receiver ready");
    }
    friendmesh::storage::FriendMeshStorageCrypto storageCrypto;
    storageCryptoReady = storageCrypto.selfTest();
    if (storageCryptoReady) {
        LOG_INFO("FriendMesh XChaCha20-Poly1305 storage self-test passed; encrypted record codec ready");
    } else {
        LOG_ERROR("FriendMesh XChaCha20-Poly1305 storage self-test failed; encrypted persistence disabled");
    }
    const auto identityStatus = signingIdentity.initialize(nullptr);
    LOG_INFO("FriendMesh outbound signing identity: %s; transmit disabled until protected storage is available",
             friendmesh::security::signingIdentityStatusName(identityStatus));
}

ProcessMessage FriendMeshModule::handleReceived(const meshtastic_MeshPacket &packet)
{
    friendmesh_FriendMeshEnvelope envelope = friendmesh_FriendMeshEnvelope_init_zero;
    const auto decoded = friendmesh::protocol::decodeFrame(packet.decoded.payload.bytes, packet.decoded.payload.size, envelope);
    if (decoded == friendmesh::protocol::DecodeResult::NOT_FRIENDMESH) {
        return ProcessMessage::CONTINUE;
    }
    if (decoded != friendmesh::protocol::DecodeResult::OK || !cryptoReady) {
        LOG_WARN("Rejected FriendMesh frame before signature validation: %u", static_cast<unsigned>(decoded));
        return ProcessMessage::STOP;
    }

    if (!friendmesh::protocol::isKnownEventType(envelope.signed_fields)) {
        LOG_WARN("Rejected FriendMesh frame before payload dispatch: unknown event type");
        return ProcessMessage::STOP;
    }
    friendmesh_FriendMeshIdentityBinding binding = friendmesh_FriendMeshIdentityBinding_init_zero;
    const int32_t eventType = friendmesh::protocol::eventTypeValue(envelope.signed_fields);
    const bool identityBinding = eventType == friendmesh_FriendMeshEventType_FRIENDMESH_EVENT_IDENTITY_BINDING;
    if (identityBinding) {
        if (!friendmesh::security::decodeIdentityBinding(envelope.signed_fields.payload.bytes,
                                                        envelope.signed_fields.payload.size, binding)) {
            LOG_WARN("Rejected FriendMesh identity binding: malformed payload");
            return ProcessMessage::STOP;
        }
        const auto *node = nodeDB->getMeshNode(packet.from);
        if (!node || !node->has_user || node->user.public_key.size != sizeof(binding.meshtastic_public_key) ||
            std::memcmp(node->user.public_key.bytes, binding.meshtastic_public_key,
                        sizeof(binding.meshtastic_public_key)) != 0) {
            LOG_WARN("Rejected FriendMesh identity binding: Meshtastic PKI key mismatch");
            return ProcessMessage::STOP;
        }
    } else if (envelope.signed_fields.payload.size != 0) {
        LOG_WARN("Rejected FriendMesh protocol probe: unexpected payload");
        return ProcessMessage::STOP;
    }

    friendmesh::protocol::ValidationContext context;
    context.requireSender = true;
    context.senderNode = packet.from;
    const uint32_t currentTime = getValidTime(RTCQualityDevice);
    if (currentTime != 0) {
        context.requireTimestamp = true;
        context.currentTime = currentTime;
        context.maxPastSeconds = 24U * 60U * 60U;
        context.maxFutureSeconds = 10U * 60U;
    }
    const auto validation = friendmesh::protocol::validateEnvelope(
        envelope, context, friendmesh::security::FriendMeshCrypto::verify, replayWindow);
    if (validation != friendmesh::protocol::ValidationResult::ACCEPTED) {
        LOG_WARN("Rejected FriendMesh envelope: %s", friendmesh::protocol::validationResultName(validation));
        return ProcessMessage::STOP;
    }

    if (identityBinding) {
        LOG_INFO("Accepted untrusted FriendMesh identity binding for node 0x%08x; in-person verification required",
                 packet.from);
    } else {
        LOG_INFO("Accepted untrusted FriendMesh protocol event type=%u sequence=%llu",
                 static_cast<unsigned>(eventType),
                 static_cast<unsigned long long>(envelope.signed_fields.sender_sequence));
    }
    return ProcessMessage::STOP;
}

#endif
