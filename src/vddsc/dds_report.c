#include "dds.h"
#include "os/os.h"
#include "os/os_report.h"
#include <assert.h>
#include <string.h>
#include "kernel/dds_types.h"
#include "kernel/dds_report.h"

void
dds_report(
    os_reportType reportType,
    const char *function,
    int32_t line,
    const char *file,
    dds_retcode_t code,
    const char *format,
    ...)
{
    const char *retcode = NULL;
    /* os_report truncates messages to <OS_REPORT_BUFLEN> bytes */
    char buffer[OS_REPORT_BUFLEN];
    size_t offset = 0;
    va_list args;

    assert (function != NULL);
    assert (file != NULL);
    assert (format != NULL);
    /* probably never happens, but you can never be to sure */
    assert (OS_REPORT_BUFLEN > 0);

    retcode = dds_err_str(code*-1);
    offset = strlen(retcode);
    assert (offset < OS_REPORT_BUFLEN);
    (void)memcpy(buffer, retcode, offset);
    buffer[offset] = ' ';
    offset++;

    va_start (args, format);
    (void)os_vsnprintf (buffer + offset, sizeof(buffer) - offset, format, args);
    va_end (args);
    os_report (reportType, function, file, line, code, "%s", buffer);
}

