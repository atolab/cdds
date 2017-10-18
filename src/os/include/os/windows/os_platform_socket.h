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

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#if defined (__cplusplus)
extern "C" {
#endif

#define OS_SOCK_VERSION         2
#define OS_SOCK_REVISION        0

/* Keep defines before common header */
#define OS_SOCKET_HAS_IPV6      1
#define OS_IFNAMESIZE           128
#define OS_SOCKET_HAS_SA_LEN    0
#define OS_NO_SIOCGIFINDEX      1
#define OS_NO_NETLINK           1

#if defined NTDDI_VERSION && defined _WIN32_WINNT_WS03 && NTDDI_VERSION >= _WIN32_WINNT_WS03
#define OS_SOCKET_HAS_SSM 1
#else
#define OS_SOCKET_HAS_SSM 0
#endif

#define IFF_POINTOPOINT     IFF_POINTTOPOINT

#define os_sockEAGAIN       WSAEWOULDBLOCK /* Operation would block, or a timeout expired before operation succeeded */
#define os_sockEWOULDBLOCK  WSAEWOULDBLOCK /* Operation would block */
#define os_sockENOMEM       WSABASEERR
#define os_sockENOSR        WSABASEERR
#define os_sockENOENT       WSABASEERR
#define os_sockEPERM        WSABASEERR
#define os_sockEINTR        WSAEINTR
#define os_sockEBADF        WSAEBADF
#define os_sockEACCES       WSAEACCES
#define os_sockEINVAL       WSAEINVAL
#define os_sockEMFILE       WSAEMFILE
#define os_sockENOTSOCK     WSAENOTSOCK
#define os_sockEMSGSIZE     WSAEMSGSIZE
#define os_sockENOPROTOOPT  WSAENOPROTOOPT
#define os_sockEPROTONOSUPPORT  WSAEPROTONOSUPPORT
#define os_sockEADDRINUSE   WSAEADDRINUSE
#define os_sockEADDRNOTAVAIL    WSAEADDRNOTAVAIL
#define os_sockEHOSTUNREACH WSAEHOSTUNREACH
#define os_sockENOBUFS      WSAENOBUFS
#define os_sockECONNRESET   WSAECONNRESET   /* Connection reset by peer */

    typedef SOCKET os_socket;
    #define PRIsock PRIuPTR
#define OS_SOCKET_INVALID (-1)

	void os_socketModuleInit(void);
	void os_socketModuleExit(void);

#if defined (__cplusplus)
}
#endif

#endif /* OS_PLATFORM_SOCKET_H */
