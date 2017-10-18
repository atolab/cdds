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
#ifndef Q_STATIC_ASSERT_H
#define Q_STATIC_ASSERT_H

/* There are many tricks to use a constant expression to yield an
   illegal type or expression at compile time, such as zero-sized
   arrays and duplicate case or enum labels. So this is but one of the
   many tricks. */

#ifndef _MSC_VER

#define Q_STATIC_ASSERT_CODE(pred) do { switch(0) { case 0: case pred: ; } } while (0)

#else

/* Temporarily disabling warning C6326: Potential comparison of a
   constant with another constant. */
#define Q_STATIC_ASSERT_CODE(pred) do {         \
    __pragma (warning (push))                   \
      __pragma (warning (disable : 6326))       \
      switch(0) { case 0: case pred: ; }        \
    __pragma (warning (pop))                    \
  } while (0)

#endif

#endif /* Q_STATIC_ASSERT_H */
