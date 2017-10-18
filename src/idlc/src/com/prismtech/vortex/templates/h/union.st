union (name, scope, disc, discscope, fields, declarations) ::= <<

<declarations; separator="\n">

typedef struct <scopedname(...)>
{
  <ctype(discscope, disc)> _d;
  union
  {
    <fields; separator="\n">
  } _u;
} <scopedname(...)>;

<allocs(...)>
>>
