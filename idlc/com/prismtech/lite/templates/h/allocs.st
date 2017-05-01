allocs (scope, name) ::= <<
#define <scopedname(...)>__alloc() \
((<scopedname(...)>*) dds_alloc (sizeof (<scopedname(...)>)));
>>
