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
#include <assert.h>
#include <string.h>
#include <unistd.h>
#ifndef __VXWORKS__
#include <sys/fcntl.h>
#endif
#include "os/os.h"

os_socket
os_sockNew(
    int domain,
    int type)
{
    return socket (domain, type, 0);
}

os_result
os_sockBind(
    os_socket s,
    const struct sockaddr *name,
    uint32_t namelen)
{
    os_result result = os_resultSuccess;

    if (bind(s, (struct sockaddr *)name, (unsigned)namelen) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_result
os_sockGetsockname(
    os_socket s,
    const struct sockaddr *name,
    uint32_t namelen)
{
    os_result result = os_resultSuccess;
    socklen_t len = namelen;

    if (getsockname(s, (struct sockaddr *)name, &len) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_result
os_sockSendto(
    os_socket s,
    const void *msg,
    size_t len,
    const struct sockaddr *to,
    size_t tolen,
    size_t *bytesSent)
{
    ssize_t res;
    assert (tolen <= 0x7fffffff);
    res = sendto(s, msg, len, 0, to, (socklen_t) tolen);
    if (res < 0)
    {
        *bytesSent = 0;
        return os_resultFail;
    }
    else
    {
        *bytesSent = (size_t) res;
        return os_resultSuccess;
    }
}

os_result
os_sockRecvfrom(
    os_socket s,
    void *buf,
    size_t len,
    struct sockaddr *from,
    size_t *fromlen,
    size_t *bytesRead)
{
    ssize_t res;
    socklen_t fl;
    assert (*fromlen <= 0x7fffffff);
    fl = (socklen_t) *fromlen;
    res = recvfrom(s, buf, len, 0, from, &fl);
    if (res < 0)
    {
        *bytesRead = 0;
        return os_resultFail;
    }
    else
    {
        *fromlen=fl;
        *bytesRead = (size_t)res;
        return os_resultSuccess;
    }
}

os_result
os_sockGetsockopt(
    os_socket s,
    int32_t level,
    int32_t optname,
    void *optval,
    uint32_t *optlen)
{
    int res;
    socklen_t ol = *optlen;
    res = getsockopt(s, level, optname, optval, &ol);
    *optlen = ol;
    return ( res == -1 ? os_resultFail : os_resultSuccess );
}

os_result
os_sockSetsockopt(
    os_socket s,
    int32_t level,
    int32_t optname,
    const void *optval,
    uint32_t optlen)
{
    os_result result = os_resultSuccess;

    if (optname == SO_SNDBUF || optname == SO_RCVBUF)
    {
      if (optlen == 4 && *((unsigned *) optval) == 0)
      {
        /* We know this won't work */
        return os_resultSuccess;
      }
    }
    else if (optname == SO_DONTROUTE)
    {
        /* Enabling DONTROUTE gives nothing but grief on MacOS (e.g., no multicasting),
           and I'm not aware of any serious use-case. */
        return os_resultSuccess;
    }

    if (setsockopt(s, level, optname, optval, optlen) == -1) {
        result = os_resultFail;
    }

#ifdef __APPLE__
    if (result == os_resultSuccess && level == SOL_SOCKET && optname == SO_REUSEADDR)
    {
       if (setsockopt(s, level, SO_REUSEPORT, optval, optlen) == -1)
       {
          result = os_resultFail;
       }
    }
#endif

    return result;
}

os_result
os_sockSetNonBlocking(
    os_socket s,
    bool nonblock)
{
    int oldflags;
    os_result r;

    assert(nonblock == false || nonblock == true);

    oldflags = fcntl(s, F_GETFL, 0);
    if(oldflags >= 0){
        if (nonblock){
            oldflags |= O_NONBLOCK;
        } else {
            oldflags &= ~O_NONBLOCK;
        }
        if(fcntl (s, F_SETFL, oldflags) == 0){
            r = os_resultSuccess;
        } else {
            r = os_resultFail;
        }
    } else {
        switch(os_getErrno()){
            case EAGAIN:
                r = os_resultBusy;
                break;
            case EBADF:
                r = os_resultInvalid;
                break;
            default:
                r = os_resultFail;
                break;
        }
    }

    return r;
}

os_result
os_sockFree(
    os_socket s)
{
    os_result result = os_resultSuccess;

    if (close(s) == -1) {
        result = os_resultFail;
    }
    return result;
}

int32_t
os_sockSelect(
    int32_t nfds,
    fd_set *readfds,
    fd_set *writefds,
    fd_set *errorfds,
    os_time *timeout)
{
    struct timeval t;
    int r;

    t.tv_sec = timeout->tv_sec;
    t.tv_usec = timeout->tv_nsec / 1000;
    r = select(nfds, readfds, writefds, errorfds, &t);

    return r;
}

_Check_return_
static os_result
os_queryInterfaceAttributesIPv4 (_In_ const struct ifaddrs *ifa, _Inout_ os_ifAttributes *ifElement)
{
  os_result result = os_resultSuccess;
  strncpy (ifElement->name, ifa->ifa_name, OS_IFNAMESIZE);
  ifElement->name[OS_IFNAMESIZE - 1] = '\0';
  memcpy (&ifElement->address, ifa->ifa_addr, sizeof (os_sockaddr_in));
  ifElement->flags = ifa->ifa_flags;
  if (ifElement->flags & IFF_BROADCAST)
    memcpy (&ifElement->broadcast_address, ifa->ifa_broadaddr, sizeof (os_sockaddr_in));
  else
    memset (&ifElement->broadcast_address, 0, sizeof (ifElement->broadcast_address));
  memcpy (&ifElement->network_mask, ifa->ifa_netmask, sizeof (os_sockaddr_in));
  ifElement->interfaceIndexNo = (unsigned) if_nametoindex (ifa->ifa_name);
  return result;
}

_Check_return_
static os_result
os_queryInterfaceAttributesIPv6 (_In_ const struct ifaddrs *ifa, _Inout_ os_ifAttributes *ifElement)
{
  os_result result = os_resultSuccess;
  strncpy (ifElement->name, ifa->ifa_name, OS_IFNAMESIZE);
  ifElement->name[OS_IFNAMESIZE - 1] = '\0';
  memcpy (&ifElement->address, ifa->ifa_addr, sizeof (os_sockaddr_in6));
  ifElement->flags = ifa->ifa_flags;
  memset (&ifElement->broadcast_address, 0, sizeof (ifElement->broadcast_address));
  memset (&ifElement->network_mask, 0, sizeof (ifElement->network_mask));
  ifElement->interfaceIndexNo = (unsigned) if_nametoindex (ifa->ifa_name);
  return result;
}

os_result os_sockQueryInterfaces (os_ifAttributes *ifList, uint32_t listSize, uint32_t *validElements)
{
  os_result result = os_resultSuccess;
  unsigned int listIndex;
  struct ifaddrs *ifa_first, *ifa;
  if (getifaddrs (&ifa_first) != 0)
  {
    perror ("getifaddrs");
    return os_resultFail;
  }
  listIndex = 0;
  for (ifa = ifa_first; ifa && listIndex < listSize; ifa = ifa->ifa_next)
  {
    if (ifa->ifa_addr && ((struct sockaddr_in *) ifa->ifa_addr)->sin_family == AF_INET)
    {
      /* Get other interface attributes */
      result = os_queryInterfaceAttributesIPv4 (ifa, &ifList[listIndex]);
      if (result == os_resultSuccess)
        listIndex++;
    }

    if (result == os_resultSuccess)
      *validElements = listIndex;
  }
  freeifaddrs (ifa_first);
  return result;
}

os_result os_sockQueryIPv6Interfaces (os_ifAttributes *ifList, uint32_t listSize, uint32_t *validElements)
{
  struct ifaddrs* interfaceList = NULL;
  struct ifaddrs* nextInterface = NULL;
  unsigned int listIndex = 0;

  *validElements = 0;
  if (getifaddrs (&interfaceList) != 0)
  {
    return os_resultFail;
  }

  nextInterface = interfaceList;
  while (nextInterface != NULL && listIndex < listSize)
  {
    if (nextInterface->ifa_addr && nextInterface->ifa_addr->sa_family == AF_INET6)
    {
      os_sockaddr_in6 *v6Address = (os_sockaddr_in6 *) nextInterface->ifa_addr;
      if (!IN6_IS_ADDR_UNSPECIFIED (&v6Address->sin6_addr))
      {
        os_result result = os_resultSuccess;
        result = os_queryInterfaceAttributesIPv6 (nextInterface, &ifList[listIndex]);
        if (result == os_resultSuccess)
          listIndex++;
      }
    }
    nextInterface = nextInterface->ifa_next;
  }
  *validElements = listIndex;
  freeifaddrs(interfaceList);
  return os_resultSuccess;
}

