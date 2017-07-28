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
#ifndef _DDSI_SSL_H_
#define _DDSI_SSL_H_

#include "ddsi/q_feature_check.h"

#ifdef DDSI_INCLUDE_SSL

#ifdef _WIN32
/* WinSock2 must be included before openssl headers
   otherwise winsock will be used */
#include <WinSock2.h>
#endif

#include <openssl/ssl.h>

void ddsi_ssl_plugin (void);

#endif
#endif
