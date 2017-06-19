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
#ifndef UT_XMLPARSER_H
#define UT_XMLPARSER_H

#include "os/os.h"

#if defined (__cplusplus)
extern "C" {
#endif

    typedef int (*ut_xmlpProcElemOpen_t) (void *varg, uintptr_t parentinfo, uintptr_t *eleminfo, const char *name);
    typedef int (*ut_xmlpProcAttr_t) (void *varg, uintptr_t eleminfo, const char *name, const char *value);
    typedef int (*ut_xmlpProcElemData_t) (void *varg, uintptr_t eleminfo, const char *data);
    typedef int (*ut_xmlpProcElemClose_t) (void *varg, uintptr_t eleminfo);
    typedef void (*ut_xmlpError) (void *varg, const char *msg, int line);

    struct ut_xmlpCallbacks {
        ut_xmlpProcElemOpen_t elem_open;
        ut_xmlpProcAttr_t attr;
        ut_xmlpProcElemData_t elem_data;
        ut_xmlpProcElemClose_t elem_close;
        ut_xmlpError error;
    };

    struct ut_xmlpState;

    OSAPI_EXPORT struct ut_xmlpState *ut_xmlpNewFile (FILE *fp, void *varg, const struct ut_xmlpCallbacks *cb);
    OSAPI_EXPORT struct ut_xmlpState *ut_xmlpNewString (const char *string, void *varg, const struct ut_xmlpCallbacks *cb);
    OSAPI_EXPORT void ut_xmlpFree (struct ut_xmlpState *st);
    OSAPI_EXPORT int ut_xmlpParse (struct ut_xmlpState *st);

    OSAPI_EXPORT int ut_xmlUnescapeInsitu (char *buffer, size_t *n);

#if defined (__cplusplus)
}
#endif

#endif
