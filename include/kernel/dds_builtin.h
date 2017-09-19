#ifndef _DDS_BUILTIN_H_
#define _DDS_BUILTIN_H_

#include "ddsi/q_time.h"
#include "dds_builtinTopics.h"


#if defined (__cplusplus)
extern "C"
{
#endif

void
dds__builtin_participant(
        DDS_ParticipantBuiltinTopicData *data,
        nn_wctime_t timestamp);

void
dds__builtin_cmparticipant(
        DDS_CMParticipantBuiltinTopicData *data,
        nn_wctime_t timestamp);

void
dds__builtin_init(
        void);

void
dds__builtin_fini(
        void);

#if defined (__cplusplus)
}
#endif

#endif /* _DDS_BUILTIN_H_ */

