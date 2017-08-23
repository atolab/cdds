#include "CUnit/Runner.h"
#include "os/os.h"

#include <stdio.h>

#define ERROR_FILE "vdds_test_error"
#define INFO_FILE  "vdds_test_info"

CUnit_Suite_Initialize(os_report)
{
  os_putenv("VORTEX_ERRORFILE=vdds_test_error");
  os_putenv("VORTEX_INFOFILE=vdds_test_info");

  os_reportInit(true);

  return 0;
}

void remove_logs()
{
  char * error_file_name = os_reportGetErrorFileName();
  char * info_file_name = os_reportGetInfoFileName();

  os_remove(error_file_name);
  os_remove(info_file_name);

  os_free(error_file_name);
  os_free(info_file_name);
}

void check_existence(os_result error_log_existence, os_result info_log_existence)
{
  char * error_file_name = os_reportGetErrorFileName();
  char * info_file_name = os_reportGetInfoFileName();

  CU_ASSERT(os_access(error_file_name, OS_ROK) == error_log_existence);
  CU_ASSERT(os_access(info_file_name, OS_ROK) == info_log_existence);

  os_free(error_file_name);
  os_free(info_file_name);
}


CUnit_Suite_Cleanup(os_report)
{
  os_reportExit();
  remove_logs();

  return 0;
}

CUnit_Test(os_report, os_report_stack_critical)
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

CUnit_Test(os_report, os_report_stack_non_critical)
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


CUnit_Test(os_report, os_report_set_verbosity)
{
  os_reportSetVerbosity("NONE");

  check_existence(os_resultFail, os_resultFail);

  OS_CRITICAL(OS_FUNCTION, 0, "os_report-critical-test %d", 123);

  check_existence(os_resultFail, os_resultFail);

  remove_logs();
}

CUnit_Test(os_report, os_report_error_file_creation_critical)
{
  check_existence(os_resultFail, os_resultFail);

  OS_CRITICAL(OS_FUNCTION, 0, "os_report-critical-test %d", 123);

  check_existence(os_resultSuccess, os_resultFail);

  remove_logs();
}

CUnit_Test(os_report, os_report_error_file_creation_fatal)
{
  check_existence(os_resultFail, os_resultFail);

  OS_FATAL(OS_FUNCTION, 0, "os_report-fatal-test %d", 123);

  check_existence(os_resultSuccess, os_resultFail);

  remove_logs();
}

CUnit_Test(os_report, os_reportGetErrorFileName)
{
  char * error_file_report;
  char * error_file_check = os_malloc(strlen(ERROR_FILE) + 3);
  char * error_file_normalized;

  error_file_report = os_reportGetErrorFileName();

  sprintf(error_file_check, "./%s", os_getenv("VORTEX_ERRORFILE"));

  error_file_normalized = os_fileNormalize(error_file_check);

  CU_ASSERT(strcmp (error_file_normalized, error_file_report) == 0);

  os_free(error_file_report);
  os_free(error_file_check);
  os_free(error_file_normalized);
}

CUnit_Test(os_report, os_reportGetInfoFileName)
{
  char * info_file_report;
  char * info_file_check = os_malloc(strlen(INFO_FILE) + 3);
  char * info_file_normalized;

  info_file_report = os_reportGetInfoFileName();

  sprintf(info_file_check, "./%s", os_getenv("VORTEX_INFOFILE"));

  info_file_normalized = os_fileNormalize(info_file_check);

  CU_ASSERT(strcmp (info_file_normalized, info_file_report) == 0);

  os_free(info_file_report);
  os_free(info_file_check);
  os_free(info_file_normalized);
}
