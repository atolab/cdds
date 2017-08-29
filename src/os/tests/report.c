#include "CUnit/Runner.h"
#include "os/os.h"

#include <stdio.h>

CUnit_Suite_Initialize(os_report)
{
  os_putenv("VORTEX_ERRORFILE=vdds_test_error");
  os_putenv("VORTEX_INFOFILE=vdds_test_info");

  os_reportInit(true);

  return 0;
}

void remove_logs()
{
  const char * error_file_name = os_getenv("VORTEX_ERRORFILE");
  const char * info_file_name = os_getenv("VORTEX_INFOFILE");

  os_remove(error_file_name);
  os_remove(info_file_name);
}

void check_existence(os_result error_log_existence, os_result info_log_existence)
{
  const char * error_file_name = os_getenv("VORTEX_ERRORFILE");
  const char * info_file_name = os_getenv("VORTEX_INFOFILE");

  CU_ASSERT(os_access(error_file_name, OS_ROK) == error_log_existence);
  CU_ASSERT(os_access(info_file_name, OS_ROK) == info_log_existence);
}


CUnit_Suite_Cleanup(os_report)
{
  os_reportExit();
  remove_logs();

  return 0;
}

CUnit_Test(os_report, stack_critical)
{
  check_existence(os_resultFail, os_resultFail);

  OS_REPORT_STACK();

  OS_CRITICAL(OS_FUNCTION, 0, "os_report-error-test %d", 123);

  check_existence(os_resultFail, os_resultFail);

  OS_REPORT_FLUSH(false);

  // Since a critical is logged, the error log should be created
  check_existence(os_resultSuccess, os_resultFail);

  remove_logs();
}

CUnit_Test(os_report, stack_non_critical)
{
  check_existence(os_resultFail, os_resultFail);

  OS_REPORT_STACK();

  OS_ERROR(OS_FUNCTION, 0, "os_report-error-test %d", 123);

  check_existence(os_resultFail, os_resultFail);

  OS_REPORT_FLUSH(false);

  // Since a non critical is logged, the error log should not be created
  check_existence(os_resultFail, os_resultFail);

  remove_logs();
}

CUnit_Test(os_report, error_file_creation_critical)
{
  check_existence(os_resultFail, os_resultFail);

  OS_CRITICAL(OS_FUNCTION, 0, "os_report-critical-test %d", 123);

  check_existence(os_resultSuccess, os_resultFail);

  remove_logs();
}

CUnit_Test(os_report, error_file_creation_fatal)
{
  check_existence(os_resultFail, os_resultFail);

  OS_FATAL(OS_FUNCTION, 0, "os_report-fatal-test %d", 123);

  check_existence(os_resultSuccess, os_resultFail);

  remove_logs();
}

CUnit_Test(os_report, info_file_creation_warning)
{
  check_existence(os_resultFail, os_resultFail);

  OS_WARNING(OS_FUNCTION, 0, "os_report-warning-test %d", 123);

  check_existence(os_resultFail, os_resultSuccess);

  remove_logs();
}

CUnit_Test(os_report, info_file_creation_info)
{
  check_existence(os_resultFail, os_resultFail);

  OS_INFO(OS_FUNCTION, 0, "os_report-info-test %d", 123);

  check_existence(os_resultFail, os_resultSuccess);

  remove_logs();
}
