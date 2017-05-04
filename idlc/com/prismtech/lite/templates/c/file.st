file (banner, name, nameupper, declarations, dll, includes) ::= <<
<banner>
<if(dll)><dll><endif>
#include "<name>.h"

<declarations; separator="\n">
>>
