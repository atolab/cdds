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
#ifndef _DDSI_TCP_H_
#define _DDSI_TCP_H_

#include "ddsi_tran.h"

#ifdef DDSI_INCLUDE_SSL

#include "ddsi_ssl.h"

struct ddsi_ssl_plugins
{
  void (*config) (void);
  c_bool (*init) (void);
  void (*fini) (void);
  void (*ssl_free) (SSL * ssl);
  void (*bio_vfree) (BIO * bio);
  os_ssize_t (*read) (SSL * ssl, void * buf, os_size_t len, int * err);
  os_ssize_t (*write) (SSL * ssl, const void * msg, os_size_t len, int * err);
  SSL * (*connect) (os_socket sock);
  BIO * (*listen) (os_socket sock);
  SSL * (*accept) (BIO * bio, os_socket * sock);
};

struct ddsi_ssl_plugins ddsi_tcp_ssl_plugin;

#endif

int ddsi_tcp_init (void);

#endif
