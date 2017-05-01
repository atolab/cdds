file (banner, name, nameupper, declarations, dll, includes) ::= <<
<banner>

#include "dds.h"
<includes:{n | #include "<n>.h"}; separator="\n">

#ifndef _DDSL_<nameupper>_H_
#define _DDSL_<nameupper>_H_

<if(dll)><dll><endif>

#ifdef __cplusplus
extern "C" {
#endif

<declarations; separator="\n">
#ifdef __cplusplus
}
#endif
<if(dll)>#undef DDS_EXPORT<endif>
#endif /* _DDSL_<nameupper>_H_ */

>>
