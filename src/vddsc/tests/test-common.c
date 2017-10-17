#include "dds.h"

const char*
entity_kind_str(dds_entity_t ent) {
    if(ent <= 0) {
        return "(ERROR)";
    }
    switch(ent & DDS_ENTITY_KIND_MASK) {
        case DDS_KIND_TOPIC:        return "Topic";
        case DDS_KIND_PARTICIPANT:  return "Participant";
        case DDS_KIND_READER:       return "Reader";
        case DDS_KIND_WRITER:       return "Writer";
        case DDS_KIND_SUBSCRIBER:   return "Subscriber";
        case DDS_KIND_PUBLISHER:    return "Publisher";
        case DDS_KIND_COND_READ:    return "ReadCondition";
        case DDS_KIND_COND_QUERY:   return "QueryCondition";
        case DDS_KIND_WAITSET:      return "WaitSet";
        default:                    return "(INVALID_ENTITY)";
    }
}
