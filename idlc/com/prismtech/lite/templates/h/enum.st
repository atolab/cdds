enum (name, scope, values) ::= <<
typedef enum <scopedname(...)>
{
  <values:{ v | <scopedname(scope,v)>}; separator=",\n">
} <scopedname(...)>;

<allocs(...)>
>>
