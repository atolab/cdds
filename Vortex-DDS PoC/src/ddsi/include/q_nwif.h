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
#ifndef Q_NWIF_H
#define Q_NWIF_H

#include "os_socket.h"
#include "q_protocol.h" /* for nn_locator_t */

#if defined (__cplusplus)
extern "C" {
#endif

#define INET6_ADDRSTRLEN_EXTENDED (INET6_ADDRSTRLEN + 8) /* 13: '[' + ']' + ':' + PORT */
#define MAX_INTERFACES 128
struct nn_interface {
  os_sockaddr_storage addr;
  os_sockaddr_storage netmask;
  unsigned if_index;
  unsigned mc_capable: 1;
  unsigned point_to_point: 1;
  char *name;
};

struct nn_group_membership;

void nn_loc_to_address (os_sockaddr_storage *dst, const nn_locator_t *src);
void nn_address_to_loc (nn_locator_t *dst, const os_sockaddr_storage *src, int32_t kind);

char *sockaddr_to_string_no_port (char addrbuf[INET6_ADDRSTRLEN_EXTENDED], const os_sockaddr_storage *src);
char *sockaddr_to_string_with_port (char addrbuf[INET6_ADDRSTRLEN_EXTENDED], const os_sockaddr_storage *src);
char *locator_to_string_no_port (char addrbuf[INET6_ADDRSTRLEN_EXTENDED], const nn_locator_t *loc);
char *locator_to_string_with_port (char addrbuf[INET6_ADDRSTRLEN_EXTENDED], const nn_locator_t *loc);
void print_sockerror (const char *msg);
int make_socket (os_socket *socket, unsigned short port, bool stream, bool reuse);
int find_own_ip (const char *requested_address);
unsigned short get_socket_port (os_socket socket);
struct nn_group_membership *new_group_membership (void);
void free_group_membership (struct nn_group_membership *mship);
int join_mcgroups (struct nn_group_membership *mship, os_socket socket, const os_sockaddr_storage *srcip, const os_sockaddr_storage *mcip);
int leave_mcgroups (struct nn_group_membership *mship, os_socket socket, const os_sockaddr_storage *srcip, const os_sockaddr_storage *mcip);
void sockaddr_set_port (os_sockaddr_storage *addr, unsigned short port);
unsigned short sockaddr_get_port (const os_sockaddr_storage *addr);
#if LITE
unsigned sockaddr_to_hopefully_unique_uint32 (const os_sockaddr_storage *src);
#endif

#if defined (__cplusplus)
}
#endif

#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
void set_socket_diffserv (os_socket sock, int diffserv);
#endif

#endif /* Q_NWIF_H */
