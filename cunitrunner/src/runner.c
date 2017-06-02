#include "cunitrunner/runner.h"

#include "os/os.h"

#include "CUnit/Basic.h"
#include "CUnit/Automated.h"

static struct cunit_runner {
    bool automated;
    bool junit;
    const char * results;
    CU_BasicRunMode mode;
    CU_ErrorAction error_action;
} runner;

static void usage (const char * name)
{
    fprintf(stderr, "usage: %s [flags]\n", name);
    fprintf(stderr, "Supported flags:\n");
    fprintf(stderr, " -a run in automated mode\n");
    fprintf(stderr, " -r <file_name> results file for automated run\n");
    fprintf(stderr, " -j junit format results \n");
    fprintf(stderr, " -f fail fast \n");
}

CU_ErrorCode runner_init (int argc, char* argv[])
{
    int i;
    CU_ErrorCode result;

    runner.automated = false;
    runner.junit = false;
    runner.results = NULL;
    runner.mode = CU_BRM_NORMAL;
    runner.error_action = CUEA_IGNORE;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'a':
                {
                    runner.automated = true;
                    break;
                }
                case 'f':
                {
                    runner.error_action = CUEA_FAIL;
                    break;
                }
                case 'j':
                {
                    runner.junit = true;
                    break;
                }
                case 'r':
                {
                    if(i +1 < argc){
                        runner.results = argv[++i];
                    } else {
                        CU_set_error(256);
                    }
                    break;
                }
                default:
                {
                    CU_set_error(256); /* Will print as "Undefined Errpr" */
                    goto err;
                }
            }
        }
    }

    if (CU_initialize_registry() != CUE_SUCCESS) {
        fprintf(stderr, "\nInitialization of C Unit Registry failed: %s", CU_get_error_msg());
        goto err;
    }
    CU_set_error_action (runner.error_action);

err:
    return CU_get_error();
}

CU_ErrorCode runner_run (void)
{
    if (runner.automated) {
        /* Generate CUnit or JUnit format results */
        if (runner.results != NULL) {
            CU_set_output_filename(runner.results);
        }

        if (runner.junit) {
            CU_automated_enable_junit_xml(CU_TRUE);
        } else {
            CU_list_tests_to_file();
        }
        CU_automated_run_tests();
    } else {
        CU_basic_set_mode(runner.mode);
        CU_basic_run_tests();
        printf("\nRun completed: %s\n", CU_get_error_msg());
    }

    return CU_get_error();
}

void runner_fini (void)
{
    CU_cleanup_registry();
}
