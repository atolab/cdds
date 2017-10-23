//
//  porting.h
//  ospli-osplo
//
//  Created by Erik Boasson on 11-02-2015.
//  Copyright (c) 2015 PrismTech. All rights reserved.
//

#ifndef __ospli_osplo__porting__
#define __ospli_osplo__porting__

#ifndef __GNUC__
#define __attribute__(x)
#endif

#if __SunOS_5_10 && ! defined NEED_STRSEP
#define NEED_STRSEP 1
#endif

#if NEED_STRSEP
char *strsep (char **str, const char *sep);
#endif

#endif /* defined(__ospli_osplo__porting__) */
