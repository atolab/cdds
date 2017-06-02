#ifndef CUNITRUNNER_RUNNER_H
#define CUNITRUNNER_RUNNER_H

#include "CUnit/CUnit.h"
#include "cunitrunner_export.h"

#if defined (__cplusplus)
extern "C" {
#endif

CUNITRUNNER_EXPORT CU_ErrorCode runner_init (int argc, char* argv[]);
CUNITRUNNER_EXPORT CU_ErrorCode runner_run (void);
CUNITRUNNER_EXPORT void runner_fini (void);

#if defined (__cplusplus)
}
#endif
#endif /* CUNITRUNNER_RUNNER_H */
