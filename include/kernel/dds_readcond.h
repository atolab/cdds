#ifndef _DDS_READCOND_H_
#define _DDS_READCOND_H_

#include "kernel/dds_entity.h"

_Must_inspect_result_ dds_readcond*
dds_create_readcond(
        _In_ dds_reader *rd,
        _In_ dds_entity_kind_t kind,
        _In_ uint32_t mask);

#endif
