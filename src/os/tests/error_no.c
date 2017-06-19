#include "dds.h"
#include "CUnit/Runner.h"
#include "os/os.h"

#define ENABLE_TRACING 0

CUnit_Suite_Initialize(error_no)
{
    int result = DDS_RETCODE_OK;
    os_osInit();
  #if ENABLE_TRACING
    printf("Run suite_abstraction_errno_init\n");
  #endif

    return result;
}

CUnit_Suite_Cleanup(error_no)
{
    int result = DDS_RETCODE_OK;

  #if ENABLE_TRACING
    printf("Run suite_abstraction_errno_clean\n");
  #endif
    os_osExit();
    return result;
}

CUnit_Test(error_no, get_and_set)
{
  #if ENABLE_TRACING
    printf ("Starting tc_os_errno_001\n");
  #endif
    os_setErrno (0);
    CU_ASSERT (os_getErrno () == 0);

  #if ENABLE_TRACING
    printf ("Starting tc_os_errno_002\n");
  #endif
    os_setErrno (0);
    /* Call strtol with an invalid format on purpose. */
    (void)strtol ("1000000000000000000000000000000000000000000000000", NULL, 10);
    CU_ASSERT (os_getErrno () != 0);

  #if ENABLE_TRACING
    printf ("Starting tc_os_errno_003\n");
  #endif

  #if ENABLE_TRACING
    printf ("Ending tc_os_errno\n");
  #endif
}
