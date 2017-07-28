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

#include "ddsi/ddsi_tran.h"

#ifdef DDSI_INCLUDE_SSL

#include "ddsi/ddsi_ssl.h"

struct ddsi_ssl_plugins
{
  void (*config) (void);
  bool (*init) (void);
  void (*fini) (void);
  void (*ssl_free) (SSL * ssl);
  void (*bio_vfree) (BIO * bio);
  ssize_t (*read) (SSL * ssl, void * buf, size_t len, int * err);
  ssize_t (*write) (SSL * ssl, const void * msg, size_t len, int * err);
  SSL * (*connect) (os_socket sock);
  BIO * (*listen) (os_socket sock);
  SSL * (*accept) (BIO * bio, os_socket * sock);
};

struct ddsi_ssl_plugins ddsi_tcp_ssl_plugin;

#endif

int ddsi_tcp_init (void);

#endif
