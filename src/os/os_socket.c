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

/****************************************************************
 * Implementation for socket services conforming to             *
 * OpenSplice requirements                                      *
 ****************************************************************/

/** \file os/code/os_socket.c
 *  \brief socket management
 */

#include <assert.h>
#include <string.h>
#include "os/os.h"

#if (OS_SOCKET_HAS_IPV6 == 1)
#ifndef _VXWORKS
const os_in6_addr os_in6addr_any = IN6ADDR_ANY_INIT;
const os_in6_addr os_in6addr_loopback = IN6ADDR_LOOPBACK_INIT;
#else
const os_in6_addr os_in6addr_any = { { 0 } };
const os_in6_addr os_in6addr_loopback = { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 } } };
#endif
#endif

#ifndef OS_INET_NTOP
#define OS_INET_NTOP inet_ntop
#endif

#ifndef OS_INET_PTON
#define OS_INET_PTON inet_pton
#endif

static
void os__sockaddrInit4(os_sockaddr* sa)
{
    assert(sa);
    /* 0 is a valid value for all members besides sa_family */
    memset(sa, 0, sizeof(os_sockaddr_in));
    sa->sa_family = AF_INET;
}


#if (OS_SOCKET_HAS_IPV6 == 1)
static
void os__sockaddrInit6(os_sockaddr* sa)
{
    assert(sa);
    /* 0 is a valid value for all members besides sa_family */
    memset(sa, 0, sizeof(os_sockaddr_in6));
    sa->sa_family = AF_INET6;
}
#endif


/**
 * Initialises the memory pointed to by sa. The address family
 * will be set correctly according to isIPv4.
 * @param sa Pointer to the os_sockaddr to be initialised
 * @param isIPv4 Flag indicating whether *sa will be IPv4 or IPv6. If
 * IPv6 is not supported but this flag is FALSE, sa will be initialised
 * as IPv4 and an API-warning is logged.
 * @pre sa != NULL
 * @return os_resultSuccess on successful initialisation, os_resultInvalid
 * if isIPv4 was FALSE but IPv6 is not supported.
 * @post sa is initialised
 * @note Please be aware that the memory will be memset; make sure that
 * enough memory for the requested address kind is available. Allocating
 * os_sockaddr_storage always suffices.
 */
os_result
os_sockaddrInit(os_sockaddr* sa,
                bool isIPv4)
{
    os_result result = os_resultSuccess;

    assert(sa);
    if (!isIPv4)
    {
#if (OS_SOCKET_HAS_IPV6 == 1)
        os__sockaddrInit6(sa);
#else
        OS_REPORT(OS_ERROR,"os_sockaddrInit", 0,
                  "Unsupported parameter value: IPV6 address requested but not supported by this platform");
        os__sockaddrInit4(sa);
        result = os_resultInvalid;
#endif
    }
    else
    {
        os__sockaddrInit4(sa);
    }

    return result;
}


/**
* Compare two socket IP host addresses for equality - does not consider the port number.
* This is a 'straight' equal i.e. family must match and address bytes
* must correspond. So it will not consider the possibility of IPv6 mapped
* IPv4 addresses or anything arcane like that.
* @param thisSock First address
* @param thatSock Second address.
* @return true if equal, false otherwise.
*/
bool
os_sockaddrIPAddressEqual(const os_sockaddr* thisSock,
                           const os_sockaddr* thatSock)
{
    bool result = false;
#if (OS_SOCKET_HAS_IPV6 == 1)
    os_sockaddr_in6 * thisV6, * thatV6;
#endif

    if (thisSock->sa_family == thatSock->sa_family)
    {
        if (thisSock->sa_family == AF_INET)
        {
            /* IPv4 */
            result = (((os_sockaddr_in*)thisSock)->sin_addr.s_addr ==
                     ((os_sockaddr_in*)thatSock)->sin_addr.s_addr ?
                     true: false);
        }
#if (OS_SOCKET_HAS_IPV6 == 1)
        else
        {
            /* IPv6 */
            thisV6 = (os_sockaddr_in6*) thisSock;
            thatV6 = (os_sockaddr_in6*) thatSock;
            result = (memcmp(&thisV6->sin6_addr.s6_addr, &thatV6->sin6_addr.s6_addr, sizeof(unsigned char) * 16) ?
                     false : true);
        }
#endif
    }
    return result;
}

int
os_sockaddrIpAddressCompare(const os_sockaddr* addr1,
                            const os_sockaddr* addr2)
{
    int result = -1;
    uint16_t port1, port2;
    int r;

#if (OS_SOCKET_HAS_IPV6 == 1)
    os_sockaddr_in6 * thisV6, * thatV6;
#endif

    if (addr1->sa_family == addr2->sa_family)
    {
        if (addr1->sa_family == AF_INET)
        {
            /* IPv4 */
            if (((os_sockaddr_in*)addr1)->sin_addr.s_addr ==
                ((os_sockaddr_in*)addr2)->sin_addr.s_addr) {
                port1 = os_sockaddrGetPort(addr1);
                port2 = os_sockaddrGetPort(addr2);
                if (port1 == port2) {
                    result = 0;
                } else {
                    if (port1 > port2) {
                        result = 1;
                    } else {
                        result = -1;
                    }
                }
            } else {
                if (((os_sockaddr_in*)addr1)->sin_addr.s_addr >
                    ((os_sockaddr_in*)addr2)->sin_addr.s_addr) {
                    result = 1;
                } else {
                    result = -1;
                }
            }
        }
#if (OS_SOCKET_HAS_IPV6 == 1)
        else
        {
            /* IPv6 */
            thisV6 = (os_sockaddr_in6*) addr1;
            thatV6 = (os_sockaddr_in6*) addr2;
            r = memcmp(&thisV6->sin6_addr.s6_addr, &thatV6->sin6_addr.s6_addr, sizeof(unsigned char) * 16);
            if (r == 0) {
                port1 = os_sockaddrGetPort(addr1);
                port2 = os_sockaddrGetPort(addr2);
                if (port1 == port2) {
                    result = 0;
                } else {
                    if (port1 > port2) {
                        result = 1;
                    } else {
                        result = -1;
                    }
                }
            } else {
                if (r > 0) {
                    result = 1;
                } else {
                    result = -1;
                }
            }
        }
#endif
    }
    return result;
}

/**
* Checks two socket IP host addresses for be on the same subnet, considering the given subnetmask.
* It will not consider the possibility of IPv6 mapped IPv4 addresses or anything arcane like that.
* @param thisSock First address
* @param thatSock Second address.
* @param mask Subnetmask.
* @return true if equal, false otherwise.
*/
bool
os_sockaddrSameSubnet(const os_sockaddr* thisSock,
                           const os_sockaddr* thatSock,
                           const os_sockaddr* mask)
{
    bool result = false;
#if (OS_SOCKET_HAS_IPV6 == 1)
    os_sockaddr_in6 thisV6, thatV6, *maskV6;
#endif

    if (thisSock->sa_family == thatSock->sa_family &&
        thisSock->sa_family == mask->sa_family)
    {
        if (thisSock->sa_family == AF_INET)
        {
            /* IPv4 */
            result = ((((os_sockaddr_in*)thisSock)->sin_addr.s_addr & ((os_sockaddr_in*)mask)->sin_addr.s_addr ) ==
                     (((os_sockaddr_in*)thatSock)->sin_addr.s_addr & ((os_sockaddr_in*)mask)->sin_addr.s_addr) ?
                     true: false);
        }
#if (OS_SOCKET_HAS_IPV6 == 1)
        else
        {
            size_t i, size;
            /* IPv6 */
            memcpy(&thisV6, thisSock, sizeof(thisV6));
            memcpy(&thatV6, thatSock, sizeof(thatV6));
            maskV6 = (os_sockaddr_in6*) mask;
            size = sizeof(thisV6.sin6_addr.s6_addr);
            for (i=0; i < size; i++) {
                thisV6.sin6_addr.s6_addr[i] &= maskV6->sin6_addr.s6_addr[i];
                thatV6.sin6_addr.s6_addr[i] &= maskV6->sin6_addr.s6_addr[i];
            }
            result = (memcmp(&thisV6.sin6_addr.s6_addr, &thatV6.sin6_addr.s6_addr, size) ?
                     false : true);
        }
#endif
    }
    return result;
}

/**
* Convert the provided addressString into a os_sockaddr.
* @return true on successful conversion. false otherwise
* @param addressString The string representation of a network address.
* @param addressOut A pointer to an os_sockaddr. Must be big enough for
* the address type specified by the string. This implies it should
* generally be the address of an os_sockaddr_storage for safety's sake.
* @param isIPv4 Iff the addressString is a hostname specifies whether
* and IPv4 address should be returned. If false an Ipv6 address will be
* requested. If the address is in either valid decimal presentation format
* param will be ignored.
*/
bool
os_sockaddrStringToAddress(const char *addressString,
                           os_sockaddr* addressOut,
                           bool isIPv4)
{
    bool result = false;
    os_sockaddr_storage buf;

    assert(addressOut);

    if (strchr(addressString, ':') == NULL) {
        if (OS_INET_PTON(
            AF_INET, addressString, ((os_sockaddr *)&buf)) == 1)
        {
            (void)memset(addressOut, 0, sizeof(os_sockaddr_in));
            ((os_sockaddr_in *)addressOut)->sin_family = AF_INET;
            ((os_sockaddr_in *)addressOut)->sin_addr =
                 ((os_sockaddr_in *)&buf)->sin_addr;
            result = true;
        }
    } else {
        if (OS_INET_PTON(
            AF_INET6, addressString, ((os_sockaddr *)&buf)))
        {
            (void)memset(addressOut, 0, sizeof(os_sockaddr_in6));
            ((os_sockaddr_in6 *)addressOut)->sin6_family = AF_INET6;
            ((os_sockaddr_in6 *)addressOut)->sin6_addr =
                ((os_sockaddr_in6 *)&buf)->sin6_addr;
            result = true;
        }
    }

    if (!result)
    {
#ifdef DO_HOST_BY_NAME
        struct addrinfo template;
        struct addrinfo *resultList;
        int retCode;

        memset (&template, 0, sizeof(template));
        template.ai_family = (isIPv4 ? AF_INET : AF_INET6);
        template.ai_socktype = SOCK_DGRAM;

        retCode = getaddrinfo(addressString, NULL, &template, &resultList);
        if (retCode != 0)
        {
            OS_REPORT(OS_DEBUG,"os_sockaddrStringToAddress", 0,
                "error calling getaddrinfo(\"%s\"): %s",
                addressString, gai_strerror(retCode));
        }
        else
        {
            if (resultList)
            {
                memcpy(addressOut, resultList->ai_addr, resultList->ai_addrlen);
                /* Ignore other entries, just take first */
                freeaddrinfo(resultList);
                result = true;

            }
            else
            {
                OS_REPORT(OS_DEBUG,"os_sockaddrStringToAddress", 0,
                      "could not lookup host \"%s\"",
                      addressString);
            }
        }
#else
        OS_UNUSED_ARG(isIPv4);
#endif
    }

    return result;
}

/**
* Check this address to see if it represents loopback.
* @return true if it does. false otherwise, or if unknown address type.
* @param thisSock A pointer to an os_sockaddr to be checked.
*/
bool
os_sockaddrIsLoopback(const os_sockaddr* thisSock)
{
    bool result = false;

#if (OS_SOCKET_HAS_IPV6 == 1)
    static os_sockaddr_storage linkLocalLoopback;
    static os_sockaddr* linkLocalLoopbackPtr = NULL;

    if (linkLocalLoopbackPtr == NULL)
    {
        /* Initialise once (where 'once' implies some small integer) */
        os_sockaddrStringToAddress("fe80::1", (os_sockaddr*) &linkLocalLoopback, false /* ! ipv4 */ );
        linkLocalLoopbackPtr = (os_sockaddr*) &linkLocalLoopback;
    }

    if (thisSock->sa_family == AF_INET6)
    {
        result = IN6_IS_ADDR_LOOPBACK(&((os_sockaddr_in6*)thisSock)->sin6_addr) ||
                 os_sockaddrIPAddressEqual(thisSock, linkLocalLoopbackPtr) ? true : false;
    }
    else
#endif
    if (thisSock->sa_family == AF_INET)
    {
        result = (INADDR_LOOPBACK == ntohl(((os_sockaddr_in*)thisSock)->sin_addr.s_addr)) ? true : false;
    }

    return result;
}

size_t
os_sockaddrSizeof(
    const os_sockaddr* sa)
{
    size_t result;
    assert(sa);
    switch(sa->sa_family){
#if (OS_SOCKET_HAS_IPV6 == 1)
        case AF_INET6:
            result = sizeof(os_sockaddr_in6);
            break;
#endif
        case AF_INET:
            result = sizeof(os_sockaddr_in);
            break;
        default:
#if (OS_SOCKET_HAS_IPV6 == 1)
            OS_REPORT(OS_ERROR,"os_sockaddrSizeof", 0,
                "Unkown address family specified: %d. Should be AF_INET (%d) or AF_INET6 (%d)",
                (int)sa->sa_family, AF_INET, AF_INET6);
#else
            OS_REPORT(OS_ERROR,"os_sockaddrSizeof", 0,
                "Unkown address family specified: %d. Should be AF_INET (%d)",
                (int)sa->sa_family, AF_INET);
#endif
            result = 0;
            break;
    }
    return result;
}


void
os_sockaddrSetInAddrAny(
    os_sockaddr* sa)
{
    assert(sa);
#if (OS_SOCKET_HAS_IPV6 == 1)
    assert(sa->sa_family == AF_INET6 || sa->sa_family == AF_INET);
    if (sa->sa_family == AF_INET6){
        ((os_sockaddr_in6*)sa)->sin6_addr = os_in6addr_any;
        ((os_sockaddr_in6*)sa)->sin6_scope_id = 0;
    }
    else
#else
    assert(sa->sa_family == AF_INET);
#endif
    if (sa->sa_family == AF_INET){
        ((os_sockaddr_in*)sa)->sin_addr.s_addr = htonl(INADDR_ANY);
    }
}


void
os_sockaddrSetPort(
    os_sockaddr* sa,
    uint16_t port /* network byte order */)
{
    assert(sa);
#if (OS_SOCKET_HAS_IPV6 == 1)
    assert(sa->sa_family == AF_INET6 || sa->sa_family == AF_INET);
    if (sa->sa_family == AF_INET6)
    {
        ((os_sockaddr_in6*)sa)->sin6_port = port;
    }
    else
#else
    assert(sa->sa_family == AF_INET);
#endif
    if (sa->sa_family == AF_INET)
    {
        ((os_sockaddr_in*)sa)->sin_port = port;
    }
}

uint16_t /* network byte order */
os_sockaddrGetPort(
  const os_sockaddr* const sa)
{
    uint16_t port = 0;
    assert(sa);
#if (OS_SOCKET_HAS_IPV6 == 1)
    assert(sa->sa_family == AF_INET6 || sa->sa_family == AF_INET);
    if (sa->sa_family == AF_INET6)
    {
        port = (uint16_t)((os_sockaddr_in6*)sa)->sin6_port;
    }
    else
#else
    assert(sa->sa_family == AF_INET);
#endif
    if (sa->sa_family == AF_INET)
    {
        port = (uint16_t)((os_sockaddr_in*)sa)->sin_port;
    }

    return port;
}


char*
os_sockaddrAddressToString(const os_sockaddr* sa,
                            char* buffer, size_t buflen)
{
#if defined (WIN32) || (WINCE)
    socklen_t structLength;
    int errorCode;

    strncpy(buffer, "Unknown address family", buflen);

    structLength = (sa->sa_family == AF_INET6 ? sizeof (os_sockaddr_in6) : sizeof (os_sockaddr_in));
    if (errorCode = getnameinfo(sa,
                   structLength,
                   buffer,
                   (DWORD)buflen,
                   NULL,
                   0,
                   NI_NUMERICHOST))
    {
                char errmsg[1024];
                int errNo = os_getErrno();
                (void)os_strerror_r(errNo, errmsg, sizeof errmsg);

        OS_REPORT(OS_ERROR,"os_sockaddrAddressToString", 0,
                "error calling getnameinfo to stringify network address. Error: %d (%s)",
                errNo, errmsg);
    }
#else
    assert (buflen <= 0x7fffffff);
    switch(sa->sa_family) {
        case AF_INET:
            OS_INET_NTOP(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                    buffer, (socklen_t) buflen);
            break;

#if (OS_SOCKET_HAS_IPV6 == 1)
        case AF_INET6:
            OS_INET_NTOP(AF_INET6, &(((os_sockaddr_in6 *)sa)->sin6_addr),
                    buffer, (socklen_t) buflen);
            break;
#endif

        default:
            snprintf(buffer, buflen, "Unknown address family");
    }
#endif
    return buffer;
}

char*
os_sockaddrAddressPortToString(
  const os_sockaddr* sa,
  char* buffer,
  size_t buflen)
{
  size_t pos;
  switch (sa->sa_family)
  {
    case AF_INET:
      os_sockaddrAddressToString (sa, buffer, buflen);
      pos = strlen (buffer);
      (void) snprintf (buffer + pos, buflen - pos, ":%hu", ntohs (((os_sockaddr_in *) sa)->sin_port));
      break;
#if OS_SOCKET_HAS_IPV6
    case AF_INET6:
      if(buflen){
          buffer[0] = '[';
          os_sockaddrAddressToString (sa, buffer + 1, buflen - 1);
          pos = strlen (buffer);
          (void) snprintf (buffer + pos, buflen - pos, "]:%hu", ntohs (((os_sockaddr_in6 *) sa)->sin6_port));
      }
      break;
#endif
    default:
      (void) snprintf(buffer, buflen, "Unknown address family");
      break;
  }
  return buffer;
}
