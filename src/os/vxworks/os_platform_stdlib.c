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
#include <string.h>
#include <unistd.h>
#include <hostLib.h> /* MAXHOSTNAMELEN */

#include "os/os.h"

#ifdef _WRS_KERNEL
#include <envLib.h>

extern char *os_environ[];

void
os_stdlibInitialize(
    void)
{
   char **varset;

   for ( varset = &os_environ[0]; *varset != NULL; varset++ )
   {
      char *savePtr=NULL;
      char *varName;
      char *tmp = os_strdup( *varset );
      varName = strtok_r( tmp, "=", &savePtr );
      if ( os_getenv( varName ) == NULL )
      {
         os_putenv( *varset );
      }
      os_free(tmp);
   }
}
#endif

#define OS_HAS_STRTOK_R 1 /* FIXME: Should be handled by CMake */
#include "../snippets/code/os_gethostname.c"
#include "../snippets/code/os_stdlib.c"
#include "../snippets/code/os_stdlib_bsearch.c"
#include "../snippets/code/os_stdlib_strtod.c"
#include "../snippets/code/os_stdlib_strtol.c"
#include "../snippets/code/os_stdlib_strtok_r.c"
