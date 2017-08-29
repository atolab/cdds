#ifndef DDS_REPORT_H
#define DDS_REPORT_H

#include <stdarg.h>
#include "os/os_report.h"

#define DDS_REPORT_STACK()          \
    os_report_stack ()

#define DDS_CRITICAL(...)           \
    dds_report (                    \
        OS_REPORT_CRITICAL,         \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION,                \
        DDS_RETCODE_ERROR,          \
        __VA_ARGS__)

#define DDS_ERROR(code,...)        \
    dds_report (                    \
        OS_REPORT_ERROR,            \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION,                \
        (code),                     \
        __VA_ARGS__)

#define DDS_INFO(...)        \
    dds_report (                    \
        OS_REPORT_INFO,             \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION,                \
        DDS_RETCODE_OK,             \
        __VA_ARGS__)

#define DDS_WARNING(code,...)       \
    dds_report (                    \
        OS_REPORT_WARNING,          \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION,                \
        (code),                     \
        __VA_ARGS__)

#define DDS_REPORT(type, code,...) \
    dds_report (                    \
        type,                       \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION,                \
        (code),                     \
        __VA_ARGS__)

#define DDS_REPORT_FLUSH OS_REPORT_FLUSH

void
dds_report(
    os_reportType reportType,
    const char *file,
    int32_t line,
    const char *function,
    int32_t code,
    const char *format,
    ...);

#endif
