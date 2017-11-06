#ifndef OS_DEFS_H
#define OS_DEFS_H

#define OS_LITTLE_ENDIAN 1
#define OS_BIG_ENDIAN 2

#if OS_ENDIANNESS != OS_LITTLE_ENDIAN && OS_ENDIANNESS != OS_BIG_ENDIAN
#error "OS_ENDIANNESS not set correctly"
#endif
#ifndef OS_HAS_UCONTEXT_T
#error "OS_HAS_UCONTEXT_T not set"
#endif
#ifndef OS_SOCKET_USE_FCNTL
#error "OS_SOCKET_USE_FCNTL must be defined for this platform."
#endif
#ifndef OS_SOCKET_USE_IOCTL
#error "OS_SOCKET_USE_IOCTL must be defined for this platform."
#endif
#if (OS_SOCKET_USE_IOCTL == 1) && (OS_SOCKET_USE_FCNTL == 1)
#error "this platform must set only one of OS_SOCKET_USE_IOCTL and OS_SOCKET_USE_FCNTL to 1"
#endif
#ifndef OS_FILESEPCHAR
#error "OS_FILESEPCHAR must be defined for this platform."
#endif

#include "os/os_decl_attributes.h"
#include <limits.h>

#if defined (__cplusplus)
extern "C" {
#endif

    /* \brief OS_FUNCTION provides undecorated function name of current function
     *
     * Behavior of OS_FUNCTION outside a function is undefined. Note that
     * implementations differ across compilers and compiler versions. It might be
     * implemented as either a string literal or a constant variable.
     */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#   define OS_FUNCTION __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#   define OS_FUNCTION __func__
#elif defined(__GNUC__)
#   define OS_FUNCTION __FUNCTION__
#elif defined(__clang__)
#   define OS_FUNCTION __FUNCTION__
#elif defined(__ghs__)
#   define OS_FUNCTION __FUNCTION__
#elif (defined(__SUNPRO_C) || defined(__SUNPRO_CC))
    /* Solaris Studio had support for __func__ before it supported __FUNCTION__.
       Compiler flag -features=extensions is required on older versions. */
#   define OS_FUNCTION __func__
#elif defined(__FUNCTION__)
    /* Visual Studio */
#   define OS_FUNCTION __FUNCTION__
#elif defined(__vxworks)
    /* At least versions 2.9.6 and 3.3.4 of the GNU C Preprocessor only define
       __GNUC__ if the entire GNU C compiler is in use. VxWorks 5.5 targets invoke
       the preprocessor separately resulting in __GNUC__ not being defined. */
#   define OS_FUNCTION __FUNCTION__
#else
#   warning "OS_FUNCTION is not supported"
#endif

    /* \brief OS_PRETTY_FUNCTION provides function signature of current function
     *
     * See comments on OS_FUNCTION for details.
     */
#if defined(__GNUC__)
#   define OS_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(__clang__)
#   define OS_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(__ghs__)
#   define OS_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif (defined(__SUNPRO_C) && __SUNPRO_C >= 0x5100)
    /* Solaris Studio supports __PRETTY_FUNCTION__ in C since version 12.1 */
#   define OS_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif (defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x5120)
    /* Solaris Studio supports __PRETTY_FUNCTION__ in C++ since version 12.3 */
#   define OS_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
    /* Visual Studio */
#   define OS_PRETTY_FUNCTION __FUNCSIG__
#elif defined(__vxworks)
    /* See comments on __vxworks macro above. */
#   define OS_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
    /* Do not warn user about OS_PRETTY_FUNCTION falling back to OS_FUNCTION.
       #   warning "OS_PRETTY_FUNCTION is not supported, using OS_FUNCTION"
    */
#   define OS_PRETTY_FUNCTION OS_FUNCTION
#endif

#if defined(__GNUC__)
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#define OSPL_GCC_DIAG_STR(s) #s
#define OSPL_GCC_DIAG_JOINSTR(x,y) OSPL_GCC_DIAG_STR(x ## y)
#define OSPL_GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
#define OSPL_GCC_DIAG_PRAGMA(x) OSPL_GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#define OS_WARNING_GNUC_OFF(x) OSPL_GCC_DIAG_PRAGMA(push) OSPL_GCC_DIAG_PRAGMA(ignored OSPL_GCC_DIAG_JOINSTR(-W,x))
#define OS_WARNING_GNUC_ON(x) OSPL_GCC_DIAG_PRAGMA(pop)
#else
#define OS_WARNING_GNUC_OFF(x) OSPL_GCC_DIAG_PRAGMA(ignored OSPL_GCC_DIAG_JOINSTR(-W,x))
#define OS_WARNING_GNUC_ON(x)  OSPL_GCC_DIAG_PRAGMA(warning OSPL_GCC_DIAG_JOINSTR(-W,x))
#endif
#else
#define OS_WARNING_GNUC_OFF(x)
#define OS_WARNING_GNUC_ON(x)
#endif
#else
#define OS_WARNING_GNUC_OFF(x)
#define OS_WARNING_GNUC_ON(x)
#endif

#ifdef _MSC_VER

#define OS_WARNING_MSVC_OFF(x)               \
__pragma (warning(push))             \
__pragma (warning(disable: ## x))

#define OS_WARNING_MSVC_ON(x)               \
__pragma (warning(pop))
#else
#define OS_WARNING_MSVC_OFF(x)
#define OS_WARNING_MSVC_ON(x)
#endif

/**
 * \brief Calculate maximum value of an integer type
 *
 * A somewhat complex, but efficient way to calculate the maximum value of an
 * integer type at compile time.
 *
 * For unsigned numerical types the first part up to XOR is enough. The second
 * part is to make up for signed numerical types.
 */
#define OS_MAX_INTEGER(T) \
    ((T)(((T)~0) ^ ((T)!((T)~0 > 0) << (CHAR_BIT * sizeof(T) - 1))))
/**
 * \brief Calculate minimum value of an integer type
 */
#define OS_MIN_INTEGER(T) \
    ((-OS_MAX_INTEGER(T)) - 1)

#if !defined (OS_UNUSED_ARG)
#define OS_UNUSED_ARG(a) (void) (a)
#endif

#ifdef __GNUC__
#define OS_UNUSED_PAR(x) x __attribute__ ((unused))
#else
#define OS_UNUSED_PAR(x) x
#endif

    /** \brief Time structure definition
     */
    typedef struct os_time {
        /** Seconds since the Unix epoch; 1-jan-1970 00:00:00 (UTC) */
        os_timeSec tv_sec;
        /** Number of nanoseconds since the Unix epoch, modulo 10^9. */
        int32_t tv_nsec;
        /** os_time can be used for a duration type with the following
            semantics for negative durations: tv_sec specifies the
            sign of the duration, tv_nsec is always positive and added
            to the real value (thus real value is tv_sec+tv_nsec/10^9,
            for example { -1, 500000000 } is -0.5 seconds) */
    } os_time;

    /** \brief Types on which we define atomic operations.  The 64-bit
     *  types are always defined, even if we don't really support atomic
     *   operations on them.
     */
    typedef struct { uint32_t v; } os_atomic_uint32_t;
    typedef struct { uint64_t v; } os_atomic_uint64_t;
    typedef struct { uintptr_t v; } os_atomic_uintptr_t;
    typedef os_atomic_uintptr_t os_atomic_voidp_t;

    /** \brief Initializers for the types on which atomic operations are
        defined.
    */
#define OS_ATOMIC_UINT32_INIT(v) { (v) }
#define OS_ATOMIC_UINT64_INIT(v) { (v) }
#define OS_ATOMIC_UINTPTR_INIT(v) { (v) }
#define OS_ATOMIC_VOIDP_INIT(v) { (uintptr_t) (v) }

    /** \brief Definition of the service return values */
    typedef _Return_type_success_(return == os_resultSuccess) enum os_result {
        /** The service is successfully completed */
        os_resultSuccess,
        /** A resource was not found */
        os_resultUnavailable,
        /** The service is timed out */
        os_resultTimeout,
        /** The requested resource is busy */
        os_resultBusy,
        /** An invalid argument is passed */
        os_resultInvalid,
        /** The operating system returned a failure */
        os_resultFail
    } os_result;

    /* We want to inline these, but we don't want to emit an exernally
     visible symbol for them and we don't want warnings if we don't use
     them.

     It appears as if a plain "inline" will do just that in C99.

     In traditional GCC one had to use "extern inline" to achieve that
     effect, but that will cause an externally visible symbol to be
     emitted by a C99 compiler.

     Starting with GCC 4.3, GCC conforms to the C99 standard if
     compiling in C99 mode, unless -fgnu89-inline is specified. It
     defines __GNUC_STDC_INLINE__ if "inline"/"extern inline" behaviour
     is conforming the C99 standard.

     So: GCC >= 4.3: choose between "inline" & "extern inline" based
     upon __GNUC_STDC_INLINE__; for GCCs < 4.2, rely on the traditional
     GCC behaiour; and for other compilers assume they behave conforming
     the standard if they advertise themselves as C99 compliant (use
     "inline"), and assume they do not support the inline keywords
     otherwise.

     GCC when not optimizing ignores "extern inline" functions. So we
     need to distinguish between optimizing & non-optimizing ... */

    /* Defining OS_HAVE_INLINE is a supported way of overruling this file */
#ifndef OS_HAVE_INLINE

#if __STDC_VERSION__ >= 199901L
#  /* C99, but old GCC nonetheless doesn't implement C99 semantics ... */
#  if __GNUC__ && ! defined __GNUC_STDC_INLINE__
#    define OS_HAVE_INLINE 1
#    define VDDS_INLINE extern __inline__
#  else
#    define OS_HAVE_INLINE 1
#    define VDDS_INLINE inline
#  endif
#elif defined __STDC__ && defined __GNUC__ && ! defined __cplusplus
#  if __OPTIMIZE__
#    if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)
#      ifdef __GNUC_STDC_INLINE__
#        define OS_HAVE_INLINE 1
#        define VDDS_INLINE __inline__
#      else
#        define OS_HAVE_INLINE 1
#        define VDDS_INLINE extern __inline__
#      endif
#    else
#      define OS_HAVE_INLINE 1
#      define VDDS_INLINE extern __inline__
#    endif
#  endif
#endif

#if ! OS_HAVE_INLINE
#define VDDS_INLINE
#endif

#endif /* not defined OS_HAVE_INLINE */

#if defined(_MSC_VER)
    /* Thread-local storage using __declspec(thread) on Windows versions before
       Vista and Server 2008 works in DLLs if they are bound to the executable,
       it does not work if the library is loaded using LoadLibrary. */
#define os_threadLocal __declspec(thread)
#elif defined(__GNUC__) || (defined(__clang__) && __clang_major__ >= 2)
    /* GCC supports Thread-local storage for x86 since version 3.3. Clang
       supports Thread-local storage since version 2.0. */
    /* VxWorks 7 supports __thread for both GCC and DIAB, older versions may
       support it as well, but that is not verified. */
#define os_threadLocal __thread
#else
#error "os_threadLocal is not supported"
#endif

#if defined (__cplusplus)
}
#endif

#endif
