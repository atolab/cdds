seqdef (name, scope, type, typescope) ::= <<
typedef struct <scopedname(...)>
{
  uint32_t _maximum;
  uint32_t _length;
  <scopedname(scope=typescope,name=type)> *_buffer;
  bool _release;
} <scopedname(...)>;

<allocs(...)>

#define <scopedname(...)>_allocbuf(l) \
((<scopedname(scope=typescope,name=type)> *) dds_alloc ((l) * sizeof (<scopedname(scope=typescope,name=type)>)))

>>
