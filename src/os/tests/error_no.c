#include "dds.h"
#include "cunitrunner/runner.h"
#include "os/os.h"

#define ENABLE_TRACING 0

static int  suite_abstraction_errno_init (void)
{
    int result = DDS_RETCODE_OK;
    os_osInit();
  #if ENABLE_TRACING
    printf("Run suite_abstraction_errno_init\n");
  #endif

    return result;
}

static int suite_abstraction_errno_clean (void)
{
    int result = DDS_RETCODE_OK;

  #if ENABLE_TRACING
    printf("Run suite_abstraction_errno_clean\n");
  #endif
    os_osExit();
    return result;
}

static void tc_os_errno (void)
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
    strtol ("1000000000000000000000000000000000000000000000000", NULL, 10);
    CU_ASSERT (os_getErrno () != 0);

  #if ENABLE_TRACING
    printf ("Starting tc_os_errno_003\n");
  #endif

  #if ENABLE_TRACING
    printf ("Ending tc_os_errno\n");
  #endif
}

int main (int argc, char *argv[])
{
    CU_pSuite suite;
    if(runner_init(argc, argv)){
        goto err_init;
    }
    if((suite = CU_add_suite ("abstraction_errno", suite_abstraction_errno_init, suite_abstraction_errno_clean)) == NULL){
        goto err;
    }
    if(CU_add_test (suite, "tc_os_errno", tc_os_errno) == NULL){
        goto err;
    }
    runner_run();
err:
    runner_fini();
err_init:
    return CU_get_error();
}
