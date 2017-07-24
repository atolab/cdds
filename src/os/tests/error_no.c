#include "CUnit/Runner.h"
#include "os/os.h"


CUnit_Suite_Initialize(os_errno)
{
    int result = 0;
    os_osInit();
    printf("Run os_errno_Initialize\n");

    return result;
}

CUnit_Suite_Cleanup(os_errno)
{
    int result = 0;
    os_osExit();
    printf("Run os_errno_Cleanup\n");

    return result;
}

CUnit_Test(os_errno, get_and_set)
{
    printf ("Starting os_errno_get_and_set_001\n");
    os_setErrno (0);
    CU_ASSERT (os_getErrno () == 0);

    printf ("Starting os_errno_get_and_set_002\n");
    os_setErrno (0);
    /* Call strtol with an invalid format on purpose. */
    (void)strtol ("1000000000000000000000000000000000000000000000000", NULL, 10);
    CU_ASSERT (os_getErrno () != 0);

    printf ("Ending tc_os_errno\n");
}
