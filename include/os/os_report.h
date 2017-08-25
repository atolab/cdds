/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef OS_REPORT_H
#define OS_REPORT_H

#include <stdarg.h>

#if defined (__cplusplus)
extern "C" {
#endif

    /* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

    /* Subcomponents might need to alter the report before actually handing it over
     * to os_report. Since os_report truncates messages, those components can get
     * away with fixed size buffers as well, but the maximum size must known at
     * that point.
     */
#define OS_REPORT_BUFLEN 1024

    /*
     Note - in the below the check of reportType against os_reportVerbosity is also present
     in os_report. By duplicating it we avoid putting the call onto the stack and evaluating
     args if not necessary.
     */

#define OS_REPORT(type,context,code,...) \
(((type) >= os_reportVerbosity) ? os_report((type),(context),__FILE__,__LINE__,(code),__VA_ARGS__) : (void)0)


#define OS_DEBUG(context,code,...) OS_REPORT(OS_REPORT_DEBUG,(context),(code),##__VA_ARGS__)
#define OS_INFO(context,code,...) OS_REPORT(OS_REPORT_INFO,(context),(code),##__VA_ARGS__)
#define OS_WARNING(context,code,...) OS_REPORT(OS_REPORT_WARNING,(context),(code),##__VA_ARGS__)
#define OS_ERROR(context,code,...) OS_REPORT(OS_REPORT_ERROR,(context),(code),##__VA_ARGS__)
#define OS_CRITICAL(context,code,...) OS_REPORT(OS_REPORT_CRITICAL,(context),(code),##__VA_ARGS__)
#define OS_FATAL(context,code,...) OS_REPORT(OS_REPORT_FATAL,(context),(code),##__VA_ARGS__)

#define OS_REPORT_STACK() \
os_report_stack()

#define OS_REPORT_FLUSH(condition) \
os_report_flush((condition), OS_FUNCTION, __FILE__, __LINE__)

    /**
     * These types are an ordered series of incremental 'importance' (to the user)
     * levels.
     * @see os_reportVerbosity
     */
    typedef enum os_reportType {
        OS_REPORT_DEBUG,
        OS_REPORT_INFO,
        OS_REPORT_WARNING,
        OS_REPORT_ERROR,
        OS_REPORT_FATAL,
        OS_REPORT_CRITICAL,
        OS_REPORT_NONE
    } os_reportType;

    OSAPI_EXPORT extern os_reportType os_reportVerbosity;

    OSAPI_EXPORT void
    os_reportInit(bool forceReInit);

    OSAPI_EXPORT void
    os_reportExit(void);

    /** \brief Report message
     *
     * Consider this function private. It should be invoked by reporting functions
     * specified in the language bindings only.
     *
     * @param type type of report
     * @param context context in which report was generated, often function name
     *                from which function was invoked
     * @param path path of file from which function was invoked
     * @param line line of file from which function was invoked
     * @param code error code associated with the report
     * @param format message to log
     * @param ... Parameter to log
     */

    // SAL annotation
    OSAPI_EXPORT void
    os_report(
              os_reportType type,
              const char *context,
              const char *path,
              int32_t line,
              int32_t code,
              const char *format,
              ...) __attribute_format__((printf,6,7));

    /*****************************************
     * Report stack related functions
     *****************************************/

    /**
     * The os_report_stack operation enables a report stack for the current thread.
     * The stack will be disabled again by the os_report_flush operation.
     */
    OSAPI_EXPORT void
    os_report_stack(
                    void);

    /**
     * The os_report_stack_free operation frees all memory allocated by the current
     * thread for the report stack.
     */
    OSAPI_EXPORT void
    os_report_stack_free(
                         void);

    /**
     * The os_report_flush operation removes the report message from the stack,
     * and if valid is TRUE also writes them into the report device.
     * This operation additionally disables the stack.
     */
    // SAL annotation
    OSAPI_EXPORT void
    os_report_flush(
                    bool valid,
                    const char *context,
                    const char *file,
                    const int line);

#if defined (__cplusplus)
}
#endif

#endif /* OS_REPORT_H */
