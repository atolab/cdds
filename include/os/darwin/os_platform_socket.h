/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifndef OS_PLATFORM_SOCKET_H
#define OS_PLATFORM_SOCKET_H

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <sys/select.h>
#include <sys/sockio.h>
#include <unistd.h>

#include <ifaddrs.h>

#if defined (__cplusplus)
extern "C" {
#endif

/* Keep defines before common header */
#define OS_SOCKET_HAS_IPV6      1
#define OS_IFNAMESIZE           IF_NAMESIZE
#define OS_SOCKET_HAS_SA_LEN    1
#define OS_NO_SIOCGIFINDEX      1
#define OS_NO_NETLINK           1
#define OS_SOCKET_HAS_SSM       1

#define os_sockEAGAIN       EAGAIN      /* Operation would block, or a timeout expired before operation succeeded */
#define os_sockEWOULDBLOCK  EWOULDBLOCK /* Operation would block */
#define os_sockEPERM        EPERM       /* Operation not permitted */
#define os_sockENOENT       ENOENT      /* No such file or directory */
#define os_sockEINTR        EINTR       /* Interrupted system call */
#define os_sockEBADF        EBADF       /* Bad file number */
#define os_sockENOMEM       ENOMEM      /* Out of memory */
#define os_sockEACCES       EACCES      /* Permission denied */
#define os_sockEINVAL       EINVAL      /* Invalid argument */
#define os_sockEMFILE       EMFILE          /* Too many open files */
#define os_sockENOSR        ENOSR       /* Out of streams resources */
#define os_sockENOTSOCK     ENOTSOCK    /* Socket operation on non-socket */
#define os_sockEMSGSIZE     EMSGSIZE    /* Message too long */
#define os_sockENOPROTOOPT  ENOPROTOOPT /* Protocol not available */
#define os_sockEPROTONOSUPPORT  EPROTONOSUPPORT /* Protocol not supported */
#define os_sockEADDRINUSE   EADDRINUSE  /* Address already in use */
#define os_sockEADDRNOTAVAIL    EADDRNOTAVAIL   /* Cannot assign requested address */
#define os_sockENETUNREACH  ENETUNREACH /* Network is unreachable */
#define os_sockENOBUFS      ENOBUFS     /* No buffer space available */
#define os_sockECONNRESET   ECONNRESET  /* Connection reset by peer */

    typedef int os_socket; /* signed */
#define OS_SOCKET_INVALID (-1)

#if defined (__cplusplus)
}
#endif

#endif /* OS_PLATFORM_SOCKET_H */
