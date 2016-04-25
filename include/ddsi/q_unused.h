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
#ifndef NN_UNUSED_H
#define NN_UNUSED_H

#ifdef __GNUC__
#define UNUSED_ARG(x) x __attribute__ ((unused))
#else
#define UNUSED_ARG(x) x
#endif

#ifndef NDEBUG
#define UNUSED_ARG_NDEBUG(x) x
#else
#define UNUSED_ARG_NDEBUG(x) UNUSED_ARG (x)
#endif

#endif /* NN_UNUSED_H */
