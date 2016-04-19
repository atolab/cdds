#ifndef DDS_IMPL_H
#define DDS_IMPL_H

#include "dds_public_alloc.h"
#include "dds_public_stream.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct dds_sequence
{
  uint32_t _maximum;
  uint32_t _length;
  uint8_t * _buffer;
  bool _release;
}
dds_sequence_t;

#define DDS_LENGTH_UNLIMITED -1

typedef struct dds_key_descriptor
{
  const char * m_name;
  uint32_t m_index;
}
dds_key_descriptor_t;

/*
  Topic definitions are output by a preprocessor and have an
  implementation-private definition. The only thing exposed on the
  API is a pointer to the "topic_descriptor_t" struct type.
*/

typedef struct dds_topic_descriptor
{
  const size_t m_size;                 /* Size of topic type */
  const uint32_t m_align;              /* Alignment of topic type */
  const uint32_t m_flagset;            /* Flags */
  const uint32_t m_nkeys;              /* Number of keys (can be 0) */
  const char * m_typename;             /* Type name */
  const dds_key_descriptor_t * m_keys; /* Key descriptors (NULL iff m_nkeys 0) */
  const uint32_t m_nops;               /* Number of ops in m_ops */
  const uint32_t * m_ops;              /* Marshalling meta data */
  const char * m_meta;                 /* XML topic description meta data */
}
dds_topic_descriptor_t;

/* Topic descriptor flag values */

#define DDS_TOPIC_NO_OPTIMIZE 0x0001
#define DDS_TOPIC_FIXED_KEY 0x0002

/*
  Masks for read condition, read, take: there is only one mask here,
  which combines the sample, view and instance states.
*/

#define DDS_READ_SAMPLE_STATE 1u
#define DDS_NOT_READ_SAMPLE_STATE 2u
#define DDS_ANY_SAMPLE_STATE (1u | 2u)

#define DDS_NEW_VIEW_STATE 4u
#define DDS_NOT_NEW_VIEW_STATE 8u
#define DDS_ANY_VIEW_STATE (4u | 8u)

#define DDS_ALIVE_INSTANCE_STATE 16u
#define DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE 32u
#define DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE 64u
#define DDS_ANY_INSTANCE_STATE (16u | 32u | 64u)

#define DDS_ANY_STATE (DDS_ANY_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE)

#define DDS_DOMAIN_DEFAULT -1
#define DDS_HANDLE_NIL 0

/* Handles are opaque pointers to implementation types */

typedef struct dds_entity * dds_entity_t;
typedef struct dds_condition * dds_condition_t;
typedef void * dds_listener_t;
typedef uint64_t dds_instance_handle_t;
typedef int32_t dds_domainid_t;
typedef struct dds_waitset * dds_waitset_t;

typedef struct dds_condition_seq
{
  uint32_t _length;
  dds_condition_t * _buffer;
  bool _release;
}
dds_condition_seq;

/* Topic encoding instruction types */

#define DDS_OP_RTS 0x00000000
#define DDS_OP_ADR 0x01000000
#define DDS_OP_JSR 0x02000000
#define DDS_OP_JEQ 0x03000000

/* Core type flags

  1BY : One byte simple type
  2BY : Two byte simple type
  4BY : Four byte simple type
  8BY : Eight byte simple type
  STR : String
  BST : Bounded string
  SEQ : Sequence
  ARR : Array
  UNI : Union
  STU : Struct
*/

#define DDS_OP_VAL_1BY 0x01
#define DDS_OP_VAL_2BY 0x02
#define DDS_OP_VAL_4BY 0x03
#define DDS_OP_VAL_8BY 0x04
#define DDS_OP_VAL_STR 0x05
#define DDS_OP_VAL_BST 0x06
#define DDS_OP_VAL_SEQ 0x07
#define DDS_OP_VAL_ARR 0x08
#define DDS_OP_VAL_UNI 0x09
#define DDS_OP_VAL_STU 0x0a

#define DDS_OP_TYPE_1BY (DDS_OP_VAL_1BY << 16) 
#define DDS_OP_TYPE_2BY (DDS_OP_VAL_2BY << 16)
#define DDS_OP_TYPE_4BY (DDS_OP_VAL_4BY << 16)
#define DDS_OP_TYPE_8BY (DDS_OP_VAL_8BY << 16)
#define DDS_OP_TYPE_STR (DDS_OP_VAL_STR << 16)
#define DDS_OP_TYPE_SEQ (DDS_OP_VAL_SEQ << 16)
#define DDS_OP_TYPE_ARR (DDS_OP_VAL_ARR << 16)
#define DDS_OP_TYPE_UNI (DDS_OP_VAL_UNI << 16)
#define DDS_OP_TYPE_STU (DDS_OP_VAL_STU << 16)
#define DDS_OP_TYPE_BST (DDS_OP_VAL_BST << 16)

#define DDS_OP_SUBTYPE_1BY (DDS_OP_VAL_1BY << 8) 
#define DDS_OP_SUBTYPE_2BY (DDS_OP_VAL_2BY << 8)
#define DDS_OP_SUBTYPE_4BY (DDS_OP_VAL_4BY << 8)
#define DDS_OP_SUBTYPE_8BY (DDS_OP_VAL_8BY << 8)
#define DDS_OP_SUBTYPE_STR (DDS_OP_VAL_STR << 8)
#define DDS_OP_SUBTYPE_SEQ (DDS_OP_VAL_SEQ << 8)
#define DDS_OP_SUBTYPE_ARR (DDS_OP_VAL_ARR << 8)
#define DDS_OP_SUBTYPE_UNI (DDS_OP_VAL_UNI << 8)
#define DDS_OP_SUBTYPE_STU (DDS_OP_VAL_STU << 8)
#define DDS_OP_SUBTYPE_BST (DDS_OP_VAL_BST << 8)

#define DDS_OP_FLAG_KEY 0x01
#define DDS_OP_FLAG_DEF 0x02

#undef DDS_EXPORT
#ifdef _WIN32_DLL_
  #if defined VL_BUILD_DDS_DLL
    #define DDS_EXPORT extern __declspec (dllexport)
  #else
    #define DDS_EXPORT extern __declspec (dllimport)
  #endif
#else
  #define DDS_EXPORT extern
#endif

/**
 * Description : Enable or disable write batching. Overrides default configuration
 * setting for write batching (DDSI2E/Internal/WriteBatch).
 *
 * Arguments :
 *   -# enable Enables or disables write batching for all writers.
 */
DDS_EXPORT void dds_write_set_batch (bool enable);

#undef DDS_EXPORT
#ifdef _WIN32_DLL_
  #if defined VL_BUILD_DDS_SSL_DLL
    #define DDS_EXPORT extern __declspec (dllexport)
  #else
    #define DDS_EXPORT extern __declspec (dllimport)
  #endif
#else
  #define DDS_EXPORT extern
#endif

/**
 * Description : Install tcp/ssl and encryption support. Depends on openssl.
 *
 * Arguments :
 *   -# None
 */
DDS_EXPORT void dds_ssl_plugin (void);

#undef DDS_EXPORT
#ifdef _WIN32_DLL_
  #if defined VL_BUILD_DDS_DUR_DLL
    #define DDS_EXPORT extern __declspec (dllexport)
  #else
    #define DDS_EXPORT extern __declspec (dllimport)
  #endif
#else
  #define DDS_EXPORT extern
#endif

/**
 * Description : Install client durability support. Depends on OSPL server.
 *
 * Arguments :
 *   -# None
 */
DDS_EXPORT void dds_durability_plugin (void);

#undef DDS_EXPORT
#if defined (__cplusplus)
}
#endif
#endif
