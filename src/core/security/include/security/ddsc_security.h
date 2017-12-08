//
// Created by kurtulus on 29-11-17.
//
//#include "os/os_public.h"
//#include "os/os_decl_attributes_sal.h"

#include "ddsc/dds.h"
#include "dds_security_builtintopics.h"
#include "dds_security_interface_types.h"

#ifndef DDSC_SECURITY_H
#define DDSC_SECURITY_H

//Note – It is recommended that native types be mapped to equivalent type
// names in each programming language, subject to the normal mapping rules for type names in that language

/**
 * Authentication Component
 */

typedef struct DDS_Security_Authentication DDS_Security_Authentication;

/**
 * AuthenticationListener interface
 */

typedef bool
(*DDS_Security_AuthenticationListener_OnRevokeIdentityListener)
        (void *listener_data,
         _In_ DDS_Security_Authentication *plugin,
         _In_ DDS_Security_IdentityHandle handle,
         _Inout_ DDS_Security_SecurityException *ex
        );

typedef bool
(*DDS_Security_AuthenticationListener_OnStatusChangedListener)
        (void *listener_data,
         _In_ DDS_Security_Authentication *plugin,
         _In_ DDS_Security_IdentityHandle handle,
         _In_ DDS_Security_AuthStatusKind status_kind,
         _Inout_ DDS_Security_SecurityException *ex
        );


typedef struct DDS_Security_AuthenticationListener
{
  void *listener_data;

  DDS_Security_AuthenticationListener_OnRevokeIdentityListener on_revoke_identity;

  DDS_Security_AuthenticationListener_OnStatusChangedListener on_status_changed;
} DDS_Security_AuthenticationListener;

struct DDS_Security_AuthenticationListener *DDS_Security_AuthenticationListener__alloc(void);


typedef DDS_Security_ValidationResult_t
(*DDS_Security_Authentication_ValidateLocalIdentity)
        (void *listener_data,
         _Inout_ DDS_Security_IdentityHandle local_identity_handle,
         _Inout_ DDS_Security_GUID_t *adjusted_participant_guid,
         _In_ DDS_DomainId_t domain_id,
         _In_ DDS_Security_DomainParticipantQos *participant_qos,
         _In_ DDS_Security_GUID_t *candidate_participant_guid,
         _Inout_ DDS_Security_SecurityException *ex
        );


typedef bool
(*DDS_Security_Authentication_get_identity_token)
        (void *listener_data,
         _Inout_ DDS_Security_IdentityToken identity_token,
         _In_ DDS_Security_IdentityHandle handle,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-82
typedef bool
(*DDS_Security_Authentication_get_identity_status_token)
        (void *listener_data,
         _Inout_ DDS_Security_IdentityStatusToken identity_status_token,
         _In_ DDS_Security_IdentityHandle handle,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_Authentication_set_permissions_credential_and_token)
        (void *listener_data,
         _In_ DDS_Security_IdentityHandle handle,
         _In_ DDS_Security_PermissionsCredentialToken permissions_credential,
         _In_ DDS_Security_PermissionsToken permissions_token,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-21
// DDSSEC11-88
// DDSSEC11-85
typedef DDS_Security_ValidationResult_t
(*DDS_Security_Authentication_validate_remote_identity)
        (void *listener_data,
         _Inout_ DDS_Security_IdentityHandle remote_identity_handle,
         _Inout_ DDS_Security_AuthRequestMessageToken local_auth_request_token,
         _In_ DDS_Security_AuthRequestMessageToken remote_auth_request_token,
         _In_ DDS_Security_IdentityHandle local_identity_handle,
         _In_ DDS_Security_IdentityToken remote_identity_token,
         _In_ DDS_Security_GUID_t remote_participant_guid,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-46
// DDSSEC11-118
typedef DDS_Security_ValidationResult_t
(*DDS_Security_Authentication_begin_handshake_request)
        (void *listener_data,
         _Inout_ DDS_Security_HandshakeHandle handshake_handle,
         _Inout_ DDS_Security_HandshakeMessageToken handshake_message,
         _In_ DDS_Security_HandshakeMessageToken handshake_message_in,
         _In_ DDS_Security_IdentityHandle initiator_identity_handle,
         _In_ DDS_Security_IdentityHandle replier_identity_handle,
         _In_ DDS_OctetSeq serialized_local_participant_data,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-46
typedef DDS_Security_ValidationResult_t
(*DDS_Security_Authentication_begin_handshake_reply)
        (void *listener_data,
         _Inout_ DDS_Security_HandshakeHandle handshake_handle,
         _Inout_ DDS_Security_HandshakeMessageToken handshake_message_out,
         _In_ DDS_Security_HandshakeMessageToken handshake_message_in,
         _In_ DDS_Security_IdentityHandle initiator_identity_handle,
         _In_ DDS_Security_IdentityHandle replier_identity_handle,
         _In_ DDS_OctetSeq serialized_local_participant_data,
         _Inout_ DDS_Security_SecurityException ex);

typedef DDS_Security_ValidationResult_t
(*DDS_Security_Authentication_process_handshake)
        (void *listener_data,
         _Inout_ DDS_Security_HandshakeMessageToken handshake_message_out,
         _In_ DDS_Security_HandshakeMessageToken handshake_message_in,
         _In_ DDS_Security_HandshakeHandle handshake_handle,
         _Inout_ DDS_Security_SecurityException ex);

typedef DDS_Security_SharedSecretHandle
(*DDS_Security_Authentication_get_shared_secret)
        (void *listener_data,
         _In_ DDS_Security_HandshakeHandle handshake_handle,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_Authentication_get_authenticated_peer_credential_token)
        (void *listener_data,
         _Inout_ DDS_Security_AuthenticatedPeerCredentialToken peer_credential_token,
         _In_ DDS_Security_HandshakeHandle handshake_handle,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_Authentication_set_listener)
        (void *listener_data,
         _In_ DDS_Security_AuthenticationListener listener,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_Authentication_return_identity_token)
        (void *listener_data,
         _In_ DDS_Security_IdentityToken token,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-82
typedef bool
(*DDS_Security_Authentication_return_identity_status_token)
        (void *listener_data,
         _In_ DDS_Security_IdentityStatusToken token,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_Authentication_return_authenticated_peer_credential_token)
        (void *listener_data,
         _In_ DDS_Security_AuthenticatedPeerCredentialToken peer_credential_token,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_Authentication_return_handshake_handle)
        (void *listener_data,
         _In_ DDS_Security_HandshakeHandle handshake_handle,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_Authentication_return_identity_handle)
        (void *listener_data,
         _In_ DDS_Security_IdentityHandle identity_handle,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_Authentication_return_sharedsecret_handle)
        (void *listener_data,
         _In_ DDS_Security_SharedSecretHandle sharedsecret_handle,
         _Inout_ DDS_Security_SecurityException ex);


// DDSSEC11-96
struct DDS_Security_Authentication
{
  // DDSSEC11-88
  DDS_Security_Authentication_ValidateLocalIdentity validate_local_identity;

  DDS_Security_Authentication_get_identity_token get_identity_token;

  DDS_Security_Authentication_get_identity_status_token get_identity_status_token;

  DDS_Security_Authentication_set_permissions_credential_and_token set_permissions_credential_and_token;

  DDS_Security_Authentication_validate_remote_identity validate_remote_identity;

  DDS_Security_Authentication_begin_handshake_request begin_handshake_request;

  DDS_Security_Authentication_begin_handshake_reply begin_handshake_reply;

  DDS_Security_Authentication_process_handshake process_handshake;

  DDS_Security_Authentication_get_shared_secret get_shared_secret;

  DDS_Security_Authentication_get_authenticated_peer_credential_token get_authenticated_peer_credential_token;

  DDS_Security_Authentication_set_listener set_listener;

  DDS_Security_Authentication_return_identity_token return_identity_token;

  DDS_Security_Authentication_return_identity_status_token return_identity_status_token;

  DDS_Security_Authentication_return_authenticated_peer_credential_token return_authenticated_peer_credential_token;

  DDS_Security_Authentication_return_handshake_handle return_handshake_handle;

  DDS_Security_Authentication_return_identity_handle return_identity_handle;

  DDS_Security_Authentication_return_sharedsecret_handle return_sharedsecret_handle;
};

struct DDS_Security_Authentication *DDS_Security_Authentication__alloc(void);


/**
 * AccessControl Component
 */

typedef struct DDS_Security_AccessControl DDS_Security_AccessControl;

/**
 * AccessControlListener Interface
 * */


typedef bool// DDSSEC11-96
(*DDS_Security_AccessControlListener_on_revoke_permissions)
        (void *listener_data,
         _In_ DDS_Security_AccessControl plugin,
         _In_ DDS_Security_PermissionsHandle handle);

typedef struct DDS_Security_AccessControlListener
{
  DDS_Security_AccessControlListener_on_revoke_permissions on_revoke_permissions;
} DDS_Security_AccessControlListener;


/**
 * AccessControl Interface
 */

typedef DDS_Security_PermissionsHandle
(*DDS_Security_AccessControl_validate_local_permissions)
        (void *listener_data,
         _In_ DDS_Security_Authentication auth_plugin,
         _In_ DDS_Security_IdentityHandle identity,
         _In_ DDS_DomainId_t domain_id,
         _In_ DDS_Security_DomainParticipantQos participant_qos,
         _Inout_ DDS_Security_SecurityException ex);

typedef DDS_Security_PermissionsHandle
(*DDS_Security_AccessControl_validate_remote_permissions)
        (void *listener_data,
         _In_ DDS_Security_Authentication auth_plugin,
         _In_ DDS_Security_IdentityHandle local_identity_handle,
         _In_ DDS_Security_IdentityHandle remote_identity_handle,
         _In_ DDS_Security_PermissionsToken remote_permissions_token,
         _In_ DDS_Security_AuthenticatedPeerCredentialToken remote_credential_token,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_check_create_participant)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ DDS_DomainId_t domain_id,
         _In_ DDS_Security_DomainParticipantQos qos,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_check_create_datawriter)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ DDS_DomainId_t domain_id,
         _In_ char *topic_name,
         _In_ DDS_Security_DataWriterQos qos,
         _In_ DDS_PartitionQosPolicy partition,
         _In_ DDS_Security_DataTags data_tag,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_check_create_datareader)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ DDS_DomainId_t domain_id,
         _In_ char *topic_name,
         _In_ DDS_Security_DataReaderQos qos,
         _In_ DDS_PartitionQosPolicy partition,
         _In_ DDS_Security_DataTags data_tag,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-33
typedef bool
(*DDS_Security_AccessControl_check_create_topic)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ DDS_DomainId_t domain_id,
         _In_ char *topic_name,
         _In_ DDS_TopicQos qos,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_check_local_datawriter_register_instance)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ dds_entity_t writer,
         _In_ DDS_Security_DynamicData key,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_check_local_datawriter_dispose_instance)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ dds_entity_t writer,
         _In_ DDS_Security_DynamicData key,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_check_remote_participant)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ DDS_DomainId_t domain_id,
         _In_ DDS_Security_ParticipantBuiltinTopicDataSecure participant_data,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_check_remote_datawriter)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ DDS_DomainId_t domain_id,
         _In_ DDS_Security_PublicationBuiltinTopicDataSecure publication_data,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_check_remote_datareader)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ DDS_DomainId_t domain_id,
         _In_ DDS_Security_SubscriptionBuiltinTopicDataSecure subscription_data,
         _Inout_ bool relay_only,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_check_remote_topic)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ DDS_DomainId_t domain_id,
         _In_ DDS_TopicBuiltinTopicData topic_data,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-34
typedef bool
(*DDS_Security_AccessControl_check_local_datawriter_match)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle writer_permissions_handle,
         _In_ DDS_Security_PermissionsHandle reader_permissions_handle,
         _In_ DDS_Security_PublicationBuiltinTopicDataSecure publication_data,
         _In_ DDS_Security_SubscriptionBuiltinTopicDataSecure subscription_data,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-34
typedef bool
(*DDS_Security_AccessControl_check_local_datareader_match)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle reader_permissions_handle,
         _In_ DDS_Security_PermissionsHandle writer_permissions_handle,
         _In_ DDS_Security_SubscriptionBuiltinTopicDataSecure subscription_data,
         _In_ DDS_Security_PublicationBuiltinTopicDataSecure publication_data,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_check_remote_datawriter_register_instance)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ dds_entity_t reader,
         _In_ DDS_InstanceHandle_t publication_handle,
         _In_ DDS_Security_DynamicData key,
         _In_ DDS_InstanceHandle_t instance_handle,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_check_remote_datawriter_dispose_instance)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ dds_entity_t reader,
         _In_ DDS_InstanceHandle_t publication_handle,
         _In_ DDS_Security_DynamicData key,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_get_permissions_token)
        (void *listener_data,
         _Inout_ DDS_Security_PermissionsToken permissions_token,
         _In_ DDS_Security_PermissionsHandle handle,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_get_permissions_credential_token)
        (void *listener_data,
         _Inout_ DDS_Security_PermissionsCredentialToken permissions_credential_token,
         _In_ DDS_Security_PermissionsHandle handle,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_set_listener)
        (void *listener_data,
         _In_ DDS_Security_AccessControlListener listener,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_return_permissions_token)
        (void *listener_data,
         _In_ DDS_Security_PermissionsToken token,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_return_permissions_credential_token)
        (void *listener_data,
         _In_ DDS_Security_PermissionsCredentialToken permissions_credential_token,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_AccessControl_get_participant_sec_attributes)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _Inout_ DDS_Security_ParticipantSecurityAttributes attributes,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-16
typedef bool
(*DDS_Security_AccessControl_get_topic_sec_attributes)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ char *topic_name,
         _Inout_ DDS_Security_TopicSecurityAttributes attributes,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-16
typedef bool
(*DDS_Security_AccessControl_get_datawriter_sec_attributes)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ DDS_PartitionQosPolicy partition,
         _In_ DDS_Security_DataTagQosPolicy data_tag,
         _Inout_ DDS_Security_EndpointSecurityAttributes attributes,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-16
typedef bool
(*DDS_Security_AccessControl_get_datareader_sec_attributes)
        (void *listener_data,
         _In_ DDS_Security_PermissionsHandle permissions_handle,
         _In_ DDS_PartitionQosPolicy partition,
         _In_ DDS_Security_DataTagQosPolicy data_tag,
         _Inout_ DDS_Security_EndpointSecurityAttributes attributes,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-112
typedef bool
(*DDS_Security_AccessControl_return_participant_sec_attributes)
        (void *listener_data,
         _In_ DDS_Security_ParticipantSecurityAttributes attributes,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-112
typedef bool
(*DDS_Security_AccessControl_return_datawriter_sec_attributes)
        (void *listener_data,
         _In_ DDS_Security_EndpointSecurityAttributes attributes,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-112
typedef bool
(*DDS_Security_AccessControl_return_datareader_sec_attributes)
        (void *listener_data,
         _In_ DDS_Security_EndpointSecurityAttributes attributes,
         _Inout_ DDS_Security_SecurityException ex);


// DDSSEC11-96
struct DDS_Security_AccessControl
{
  DDS_Security_AccessControl_validate_local_permissions validate_local_permissions;

  DDS_Security_AccessControl_validate_remote_permissions validate_remote_permissions;

  DDS_Security_AccessControl_check_create_participant check_create_participant;

  DDS_Security_AccessControl_check_create_datawriter check_create_datawriter;

  DDS_Security_AccessControl_check_create_datareader check_create_datareader;

  DDS_Security_AccessControl_check_create_topic check_create_topic;

  DDS_Security_AccessControl_check_local_datawriter_register_instance check_local_datawriter_register_instance;

  DDS_Security_AccessControl_check_local_datawriter_dispose_instance check_local_datawriter_dispose_instance;

  DDS_Security_AccessControl_check_remote_participant check_remote_participant;

  DDS_Security_AccessControl_check_remote_datawriter check_remote_datawriter;

  DDS_Security_AccessControl_check_remote_datareader check_remote_datareader;

  DDS_Security_AccessControl_check_remote_topic check_remote_topic;

  DDS_Security_AccessControl_check_local_datawriter_match check_local_datawriter_match;

  DDS_Security_AccessControl_check_local_datareader_match check_local_datareader_match;

  DDS_Security_AccessControl_check_remote_datawriter_register_instance check_remote_datawriter_register_instance;

  DDS_Security_AccessControl_check_remote_datawriter_dispose_instance check_remote_datawriter_dispose_instance;

  DDS_Security_AccessControl_get_permissions_token get_permissions_token;

  DDS_Security_AccessControl_get_permissions_credential_token get_permissions_credential_token;

  DDS_Security_AccessControl_set_listener set_listener;

  DDS_Security_AccessControl_return_permissions_token return_permissions_token;

  DDS_Security_AccessControl_return_permissions_credential_token return_permissions_credential_token;

  DDS_Security_AccessControl_get_participant_sec_attributes get_participant_sec_attributes;

  DDS_Security_AccessControl_get_topic_sec_attributes get_topic_sec_attributes;

  DDS_Security_AccessControl_get_datawriter_sec_attributes get_datawriter_sec_attributes;

  DDS_Security_AccessControl_get_datareader_sec_attributes get_datareader_sec_attributes;

  DDS_Security_AccessControl_return_participant_sec_attributes return_participant_sec_attributes;

  DDS_Security_AccessControl_return_datawriter_sec_attributes return_datawriter_sec_attributes;

  DDS_Security_AccessControl_return_datareader_sec_attributes return_datareader_sec_attributes;

} ;


/*
 *
 */
typedef DDS_Security_ParticipantCryptoHandle
(*DDS_Security_CryptoKeyFactory_register_local_participant)
        (void *listener_data,
         _In_ DDS_Security_IdentityHandle participant_identity,
         _In_ DDS_Security_PermissionsHandle participant_permissions,
         _In_ DDS_Security_PropertySeq participant_properties,
         _In_ DDS_Security_ParticipantSecurityAttributes participant_security_attributes,
         _Inout_ DDS_Security_SecurityException ex);

typedef DDS_Security_ParticipantCryptoHandle
(*DDS_Security_CryptoKeyFactory_register_matched_remote_participant)
        (void *listener_data,
         _In_ DDS_Security_ParticipantCryptoHandle local_participant_crypto_handle,
         _In_ DDS_Security_IdentityHandle remote_participant_identity,
         _In_ DDS_Security_PermissionsHandle remote_participant_permissions,
         _In_ DDS_Security_SharedSecretHandle shared_secret,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-3 DDSSEC11-85
typedef DDS_Security_DatawriterCryptoHandle
(*DDS_Security_CryptoKeyFactory_register_local_datawriter)
        (void *listener_data,
         _In_ DDS_Security_ParticipantCryptoHandle participant_crypto,
         _In_ DDS_Security_PropertySeq datawriter_properties,
         _In_ DDS_Security_EndpointSecurityAttributes datawriter_security_attributes,
         _Inout_ DDS_Security_SecurityException ex);

typedef DDS_Security_DatareaderCryptoHandle
(*DDS_Security_CryptoKeyFactory_register_matched_remote_datareader)
        (void *listener_data,
         _In_ DDS_Security_DatawriterCryptoHandle local_datawritert_crypto_handle,
         _In_ DDS_Security_ParticipantCryptoHandle remote_participant_crypto,
         _In_ DDS_Security_SharedSecretHandle shared_secret,
         _In_ bool relay_only,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-3 DDSSEC11-85
typedef DDS_Security_DatareaderCryptoHandle
(*DDS_Security_CryptoKeyFactory_register_local_datareader)
        (void *listener_data,
         _In_ DDS_Security_ParticipantCryptoHandle participant_crypto,
         _In_ DDS_Security_PropertySeq datareader_properties,
         _In_ DDS_Security_EndpointSecurityAttributes datareader_security_attributes,
         _Inout_ DDS_Security_SecurityException ex);

typedef DDS_Security_DatawriterCryptoHandle
(*DDS_Security_CryptoKeyFactory_register_matched_remote_datawriter)
        (void *listener_data,
         _In_ DDS_Security_DatareaderCryptoHandle local_datareader_crypto_handle,
         _In_ DDS_Security_ParticipantCryptoHandle remote_participant_crypt,
         _In_ DDS_Security_SharedSecretHandle shared_secret,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoKeyFactory_unregister_participant)
        (void *listener_data,
         _In_ DDS_Security_ParticipantCryptoHandle participant_crypto_handle,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoKeyFactory_unregister_datawriter)
        (void *listener_data,
         _In_ DDS_Security_DatawriterCryptoHandle datawriter_crypto_handle,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoKeyFactory_unregister_datareader)
        (void *listener_data,
         _In_ DDS_Security_DatareaderCryptoHandle datareader_crypto_handle,
         _Inout_ DDS_Security_SecurityException ex);

typedef struct CryptoKeyFactory
{

  DDS_Security_CryptoKeyFactory_register_local_participant register_local_participant;

  DDS_Security_CryptoKeyFactory_register_matched_remote_participant register_matched_remote_participant;

  DDS_Security_CryptoKeyFactory_register_local_datawriter register_local_datawriter;

  DDS_Security_CryptoKeyFactory_register_matched_remote_datareader register_matched_remote_datareader;

  DDS_Security_CryptoKeyFactory_register_local_datareader register_local_datareader;

  DDS_Security_CryptoKeyFactory_register_matched_remote_datawriter register_matched_remote_datawriter;

  DDS_Security_CryptoKeyFactory_unregister_participant unregister_participant;

  DDS_Security_CryptoKeyFactory_unregister_datawriter unregister_datawriter;

  DDS_Security_CryptoKeyFactory_unregister_datareader unregister_datareader;
} CryptoKeyFactory;


/**
 * CryptoKeyExchange Interface
 */
typedef bool
(*DDS_Security_CryptoKeyExchange_create_local_participant_crypto_tokens)
        (void *listener_data,
         _Inout_ DDS_Security_ParticipantCryptoTokenSeq local_participant_crypto_tokens,
         _In_ DDS_Security_ParticipantCryptoHandle local_participant_crypto,
         _In_ DDS_Security_ParticipantCryptoHandle remote_participant_crypto,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoKeyExchange_set_remote_participant_crypto_tokens)
        (void *listener_data,
         _In_ DDS_Security_ParticipantCryptoHandle local_participant_crypto,
         _In_ DDS_Security_ParticipantCryptoHandle remote_participant_crypto,
         _In_ DDS_Security_ParticipantCryptoTokenSeq remote_participant_tokens,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoKeyExchange_create_local_datawriter_crypto_tokens)
        (void *listener_data,
         _Inout_ DDS_Security_DatawriterCryptoTokenSeq local_datawriter_crypto_tokens,
         _In_ DDS_Security_DatawriterCryptoHandle local_datawriter_crypto,
         _In_ DDS_Security_DatareaderCryptoHandle remote_datareader_crypto,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoKeyExchange_set_remote_datawriter_crypto_tokens)
        (void *listener_data,
         _In_ DDS_Security_DatareaderCryptoHandle local_datareader_crypto,
         _In_ DDS_Security_DatawriterCryptoHandle remote_datawriter_crypto,
         _In_ DDS_Security_DatawriterCryptoTokenSeq remote_datawriter_tokens,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoKeyExchange_create_local_datareader_crypto_tokens)
        (void *listener_data,
         _Inout_ DDS_Security_DatareaderCryptoTokenSeq local_datareader_cryto_tokens,
         _In_ DDS_Security_DatareaderCryptoHandle local_datareader_crypto,
         _In_ DDS_Security_DatawriterCryptoHandle remote_datawriter_crypto,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoKeyExchange_set_remote_datareader_crypto_tokens)
        (void *listener_data,
         _In_ DDS_Security_DatawriterCryptoHandle local_datawriter_crypto,
         _In_ DDS_Security_DatareaderCryptoHandle remote_datareader_crypto,
         _In_ DDS_Security_DatareaderCryptoTokenSeq remote_datareader_tokens,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoKeyExchange_return_crypto_tokens)
        (void *listener_data,
         _In_ DDS_Security_CryptoTokenSeq crypto_tokens,
         _Inout_ DDS_Security_SecurityException ex);

typedef struct CryptoKeyExchange
{
  DDS_Security_CryptoKeyExchange_create_local_participant_crypto_tokens create_local_participant_crypto_tokens;

  DDS_Security_CryptoKeyExchange_set_remote_participant_crypto_tokens set_remote_participant_crypto_tokens;

  DDS_Security_CryptoKeyExchange_create_local_datawriter_crypto_tokens create_local_datawriter_crypto_tokens;

  DDS_Security_CryptoKeyExchange_set_remote_datawriter_crypto_tokens set_remote_datawriter_crypto_tokens;

  DDS_Security_CryptoKeyExchange_create_local_datareader_crypto_tokens create_local_datareader_crypto_tokens;

  DDS_Security_CryptoKeyExchange_set_remote_datareader_crypto_tokens set_remote_datareader_crypto_tokens;

  DDS_Security_CryptoKeyExchange_return_crypto_tokens return_crypto_tokens;
} CryptoKeyExchange;



/**
 * CryptoTransform Interface
 */

typedef bool
(*DDS_Security_CryptoTransform_encode_serialized_payload)
        (void *listener_data,
         _Inout_ DDS_OctetSeq encoded_buffer,
         _Inout_ DDS_OctetSeq extra_inline_qos,
         _In_ DDS_OctetSeq plain_buffer,
         _In_ DDS_Security_DatawriterCryptoHandle sending_datawriter_crypto,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoTransform_encode_datawriter_submessage)
        (void *listener_data,
         _Inout_ DDS_OctetSeq encoded_rtps_submessage,
         _In_ DDS_OctetSeq plain_rtps_submessage,
         _In_ DDS_Security_DatawriterCryptoHandle sending_datawriter_crypto,
         _In_ DDS_Security_DatareaderCryptoHandleSeq receiving_datareader_crypto_list,
         _Inout_ int32_t receiving_datareader_crypto_list_index,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoTransform_encode_datareader_submessage)
        (void *listener_data,
         _Inout_ DDS_OctetSeq encoded_rtps_submessage,
         _In_ DDS_OctetSeq plain_rtps_submessage,
         _In_ DDS_Security_DatareaderCryptoHandle sending_datareader_crypto,
         _In_ DDS_Security_DatawriterCryptoHandleSeq receiving_datawriter_crypto_list,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-66
typedef bool
(*DDS_Security_CryptoTransform_encode_rtps_message)
        (void *listener_data,
         _Inout_ DDS_OctetSeq encoded_rtps_message,
         _In_ DDS_OctetSeq plain_rtps_message,
         _In_ DDS_Security_ParticipantCryptoHandle sending_participant_crypto,
         _In_ DDS_Security_ParticipantCryptoHandleSeq receiving_participant_crypto_list,
         _Inout_ int32_t receiving_participant_crypto_list_index,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoTransform_decode_rtps_message)
        (void *listener_data,
         _Inout_ DDS_OctetSeq plain_buffer,
         _In_ DDS_OctetSeq encoded_buffer,
         _In_ DDS_Security_ParticipantCryptoHandle receiving_participant_crypto,
         _In_ DDS_Security_ParticipantCryptoHandle sending_participant_crypto,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoTransform_preprocess_secure_submsg)
        (void *listener_data,
         _Inout_ DDS_Security_DatawriterCryptoHandle datawriter_crypto,
         _Inout_ DDS_Security_DatareaderCryptoHandle datareader_crypto,
         _Inout_ DDS_Security_SecureSumessageCategory_t secure_submessage_category,
         _In_ DDS_OctetSeq encoded_rtps_submessage,
         _In_ DDS_Security_ParticipantCryptoHandle receiving_participant_crypto,
         _In_ DDS_Security_ParticipantCryptoHandle sending_participant_crypto,
         _Inout_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoTransform_decode_datawriter_submessage)
        (void *listener_data,
         _Inout_ DDS_OctetSeq plain_rtps_submessage,
         _In_ DDS_OctetSeq encoded_rtps_submessage,
         _In_ DDS_Security_DatareaderCryptoHandle receiving_datareader_crypto,
         _In_ DDS_Security_DatawriterCryptoHandle sending_datawriter_crypto,
         _In_ DDS_Security_SecurityException ex);

typedef bool
(*DDS_Security_CryptoTransform_decode_datareader_submessage)
        (void *listener_data,
         _Inout_ DDS_OctetSeq plain_rtps_message,
         _In_ DDS_OctetSeq encoded_rtps_message,
         _In_ DDS_Security_DatawriterCryptoHandle receiving_datawriter_crypto,
         _In_ DDS_Security_DatareaderCryptoHandle sending_datareader_crypto,
         _Inout_ DDS_Security_SecurityException ex);

// DDSSEC11-123
typedef bool
(*DDS_Security_CryptoTransform_decode_serialized_payload)
        (void *listener_data,
         _Inout_ DDS_OctetSeq plain_buffer,
         _In_ DDS_OctetSeq encoded_buffer,
         _In_ DDS_OctetSeq inline_qos,
         _In_ DDS_Security_DatareaderCryptoHandle receiving_datareader_crypto,
         _In_ DDS_Security_DatawriterCryptoHandle sending_datawriter_crypto,
         _Inout_ DDS_Security_SecurityException ex);


typedef struct CryptoTransform
{
  DDS_Security_CryptoTransform_encode_serialized_payload encode_serialized_payload;

  DDS_Security_CryptoTransform_encode_datawriter_submessage encode_datawriter_submessage;

  DDS_Security_CryptoTransform_encode_datareader_submessage encode_datareader_submessage;

  DDS_Security_CryptoTransform_encode_rtps_message encode_rtps_message;

  DDS_Security_CryptoTransform_decode_rtps_message decode_rtps_message;

  DDS_Security_CryptoTransform_preprocess_secure_submsg preprocess_secure_submsg;

  DDS_Security_CryptoTransform_decode_datawriter_submessage decode_datawriter_submessage;

  DDS_Security_CryptoTransform_decode_datareader_submessage decode_datareader_submessage;

  DDS_Security_CryptoTransform_decode_serialized_payload decode_serialized_payload;
} CryptoTransform;


#endif //DDSC_SECURITY_H
