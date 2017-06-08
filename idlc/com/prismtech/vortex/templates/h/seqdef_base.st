seqdef_base (name, scope, type, arraydim, str_size) ::= <<
typedef struct <scopedname(...)>
{
  uint32_t _maximum;
  uint32_t _length;
  <type> (*_buffer)<str_size><arraydim>;
  bool _release;
} <scopedname(...)>;

<allocs(...)>

#define <scopedname(...)>_allocbuf(l) \
((<type> (*)<str_size><arraydim>) dds_alloc ((l) * sizeof (<type><str_size><arraydim>)))

>>
