struct (name, scope, fields, extern, alignment, keys, flags, declarations, marshalling, xml, istopic) ::= <<

<declarations; separator="\n">

typedef struct <scopedname(...)>
{
  <fields; separator="\n">
} <scopedname(...)>;

<if(istopic)>
<extern> const dds_topic_descriptor_t <scopedname(...)>_desc;

<allocs(...)>

#define <scopedname(...)>_free(d,o) \
dds_sample_free ((d), &<scopedname(...)>_desc, (o))
<endif>
>>
