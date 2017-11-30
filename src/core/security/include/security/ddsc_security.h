//
// Created by kurtulus on 29-11-17.
//

#ifndef DDSC_SECURITY_H
#define DDSC_SECURITY_H



/*
 * DDS_Security_rtf2_dcps.idl needed for the declarations
 * of DDS Entities and DDS Entity Qos
 */
// DDSSEC11-96
#include "dds-xtypes_discovery.idl"  /* http://www.omg.org/spec/DDS-XTypes/20170301/dds-xtypes_discovery.idl */

// The types in this file shall be serialized with XCDR encoding version 1
module DDS {
  module Security {

    // DynamicData is in DDS-XTYPES but including the XTYPES IDL
    // Would make the file not compilable by legacy IDL compilers
    // that do not understand the new anotation syntax
    int64_t DynamicData; //native=int64_t assumption

#define _DDS_Security_Security_LongLongSeq_defined
          typedef struct {
            uint32_t _maximum;
            uint32_t _length;
            int64_t *_buffer;
            bool _release;
          } DDS_Security_Security_LongLongSeq;
          DDS_Security_Security_LongLongSeq *DDS_Security_Security_LongLongSeq__alloc (void);
          int64_t *DDS_Security_Security_LongLongSeq__allocbuf (uint32_t len);
#endif /* _DDS_Security_Security_LongLongSeq_defined */

    typedef struct Property_t {
      string name;
      string value;
      boolean propagate;
    } Property_t;

#define _DDS_Security_Security_PropertySeq_defined
  typedef struct {
    uint32_t _maximum;
    uint32_t _length;
    Property_t *_buffer;
    bool _release;
  } DDS_Security_Security_PropertySeq;
  DDS_Security_Security_PropertySeq *DDS_Security_Security_PropertySeq__alloc (void);
  Property_t *DDS_Security_Security_PropertySeq__allocbuf (uint32_t len);
#endif /* _DDS_Security_Security_PropertySeq_defined */


    typedef struct BinaryProperty_t {
      string name;
      OctetSeq value;
      boolean propagate;
    } BinaryProperty_t;

    #define _DDS_Security_Security_BinaryPropertySeq_defined
          typedef struct {
            uint32_t _maximum;
            uint32_t _length;
            BinaryProperty_t  *_buffer;
            bool _release;
          } DDS_Security_Security_BinaryPropertySeq;
          DDS_Security_Security_BinaryPropertySeq *DDS_Security_Security_BinaryPropertySeq__alloc (void);
          BinaryProperty_t  *DDS_Security_Security_BinaryPropertySeq__allocbuf (uint32_t len);
#endif /* _DDS_Security_Security_BinaryPropertySeq_defined */

    // DDSSEC11-96
    typedef struct DataHolder {
      string             class_id;
      PropertySeq        properties;
      BinaryPropertySeq  binary_properties;
    } DataHolder;
    #define _DDS_Security_Security_DataHolderSeq_defined
          typedef struct {
            uint32_t _maximum;
            uint32_t _length;
            DataHolder *_buffer;
            bool _release;
          } DDS_Security_Security_DataHolderSeq;
          DDS_Security_Security_DataHolderSeq *DDS_Security_Security_DataHolderSeq__alloc (void);
          DataHolder *DDS_Security_Security_DataHolderSeq__allocbuf (uint32_t len);
#endif /* _DDS_Security_Security_DataHolderSeq_defined */

    typedef DataHolder Token;

    // DDSSEC11-43
    typedef Token MessageToken;
    typedef MessageToken AuthRequestMessageToken;
    typedef MessageToken HandshakeMessageToken;

    // DDSSEC11-82
    typedef Token IdentityStatusToken;
    #define _DDS_Security_HandshakeMessageTokenSeq_defined
          typedef struct {
            uint32_t _maximum;
            uint32_t _length;
            HandshakeMessageToken *_buffer;
            bool _release;
          } DDS_Security_HandshakeMessageTokenSeq;
          DDS_Security_HandshakeMessageTokenSeq *DDS_Security_HandshakeMessageTokenSeq__alloc (void);
          HandshakeMessageToken *DDS_Security_HandshakeMessageTokenSeq__allocbuf (uint32_t len);
#endif /* _DDS_Security_HandshakeMessageTokenSeq_defined */

    typedef Token  IdentityToken;
    typedef Token  PermissionsToken;
    typedef Token  AuthenticatedPeerCredentialToken;
    typedef Token  PermissionsCredentialToken;

    typedef Token  CryptoToken;
    #define _DDS_Security_CryptoTokenSeq_defined
          typedef struct {
            uint32_t _maximum;
            uint32_t _length;
            CryptoToken *_buffer;
            bool _release;
          } DDS_Security_CryptoTokenSeq;
          DDS_Security_CryptoTokenSeq *DDS_Security_CryptoTokenSeq__alloc (void);
          CryptoToken *DDS_Security_CryptoTokenSeq__allocbuf (uint32_t len);
#endif /* _DDS_Security_CryptoTokenSeq_defined */

    typedef Token  ParticipantCryptoToken;
    typedef Token  DatawriterCryptoToken;
    typedef Token  DatareaderCryptoToken;

    typedef CryptoTokenSeq  ParticipantCryptoTokenSeq;
    typedef CryptoTokenSeq  DatawriterCryptoTokenSeq;
    typedef CryptoTokenSeq  DatareaderCryptoTokenSeq;

    // DDSSEC11-88
    // From DDS-RTPS [2] clauses 8.4.2.1 and 9.3.1
    typedef octet GuidPrefix_t[12];
    typedef struct EntityId_t {
      octet entityKey[3];
      octet entityKind;
    } EntityId_t;

    // DDSSEC11-88
    typedef struct GUID_t {
      GuidPrefix_t prefix;
      EntityId_t   entityId;
    } GUID_t;

    // DDSSEC11-88
    typedef struct MessageIdentity {
      GUID_t     source_guid;
      long long  sequence_number;
    } MessageIdentity;

    // DDSSEC11-24
    const ReturnCode_t RETCODE_NOT_ALLOWED_BY_SECURITY = 1000;

    // DDSSEC11-96

    typedef struct PropertyQosPolicy {
      PropertySeq        value;
      BinaryPropertySeq  binary_value;
    } PropertyQosPolicy;

    // DDSSEC11-96
    // See http://www.omg.org/spec/DDS-XTypes/20170301/dds-xtypes_discovery.idl
    struct DDS_Security_Security_DomainParticipantQos :  DDS::DomainParticipantQos {
      PropertyQosPolicy  property;
    };

    struct Tag {
      string name;
      string value;
    };

    #define _DDS_Security_TagSeq_defined
          typedef struct {
            uint32_t _maximum;
            uint32_t _length;
             Tag  *_buffer;
            bool _release;
          } DDS_Security_TagSeq;
          DDS_Security_TagSeq *DDS_Security_TagSeq__alloc (void);
           Tag  *DDS_Security_TagSeq__allocbuf (uint32_t len);
#endif /* _DDS_Security_TagSeq_defined */
    struct DataTags {
      TagSeq tags;
    };

    // DDSSEC11-34
    typedef DataTags DataTagQosPolicy;

    // DDSSEC11-96
    // See http://www.omg.org/spec/DDS-XTypes/20170301/dds-xtypes_discovery.idl
    // DDSSEC11-34

    struct DataWriterQos  :  DDS::DataWriterQos {
      PropertyQosPolicy  property;
      DataTagQosPolicy   data_tags;
    };

    // DDSSEC11-96
    // See http://www.omg.org/spec/DDS-XTypes/20170301/dds-xtypes_discovery.idl
    // DDSSEC11-34
    struct DataReaderQos  :  DDS::DataReaderQos {
      PropertyQosPolicy  property;
      DataTagQosPolicy   data_tags;
    };

    // DDSSEC11-137
    typedef unsigned long ParticipantSecurityAttributesMask;
    typedef unsigned long PluginParticipantSecurityAttributesMask;

    @extensibility(APPENDABLE)
    struct ParticipantSecurityInfo {
      ParticipantSecurityAttributesMask        participant_security_attributes;
      PluginParticipantSecurityAttributesMask  plugin_participant_security_attributes;
    };

    // DDSSEC11-106
    typedef unsigned long EndpointSecurityAttributesMask;
    typedef unsigned long PluginEndpointSecurityAttributesMask;
    @extensibility(APPENDABLE)
    struct EndpointSecurityInfo {
      EndpointSecurityAttributesMask        endpoint_security_mask;
      PluginEndpointSecurityAttributesMask  plugin_endpoint_security_mask;
    };

    // DDSSEC11-96
    // See http://www.omg.org/spec/DDS-XTypes/20170301/dds-xtypes_discovery.idl
    // DDSSEC11-137
    @extensibility(MUTABLE)
    struct ParticipantBuiltinTopicData  :  DDS::ParticipantBuiltinTopicData {
      @id(0x1001)  IdentityToken     identity_token;
      @id(0x1002)  PermissionsToken  permissions_token;
      @id(0x0059)  PropertyQosPolicy property;
      @id(0x1005)  ParticipantSecurityInfo  security_info;
    };

    // DDSSEC11-82
    @extensibility(MUTABLE)
    struct ParticipantBuiltinTopicDataSecure  :  ParticipantBuiltinTopicData {
      @id(0x1006) @optional IdentityStatusToken identity_status_token;
    };

    // DDSSEC11-85
    @extensibility(MUTABLE)
    struct PublicationBuiltinTopicData: DDS::PublicationBuiltinTopicData {
      @id(0x1004) EndpointSecurityInfo  security_info;
    };

    // DDSSEC11-85
    @extensibility(MUTABLE)
    struct SubscriptionBuiltinTopicData: DDS::SubscriptionBuiltinTopicData {
      @id(0x1004) EndpointSecurityInfo  security_info;
    };

    // DDSSEC11-96
    // See http://www.omg.org/spec/DDS-XTypes/20170301/dds-xtypes_discovery.idl
    @extensibility(MUTABLE)
    struct PublicationBuiltinTopicDataSecure  :  PublicationBuiltinTopicData {
      @id(0x1003)  DataTags data_tags;
    };

    // DDSSEC11-96
    // See http://www.omg.org/spec/DDS-XTypes/20170301/dds-xtypes_discovery.idl
    @extensibility(MUTABLE)
    struct SubscriptionBuiltinTopicDataSecure  :  DDS::SubscriptionBuiltinTopicData {
      @id(0x1003)  DataTags data_tags;
    };

    // DDSSEC11-15 typedef long SecurityExceptionCode;
    typedef long SecurityExceptionCode;

    struct SecurityException {
      string  message;
      long    code;      // DDSSEC11-15
      long    minor_code;
    };

    enum ValidationResult_t {
      VALIDATION_OK,
      VALIDATION_FAILED,
      VALIDATION_PENDING_RETRY,
      VALIDATION_PENDING_HANDSHAKE_REQUEST,
      VALIDATION_PENDING_HANDSHAKE_MESSAGE,
      VALIDATION_OK_FINAL_MESSAGE
    };

    native IdentityHandle;
    native HandshakeHandle;
    native SharedSecretHandle;
    native PermissionsHandle;
    native ParticipantCryptoHandle;
    native ParticipantCryptoHandleSeq;
    native DatawriterCryptoHandle;
    native DatawriterCryptoHandleSeq;
    native DatareaderCryptoHandle;
    native DatareaderCryptoHandleSeq;

    // DDSSEC11-96
    interface Authentication;

    // DDSSEC11-82
    enum AuthStatusKind {
      @value(1) IDENTITY_STATUS
    };

    // DDSSEC11-96
    interface AuthenticationListener {
      boolean
      on_revoke_identity(
              in    Authentication     plugin,
              in    IdentityHandle     handle,
              inout SecurityException  ex);

      // DDSSEC11-82
      boolean
      on_status_changed(
              in    Authentication     plugin,
              in    IdentityHandle     handle,
              in    AuthStatusKind     status_kind,
              inout SecurityException  ex);
    };

    // DDSSEC11-96
    interface Authentication {
      // DDSSEC11-88
      ValidationResult_t
      validate_local_identity(
              inout IdentityHandle        local_identity_handle,
              inout GUID_t                adjusted_participant_guid,
              in    DomainId_t            domain_id,
              in    DomainParticipantQos  participant_qos,
              in    GUID_t                candidate_participant_guid,
              inout SecurityException     ex );

      boolean
      get_identity_token(
              inout IdentityToken      identity_token,
              in    IdentityHandle     handle,
              inout SecurityException  ex );

      // DDSSEC11-82
      boolean
      get_identity_status_token(
              inout IdentityStatusToken      identity_status_token,
              in    IdentityHandle           handle,
              inout SecurityException        ex );

      boolean
      set_permissions_credential_and_token(
              in    IdentityHandle         handle,
              in    PermissionsCredential  permissions_credential,
              in    PermissionsToken       permissions_token,
              inout SecurityException      ex );

      // DDSSEC11-21
      // DDSSEC11-88
      // DDSSEC11-85
      ValidationResult_t
      validate_remote_identity(
              inout  IdentityHandle           remote_identity_handle,
              inout  AuthRequestMessageToken  local_auth_request_token,
              in     AuthRequestMessageToken  remote_auth_request_token,
              in     IdentityHandle           local_identity_handle,
              in     IdentityToken            remote_identity_token,
              in     GUID_t                   remote_participant_guid,
              inout  SecurityException        ex );

      // DDSSEC11-46
      // DDSSEC11-118
      ValidationResult_t
      begin_handshake_request(
              inout HandshakeHandle        handshake_handle,
              inout HandshakeMessageToken  handshake_message,
              in    HandshakeMessageToken  handshake_message_in,
              in    IdentityHandle         initiator_identity_handle,
              in    IdentityHandle         replier_identity_handle,
              in    OctetSeq               serialized_local_participant_data,
              inout SecurityException      ex );

      // DDSSEC11-46
      ValidationResult_t
      begin_handshake_reply(
              inout HandshakeHandle        handshake_handle,
              inout HandshakeMessageToken  handshake_message_out,
              in    HandshakeMessageToken  handshake_message_in,
              in    IdentityHandle         initiator_identity_handle,
              in    IdentityHandle         replier_identity_handle,
              in    OctetSeq               serialized_local_participant_data,
              inout SecurityException      ex );

      ValidationResult_t
      process_handshake(
              inout HandshakeMessageToken  handshake_message_out,
              in    HandshakeMessageToken  handshake_message_in,
              in    HandshakeHandle        handshake_handle,
              inout SecurityException      ex );

      SharedSecretHandle
      get_shared_secret(
              in    HandshakeHandle    handshake_handle,
              inout SecurityException  ex );

      boolean
      get_authenticated_peer_credential_token(
              inout AuthenticatedPeerCredentialToken  peer_credential_token,
              in    HandshakeHandle                   handshake_handle,
              inout SecurityException                 ex );

      boolean
      set_listener(
              in   AuthenticationListener  listener,
              inout SecurityException   ex );

      boolean
      return_identity_token(
              in    IdentityToken      token,
              inout SecurityException  ex);

      // DDSSEC11-82
      boolean
      return_identity_status_token(
              in    IdentityStatusToken  token,
              inout SecurityException    ex);

      boolean
      return_authenticated_peer_credential_token(
              in   AuthenticatedPeerCredentialToken peer_credential_token,
              inout SecurityException  ex);

      boolean
      return_handshake_handle(
              in    HandshakeHandle    handshake_handle,
              inout SecurityException  ex);

      boolean
      return_identity_handle(
              in   IdentityHandle      identity_handle,
              inout SecurityException  ex);

      boolean
      return_sharedsecret_handle(
              in    SharedSecretHandle  sharedsecret_handle,
              inout SecurityException   ex);
    };

    // DDSSEC11-137 DDSEC11-85
    struct ParticipantSecurityAttributes {
      boolean     allow_unauthenticated_participants;
      boolean     is_access_protected;
      boolean     is_rtps_protected;
      boolean     is_discovery_protected;
      boolean     is_liveliness_protected;
      ParticipantSecurityAttributesMask plugin_participant_attributes;
      PropertySeq ac_endpoint_properties;
    };

    // DDSSEC11-16
    struct TopicSecurityAttributes {
      boolean  is_read_protected;
      boolean  is_write_protected;
      boolean  is_discovery_protected;
      boolean  is_liveliness_protected;
    };

    // DDSSEC11-16 DDSSEC11-106 DDSEC11-85
    struct EndpointSecurityAttributes : TopicSecurityAttributes {
      boolean     is_submessage_protected;
      boolean     is_payload_protected;
      boolean     is_key_protected;
      PluginEndpointSecurityAttributesMask  plugin_endpoint_attributes;
      PropertySeq ac_endpoint_properties;
    };

    // DDSSEC11-106
    struct PluginEndpointSecurityAttributes {
      boolean     is_submessage_encrypted;
      boolean     is_payload_encrypted;
      boolean     is_submessage_origin_authenticated;
    };

    // DDSSEC11-96
    interface AccessControl;
    typedef long  DomainId_t;

    // DDSSEC11-96
    interface AccessControlListener {
      boolean on_revoke_permissions(
              in   AccessControl plugin,
              in   PermissionsHandle handle);
    };

    // DDSSEC11-96
    interface AccessControl {
      PermissionsHandle
      validate_local_permissions(
              in    Authentication         auth_plugin,
              in    IdentityHandle         identity,
              in    DomainId_t             domain_id,
              in    DomainParticipantQos   participant_qos,
              inout SecurityException      ex );

      PermissionsHandle
      validate_remote_permissions(
              in    Authentication                    auth_plugin,
              in    IdentityHandle                    local_identity_handle,
              in    IdentityHandle                    remote_identity_handle,
              in    PermissionsToken                  remote_permissions_token,
              in    AuthenticatedPeerCredentialToken  remote_credential_token,
              inout SecurityException                 ex );

      boolean
      check_create_participant(
              in    PermissionsHandle     permissions_handle,
              in    DomainId_t            domain_id,
              in    DomainParticipantQos  qos,
              inout SecurityException     ex );

      boolean
      check_create_datawriter(
              in    PermissionsHandle   permissions_handle,
              in    DomainId_t          domain_id,
              in    string              topic_name,
              in    DataWriterQos       qos,
              in    PartitionQosPolicy  partition,
              in    DataTags            data_tag,
              inout SecurityException   ex);

      boolean
      check_create_datareader(
              in    PermissionsHandle   permissions_handle,
              in    DomainId_t          domain_id,
              in    string              topic_name,
              in    DataReaderQos       qos,
              in    PartitionQosPolicy  partition,
              in    DataTags            data_tag,
              inout SecurityException   ex);

      // DDSSEC11-33
      boolean
      check_create_topic(
              in    PermissionsHandle permissions_handle,
              in    DomainId_t         domain_id,
              in    string             topic_name,
              in    TopicQos           qos,
              inout SecurityException  ex);

      boolean
      check_local_datawriter_register_instance(
              in    PermissionsHandle  permissions_handle,
              in    DataWriter         writer,
              in    DynamicData        key,
              inout SecurityException  ex);

      boolean
      check_local_datawriter_dispose_instance(
              in    PermissionsHandle  permissions_handle,
              in    DataWriter         writer,
              in    DynamicData        key,
              inout SecurityException  ex);

      boolean
      check_remote_participant(
              in    PermissionsHandle                  permissions_handle,
              in    DomainId_t                         domain_id,
              in    ParticipantBuiltinTopicDataSecure  participant_data,
              inout SecurityException                  ex);

      boolean
      check_remote_datawriter(
              in   PermissionsHandle                  permissions_handle,
              in   DomainId_t                         domain_id,
              in   PublicationBuiltinTopicDataSecure  publication_data,
              inout SecurityException                 ex);

      boolean
      check_remote_datareader(
              in    PermissionsHandle                   permissions_handle,
              in    DomainId_t                          domain_id,
              in    SubscriptionBuiltinTopicDataSecure  subscription_data,
              inout boolean                             relay_only,
              inout SecurityException                   ex);

      boolean
      check_remote_topic(
              in    PermissionsHandle      permissions_handle,
              in    DomainId_t             domain_id,
              in    TopicBuiltinTopicData  topic_data,
              inout SecurityException      ex);

      // DDSSEC11-34
      boolean
      check_local_datawriter_match(
              in    PermissionsHandle  writer_permissions_handle,
              in    PermissionsHandle  reader_permissions_handle,
              in    PublicationBuiltinTopicDataSecure  publication_data,
              in    SubscriptionBuiltinTopicDataSecure subscription_data,
              inout SecurityException  ex);

      // DDSSEC11-34
      boolean
      check_local_datareader_match(
              in    PermissionsHandle  reader_permissions_handle,
              in    PermissionsHandle  writer_permissions_handle,
              in    SubscriptionBuiltinTopicDataSecure subscription_data,
              in    PublicationBuiltinTopicDataSecure  publication_data,
              inout SecurityException  ex);

      boolean
      check_remote_datawriter_register_instance(
              in    PermissionsHandle   permissions_handle,
              in    DataReader          reader,
              in    InstanceHandle_t    publication_handle,
              in    DynamicData         key,
              in    InstanceHandle_t    instance_handle,
              inout SecurityException   ex);

      boolean
      check_remote_datawriter_dispose_instance(
              in    PermissionsHandle  permissions_handle,
              in    DataReader         reader,
              in    InstanceHandle_t   publication_handle,
              in    DynamicData        key,
              inout SecurityException  ex);

      boolean
      get_permissions_token(
              inout PermissionsToken   permissions_token,
              in    PermissionsHandle  handle,
              inout SecurityException  ex);

      boolean
      get_permissions_credential_token(
              inout PermissionsCredentialToken permissions_credential_token,
              in    PermissionsHandle  handle,
              inout SecurityException  ex);

      boolean
      set_listener(
              in    AccessControlListener  listener,
              inout SecurityException      ex);

      boolean
      return_permissions_token(
              in    PermissionsToken   token,
              inout SecurityException  ex);

      boolean
      return_permissions_credential_token(
              in    PermissionsCredentialToken  permissions_credential_token,
              inout SecurityException           ex);

      boolean
      get_participant_sec_attributes(
              in    PermissionsHandle              permissions_handle,
              inout ParticipantSecurityAttributes  attributes,
              inout SecurityException              ex);

      // DDSSEC11-16
      boolean
      get_topic_sec_attributes (
              in    PermissionsHandle           permissions_handle,
              in    String                      topic_name,
              inout TopicSecurityAttributes     attributes,
              inout SecurityException           ex);

      // DDSSEC11-16
      boolean
      get_datawriter_sec_attributes(
              in    PermissionsHandle           permissions_handle,
              in    PartitionQosPolicy          partition,
              in    DataTagQosPolicy            data_tag,
              inout EndpointSecurityAttributes  attributes,
              inout SecurityException           ex);

      // DDSSEC11-16
      boolean
      get_datareader_sec_attributes(
              in    PermissionsHandle           permissions_handle,
              in    PartitionQosPolicy          partition,
              in    DataTagQosPolicy            data_tag,
              inout EndpointSecurityAttributes  attributes,
              inout SecurityException           ex);

      // DDSSEC11-112
      boolean
      return_participant_sec_attributes(
              in ParticipantSecurityAttributes  attributes,
              inout SecurityException           ex);

      // DDSSEC11-112
      boolean
      return_datawriter_sec_attributes(
              in EndpointSecurityAttributes  attributes,
              inout SecurityException        ex);

      // DDSSEC11-112
      boolean
      return_datareader_sec_attributes(
              in EndpointSecurityAttributes  attributes,
              inout SecurityException        ex);
    };

    // DDSSEC11-96
    interface CryptoKeyFactory {

      // DDSSEC11-3 DDSSEC11-85
      ParticipantCryptoHandle
      register_local_participant(
              in    IdentityHandle                 participant_identity,
              in    PermissionsHandle              participant_permissions,
              in    PropertySeq                    participant_properties,
              in    ParticipantSecurityAttributes  participant_security_attributes,
              inout SecurityException              ex  );

      ParticipantCryptoHandle
      register_matched_remote_participant(
              in    ParticipantCryptoHandle  local_participant_crypto_handle,
              in    IdentityHandle           remote_participant_identity,
              in    PermissionsHandle        remote_participant_permissions,
              in    SharedSecretHandle       shared_secret,
              inout SecurityException        ex);

      // DDSSEC11-3 DDSSEC11-85
      DatawriterCryptoHandle
      register_local_datawriter(
              in    ParticipantCryptoHandle  participant_crypto,
              in    PropertySeq              datawriter_properties,
              in    EndpointSecurityAttributes datawriter_security_attributes,
              inout SecurityException        ex);

      DatareaderCryptoHandle
      register_matched_remote_datareader(
              in    DatawriterCryptoHandle   local_datawritert_crypto_handle,
              in    ParticipantCryptoHandle  remote_participant_crypto,
              in    SharedSecretHandle       shared_secret,
              in    boolean                  relay_only,
              inout SecurityException        ex);

      // DDSSEC11-3 DDSSEC11-85
      DatareaderCryptoHandle
      register_local_datareader(
              in    ParticipantCryptoHandle     participant_crypto,
              in    PropertySeq                 datareader_properties,
              in    EndpointSecurityAttributes  datareader_security_attributes,
              inout SecurityException           ex);

      DatawriterCryptoHandle
      register_matched_remote_datawriter(
              in    DatareaderCryptoHandle   local_datareader_crypto_handle,
              in    ParticipantCryptoHandle  remote_participant_crypt,
              in    SharedSecretHandle       shared_secret,
              inout SecurityException        ex );

      boolean
      unregister_participant(
              in    ParticipantCryptoHandle  participant_crypto_handle,
              inout SecurityException        ex);

      boolean
      unregister_datawriter(
              in    DatawriterCryptoHandle  datawriter_crypto_handle,
              inout SecurityException       ex  );

      boolean
      unregister_datareader(
              in    DatareaderCryptoHandle  datareader_crypto_handle,
              inout SecurityException       ex  );
    };


    // DDSSEC11-96
    interface CryptoKeyExchange {
      boolean
      create_local_participant_crypto_tokens(
              inout ParticipantCryptoTokenSeq  local_participant_crypto_tokens,
              in    ParticipantCryptoHandle    local_participant_crypto,
              in    ParticipantCryptoHandle    remote_participant_crypto,
              inout SecurityException          ex);

      boolean
      set_remote_participant_crypto_tokens(
              in    ParticipantCryptoHandle    local_participant_crypto,
              in    ParticipantCryptoHandle    remote_participant_crypto,
              in    ParticipantCryptoTokenSeq  remote_participant_tokens,
              inout SecurityException          ex);

      boolean
      create_local_datawriter_crypto_tokens(
              inout DatawriterCryptoTokenSeq  local_datawriter_crypto_tokens,
              in    DatawriterCryptoHandle    local_datawriter_crypto,
              in    DatareaderCryptoHandle    remote_datareader_crypto,
              inout SecurityException         ex);

      boolean
      set_remote_datawriter_crypto_tokens(
              in    DatareaderCryptoHandle    local_datareader_crypto,
              in    DatawriterCryptoHandle    remote_datawriter_crypto,
              in    DatawriterCryptoTokenSeq  remote_datawriter_tokens,
              inout SecurityException         ex);

      boolean
      create_local_datareader_crypto_tokens(
              inout DatareaderCryptoTokenSeq  local_datareader_cryto_tokens,
              in    DatareaderCryptoHandle    local_datareader_crypto,
              in    DatawriterCryptoHandle    remote_datawriter_crypto,
              inout SecurityException         ex);

      boolean
      set_remote_datareader_crypto_tokens(
              in    DatawriterCryptoHandle    local_datawriter_crypto,
              in    DatareaderCryptoHandle    remote_datareader_crypto,
              in    DatareaderCryptoTokenSeq  remote_datareader_tokens,
              inout SecurityException         ex);

      boolean
      return_crypto_tokens(
              in    CryptoTokenSeq     crypto_tokens,
              inout SecurityException  ex);
    };

    enum SecureSumessageCategory_t {
      INFO_SUBMESSAGE,
      DATAWRITER_SUBMESSAGE,
      DATAREADER_SUBMESSAGE
    };

    // DDSSEC11-96
    // DDSSEC11-123
    interface CryptoTransform {
      boolean
      encode_serialized_payload(
              inout OctetSeq                encoded_buffer,
              inout OctetSeq                extra_inline_qos,
              in    OctetSeq                plain_buffer,
              in    DatawriterCryptoHandle  sending_datawriter_crypto,
              inout SecurityException       ex);

      // DDSSEC11-66
      boolean
      encode_datawriter_submessage(
              inout OctetSeq                   encoded_rtps_submessage,
              in    OctetSeq                   plain_rtps_submessage,
              in    DatawriterCryptoHandle     sending_datawriter_crypto,
              in    DatareaderCryptoHandleSeq  receiving_datareader_crypto_list,
              inout long                       receiving_datareader_crypto_list_index,
              inout SecurityException          ex);

      boolean
      encode_datareader_submessage(
              inout OctetSeq                   encoded_rtps_submessage,
              in    OctetSeq                   plain_rtps_submessage,
              in    DatareaderCryptoHandle     sending_datareader_crypto,
              in    DatawriterCryptoHandleSeq  receiving_datawriter_crypto_list,
              inout SecurityException          ex);

      // DDSSEC11-66
      boolean
      encode_rtps_message(
              inout OctetSeq encoded_rtps_message,
              in    OctetSeq plain_rtps_message,
              in    ParticipantCryptoHandle sending_participant_crypto,
              in    ParticipantCryptoHandleSeq receiving_participant_crypto_list,
              inout long                       receiving_participant_crypto_list_index,
              inout SecurityException ex);

      boolean
      decode_rtps_message(
              inout OctetSeq                 plain_buffer,
              in    OctetSeq                 encoded_buffer,
              in    ParticipantCryptoHandle  receiving_participant_crypto,
              in    ParticipantCryptoHandle  sending_participant_crypto,
              inout SecurityException        ex);

      boolean
      preprocess_secure_submsg(
              inout DatawriterCryptoHandle         datawriter_crypto,
              inout DatareaderCryptoHandle         datareader_crypto,
              inout SecureSumessageCategory_t      secure_submessage_category,
              in    OctetSeq                       encoded_rtps_submessage,
              in    ParticipantCryptoHandle        receiving_participant_crypto,
              in    ParticipantCryptoHandle        sending_participant_crypto,
              inout SecurityException              ex);

      boolean
      decode_datawriter_submessage(
              inout OctetSeq                plain_rtps_submessage,
              in    OctetSeq                encoded_rtps_submessage,
              in    DatareaderCryptoHandle  receiving_datareader_crypto,
              in    DatawriterCryptoHandle  sending_datawriter_crypto,
              in    SecurityException       ex);

      boolean
      decode_datareader_submessage(
              inout OctetSeq                plain_rtps_message,
              in    OctetSeq                encoded_rtps_message,
              in    DatawriterCryptoHandle  receiving_datawriter_crypto,
              in    DatareaderCryptoHandle  sending_datareader_crypto,
              inout SecurityException       ex );

      // DDSSEC11-123
      boolean
      decode_serialized_payload(
              inout OctetSeq                plain_buffer,
              in    OctetSeq                encoded_buffer,
              in    OctetSeq                inline_qos,
              in    DatareaderCryptoHandle  receiving_datareader_crypto,
              in    DatawriterCryptoHandle  sending_datawriter_crypto,
              inout SecurityException       ex);
    };

    enum LoggingLevel {
      EMERGENCY_LEVEL, // System is unusable. Should not continue use.
      ALERT_LEVEL,     // Should be corrected immediately
      CRITICAL_LEVEL,  // A failure in primary application.
      ERROR_LEVEL,     // General error conditions
      WARNING_LEVEL,   // May indicate future error if action not taken.
      NOTICE_LEVEL,    // Unusual, but nor erroneous event or condition.
      INFORMATIONAL_LEVEL, // Normal operational. Requires no action.
      DEBUG_LEVEL
    };

    // DDSSEC11-96
    @extensibility(FINAL)
    struct NameValuePair {
      string name;
      string value;
    };

    // DDSSEC11-85
    #define _DDS_Security_NameValuePairSeq_defined
          typedef struct {
            uint32_t _maximum;
            uint32_t _length;
            NameValuePair *_buffer;
            bool _release;
          } DDS_Security_NameValuePairSeq;
          DDS_Security_NameValuePairSeq *DDS_Security_NameValuePairSeq__alloc (void);
          NameValuePair *DDS_Security_NameValuePairSeq__allocbuf (uint32_t len);
#endif /* _DDS_Security_NameValuePairSeq_defined */

    // DDSSEC11-96
    @extensibility(FINAL)
    struct BuiltinLoggingType {
      octet  facility;  // Set to 0x10. Indicates sec/auth msgs
      LoggingLevel severity;
      DDS::Time_t timestamp; // Since epoch 1970-01-01 00:00:00 +0000 (UTC)
      string hostname;  // IP host name of originator
      string hostip;    // IP address of originator
      string appname;   // Identify the device or application
      string procid;    // Process name/ID for syslog system
      string msgid;     // Identify the type of message
      string message;   // Free-form message

      // Note that certain string keys (SD-IDs) are reserved by IANA
      map<string, NameValuePairSeq>  structured_data;
    };

    // DDSSEC11-85
    struct LogOptions {
      LoggingLevel logging_level;
      string       log_file;
      boolean      distribute;
    };

    // DDSSEC11-96
    interface LoggerListener {
      boolean on_log_message(in BuiltinLoggingType msg);
    };

    // DDSSEC11-96
    interface Logging {
      boolean set_log_options(
              in    LogOptions options,
              inout SecurityException ex);

      boolean log(
              in    BuiltinLoggingType msg,
              inout SecurityException ex);

      boolean enable_logging(
              inout SecurityException ex);

      boolean set_listener(
              in LoggerListener listener,
              inout SecurityException ex);

    };
  };
};





#endif //DDSC_SECURITY_H
