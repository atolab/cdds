/** @file
 *
 * @brief DDS C Stream API
 *
 * @todo add copyright header?
 *
 * This header file defines the public API of the Streams in the
 * VortexDDS C language binding.
 */
#ifndef DDS_STREAM_H
#define DDS_STREAM_H

#include "os/os_public.h"
#include <stdbool.h>

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef _WIN32_DLL_
  #if defined VDDS_BUILD
    #define OS_API OS_API_EXPORT
  #else
    #define OS_API OS_API_IMPORT
  #endif
#else
  #define OS_API extern
#endif
struct dds_sequence;

typedef union
{
  uint8_t * p8;
  uint16_t * p16;
  uint32_t * p32;
  uint64_t * p64;
  float * pf;
  double * pd;
  void * pv;
}
dds_uptr_t;

typedef struct dds_stream
{
  dds_uptr_t m_buffer;  /* Union of pointers to start of buffer */
  size_t m_size;      /* Buffer size */
  size_t m_index;     /* Read/write offset from start of buffer */
  bool m_endian;        /* Endian: big (false) or little (true) */
  bool m_failed;        /* Attempt made to read beyond end of buffer */
}
dds_stream_t;

#define DDS_STREAM_BE false
#define DDS_STREAM_LE true

OS_API dds_stream_t * dds_stream_create (size_t size);
OS_API void dds_stream_delete (dds_stream_t * st);
OS_API void dds_stream_fini (dds_stream_t * st);
OS_API void dds_stream_reset (dds_stream_t * st);
OS_API void dds_stream_init (dds_stream_t * st, size_t size);
OS_API void dds_stream_grow (dds_stream_t * st, size_t size);
OS_API bool dds_stream_endian (void);

OS_API bool dds_stream_read_bool (dds_stream_t * is);
OS_API uint8_t dds_stream_read_uint8 (dds_stream_t * is);
OS_API uint16_t dds_stream_read_uint16 (dds_stream_t * is);
OS_API uint32_t dds_stream_read_uint32 (dds_stream_t * is);
OS_API uint64_t dds_stream_read_uint64 (dds_stream_t * is);
OS_API float dds_stream_read_float (dds_stream_t * is);
OS_API double dds_stream_read_double (dds_stream_t * is);
OS_API char * dds_stream_read_string (dds_stream_t * is);
OS_API void dds_stream_read_buffer (dds_stream_t * is, uint8_t * buffer, uint32_t len);

#define dds_stream_read_char(s) ((char) dds_stream_read_uint8 (s))
#define dds_stream_read_int8(s) ((int8_t) dds_stream_read_uint8 (s))
#define dds_stream_read_int16(s) ((int16_t) dds_stream_read_uint16 (s))
#define dds_stream_read_int32(s) ((int32_t) dds_stream_read_uint32 (s))
#define dds_stream_read_int64(s) ((int64_t) dds_stream_read_uint64 (s))

OS_API void dds_stream_write_bool (dds_stream_t * os, bool val);
OS_API void dds_stream_write_uint8 (dds_stream_t * os, uint8_t val);
OS_API void dds_stream_write_uint16 (dds_stream_t * os, uint16_t val);
OS_API void dds_stream_write_uint32 (dds_stream_t * os, uint32_t val);
OS_API void dds_stream_write_uint64 (dds_stream_t * os, uint64_t val);
OS_API void dds_stream_write_float (dds_stream_t * os, float val);
OS_API void dds_stream_write_double (dds_stream_t * os, double val);
OS_API void dds_stream_write_string (dds_stream_t * os, const char * val);
OS_API void dds_stream_write_buffer (dds_stream_t * os, uint32_t len, uint8_t * buffer);

#define dds_stream_write_char(s,v) (dds_stream_write_uint8 ((s), (uint8_t)(v)))
#define dds_stream_write_int8(s,v) (dds_stream_write_uint8 ((s), (uint8_t)(v)))
#define dds_stream_write_int16(s,v) (dds_stream_write_uint16 ((s), (uint16_t)(v)))
#define dds_stream_write_int32(s,v) (dds_stream_write_uint32 ((s), (uint32_t)(v)))
#define dds_stream_write_int64(s,v) (dds_stream_write_uint64 ((s), (uint64_t)(v)))

#undef OS_API

#if defined (__cplusplus)
}
#endif
#endif
