//
// Created by kurtulus on 29-11-17.
//
//#include "os/os_public.h"
//#include "os/os_decl_attributes_sal.h"

#include "dds_builtinTopics.h"

#ifndef DDSC_SECURITY_H
#define DDSC_SECURITY_H

int64_t DDS_Security_DynamicData; //native=int64_t assumption
//Note – It is recommended that native types be mapped to equivalent type
// names in each programming language, subject to the normal mapping rules for type names in that language

/*
 * NOTE: ReturnCode_t is from DDS spec
 */
typedef long DDS_ReturnCode_t;
typedef long DDS_Security_DomainId_t;

#ifndef _DDS_Security_OctetSeq_defined
#define _DDS_Security_OctetSeq_defined
typedef struct
{
  uint32_t _maximum;
  uint32_t _length;
  uint8_t *_buffer; //octet : uint8_t
  bool _release;
} DDS_Security_OctetSeq;

DDS_Security_OctetSeq *DDS_Security_OctetSeq__alloc(void);

uint8_t *DDS_Security_OctetSeq__allocbuf(uint32_t len);

#endif /* _DDS_Security_OctetSeq_defined */


#ifndef _DDS_Security_LongLongSeq_defined
#define _DDS_Security_LongLongSeq_defined
typedef struct
{
  uint32_t _maximum;
  uint32_t _length;
  int64_t *_buffer;
  bool _release;
} DDS_Security_LongLongSeq;

DDS_Security_LongLongSeq *DDS_Security_LongLongSeq__alloc(void);

int64_t *DDS_Security_LongLongSeq__allocbuf(uint32_t len);

#endif /* _DDS_Security_LongLongSeq_defined */

typedef struct DDS_Security_Property_t
{
  char *name;
  char *value;
  bool propagate;
} DDS_Security_Property_t;

#ifndef _DDS_Security_PropertySeq_defined
typedef struct
{
  uint32_t _maximum;
  uint32_t _length;
  DDS_Security_Property_t *_buffer;
  bool _release;
} DDS_Security_PropertySeq;

DDS_Security_PropertySeq *DDS_Security_PropertySeq__alloc(void);

DDS_Security_Property_t *DDS_Security_PropertySeq__allocbuf(uint32_t len);

#endif /* _DDS_Security_PropertySeq_defined */


typedef struct DDS_Security_BinaryProperty_t
{
  char *name;
  DDS_Security_OctetSeq value;
  bool propagate;
} DDS_Security_BinaryProperty_t;

#ifndef _DDS_Security_BinaryPropertySeq_defined
#define _DDS_Security_BinaryPropertySeq_defined
typedef struct
{
  uint32_t _maximum;
  uint32_t _length;
  DDS_Security_BinaryProperty_t *_buffer;
  bool _release;
} DDS_Security_BinaryPropertySeq;

DDS_Security_BinaryPropertySeq *DDS_Security_BinaryPropertySeq__alloc(void);

DDS_Security_BinaryProperty_t *DDS_Security_BinaryPropertySeq__allocbuf(uint32_t len);

#endif /* _DDS_Security_BinaryPropertySeq_defined */

// DDSSEC11-96
typedef struct DDS_Security_DataHolder
{
  char *class_id;
  DDS_Security_PropertySeq properties;
  DDS_Security_BinaryPropertySeq binary_properties;
} DDS_Security_DataHolder;
#ifndef _DDS_Security_DataHolderSeq_defined
#define _DDS_Security_DataHolderSeq_defined
typedef struct
{
  uint32_t _maximum;
  uint32_t _length;
  DDS_Security_DataHolder *_buffer;
  bool _release;
} DDS_Security_DataHolderSeq;

DDS_Security_DataHolderSeq *DDS_Security_DataHolderSeq__alloc(void);

DDS_Security_DataHolder *DDS_Security_DataHolderSeq__allocbuf(uint32_t len);

#endif /* _DDS_Security_DataHolderSeq_defined */

typedef DDS_Security_DataHolder DDS_Security_Token;

// DDSSEC11-43
typedef DDS_Security_Token DDS_Security_MessageToken;
typedef DDS_Security_MessageToken DDS_Security_AuthRequestMessageToken;
typedef DDS_Security_MessageToken DDS_Security_HandshakeMessageToken;

// DDSSEC11-82
typedef DDS_Security_Token DDS_Security_IdentityStatusToken;
#ifndef _DDS_Security_HandshakeMessageTokenSeq_defined
#define _DDS_Security_HandshakeMessageTokenSeq_defined
typedef struct
{
  uint32_t _maximum;
  uint32_t _length;
  DDS_Security_HandshakeMessageToken *_buffer;
  bool _release;
} DDS_Security_HandshakeMessageTokenSeq;

DDS_Security_HandshakeMessageTokenSeq *DDS_Security_HandshakeMessageTokenSeq__alloc(void);

DDS_Security_HandshakeMessageToken *DDS_Security_HandshakeMessageTokenSeq__allocbuf(uint32_t len);

#endif /* _DDS_Security_HandshakeMessageTokenSeq_defined */

typedef DDS_Security_Token DDS_Security_IdentityToken;
typedef DDS_Security_Token DDS_Security_PermissionsToken;
typedef DDS_Security_Token DDS_Security_AuthenticatedPeerCredentialToken;
typedef DDS_Security_Token DDS_Security_PermissionsCredentialToken;

typedef DDS_Security_Token DDS_Security_CryptoToken;
#ifndef _DDS_Security_CryptoTokenSeq_defined
#define _DDS_Security_CryptoTokenSeq_defined
typedef struct
{
  uint32_t _maximum;
  uint32_t _length;
  DDS_Security_CryptoToken *_buffer;
  bool _release;
} DDS_Security_CryptoTokenSeq;

DDS_Security_CryptoTokenSeq *DDS_Security_CryptoTokenSeq__alloc(void);

DDS_Security_CryptoToken *DDS_Security_CryptoTokenSeq__allocbuf(uint32_t len);

#endif /* _DDS_Security_CryptoTokenSeq_defined */

typedef DDS_Security_Token DDS_Security_ParticipantCryptoToken;
typedef DDS_Security_Token DDS_Security_DatawriterCryptoToken;
typedef DDS_Security_Token DDS_Security_DatareaderCryptoToken;

typedef DDS_Security_CryptoTokenSeq DDS_Security_ParticipantCryptoTokenSeq;
typedef DDS_Security_CryptoTokenSeq DDS_Security_DatawriterCryptoTokenSeq;
typedef DDS_Security_CryptoTokenSeq DDS_Security_DatareaderCryptoTokenSeq;

// DDSSEC11-88
// From DDS-RTPS [2] clauses 8.4.2.1 and 9.3.1
typedef uint8_t DDS_Security_GuidPrefix_t[12]; //octet:int8
typedef struct DDS_Security_EntityId_t
{
  uint8_t entityKey[3];  //octet:uint8_t
  uint8_t entityKind;    //octet:uint8_t
} DDS_Security_EntityId_t;

// DDSSEC11-88
typedef struct DDS_Security_GUID_t
{
  DDS_Security_GuidPrefix_t prefix;
  DDS_Security_EntityId_t entityId;
} DDS_Security_GUID_t;

// DDSSEC11-88
typedef struct DDS_Security_MessageIdentity
{
  DDS_Security_GUID_t source_guid;
  long long sequence_number;
} DDS_Security_MessageIdentity;

// DDSSEC11-24
const DDS_ReturnCode_t RETCODE_NOT_ALLOWED_BY_SECURITY = 1000;

// DDSSEC11-96

typedef struct DDS_Security_PropertyQosPolicy
{
  DDS_Security_PropertySeq value;
  DDS_Security_BinaryPropertySeq binary_value;
} DDS_Security_PropertyQosPolicy;

// DDSSEC11-96
typedef struct DDS_Security_DomainParticipantQos
{
  DDS_UserDataQosPolicy user_data;
  DDS_EntityFactoryQosPolicy entity_factory;
  //DDS_SchedulingQosPolicy watchdog_scheduling;
  //DDS_SchedulingQosPolicy listener_scheduling;
  DDS_Security_PropertyQosPolicy property;
} DDS_Security_DomainParticipantQos;

typedef struct DDS_Security_Tag
{
  char *name;
  char *value;
} DDS_Security_Tag;

#ifndef _DDS_Security_TagSeq_defined
#define _DDS_Security_TagSeq_defined
typedef struct
{
  uint32_t _maximum;
  uint32_t _length;
  DDS_Security_Tag *_buffer;
  bool _release;
} DDS_Security_TagSeq;

DDS_Security_TagSeq *DDS_Security_TagSeq__alloc(void);

DDS_Security_Tag *DDS_Security_TagSeq__allocbuf(uint32_t len);

#endif /* _DDS_Security_TagSeq_defined */
typedef struct DDS_Security_DataTags
{
  DDS_Security_TagSeq DDS_Security_tags;
} DDS_Security_DataTags;

// DDSSEC11-34
typedef DDS_Security_DataTags DDS_Security_DataTagQosPolicy;

// DDSSEC11-96
// See http://www.omg.org/spec/DDS-XTypes/20170301/dds-xtypes_discovery.idl
// DDSSEC11-34

typedef struct DDS_Security_DataWriterQos
{
  DDS_DurabilityQosPolicy durability;
  DDS_DeadlineQosPolicy deadline;
  DDS_LatencyBudgetQosPolicy latency_budget;
  DDS_LivelinessQosPolicy liveliness;
  DDS_ReliabilityQosPolicy reliability;
  DDS_DestinationOrderQosPolicy destination_order;
  DDS_HistoryQosPolicy history;
  DDS_ResourceLimitsQosPolicy resource_limits;
  DDS_TransportPriorityQosPolicy transport_priority;
  DDS_LifespanQosPolicy lifespan;
  DDS_UserDataQosPolicy user_data;
  DDS_OwnershipQosPolicy ownership;
  DDS_OwnershipStrengthQosPolicy ownership_strength;
  DDS_WriterDataLifecycleQosPolicy writer_data_lifecycle;
  DDS_Security_PropertyQosPolicy property;
  DDS_Security_DataTagQosPolicy data_tags;
} DDS_Security_DataWriterQos;

// DDSSEC11-96
// See http://www.omg.org/spec/DDS-XTypes/20170301/dds-xtypes_discovery.idl
// DDSSEC11-34
typedef struct DDS_Security_DataReaderQos
{
  DDS_DurabilityQosPolicy durability;
  DDS_DeadlineQosPolicy deadline;
  DDS_LatencyBudgetQosPolicy latency_budget;
  DDS_LivelinessQosPolicy liveliness;
  DDS_ReliabilityQosPolicy reliability;
  DDS_DestinationOrderQosPolicy destination_order;
  DDS_HistoryQosPolicy history;
  DDS_ResourceLimitsQosPolicy resource_limits;
  DDS_UserDataQosPolicy user_data;
  DDS_OwnershipQosPolicy ownership;
  DDS_TimeBasedFilterQosPolicy time_based_filter;
  DDS_ReaderDataLifecycleQosPolicy reader_data_lifecycle;
  DDS_SubscriptionKeyQosPolicy subscription_keys;
  DDS_ReaderLifespanQosPolicy reader_lifespan;
  DDS_ShareQosPolicy share;
  DDS_Security_PropertyQosPolicy property;
  DDS_Security_DataTagQosPolicy data_tags;
} DDS_Security_DataReaderQos;

// DDSSEC11-137
typedef unsigned long DDS_Security_ParticipantSecurityAttributesMask;
typedef unsigned long DDS_Security_PluginParticipantSecurityAttributesMask;

typedef struct DDS_Security_ParticipantSecurityInfo
{
  DDS_Security_ParticipantSecurityAttributesMask participant_security_attributes;
  DDS_Security_PluginParticipantSecurityAttributesMask plugin_participant_security_attributes;
} DDS_Security_ParticipantSecurityInfo;

// DDSSEC11-106
typedef unsigned long DDS_Security_EndpointSecurityAttributesMask;
typedef unsigned long DDS_Security_PluginEndpointSecurityAttributesMask;

typedef struct DDS_Security_EndpointSecurityInfo
{
  DDS_Security_EndpointSecurityAttributesMask endpoint_security_mask;
  DDS_Security_PluginEndpointSecurityAttributesMask plugin_endpoint_security_mask;
}DDS_Security_EndpointSecurityInfo;


typedef struct DDS_Security_ParticipantBuiltinTopicData
{
  DDS_BuiltinTopicKey_t key;
  DDS_UserDataQosPolicy user_data;
  DDS_Security_IdentityToken identity_token;
  DDS_Security_PermissionsToken permissions_token;
  DDS_Security_PropertyQosPolicy property;
  DDS_Security_ParticipantSecurityInfo security_info;
} DDS_Security_ParticipantBuiltinTopicData;

typedef struct DDS_Security_ParticipantBuiltinTopicDataSecure
{
  DDS_BuiltinTopicKey_t key;
  DDS_UserDataQosPolicy user_data;
  DDS_Security_IdentityToken identity_token;
  DDS_Security_PermissionsToken permissions_token;
  DDS_Security_PropertyQosPolicy property;
  DDS_Security_ParticipantSecurityInfo security_info;
  DDS_Security_IdentityStatusToken identity_status_token;
} DDS_Security_ParticipantBuiltinTopicDataSecure;

typedef struct DDS_Security_PublicationBuiltinTopicData
{
  DDS_BuiltinTopicKey_t key;
  DDS_BuiltinTopicKey_t participant_key;
  char *topic_name;
  char *type_name;
  DDS_DurabilityQosPolicy durability;
  DDS_DeadlineQosPolicy deadline;
  DDS_LatencyBudgetQosPolicy latency_budget;
  DDS_LivelinessQosPolicy liveliness;
  DDS_ReliabilityQosPolicy reliability;
  DDS_LifespanQosPolicy lifespan;
  DDS_DestinationOrderQosPolicy destination_order;
  DDS_UserDataQosPolicy user_data;
  DDS_OwnershipQosPolicy ownership;
  DDS_OwnershipStrengthQosPolicy ownership_strength;
  DDS_PresentationQosPolicy presentation;
  DDS_PartitionQosPolicy partition;
  DDS_TopicDataQosPolicy topic_data;
  DDS_GroupDataQosPolicy group_data;
  DDS_Security_EndpointSecurityInfo security_info;
} DDS_Security_PublicationBuiltinTopicData;

typedef struct DDS_Security_PublicationBuiltinTopicDataSecure
{
  DDS_BuiltinTopicKey_t key;
  DDS_BuiltinTopicKey_t participant_key;
  char *topic_name;
  char *type_name;
  DDS_DurabilityQosPolicy durability;
  DDS_DeadlineQosPolicy deadline;
  DDS_LatencyBudgetQosPolicy latency_budget;
  DDS_LivelinessQosPolicy liveliness;
  DDS_ReliabilityQosPolicy reliability;
  DDS_LifespanQosPolicy lifespan;
  DDS_DestinationOrderQosPolicy destination_order;
  DDS_UserDataQosPolicy user_data;
  DDS_OwnershipQosPolicy ownership;
  DDS_OwnershipStrengthQosPolicy ownership_strength;
  DDS_PresentationQosPolicy presentation;
  DDS_PartitionQosPolicy partition;
  DDS_TopicDataQosPolicy topic_data;
  DDS_GroupDataQosPolicy group_data;
  DDS_Security_EndpointSecurityInfo security_info;
  DDS_Security_DataTags data_tags;
} DDS_Security_PublicationBuiltinTopicDataSecure;

typedef struct DDS_Security_SubscriptionBuiltinTopicData
{
  DDS_BuiltinTopicKey_t key;
  DDS_BuiltinTopicKey_t participant_key;
  char *topic_name;
  char *type_name;
  DDS_DurabilityQosPolicy durability;
  DDS_DeadlineQosPolicy deadline;
  DDS_LatencyBudgetQosPolicy latency_budget;
  DDS_LivelinessQosPolicy liveliness;
  DDS_ReliabilityQosPolicy reliability;
  DDS_OwnershipQosPolicy ownership;
  DDS_DestinationOrderQosPolicy destination_order;
  DDS_UserDataQosPolicy user_data;
  DDS_TimeBasedFilterQosPolicy time_based_filter;
  DDS_PresentationQosPolicy presentation;
  DDS_PartitionQosPolicy partition;
  DDS_TopicDataQosPolicy topic_data;
  DDS_GroupDataQosPolicy group_data;
  DDS_Security_EndpointSecurityInfo security_info;
} DDS_Security_SubscriptionBuiltinTopicData;

typedef struct DDS_Security_SubscriptionBuiltinTopicDataSecure
{
  DDS_BuiltinTopicKey_t key;
  DDS_BuiltinTopicKey_t participant_key;
  char *topic_name;
  char *type_name;
  DDS_DurabilityQosPolicy durability;
  DDS_DeadlineQosPolicy deadline;
  DDS_LatencyBudgetQosPolicy latency_budget;
  DDS_LivelinessQosPolicy liveliness;
  DDS_ReliabilityQosPolicy reliability;
  DDS_OwnershipQosPolicy ownership;
  DDS_DestinationOrderQosPolicy destination_order;
  DDS_UserDataQosPolicy user_data;
  DDS_TimeBasedFilterQosPolicy time_based_filter;
  DDS_PresentationQosPolicy presentation;
  DDS_PartitionQosPolicy partition;
  DDS_TopicDataQosPolicy topic_data;
  DDS_GroupDataQosPolicy group_data;
  DDS_Security_EndpointSecurityInfo security_info;
  DDS_Security_DataTags data_tags;
} DDS_Security_SubscriptionBuiltinTopicDataSecure;

// DDSSEC11-15 typedef long SecurityExceptionCode;
typedef long DDS_Security_SecurityExceptionCode;

typedef struct DDS_Security_SecurityException
{
  char *message;
  long code;      // DDSSEC11-15
  long minor_code;
} DDS_Security_SecurityException;

typedef enum
{
  VALIDATION_OK,
  VALIDATION_FAILED,
  VALIDATION_PENDING_RETRY,
  VALIDATION_PENDING_HANDSHAKE_REQUEST,
  VALIDATION_PENDING_HANDSHAKE_MESSAGE,
  VALIDATION_OK_FINAL_MESSAGE
} DDS_Security_ValidationResult_t;

typedef int64_t DDS_Security_IdentityHandle;
typedef int64_t DDS_Security_HandshakeHandle;
typedef int64_t DDS_Security_SharedSecretHandle;
typedef int64_t DDS_Security_PermissionsHandle;
typedef int64_t DDS_Security_ParticipantCryptoHandle;
typedef int64_t DDS_Security_ParticipantCryptoHandleSeq;
typedef int64_t DDS_Security_DatawriterCryptoHandle;
typedef int64_t DDS_Security_DatawriterCryptoHandleSeq;
typedef int64_t DDS_Security_DatareaderCryptoHandle;
typedef int64_t DDS_Security_DatareaderCryptoHandleSeq;

// DDSSEC11-96
typedef struct DDS_Security_Authentication DDS_Security_Authentication;

// DDSSEC11-82
typedef enum
{
  IDENTITY_STATUS = 1
} DDS_Security_AuthStatusKind;

// DDSSEC11-96
/**
 * interface AuthenticationListener
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
struct DDS_Security_AuthenticationListener *DDS_Security_AuthenticationListener__alloc (void);



typedef DDS_Security_ValidationResult_t
(*DDS_Security_Authentication_ValidateLocalIdentity)
      (void *listener_data,
      _Inout_ DDS_Security_IdentityHandle local_identity_handle,
      _Inout_ DDS_Security_GUID_t *adjusted_participant_guid,
      _In_ DDS_Security_DomainId_t domain_id,
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
        _Inout_ DDS_Security_SecurityException ex );

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
        _Inout_ DDS_Security_SecurityException ex );

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
        _In_ DDS_Security_OctetSeq serialized_local_participant_data,
        _Inout_ DDS_Security_SecurityException ex );

// DDSSEC11-46
typedef DDS_Security_ValidationResult_t
(*DDS_Security_Authentication_begin_handshake_reply)
      (void *listener_data,
        _Inout_ DDS_Security_HandshakeHandle handshake_handle,
        _Inout_ DDS_Security_HandshakeMessageToken handshake_message_out,
        _In_ DDS_Security_HandshakeMessageToken handshake_message_in,
        _In_ DDS_Security_IdentityHandle initiator_identity_handle,
        _In_ DDS_Security_IdentityHandle replier_identity_handle,
        _In_ DDS_Security_OctetSeq serialized_local_participant_data,
        _Inout_ DDS_Security_SecurityException ex );

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
        _Inout_ DDS_Security_SecurityException ex );

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



  bool
  get_identity_token(
          _Inout_ DDS_Security_IdentityToken identity_token,
          _In_ DDS_Security_IdentityHandle handle,
          _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-82
  bool
  get_identity_status_token(
          _Inout_ DDS_Security_IdentityStatusToken identity_status_token,
          _In_ DDS_Security_IdentityHandle handle,
          _Inout_ DDS_Security_SecurityException ex);

  bool
          set_permissions_credential_and_token(
  _In_ DDS_Security_IdentityHandle handle,
  _In_ DDS_Security_PermissionsCredentialToken permissions_credential,
  _In_ DDS_Security_PermissionsToken permissions_token,
  _Inout_ DDS_Security_SecurityException ex );

  // DDSSEC11-21
  // DDSSEC11-88
  // DDSSEC11-85
  DDS_Security_ValidationResult_t
          validate_remote_identity(
  _Inout_ DDS_Security_IdentityHandle remote_identity_handle,
  _Inout_ DDS_Security_AuthRequestMessageToken local_auth_request_token,
  _In_ DDS_Security_AuthRequestMessageToken remote_auth_request_token,
  _In_ DDS_Security_IdentityHandle local_identity_handle,
  _In_ DDS_Security_IdentityToken remote_identity_token,
  _In_ DDS_Security_GUID_t remote_participant_guid,
  _Inout_ DDS_Security_SecurityException ex );

  // DDSSEC11-46
  // DDSSEC11-118
  DDS_Security_ValidationResult_t
          begin_handshake_request(
  _Inout_ DDS_Security_HandshakeHandle handshake_handle,
  _Inout_ DDS_Security_HandshakeMessageToken handshake_message,
  _In_ DDS_Security_HandshakeMessageToken handshake_message_in,
  _In_ DDS_Security_IdentityHandle initiator_identity_handle,
  _In_ DDS_Security_IdentityHandle replier_identity_handle,
  _In_ DDS_Security_OctetSeq serialized_local_participant_data,
  _Inout_ DDS_Security_SecurityException ex );

  // DDSSEC11-46
  DDS_Security_ValidationResult_t
          begin_handshake_reply(
  _Inout_ DDS_Security_HandshakeHandle handshake_handle,
  _Inout_ DDS_Security_HandshakeMessageToken handshake_message_out,
  _In_ DDS_Security_HandshakeMessageToken handshake_message_in,
  _In_ DDS_Security_IdentityHandle initiator_identity_handle,
  _In_ DDS_Security_IdentityHandle replier_identity_handle,
  _In_ DDS_Security_OctetSeq serialized_local_participant_data,
  _Inout_ DDS_Security_SecurityException ex );

  DDS_Security_ValidationResult_t
  process_handshake(
          _Inout_ DDS_Security_HandshakeMessageToken handshake_message_out,
          _In_ DDS_Security_HandshakeMessageToken handshake_message_in,
          _In_ DDS_Security_HandshakeHandle handshake_handle,
          _Inout_ DDS_Security_SecurityException ex);

  DDS_Security_SharedSecretHandle
          get_shared_secret(
  _In_ DDS_Security_HandshakeHandle handshake_handle,
  _Inout_ DDS_Security_SecurityException ex );

  bool
  get_authenticated_peer_credential_token(
          _Inout_ DDS_Security_AuthenticatedPeerCredentialToken peer_credential_token,
          _In_ DDS_Security_HandshakeHandle handshake_handle,
          _Inout_ DDS_Security_SecurityException ex);

  bool
  set_listener(
          _In_ DDS_Security_AuthenticationListener listener,
          _Inout_ DDS_Security_SecurityException ex);

  bool
  return_identity_token(
          _In_ DDS_Security_IdentityToken token,
          _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-82
  bool
  return_identity_status_token(
          _In_ DDS_Security_IdentityStatusToken token,
          _Inout_ DDS_Security_SecurityException ex);

  bool
  return_authenticated_peer_credential_token(
          _In_ DDS_Security_AuthenticatedPeerCredentialToken peer_credential_token,
          _Inout_ DDS_Security_SecurityException ex);

  bool
          return_handshake_handle(
  _In_ DDS_Security_HandshakeHandle handshake_handle,
  _Inout_ DDS_Security_SecurityException ex);

  bool
          return_identity_handle(
  _In_ DDS_Security_IdentityHandle identity_handle,
  _Inout_ DDS_Security_SecurityException ex);

  bool
          return_sharedsecret_handle(
  _In_ DDS_Security_SharedSecretHandle sharedsecret_handle,
  _Inout_ DDS_Security_SecurityException ex);
};

struct DDS_Security_Authentication *DDS_Security_Authentication__alloc (void);

// DDSSEC11-137 DDSEC11-85
typedef struct DDS_Security_ParticipantSecurityAttributes
{
  bool allow_unauthenticated_participants;
  bool is_access_protected;
  bool is_rtps_protected;
  bool is_discovery_protected;
  bool is_liveliness_protected;
  ParticipantSecurityAttributesMask plugin_participant_attributes;
  PropertySeq ac_endpoint_properties;
};

// DDSSEC11-16
typedef struct DDS_Security_TopicSecurityAttributes
{
  bool is_read_protected;
  bool is_write_protected;
  bool is_discovery_protected;
  bool is_liveliness_protected;
};

// DDSSEC11-16 DDSSEC11-106 DDSEC11-85
typedef struct DDS_Security_EndpointSecurityAttributes : TopicSecurityAttributes
{
  bool is_submessage_protected;
  bool is_payload_protected;
  bool is_key_protected;
  PluginEndpointSecurityAttributesMask plugin_endpoint_attributes;
  PropertySeq ac_endpoint_properties;
};

// DDSSEC11-106
typedef struct DDS_Security_PluginEndpointSecurityAttributes
{
  bool is_submessage_encrypted;
  bool is_payload_encrypted;
  bool is_submessage_origin_authenticated;
};

// DDSSEC11-96
interface AccessControl;


// DDSSEC11-96
interface AccessControlListener
{
  bool on_revoke_permissions(
          _In_ DDS_Security_AccessControl plugin,
          _In_ DDS_Security_PermissionsHandle handle);
};

// DDSSEC11-96
interface AccessControl
{
  PermissionsHandle
  validate_local_permissions(
          _In_ DDS_Security_Authentication auth_plugin,
          _In_ DDS_Security_IdentityHandle identity,
          _In_ DDS_Security_DomainId_t domain_id,
          _In_ DDS_Security_DomainParticipantQos participant_qos,
          _Inout_ DDS_Security_SecurityException ex);

  PermissionsHandle
  validate_remote_permissions(
          _In_ DDS_Security_Authentication auth_plugin,
          _In_ DDS_Security_IdentityHandle local_identity_handle,
          _In_ DDS_Security_IdentityHandle remote_identity_handle,
          _In_ DDS_Security_PermissionsToken remote_permissions_token,
          _In_ DDS_Security_AuthenticatedPeerCredentialToken remote_credential_token,
          _Inout_ DDS_Security_SecurityException ex);

  bool
          check_create_participant(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_DomainId_t domain_id,
  _In_ DDS_Security_DomainParticipantQos qos,
  _Inout_ DDS_Security_SecurityException ex );

  bool
          check_create_datawriter(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_DomainId_t domain_id,
  in char *topic_name,
  _In_ DDS_Security_DataWriterQos qos,
  _In_ DDS_Security_PartitionQosPolicy partition,
  _In_ DDS_Security_DataTags data_tag,
  _Inout_ DDS_Security_SecurityException ex);

  bool
          check_create_datareader(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_DomainId_t domain_id,
  _In_ DDS_Security_char *topic_name,
  _In_ DDS_Security_DataReaderQos qos,
  _In_ DDS_Security_PartitionQosPolicy partition,
  _In_ DDS_Security_DataTags data_tag,
  _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-33
  bool
          check_create_topic(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_DomainId_t domain_id,
  _In_ DDS_Security_char *topic_name,
  _In_ DDS_Security_TopicQos qos,
  _Inout_ DDS_Security_SecurityException ex);

  bool
          check_local_datawriter_register_instance(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_DataWriter writer,
  _In_ DDS_Security_DynamicData key,
  _Inout_ DDS_Security_SecurityException ex);

  bool
          check_local_datawriter_dispose_instance(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_DataWriter writer,
  _In_ DDS_Security_DynamicData key,
  _Inout_ DDS_Security_SecurityException ex);

  bool
          check_remote_participant(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_DomainId_t domain_id,
  _In_ DDS_Security_ParticipantBuiltinTopicDataSecure participant_data,
  _Inout_ DDS_Security_SecurityException ex);

  bool
          check_remote_datawriter(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_DomainId_t domain_id,
  _In_ DDS_Security_PublicationBuiltinTopicDataSecure publication_data,
  _Inout_ DDS_Security_SecurityException ex);

  bool
          check_remote_datareader(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_DomainId_t domain_id,
  _In_ DDS_Security_SubscriptionBuiltinTopicDataSecure subscription_data,
  _Inout_ DDS_Security_bool relay_only,
  _Inout_ DDS_Security_SecurityException ex);

  bool
          check_remote_topic(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_DomainId_t domain_id,
  _In_ DDS_Security_TopicBuiltinTopicData topic_data,
  _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-34
  bool
          check_local_datawriter_match(
  _In_ DDS_Security_PermissionsHandle writer_permissions_handle,
  _In_ DDS_Security_PermissionsHandle reader_permissions_handle,
  _In_ DDS_Security_PublicationBuiltinTopicDataSecure publication_data,
  _In_ DDS_Security_SubscriptionBuiltinTopicDataSecure subscription_data,
  _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-34
  bool
          check_local_datareader_match(
  _In_ DDS_Security_PermissionsHandle reader_permissions_handle,
  _In_ DDS_Security_PermissionsHandle writer_permissions_handle,
  _In_ DDS_Security_SubscriptionBuiltinTopicDataSecure subscription_data,
  _In_ DDS_Security_PublicationBuiltinTopicDataSecure publication_data,
  _Inout_ DDS_Security_SecurityException ex);

  bool
          check_remote_datawriter_register_instance(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_DataReader reader,
  _In_ DDS_Security_InstanceHandle_t publication_handle,
  _In_ DDS_Security_DynamicData key,
  _In_ DDS_Security_InstanceHandle_t instance_handle,
  _Inout_ DDS_Security_SecurityException ex);

  bool
          check_remote_datawriter_dispose_instance(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_DataReader reader,
  _In_ DDS_Security_InstanceHandle_t publication_handle,
  _In_ DDS_Security_DynamicData key,
  _Inout_ DDS_Security_SecurityException ex);

  bool
  get_permissions_token(
          _Inout_ DDS_Security_PermissionsToken permissions_token,
          _In_ DDS_Security_PermissionsHandle handle,
          _Inout_ DDS_Security_SecurityException ex);

  bool
  get_permissions_credential_token(
          _Inout_ DDS_Security_PermissionsCredentialToken permissions_credential_token,
          _In_ DDS_Security_PermissionsHandle handle,
          _Inout_ DDS_Security_SecurityException ex);

  bool
  set_listener(
          _In_ DDS_Security_AccessControlListener listener,
          _Inout_ DDS_Security_SecurityException ex);

  bool
  return_permissions_token(
          _In_ DDS_Security_PermissionsToken token,
          _Inout_ DDS_Security_SecurityException ex);

  bool
  return_permissions_credential_token(
          _In_ DDS_Security_PermissionsCredentialToken permissions_credential_token,
          _Inout_ DDS_Security_SecurityException ex);

  bool
          get_participant_sec_attributes(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _Inout_ DDS_Security_ParticipantSecurityAttributes attributes,
  _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-16
  bool
          get_topic_sec_attributes(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_char *topic_name,
  _Inout_ DDS_Security_TopicSecurityAttributes attributes,
  _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-16
  bool
          get_datawriter_sec_attributes(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_PartitionQosPolicy partition,
  _In_ DDS_Security_DataTagQosPolicy data_tag,
  _Inout_ DDS_Security_EndpointSecurityAttributes attributes,
  _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-16
  bool
          get_datareader_sec_attributes(
  _In_ DDS_Security_PermissionsHandle permissions_handle,
  _In_ DDS_Security_PartitionQosPolicy partition,
  _In_ DDS_Security_DataTagQosPolicy data_tag,
  _Inout_ DDS_Security_EndpointSecurityAttributes attributes,
  _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-112
  bool
  return_participant_sec_attributes(
          _In_ DDS_Security_ParticipantSecurityAttributes attributes,
          _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-112
  bool
  return_datawriter_sec_attributes(
          _In_ DDS_Security_EndpointSecurityAttributes attributes,
          _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-112
  bool
  return_datareader_sec_attributes(
          _In_ DDS_Security_EndpointSecurityAttributes attributes,
          _Inout_ DDS_Security_SecurityException ex);
};

// DDSSEC11-96
interface CryptoKeyFactory
{

  // DDSSEC11-3 DDSSEC11-85
  ParticipantCryptoHandle
          register_local_participant(
  _In_ DDS_Security_IdentityHandle participant_identity,
  _In_ DDS_Security_PermissionsHandle participant_permissions,
  _In_ DDS_Security_PropertySeq participant_properties,
  _In_ DDS_Security_ParticipantSecurityAttributes participant_security_attributes,
  _Inout_ DDS_Security_SecurityException ex  );

  ParticipantCryptoHandle
          register_matched_remote_participant(
  _In_ DDS_Security_ParticipantCryptoHandle local_participant_crypto_handle,
  _In_ DDS_Security_IdentityHandle remote_participant_identity,
  _In_ DDS_Security_PermissionsHandle remote_participant_permissions,
  _In_ DDS_Security_SharedSecretHandle shared_secret,
  _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-3 DDSSEC11-85
  DatawriterCryptoHandle
          register_local_datawriter(
  _In_ DDS_Security_ParticipantCryptoHandle participant_crypto,
  _In_ DDS_Security_PropertySeq datawriter_properties,
  _In_ DDS_Security_EndpointSecurityAttributes datawriter_security_attributes,
  _Inout_ DDS_Security_SecurityException ex);

  DatareaderCryptoHandle
          register_matched_remote_datareader(
  _In_ DDS_Security_DatawriterCryptoHandle local_datawritert_crypto_handle,
  _In_ DDS_Security_ParticipantCryptoHandle remote_participant_crypto,
  _In_ DDS_Security_SharedSecretHandle shared_secret,
  _In_ DDS_Security_bool relay_only,
  _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-3 DDSSEC11-85
  DatareaderCryptoHandle
          register_local_datareader(
  _In_ DDS_Security_ParticipantCryptoHandle participant_crypto,
  _In_ DDS_Security_PropertySeq datareader_properties,
  _In_ DDS_Security_EndpointSecurityAttributes datareader_security_attributes,
  _Inout_ DDS_Security_SecurityException ex);

  DatawriterCryptoHandle
          register_matched_remote_datawriter(
  _In_ DDS_Security_DatareaderCryptoHandle local_datareader_crypto_handle,
  _In_ DDS_Security_ParticipantCryptoHandle remote_participant_crypt,
  _In_ DDS_Security_SharedSecretHandle shared_secret,
  _Inout_ DDS_Security_SecurityException ex );

  bool
          unregister_participant(
  _In_ DDS_Security_ParticipantCryptoHandle participant_crypto_handle,
  _Inout_ DDS_Security_SecurityException ex);

  bool
          unregister_datawriter(
  _In_ DDS_Security_DatawriterCryptoHandle datawriter_crypto_handle,
  _Inout_ DDS_Security_SecurityException ex  );

  bool
          unregister_datareader(
  _In_ DDS_Security_DatareaderCryptoHandle datareader_crypto_handle,
  _Inout_ DDS_Security_SecurityException ex  );
};


// DDSSEC11-96
interface CryptoKeyExchange
{
  bool
  create_local_participant_crypto_tokens(
          _Inout_ DDS_Security_ParticipantCryptoTokenSeq local_participant_crypto_tokens,
          _In_ DDS_Security_ParticipantCryptoHandle local_participant_crypto,
          _In_ DDS_Security_ParticipantCryptoHandle remote_participant_crypto,
          _Inout_ DDS_Security_SecurityException ex);

  bool
          set_remote_participant_crypto_tokens(
  _In_ DDS_Security_ParticipantCryptoHandle local_participant_crypto,
  _In_ DDS_Security_ParticipantCryptoHandle remote_participant_crypto,
  _In_ DDS_Security_ParticipantCryptoTokenSeq remote_participant_tokens,
  _Inout_ DDS_Security_SecurityException ex);

  bool
  create_local_datawriter_crypto_tokens(
          _Inout_ DDS_Security_DatawriterCryptoTokenSeq local_datawriter_crypto_tokens,
          _In_ DDS_Security_DatawriterCryptoHandle local_datawriter_crypto,
          _In_ DDS_Security_DatareaderCryptoHandle remote_datareader_crypto,
          _Inout_ DDS_Security_SecurityException ex);

  bool
          set_remote_datawriter_crypto_tokens(
  _In_ DDS_Security_DatareaderCryptoHandle local_datareader_crypto,
  _In_ DDS_Security_DatawriterCryptoHandle remote_datawriter_crypto,
  _In_ DDS_Security_DatawriterCryptoTokenSeq remote_datawriter_tokens,
  _Inout_ DDS_Security_SecurityException ex);

  bool
  create_local_datareader_crypto_tokens(
          _Inout_ DDS_Security_DatareaderCryptoTokenSeq local_datareader_cryto_tokens,
          _In_ DDS_Security_DatareaderCryptoHandle local_datareader_crypto,
          _In_ DDS_Security_DatawriterCryptoHandle remote_datawriter_crypto,
          _Inout_ DDS_Security_SecurityException ex);

  bool
          set_remote_datareader_crypto_tokens(
  _In_ DDS_Security_DatawriterCryptoHandle local_datawriter_crypto,
  _In_ DDS_Security_DatareaderCryptoHandle remote_datareader_crypto,
  _In_ DDS_Security_DatareaderCryptoTokenSeq remote_datareader_tokens,
  _Inout_ DDS_Security_SecurityException ex);

  bool
  return_crypto_tokens(
          _In_ DDS_Security_CryptoTokenSeq crypto_tokens,
          _Inout_ DDS_Security_SecurityException ex);
};

typedef enum
{
  INFO_SUBMESSAGE,
  DATAWRITER_SUBMESSAGE,
  DATAREADER_SUBMESSAGE
} SecureSumessageCategory_t;

// DDSSEC11-96
// DDSSEC11-123
interface CryptoTransform
{
  bool
  encode_serialized_payload(
          _Inout_ DDS_Security_OctetSeq encoded_buffer,
          _Inout_ DDS_Security_OctetSeq extra_inline_qos,
          _In_ DDS_Security_OctetSeq plain_buffer,
          _In_ DDS_Security_DatawriterCryptoHandle sending_datawriter_crypto,
          _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-66
  bool
  encode_datawriter_submessage(
          _Inout_ DDS_Security_OctetSeq encoded_rtps_submessage,
          _In_ DDS_Security_OctetSeq plain_rtps_submessage,
          _In_ DDS_Security_DatawriterCryptoHandle sending_datawriter_crypto,
          _In_ DDS_Security_DatareaderCryptoHandleSeq receiving_datareader_crypto_list,
          _Inout_ DDS_Security_long receiving_datareader_crypto_list_index,
          _Inout_ DDS_Security_SecurityException ex);

  bool
  encode_datareader_submessage(
          _Inout_ DDS_Security_OctetSeq encoded_rtps_submessage,
          _In_ DDS_Security_OctetSeq plain_rtps_submessage,
          _In_ DDS_Security_DatareaderCryptoHandle sending_datareader_crypto,
          _In_ DDS_Security_DatawriterCryptoHandleSeq receiving_datawriter_crypto_list,
          _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-66
  bool
  encode_rtps_message(
          _Inout_ DDS_Security_OctetSeq encoded_rtps_message,
          _In_ DDS_Security_OctetSeq plain_rtps_message,
          _In_ DDS_Security_ParticipantCryptoHandle sending_participant_crypto,
          _In_ DDS_Security_ParticipantCryptoHandleSeq receiving_participant_crypto_list,
          _Inout_ DDS_Security_long receiving_participant_crypto_list_index,
          _Inout_ DDS_Security_SecurityException ex);

  bool
  decode_rtps_message(
          _Inout_ DDS_Security_OctetSeq plain_buffer,
          _In_ DDS_Security_OctetSeq encoded_buffer,
          _In_ DDS_Security_ParticipantCryptoHandle receiving_participant_crypto,
          _In_ DDS_Security_ParticipantCryptoHandle sending_participant_crypto,
          _Inout_ DDS_Security_SecurityException ex);

  bool
          preprocess_secure_submsg(
  _Inout_ DDS_Security_DatawriterCryptoHandle datawriter_crypto,
  _Inout_ DDS_Security_DatareaderCryptoHandle datareader_crypto,
  _Inout_ DDS_Security_SecureSumessageCategory_t secure_submessage_category,
  _In_ DDS_Security_OctetSeq encoded_rtps_submessage,
  _In_ DDS_Security_ParticipantCryptoHandle receiving_participant_crypto,
  _In_ DDS_Security_ParticipantCryptoHandle sending_participant_crypto,
  _Inout_ DDS_Security_SecurityException ex);

  bool
  decode_datawriter_submessage(
          _Inout_ DDS_Security_OctetSeq plain_rtps_submessage,
          _In_ DDS_Security_OctetSeq encoded_rtps_submessage,
          _In_ DDS_Security_DatareaderCryptoHandle receiving_datareader_crypto,
          _In_ DDS_Security_DatawriterCryptoHandle sending_datawriter_crypto,
          _In_ DDS_Security_SecurityException ex);

  bool
  decode_datareader_submessage(
          _Inout_ DDS_Security_OctetSeq plain_rtps_message,
          _In_ DDS_Security_OctetSeq encoded_rtps_message,
          _In_ DDS_Security_DatawriterCryptoHandle receiving_datawriter_crypto,
          _In_ DDS_Security_DatareaderCryptoHandle sending_datareader_crypto,
          _Inout_ DDS_Security_SecurityException ex);

  // DDSSEC11-123
  bool
  decode_serialized_payload(
          _Inout_ DDS_Security_OctetSeq plain_buffer,
          _In_ DDS_Security_OctetSeq encoded_buffer,
          _In_ DDS_Security_OctetSeq inline_qos,
          _In_ DDS_Security_DatareaderCryptoHandle receiving_datareader_crypto,
          _In_ DDS_Security_DatawriterCryptoHandle sending_datawriter_crypto,
          _Inout_ DDS_Security_SecurityException ex);
};

typedef enum
{
  EMERGENCY_LEVEL, // System is unusable. Should not continue use.
  ALERT_LEVEL,     // Should be corrected immediately
  CRITICAL_LEVEL,  // A failure _In_ DDS_Security_primary application.
  ERROR_LEVEL,     // General error conditions
  WARNING_LEVEL,   // May indicate future error if action not taken.
  NOTICE_LEVEL,    // Unusual, but nor erroneous event or condition.
  INFORMATIONAL_LEVEL, // Normal operational. Requires no action.
  DEBUG_LEVEL
}LoggingLevel;

// DDSSEC11-96
@
extensibility(FINAL)
        typedef struct DDS_Security_NameValuePair
        {
          char *name;
          char *value;
        };

                // DDSSEC11-85
#ifndef _DDS_Security_NameValuePairSeq_defined
#define _DDS_Security_NameValuePairSeq_defined
        typedef struct
        {
          uint32_t _maximum;
          uint32_t _length;
          NameValuePair *_buffer;
          bool _release;
        } DDS_Security_NameValuePairSeq;
        DDS_Security_NameValuePairSeq *DDS_Security_NameValuePairSeq__alloc(void);
        NameValuePair *DDS_Security_NameValuePairSeq__allocbuf(uint32_t len);
#endif /* _DDS_Security_NameValuePairSeq_defined */

        // DDSSEC11-96
@extensibility(FINAL)
        struct DDS_Security_BuiltinLoggingType
        {
          uint8_t facility;  // Set to 0x10. Indicates sec/auth msgs  //octet:int8_t
          LoggingLevel severity;
          DDS::Time_t timestamp; // Since epoch 1970-01-01 00:00:00 +0000 (UTC)
          char *hostname;  // IP host name of originator
          char *hostip;    // IP address of originator
          char *appname;   // Identify the device or application
          char *procid;    // Process name/ID for syslog system
          char *msgid;     // Identify the type of message
          char *message;   // Free-form message

          // Note that certa_In_ DDS_Security_char * keys (SD-IDs) are reserved by IANA
          map<char *, NameValuePairSeq> structured_data;
        };

                // DDSSEC11-85
        struct DDS_Security_LogOptions
        {
          LoggingLevel logging_level;
          char *log_file;
          bool distribute;
        };

                // DDSSEC11-96
        interface LoggerListener {
  bool on_log_message(_In_ DDS_Security_BuiltinLoggingType msg);
};

// DDSSEC11-96
interface Logging
{
  bool set_log_options(
          _In_ DDS_Security_LogOptions options,
          _Inout_ DDS_Security_DDS_Security_SecurityException ex);

  bool log(
          _In_ DDS_Security_BuiltinLoggingType msg,
          _Inout_ DDS_Security_SecurityException ex);

  bool enable_logging(
          _Inout_ DDS_Security_SecurityException ex);

  bool set_listener(
          _In_ DDS_Security_LoggerListener listener,
          _Inout_ DDS_Security_SecurityException ex);

};
};
};


#endif //DDSC_SECURITY_H
