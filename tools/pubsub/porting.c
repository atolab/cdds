//
//    porting.c
//    ospli-osplo
//
//    Created by Erik Boasson on 11-02-2015.
//    Copyright (c) 2015 PrismTech. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "porting.h"

#if NEED_STRSEP
char *strsep(char **str, const char *sep) {
    char *ret;
    if (*str == NULL)
        return NULL;
    ret = *str;
    while (**str && strchr(sep, **str) == 0)
        (*str)++;
    if (**str == '\0') {
        *str = NULL;
    } else {
        **str = '\0';
        (*str)++;
    }
    return ret;
}
#endif
