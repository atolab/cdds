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
#include "os/os_stdlib.h"

char *os_strsep (char **str, const char *sep)
{
  char *ret;
  if (**str == '\0')
    return 0;
  ret = *str;
  while (**str && strchr (sep, **str) == 0)
    (*str)++;
  if (**str != '\0')
  {
    **str = '\0';
    (*str)++;
  }
  return ret;
}
