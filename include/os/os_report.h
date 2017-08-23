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

    /* following new line macro can be used in report description to insert
     * a new line and indent to align next line.
     */
#define OS_REPORT_NL "\n              "

    /*
     Note - in the below the check of reportType against os_reportVerbosity is also present
     in os_report. By duplicating it we avoid putting the call onto the stack and evaluating
     args if not necessary.
     */
#define OS_REPORT(type,context,code,...) \
(((type) >= os_reportVerbosity) ? os_report((type),(context),__FILE__,__LINE__,(code),__VA_ARGS__) : (void)0)


#define OS_DEBUG(context,code,...) OS_REPORT(OS_DEBUG_TYPE,(context),(code),##__VA_ARGS__)
#define OS_INFO(context,code,...) OS_REPORT(OS_INFO_TYPE,(context),(code),##__VA_ARGS__)
#define OS_WARNING(context,code,...) OS_REPORT(OS_WARNING_TYPE,(context),(code),##__VA_ARGS__)
#define OS_ERROR(context,code,...) OS_REPORT(OS_ERROR_TYPE,(context),(code),##__VA_ARGS__)
#define OS_CRITICAL(context,code,...) OS_REPORT(OS_CRITICAL_TYPE,(context),(code),##__VA_ARGS__)
#define OS_FATAL(context,code,...) OS_REPORT(OS_FATAL_TYPE,(context),(code),##__VA_ARGS__)

#define OS_REPORT_STACK() \
os_report_stack()

#define OS_REPORT_FLUSH(condition) \
os_report_flush((condition), OS_FUNCTION, __FILE__, __LINE__)

#define OS_REPORT_DUMPSTACK() \
os_report_dumpStack(OS_FUNCTION, __FILE__, __LINE__)

    /**
     * 'Base' of all ::os_report event data structures.
     * It's anticipated that the contents might be added to so it's desirable
     * to do something to enable various version compatibility
     */
    struct os_reportEventBase_s
    {
        /** The version of the data struct in use */
        uint32_t version;
    };
    /**
     * Generic ::os_report event data pointer
     */
    typedef struct os_reportEventBase_s* os_reportEvent;

    /**
     * These types are an ordered series of incremental 'importance' (to the user)
     * levels.
     * @see os_reportVerbosity
     */
    typedef enum os_reportType {
        OS_DEBUG_TYPE,
        OS_INFO_TYPE,
        OS_WARNING_TYPE,
        OS_ERROR_TYPE,
        OS_CRITICAL_TYPE,
        OS_FATAL_TYPE,
        OS_NONE_TYPE
    } os_reportType;

    /**
     * The information that is made available to a plugged in logger
     * via its TypedReport symbol.
     */
    struct os_reportEventV1_s
    {
        /** The version of this struct i.e. 1. */
        uint32_t version;
        /** The type / level of this report.
         * @see os_reportType */
        os_reportType reportType;
        /** Context information relating to where the even was generated.
         * May contain a function or compnent name or a stacktrace */
        char* reportContext;
        /** The source file name where the report even was generated */
        char* fileName;
        /** The source file line number where the report was generated */
        int32_t lineNo;
        /** An integer code associated with the event. */
        int32_t code;
        /** A description of the reported event */
        char *description;
        /** A string identifying the thread the event occured in */
        char* threadDesc;
        /** A string identifying the process the event occured in */
        char* processDesc;
    };

#define OS_REPORT_EVENT_V1 1

    /**
     * Pointer to an os_reportEventV1_s struct.
     * @see os_reportEventV1_s
     */
    typedef struct os_reportEventV1_s* os_reportEventV1;

    /**
     * Labels corresponding to os_reportType values.
     * @see os_reportType
     */
    OSAPI_EXPORT extern const char *os_reportTypeText [];

    typedef struct os_reportInfo_s {
        char *reportContext;
        char *sourceLine;
        char *callStack;
        int32_t reportCode;
        char *description;
    } os_reportInfo;

    /* Docced in impl file */
    OSAPI_EXPORT extern os_reportType os_reportVerbosity;

    OSAPI_EXPORT void
    os_reportInit(bool forceReInit);

    OSAPI_EXPORT void os_reportExit(void);

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

    OSAPI_EXPORT void
    os_report(
              os_reportType type,
              const char *context,
              const char *path,
              int32_t line,
              int32_t code,
              const char *format,
              ...) __attribute_format__((printf,6,7));

    OSAPI_EXPORT char *
    os_reportGetInfoFileName(void);

    OSAPI_EXPORT char *
    os_reportGetErrorFileName(void);

    OSAPI_EXPORT os_result
    os_reportSetVerbosity(
                          const char* newVerbosityString);

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
     * The os_report_flush_required checks if the report stack contains errors or
     * warning which always need to be reported.
     */
    OSAPI_EXPORT bool
    os_report_flush_required(
                             void);

    /**
     * The os_report_flush operation removes the report message from the stack,
     * and if valid is TRUE also writes them into the report device.
     * This operation additionally disables the stack.
     */
    OSAPI_EXPORT void
    os_report_flush(
                    bool valid,
                    const char *context,
                    const char *file,
                    const int line);

    /**
     * The os_report_flush operation removes the report message from the stack,
     * and also writes them into the report device. Thus the report stack is
     * always unwind. This operation additionally disables the stack.
     */
    OSAPI_EXPORT void
    os_report_flush_unconditional(
                                  bool valid,
                                  const char *context,
                                  const char *file,
                                  const int line);

    typedef char * (os_report_context_callback)(const char *context, char *buffer, unsigned buflen, void *arg);

    /**
     * The os_report_flush_context operation removes the report message
     * from the stack, and if valid is TRUE also writes them into the report device.
     * It uses the context information stored in the report.
     * The provided callback function is called to convert the function signature.
     * This operation additionally disables the stack.
     */
    OSAPI_EXPORT void
    os_report_flush_context(
                            bool valid,
                            os_report_context_callback callback,
                            void *arg);

    /**
     * The os_report_flush_context_unconditional operation removes the report message
     * from the stack and writes them into the report device.
     * It uses the context information stored in the report.
     * The provided callback function is called to convert the function signature.
     * This operation additionally disables the stack.
     */
    OSAPI_EXPORT void
    os_report_flush_context_unconditional(
                                          os_report_context_callback callback,
                                          void *arg);

    /**
     * The os_report_get_context operation returns the context information
     * saved in the report stack
     */
    OSAPI_EXPORT bool
    os_report_get_context(
                          const char **file,
                          int *lineno,
                          const char **signature);


    /**
     * The os_report_dumpStack operation removes the report messages from the stack
     * and writes them into the report device, regardless of the state the stack is
     * in. This is useful in panic situations, where you want to dump the content of
     * the current stack prior forcefully terminating the application.
     * This operation keeps the stack in the same state as it was in before invocation,
     * with the one exception that all messages in the stack that have been reported
     * are now removed.
     */
    OSAPI_EXPORT void
    os_report_dumpStack(
                        const char *context,
                        const char *file,
                        const int line);

    /**
     * The os_report_read operation returns the report message specified by a given index in the stack.
     * This operation will return a null pointer when the index is out of range.
     */
    OSAPI_EXPORT os_reportEventV1
    os_report_read(
                   int32_t index);

#if defined (__cplusplus)
}
#endif

#endif /* OS_REPORT_H */
