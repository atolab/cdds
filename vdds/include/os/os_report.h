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
#include "os/os.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
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

#define OS_REPORT_STACK() \
os_report_stack()

#define OS_REPORT_FLUSH(condition) \
os_report_flush((condition), OS_FUNCTION, __FILE__, __LINE__)

#define OS_REPORT_DUMPSTACK() \
os_report_dumpStack(OS_FUNCTION, __FILE__, __LINE__)

    typedef void * os_reportPlugin;

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
        OS_DEBUG,
        OS_INFO,
        OS_WARNING,
        OS_API_INFO, /* Deprecated, only here for backwards compatibility */
        OS_ERROR,
        OS_CRITICAL,
        OS_FATAL,
        OS_REPAIRED,
        OS_NONE
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
    OS_API extern const char *os_reportTypeText [];

    typedef struct os_reportInfo_s {
        char *reportContext;
        char *sourceLine;
        char *callStack;
        int32_t reportCode;
        char *description;
    } os_reportInfo;

    /* Docced in impl file */
    OS_API extern os_reportType os_reportVerbosity;

    OS_API void
    os_reportInit(bool forceReInit);

    OS_API void os_reportExit(void);

    /** \brief Report message directly and do not treat as formatting string
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
     * @param message message to report
     */
    OS_API void
    os_report_noargs(
                     os_reportType type,
                     const char *context,
                     const char *path,
                     int32_t line,
                     int32_t code,
                     const char *message);

    OS_API void
    os_report(
              os_reportType type,
              const char *context,
              const char *path,
              int32_t line,
              int32_t code,
              const char *format,
              ...) __attribute_format__((printf,6,7));

    OS_API os_reportInfo *
    os_reportGetApiInfo(void);

    OS_API void
    os_reportClearApiInfo(void);

    OS_API int32_t
    os_reportRegisterPlugin(
                            const char *library_file_name,
                            const char *initialize_method_name,
                            const char *argument,
                            const char *report_method_name,
                            const char *typedreport_method_name,
                            const char *finalize_method_name,
                            bool suppressDefaultLogs,
                            os_reportPlugin *plugin);

    typedef void *os_reportPlugin_context;

    typedef int
    (*os_reportPlugin_initialize)(
    const char *argument,
    os_reportPlugin_context *context);

    typedef int
    (*os_reportPlugin_report)(
    os_reportPlugin_context context,
    const char *report);

    /**
     * Function pointer type for a plugged in report method
     * taking a typed report event
     */
    typedef int
    (*os_reportPlugin_typedreport)(
    os_reportPlugin_context context,
    os_reportEvent report);

    typedef int
    (*os_reportPlugin_finalize)(
    os_reportPlugin_context context);

    OS_API int32_t
    os_reportInitPlugin(
                        const char *argument,
                        os_reportPlugin_initialize initFunction,
                        os_reportPlugin_finalize finalizeFunction,
                        os_reportPlugin_report reportFunction,
                        os_reportPlugin_typedreport typedReportFunction,
                        bool suppressDefaultLogs,
                        os_reportPlugin *plugin);

    OS_API int32_t
    os_reportUnregisterPlugin(
                              os_reportPlugin plugin);

    OS_API void
    os_reportDisplayLogLocations(void);

    OS_API char *
    os_reportGetInfoFileName(void);

    OS_API char *
    os_reportGetErrorFileName(void);

    OS_API os_result
    os_reportSetVerbosity(
                          const char* newVerbosityString);

    OS_API void
    os_reportRemoveStaleLogs(void);

    /*****************************************
     * Report stack related functions
     *****************************************/

    /**
     * The os_report_stack operation enables a report stack for the current thread.
     * The stack will be disabled again by the os_report_flush operation.
     */
    OS_API void
    os_report_stack(void);

    /**
     * The os_report_stack_open operation enables a report stack for the current thread.
     * It initializes the report record with the file, line and the signature of the
     * operation that called this operation.
     * The stack will be disabled again by the os_report_flush operation.
     */
    OS_API void
    os_report_stack_open(
                         const char *file,
                         int lineno,
                         const char *signature);

    /**
     * The os_report_stack_free operation frees all memory allocated by the current
     * thread for the report stack.
     */
    OS_API void
    os_report_stack_free(
                         void);

    /**
     * The os_report_flush_required checks if the report stack contains errors or
     * warning which always need to be reported.
     */
    OS_API bool
    os_report_flush_required(
                             void);

    /**
     * The os_report_flush operation removes the report message from the stack,
     * and if valid is TRUE also writes them into the report device.
     * This operation additionally disables the stack.
     */
    OS_API void
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
    OS_API void
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
    OS_API void
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
    OS_API void
    os_report_flush_context_unconditional(
                                          os_report_context_callback callback,
                                          void *arg);
    
    /**
     * The os_report_get_context operation returns the context information
     * saved in the report stack
     */
    OS_API bool
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
    OS_API void
    os_report_dumpStack(
                        const char *context,
                        const char *file,
                        const int line);
    
    /**
     * The os_report_stack_size operation returns the number of messages in the report stack.
     * This operation will return -1 when no stack is active.
     */
    OS_API int32_t
    os_report_stack_size(void);
    
    /**
     * The os_report_read operation returns the report message specified by a given index in the stack.
     * This operation will return a null pointer when the index is out of range.
     */
    OS_API os_reportEventV1
    os_report_read(
                   int32_t index);
    
#undef OS_API
    
#if defined (__cplusplus)
}
#endif

#endif /* OS_REPORT_H */
