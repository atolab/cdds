#include "dds.h"
#include "os/os.h"
#include "os/os_report.h"
#include "kernel/dds_report.h"
#include <criterion/criterion.h>


//Test fixtures
static dds_entity_t g_participant = 0;
static const char * error_file_name;
static const char * info_file_name;
static os_result result;

static void
report_init(void)
{
    g_participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(g_participant, 0, "Failed to create prerequisite g_participant");

    result = os_putenv("VORTEX_ERRORFILE=vdds_test_error");
    cr_assert_eq(result, os_resultSuccess);
    os_putenv("VORTEX_INFOFILE=vdds_test_info");
    cr_assert_eq(result, os_resultSuccess);

    error_file_name = os_getenv("VORTEX_ERRORFILE");
    info_file_name = os_getenv("VORTEX_INFOFILE");

    // check existance
    result= os_access(error_file_name, OS_ROK);
    cr_assert_eq(result, os_resultFail);
    result = os_access(info_file_name, OS_ROK);
    cr_assert_eq(result, os_resultFail);
}

static void
report_fini(void)
{
    dds_delete(g_participant);
    //remove logs
    os_remove(error_file_name);
    os_remove(info_file_name);
}

Test(vddsc_report, warning, .init=report_init, .fini=report_fini)
{
    DDS_REPORT_STACK();
    DDS_WARNING(0, "os_report-info-test %d", __LINE__);
    DDS_REPORT_FLUSH(true);

    // check existance
    result= os_access(error_file_name, OS_ROK);
    cr_assert_eq(result, os_resultFail);
    result = os_access(info_file_name, OS_ROK);
    cr_assert_eq(result, os_resultSuccess);
}

Test(vddsc_report, error, .init=report_init, .fini=report_fini)
{
    DDS_REPORT_STACK();
    DDS_ERROR(0, "os_report-info-test %d", __LINE__);
    DDS_REPORT_FLUSH(true);

    // check existance
    result= os_access(error_file_name, OS_ROK);
    cr_assert_eq(result, os_resultSuccess);
    result = os_access(info_file_name, OS_ROK);
    cr_assert_eq(result, os_resultFail);

}
