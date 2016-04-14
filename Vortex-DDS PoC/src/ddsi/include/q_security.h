#ifdef DDSI_INCLUDE_ENCRYPTION

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
#ifndef Q_SECURITY_H
#define Q_SECURITY_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Generic class */
C_CLASS(q_securityEncoderSet);
C_CLASS(q_securityDecoderSet);

/* Set of supported ciphers */
typedef enum 
{
  Q_CIPHER_UNDEFINED,
  Q_CIPHER_NULL,
  Q_CIPHER_BLOWFISH,
  Q_CIPHER_AES128,
  Q_CIPHER_AES192,
  Q_CIPHER_AES256,
  Q_CIPHER_NONE,
  Q_CIPHER_MAX
} q_cipherType;

void ddsi_security_plugin (void);

#if defined (__cplusplus)
}
#endif

#endif
#endif
