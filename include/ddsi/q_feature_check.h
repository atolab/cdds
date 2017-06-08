/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                   $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
/* Feature macros:

   - SSM: support for source-specific multicast
     requires: NETWORK_PARTIITONS
     also requires platform support; SSM is silently disabled if the
     platform doesn't support it

   - BANDWIDTH_LIMITING: transmit-side bandwidth limiting
     requires: NETWORK_CHANNELS (for now, anyway)

   - IPV6: support for IPV6
     requires: platform support (which itself is not part of DDSI)

   - NETWORK_PARTITIONS: support for multiple network partitions

   - NETWORK_CHANNELS: support for multiple network channels

*/

#ifdef DDSI_INCLUDE_SSM
  #ifndef DDSI_INCLUDE_NETWORK_PARTITIONS
    #error "SSM requires NETWORK_PARTITIONS"
  #endif

  #include "os/os_socket.h"
  #ifndef OS_SOCKET_HAS_SSM
    #error "OS_SOCKET_HAS_SSM should be defined"
  #elif ! OS_SOCKET_HAS_SSM
    #undef DDSI_INCLUDE_SSM
  #endif
#endif

#ifdef DDSI_INCLUDE_BANDWIDTH_LIMITING
  #ifndef DDSI_INCLUDE_NETWORK_CHANNELS
    #error "BANDWIDTH_LIMITING requires NETWORK_CHANNELS"
  #endif
#endif
