#include "dds.h"
#include "cunitrunner/runner.h"
#include "os/os.h"

#ifndef WINCE
#include <fcntl.h>
#endif

#if (defined WIN32 || defined WIN64)
#include <direct.h>
#include <Windows.h>
#endif

#define ENABLE_TRACING 0

static int
vsnprintfTest(
              const char *format,
              ...)
{
    va_list varargs;
    int result = 0;
    char description[10];
    va_start(varargs, format);
    memset(description, 0, sizeof(description));

    result = os_vsnprintf(description, sizeof(description)-1, format, varargs);
    va_end(varargs);
    return result;
}

static int suite_abstraction_stdlib_init (void)
{
    int result = DDS_RETCODE_OK;
    os_osInit();
    return result;
}

static int suite_abstraction_stdlib_clean (void)
{
  /* Remove files used to test permissions */

  remove ("exec_only");
  remove ("read_exec");
  remove ("read_only");
  remove ("read_write_exec");
  remove ("write_only");
  remove ("existing_file");
  os_osExit();
  return 0;
}

static void stdlib_strcasecmp (void)
{
    int res;
    char *s1, *s2;
    
    s1 = "a";
    s2 = "a";
#if ENABLE_TRACING
  printf ("Starting stdlib_strcasecmp_001\n");
#endif
    res = os_strcasecmp (s1,s2);
    CU_ASSERT (res == 0);

  #if ENABLE_TRACING
    printf ("Starting stdlib_strcasecmp_002\n");
  #endif
    s1 = "aa";
    s2 = "a";
    res = os_strcasecmp (s1,s2);
    CU_ASSERT (res > 0);

  #if ENABLE_TRACING
    printf ("Starting stdlib_strcasecmp_003\n");
  #endif
    s1 = "a";
    s2 = "aa";
    res = os_strcasecmp (s1,s2);
    CU_ASSERT (res < 0);

  #if ENABLE_TRACING
    printf ("Starting stdlib_strcasecmp_004\n");
  #endif
    s1 = "a";
    s2 = "A";
    res = os_strcasecmp (s1,s2);
    CU_ASSERT (res == 0);

  #if ENABLE_TRACING
    printf ("Starting stdlib_strcasecmp_005\n");
  #endif
    s1 = "A";
    s2 = "a";
    res = os_strcasecmp (s1,s2);
    CU_ASSERT (res == 0);

  #if ENABLE_TRACING
    printf ("Starting stdlib_strcasecmp_006\n");
  #endif
    s1 = "a";
    s2 = "b";
    res = os_strcasecmp (s1,s2);
    CU_ASSERT (res < 0);

  #if ENABLE_TRACING
    printf ("Starting stdlib_strcasecmp_007\n");
  #endif
    s1 = "b";
    s2 = "a";
    res = os_strcasecmp (s1,s2);
    CU_ASSERT (res > 0);

  #if ENABLE_TRACING
    printf ("Ending stdlib_strcasecmp\n");
  #endif
}

static void stdlib_strncasecmp (void)
{
    int res;
    char *s1, *s2;

    s1 = "a";
    s2 = "a";
#if ENABLE_TRACING
  printf ("Starting stdlib_strncasecmp_001\n");
#endif
    res = os_strncasecmp (s1,s2,2);
    CU_ASSERT (res == 0);

    s1 = "aa";
    s2 = "a";
  #if ENABLE_TRACING
    printf ("Starting stdlib_strncasecmp_002\n");
  #endif
    res = os_strncasecmp (s1,s2,2);
    CU_ASSERT (res > 0);

    s1 = "a";
    s2 = "aa";
  #if ENABLE_TRACING
    printf ("Starting stdlib_strncasecmp_003\n");
  #endif
    res = os_strncasecmp (s1,s2,2);
    CU_ASSERT (res < 0);

    s1 = "a";
    s2 = "A";
  #if ENABLE_TRACING
    printf ("Starting stdlib_strncasecmp_004\n");
  #endif
    res = os_strncasecmp (s1,s2,2);
    CU_ASSERT (res == 0);

    s1 = "A";
    s2 = "a";
  #if ENABLE_TRACING
    printf ("Starting stdlib_strncasecmp_005\n");
  #endif
    res = os_strncasecmp (s1,s2,2);
    CU_ASSERT (res == 0);

    s1 = "a";
    s2 = "b";
  #if ENABLE_TRACING
    printf ("Starting stdlib_strncasecmp_006\n");
  #endif
    res = os_strncasecmp (s1,s2,2);
    CU_ASSERT (res < 0);

    s1 = "b";
    s2 = "a";
  #if ENABLE_TRACING
    printf ("Starting stdlib_strncasecmp_007\n");
  #endif
    res = os_strncasecmp (s1,s2,2);
    CU_ASSERT (res > 0);

    s1 = "abcdefghijkl";
    s2 = "AbCdEaGhIjKl";
  #if ENABLE_TRACING
    printf ("Starting stdlib_strncasecmp_008\n");
  #endif
    res = os_strncasecmp (s1,s2,5);
    CU_ASSERT (res == 0);

    s1 = "abcdefghijkl";
    s2 = "AbCdEaGhIjKl";
  #if ENABLE_TRACING
    printf ("Starting stdlib_strncasecmp_009\n");
  #endif
    res = os_strncasecmp (s1,s2,6);
    CU_ASSERT (res > 0);

  #if ENABLE_TRACING
    printf ("Ending stdlib_strcasecmp\n");
  #endif
}

static void stdlib_gethostname (void)
{
    int res;
    os_result os_res;
    char os_cpu[200];
    char cpu[200];

  #if ENABLE_TRACING
    printf ("Starting stdlib_os_gethostname_001\n");
  #endif
    os_cpu[0] = '\0';
    os_res = os_gethostname (os_cpu, sizeof(os_cpu));
    CU_ASSERT (os_res == os_resultSuccess);
    
    cpu[0] = '\0';
    res = gethostname (cpu, sizeof(cpu));
    CU_ASSERT (res == 0);
  
  #if ENABLE_TRACING
    printf ("Starting stdlib_os_gethostname_002\n");
  #endif
    os_res = os_gethostname (os_cpu, strlen(os_cpu)-1);
    CU_ASSERT (os_res == os_resultFail);
  #if ENABLE_TRACING
    printf ("Ending stdlib_os_gethostname\n");
  #endif
}

static void stdlib_putenv (void)
{
    os_result os_res;
    
  #if ENABLE_TRACING
    printf ("Starting stdlib_os_putenv_001\n");
  #endif
    os_res = os_putenv ("ABCDE=FGHIJ");
    CU_ASSERT (os_res == os_resultSuccess);
    CU_ASSERT (strcmp (os_getenv("ABCDE"), "FGHIJ") == 0);
  #if ENABLE_TRACING
    printf ("Ending stdlib_os_putenv\n");
  #endif
}

static void stdlib_getenv (void)
{
  #if ENABLE_TRACING
    printf ("Starting stdlib_os_getenv_001\n");
  #endif
    CU_ASSERT (strcmp (os_getenv("ABCDE"), "FGHIJ") == 0);
  #if ENABLE_TRACING
    printf ("Starting stdlib_os_getenv_002\n");
  #endif
    CU_ASSERT (os_getenv("XXABCDEXX") == NULL );
  #if ENABLE_TRACING
    printf ("Ending stdlib_os_getenv\n");
  #endif
}

static void stdlib_fileSep (void) 
{
  #if defined WIN32
    const char *wanted= "\\";
  #else
    const char *wanted= "/";
  #endif
  #if ENABLE_TRACING
    printf ("Starting stdlib_os_fileSep_001\n");
  #endif
    CU_ASSERT (strcmp(os_fileSep(), wanted) == 0);
  #if ENABLE_TRACING
    printf ("Ending stdlib_os_fileSep\n");
  #endif
}

static void stdlib_access (void) 
{
    os_result os_res;
    os_result wanted;
    int fh;
    
  #if ENABLE_TRACING
    /* Check correct functioning of os_access, non existing file read access */
    printf ("Starting stdlib_os_access_001\n");
  #endif
#if defined VXWORKS_RTP || defined _WRS_KERNEL
  #ifdef ENABLE_TRACING
    printf ("N.A - Not tested for vxworks.\n");
  #endif
#else
    os_res = os_access("non_existing_file", OS_ROK);
    CU_ASSERT (os_res == os_resultFail);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, non existing file write access */
    printf ("Starting stdlib_os_access_002\n");
  #endif
    os_res = os_access("non_existing_file", OS_WOK);
    CU_ASSERT (os_res == os_resultFail);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, non existing file execute access */
    printf ("Starting stdlib_os_access_003\n");
  #endif
    os_res = os_access("non_existing_file", OS_XOK);
    CU_ASSERT (os_res == os_resultFail);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, non existing file existence */
    printf ("Starting stdlib_os_access_004\n");
  #endif
    os_res = os_access("non_existing_file", OS_FOK);
    CU_ASSERT (os_res == os_resultFail);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, existing file with no 
       permissions read access */
    printf ("Starting stdlib_os_access_005\n");
  #endif
  #ifdef WIN32
    fh= _creat("existing_file", 0000); /* Note always has read & execute */
    if (fh != -1)
        _close(fh);
    wanted = os_resultSuccess;
  #else
    fh= creat("existing_file", 0000);
    if (fh != -1)
        close(fh);
     wanted = os_resultFail;
  #endif /* WIN32 */
    os_res = os_access("existing_file", OS_ROK);
    CU_ASSERT (os_res == wanted);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, existing file with no 
       permissions write access */
    printf ("Starting stdlib_os_access_006\n");
  #endif
    os_res = os_access("existing_file", OS_WOK);
    CU_ASSERT (os_res == os_resultFail);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, existing file with no 
       permissions execute access */
    printf ("Starting stdlib_os_access_007\n");
  #endif
    os_res = os_access("existing_file", OS_XOK);
    CU_ASSERT (os_res == os_resultFail);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, existing file with no permissions existence */
    printf ("Starting stdlib_os_access_008\n");
  #endif
    os_res = os_access("existing_file", OS_FOK);
    CU_ASSERT (os_res == os_resultSuccess);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with read permissions read access */
    printf ("Starting stdlib_os_access_009\n");
  #endif
  #ifdef WIN32
    fh= _creat("read_only", _S_IREAD); /* Note always has read & execute */
    if (fh != -1)
        _close(fh);
  #else
    fh= creat("read_only", S_IRUSR);
    if (fh != -1)
        close(fh);
  #endif /* WIN32 */
    os_res = os_access("read_only", OS_ROK);
    CU_ASSERT (os_res == os_resultSuccess);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with read permissions write access */
    printf ("Starting stdlib_os_access_010\n");
  #endif
    os_res = os_access("read_only", OS_WOK);
    CU_ASSERT (os_res == os_resultFail);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with read permissions execute access */
    printf ("Starting stdlib_os_access_011\n");
  #endif
    os_res = os_access("read_only", OS_XOK);
    CU_ASSERT (os_res == os_resultFail);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with read permissions existence */
    printf ("Starting stdlib_os_access_012\n");
  #endif
    os_res = os_access("read_only", OS_FOK);
    CU_ASSERT (os_res == os_resultSuccess);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with write permissions read access */
    printf ("Starting stdlib_os_access_013\n");
  #endif
  #ifdef WIN32
    fh= _creat("write_only", _S_IWRITE); /* Note windows automatically has read access can't have write only */
    if (fh != -1)
        _close(fh);
    wanted = os_resultSuccess;
  #else
    fh= creat("write_only", S_IWUSR);
    if (fh != -1)
        close(fh);
    wanted = os_resultFail;
  #endif /* WIN32 */
    os_res = os_access("write_only", OS_ROK);
    CU_ASSERT (os_res == wanted);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with write permissions write access */
    printf ("Starting stdlib_os_access_014\n");
  #endif
    os_res = os_access("write_only", OS_WOK);
    CU_ASSERT (os_res == os_resultSuccess);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access,
       existing file with write permissions execute access */
    printf ("Starting stdlib_os_access_015\n");
  #endif
    os_res = os_access("write_only", OS_XOK);
    CU_ASSERT (os_res == os_resultFail);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with write permissions existence */
    printf ("Starting stdlib_os_access_016\n");
  #endif
    os_res = os_access("write_only", OS_FOK);
    CU_ASSERT (os_res == os_resultSuccess);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with exec permissions read access */
    printf ("Starting stdlib_os_access_017\n");
  #endif
  #ifdef WIN32
    fh= _creat("exec_only" OS_OS_EXESUFFIX, _S_IREAD); /* Windows always has read and can't do execute (that's based upon filename ext only) */
    if (fh != -1)
        _close(fh);
    wanted = os_resultSuccess;
  #else
    fh= creat("exec_only" OS_OS_EXESUFFIX, S_IXUSR);
    if (fh != -1)
        close(fh);
    wanted = os_resultFail;
  #endif /* WIN32 */
    os_res = os_access("exec_only" OS_OS_EXESUFFIX, OS_ROK);
    CU_ASSERT (os_res == wanted);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with exec permissions write access */
    printf ("Starting stdlib_os_access_018\n");
  #endif
    os_res = os_access("exec_only" OS_OS_EXESUFFIX, OS_WOK);
    CU_ASSERT (os_res == os_resultFail);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with exec permissions execute access */
    printf ("Starting stdlib_os_access_019\n");
  #endif
    os_res = os_access("exec_only" OS_OS_EXESUFFIX, OS_XOK);
    CU_ASSERT (os_res == os_resultSuccess);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with exec permissions existence */
    printf ("Starting stdlib_os_access_020\n");
  #endif
    os_res = os_access("exec_only" OS_OS_EXESUFFIX, OS_FOK);
    CU_ASSERT (os_res == os_resultSuccess);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with read/write/exec permissions read access */
    printf ("Starting stdlib_os_access_021\n");
  #endif
  #ifdef WIN32
    fh= _creat("read_write_exec" OS_OS_EXESUFFIX, _S_IREAD | _S_IWRITE); /* Windows always has read and can't do execute (that's based upon filename ext only) */
    if (fh != -1)
        _close(fh);
  #else
    fh= creat("read_write_exec" OS_OS_EXESUFFIX, S_IRUSR | S_IWUSR | S_IXUSR);
    if (fh != -1)
        close(fh);
  #endif /* WIN32 */
    os_res = os_access("read_write_exec" OS_OS_EXESUFFIX, OS_ROK);
    CU_ASSERT (os_res == os_resultSuccess);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with read/write/exec permissions write access */
    printf ("Starting stdlib_os_access_022\n");
  #endif
    os_res = os_access("read_write_exec" OS_OS_EXESUFFIX, OS_WOK);
    CU_ASSERT (os_res == os_resultSuccess);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with read/write/exec permissions execute access */
    printf ("Starting stdlib_os_access_023\n");
  #endif
    os_res = os_access("read_write_exec" OS_OS_EXESUFFIX, OS_XOK);
    CU_ASSERT (os_res == os_resultSuccess);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with read/write/exec permissions existence */
    printf ("Starting stdlib_os_access_024\n");
  #endif
    os_res = os_access("read_write_exec" OS_OS_EXESUFFIX, OS_FOK);
    CU_ASSERT (os_res == os_resultSuccess);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with read/exec permissions read+write access */
    printf ("Starting stdlib_os_access_025\n");
  #endif
  #ifdef WIN32
    fh= _creat("read_exec" OS_OS_EXESUFFIX, _S_IREAD); /* Windows always has read and can't do execute (that's based upon filename ext only) */
    if (fh != -1)
        _close(fh);
  #else
    fh= creat("read_exec" OS_OS_EXESUFFIX, S_IRUSR | S_IXUSR);
    if (fh != -1)
        close(fh);
  #endif /* WIN32 */
    os_res = os_access("read_exec" OS_OS_EXESUFFIX, OS_ROK|OS_WOK);
    CU_ASSERT (os_res == os_resultFail);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with read/exec permissions write+exec access */
    printf ("Starting stdlib_os_access_026\n");
  #endif
    os_res = os_access("read_exec" OS_OS_EXESUFFIX, OS_WOK|OS_XOK);
    CU_ASSERT (os_res == os_resultFail);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access,
       existing file with read/exec permissions read+exec access */
    printf ("Starting stdlib_os_access_027\n");
  #endif
    os_res = os_access("read_exec" OS_OS_EXESUFFIX, OS_ROK|OS_XOK);
    CU_ASSERT (os_res == os_resultSuccess);

  #if ENABLE_TRACING
    /* Check correct functioning of os_access, 
       existing file with read/exec permissions read+exec+existence */
    printf ("Starting stdlib_os_access_028\n");
  #endif
    os_res = os_access("read_exec" OS_OS_EXESUFFIX, OS_ROK|OS_XOK|OS_FOK);
    CU_ASSERT (os_res == os_resultSuccess);
#endif /* VXWORKS */
  
  #if ENABLE_TRACING
    printf ("Ending stdlib_os_access\n");
  #endif
}

static void stdlib_vsnprintf (void) 
{
  #if ENABLE_TRACING
    printf ("Starting stdlib_os_vsnprintf_001\n");
  #endif
    CU_ASSERT (vsnprintfTest("%s","test") == 4);
    CU_ASSERT (vsnprintfTest("%d",12) == 2);
    CU_ASSERT (vsnprintfTest("hello %s","world") == 11);
    
  #if ENABLE_TRACING
    printf ("Ending stdlib_os_vsnprintf\n");
  #endif
}

static void stdlib_locate (void) 
{
  #if ENABLE_TRACING
    printf ("stdlib_lcoate is not used in Lite.\n");
  #endif
}

static void stdlib_strtok_r (void) 
{
    char * res;
    char *strtok_r_ts1;
    char *saveptr;
    
  #if ENABLE_TRACING
    printf ("Starting stdlib_os_strtok_r_001\n");
  #endif
     strtok_r_ts1= os_strdup("123,234");
     res = os_strtok_r( strtok_r_ts1, ",", &saveptr );
     CU_ASSERT (strcmp(res, "123") == 0);

  #if ENABLE_TRACING
    printf ("Starting stdlib_os_strtok_r_002\n");
  #endif
    res = os_strtok_r( NULL, ",", &saveptr );
    CU_ASSERT (strcmp(res, "234") == 0);

  #if ENABLE_TRACING
    printf ("Starting stdlib_os_strtok_r_003\n");
  #endif
    res = os_strtok_r( NULL, ",", &saveptr );
    CU_ASSERT (res == NULL);
    os_free(strtok_r_ts1);

  #if ENABLE_TRACING
    printf ("Starting stdlib_os_strtok_r_004\n");
  #endif
    strtok_r_ts1= os_strdup(",;,123abc,,456,:,");
    res = os_strtok_r( strtok_r_ts1, ",;", &saveptr );
    CU_ASSERT (strcmp(res, "123abc") == 0);

  #if ENABLE_TRACING
    printf ("Starting stdlib_os_strtok_r_005\n");
  #endif
    res = os_strtok_r( NULL, ",", &saveptr );
    CU_ASSERT (strcmp(res, "456") == 0);

  #if ENABLE_TRACING
    printf ("Starting stdlib_os_strtok_r_006\n");
  #endif
    res = os_strtok_r( NULL, ",:", &saveptr );
    CU_ASSERT (res == NULL);
    free(strtok_r_ts1);

  #if ENABLE_TRACING
    printf ("Starting stdlib_os_strtok_r_007\n");
  #endif
    strtok_r_ts1= os_strdup(",,,123,,456,789,,,");
    res = os_strtok_r( strtok_r_ts1, ",", &saveptr );
    CU_ASSERT (strcmp(res, "123") == 0);

  #if ENABLE_TRACING
    printf ("Starting stdlib_os_strtok_r_008\n");
  #endif
    res = os_strtok_r( NULL, ",", &saveptr );
    CU_ASSERT (strcmp(res, "456") == 0);

  #if ENABLE_TRACING
    printf ("Starting stdlib_os_strtok_r_009\n");
  #endif
    res = os_strtok_r( NULL, ",", &saveptr );
    CU_ASSERT (strcmp(res, "789") == 0);

  #if ENABLE_TRACING
    printf ("Starting stdlib_os_strtok_r_010\n");
  #endif
    res = os_strtok_r( NULL, ",:", &saveptr );
    CU_ASSERT (res == NULL);
    free(strtok_r_ts1);
    
  #if ENABLE_TRACING
    printf ("Ending stdlib_os_strtok_r\n");
  #endif
}

static void stdlib_index (void) 
{
    char * res;
    char *index_ts1;
  #if ENABLE_TRACING
    printf ("Starting stdlib_os_index_001\n");
  #endif
    index_ts1 = "abc";
    res = os_index( index_ts1, 'a' );
    CU_ASSERT (res == index_ts1);

  #if ENABLE_TRACING
    printf ("Starting stdlib_os_index_002\n");
  #endif
    res = os_index( index_ts1, 'c' );
    CU_ASSERT (res == &index_ts1[2]);

  #if ENABLE_TRACING
    printf ("Starting stdlib_os_index_003\n");
  #endif
    index_ts1 = "abcdefghij";
    res = os_index( index_ts1, 'f' );
    CU_ASSERT (res == &index_ts1[5]);

  #if ENABLE_TRACING
    printf ("Starting stdlib_os_index_004\n");
  #endif
    res = os_index( index_ts1, 'k' );
    CU_ASSERT (res == NULL);
    
  #if ENABLE_TRACING
    printf ("Ending stdlib_os_index\n");
  #endif
}

static void stdlib_mkpath (void) 
{
  #if ENABLE_TRACING
    printf ("stdlib_mkpath is not used in LITE.\n");
  #endif
}

int main (int argc, char *argv[])
{
    CU_pSuite suite;

    if (runner_init(argc, argv)){
        goto err_init;
    }

    if ((suite = CU_add_suite ("abstraction_stdlib", suite_abstraction_stdlib_init, suite_abstraction_stdlib_clean)) == NULL){
        goto err;
    }
    if (CU_add_test (suite, "stdlib_os_strcasecmp", stdlib_strcasecmp) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "stdlib_os_strncasecmp", stdlib_strncasecmp) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "stdlib_os_gethostname", stdlib_gethostname) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "stdlib_os_putenv", stdlib_putenv) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "stdlib_os_putenv", stdlib_getenv) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "stdlib_os_fileSep", stdlib_fileSep) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "stdlib_os_access", stdlib_access) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "stdlib_os_vsnprintf", stdlib_vsnprintf) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "stdlib_os_locate", stdlib_locate) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "stdlib_os_strtok_r", stdlib_strtok_r) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "stdlib_os_index", stdlib_index) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "stdlib_os_mkpath", stdlib_mkpath) == NULL) {
        goto err;
    } 
    runner_run();
err:
    runner_fini();
err_init:
    return CU_get_error();
}


