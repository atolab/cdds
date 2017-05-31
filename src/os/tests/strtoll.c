#include "dds.h"
#include "cunitrunner/runner.h"
#include "os/os.h"

long long ll;
unsigned long long ull;
const char *str;
char *ptr;
char buf[100];
char str_llmin[100];
char str_llmax[100];
char str_ullmax[100];
char str_llrange[100];
char str_ullrange[100];

char str_xllmin[99], str_xllmax[99];

/* Really test with the maximum values supported on a platform, not some
   made up number. */
long long llmin;
long long llmax;
unsigned long long ullmax;

static int  suite_abstraction_strtoll_init (void)
{
    int result = DDS_RETCODE_OK;
    os_osInit();
  #if ENABLE_TRACING
    printf("Run suite_abstraction_strtoll_init\n");
  #endif
    
    (void)snprintf (str_llmin, sizeof(str_llmin), "%lld", llmin);
    (void)snprintf (str_llmax, sizeof(str_llmax), "%lld", llmax);
    (void)snprintf (str_llrange, sizeof(str_llrange), "%lld1", llmax);
    (void)snprintf (str_ullmax, sizeof(str_ullmax), "%llu", ullmax);
    (void)snprintf (str_ullrange, sizeof(str_ullrange), "%llu1", ullmax);
    (void)snprintf (str_xllmin, sizeof(str_xllmin), "-%llx", llmin);
    (void)snprintf (str_xllmax, sizeof(str_xllmax), "+%llx", llmax);

    return result;
}

static int suite_abstraction_strtoll_clean (void)
{
    int result = DDS_RETCODE_OK;
    
  #if ENABLE_TRACING
    printf("Run suite_abstraction_strtoll_clean\n");
  #endif
    os_osExit();
    return result;
}

static void tc_os_strtoll (void)
{
  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_001a\n");
  #endif
    str = "gibberish";
    ll = os_strtoll(str, &ptr, 0);
    CU_ASSERT (ll == 0 && ptr == str);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_001b\n");
  #endif
    str = "+gibberish";
    ll = os_strtoll(str, &ptr, 0);
    CU_ASSERT (ll == 0 && ptr == str);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_001c\n");
  #endif
    str = "-gibberish";
    ll = os_strtoll(str, &ptr, 0);
    CU_ASSERT (ll == 0 && ptr == str);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_001d\n");
  #endif
    str = "gibberish";
    ptr = NULL;
    errno=0;
    ll = os_strtoll(str, &ptr, 36);
    CU_ASSERT (ll == 46572948005345 && errno == 0 && ptr && *ptr == '\0');

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_001e\n");
  #endif
    str = "1050505055";
    ptr = NULL;
    errno = 0;
    ll = os_strtoll(str, &ptr, 37);
    CU_ASSERT (ll == 0LL && errno == EINVAL && ptr == str);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_001f\n");
  #endif
    str = " \t \n 1050505055";
    ll = os_strtoll(str, NULL, 10);
    CU_ASSERT (ll == 1050505055LL);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_001g\n");
  #endif
    str = " \t \n -1050505055";
    ptr = NULL;
    ll = os_strtoll(str, &ptr, 10);
    CU_ASSERT (ll == -1050505055LL);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_001h\n");
  #endif
    str = " \t \n - \t \n 1050505055";
    ptr = NULL;
    ll = os_strtoll(str, &ptr, 10);
    CU_ASSERT (ll == 0LL && ptr == str);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_002a\n");
  #endif
    str = "10x";
    ptr = NULL;
    ll = os_strtoll(str, &ptr, 10);
    CU_ASSERT (ll == 10LL && ptr && *ptr == 'x');

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_002b\n");
  #endif
    str = "+10x";
    ll = os_strtoll(str, &ptr, 10);
    CU_ASSERT (ll == 10LL && ptr && *ptr == 'x');

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_002c\n");
  #endif
    str = "-10x";
    ll = os_strtoll(str, &ptr, 10);
    CU_ASSERT (ll == -10LL && ptr && *ptr == 'x');

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_002d\n");
  #endif
    str = (const char *)str_llmax;
    ll = os_strtoll(str, NULL, 10);
    CU_ASSERT (ll == llmax);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_002e\n");
  #endif
    str = (const char *)str_llmin;
    ll = os_strtoll(str, NULL, 10);
    CU_ASSERT (ll == llmin);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_002f\n");
  #endif
    str = (const char *)str_llrange;
    ll = os_strtoll(str, &ptr, 10);
    CU_ASSERT (ll == llmax && *ptr == '1');

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_003a\n");
  #endif
    str = "0x100";
    ll = os_strtoll(str, NULL, 16);
    CU_ASSERT (ll == 0x100LL);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_003b\n");
  #endif
    str = "0X100";
    ll = os_strtoll(str, NULL, 16);
    CU_ASSERT (ll == 0x100LL);
    
  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_003c\n");
  #endif
    str = "0x1DEFCAB";
    ll = os_strtoll(str, NULL, 16);
    CU_ASSERT (ll == 0x1DEFCABLL);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_003d\n");
  #endif
    str = "0x1defcab";
    ll = os_strtoll(str, NULL, 16);
    CU_ASSERT (ll == 0x1DEFCABLL);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_003e\n");
  #endif
    str = (char *)str_xllmin;
    ll = os_strtoll(str, NULL, 16);
    CU_ASSERT (ll == llmin);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_003f\n");
  #endif
    str = (char *)str_xllmax;
    ll = os_strtoll(str, NULL, 16);
    CU_ASSERT (ll == llmax);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_003g\n");
  #endif
    str = "0x100";
    ll = os_strtoll(str, NULL, 0);
    CU_ASSERT (ll == 0x100LL);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_003h\n");
  #endif
    str = "100";
    ll = os_strtoll(str, NULL, 16);
    CU_ASSERT (ll == 0x100LL);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_003i\n");
  #endif
    /* calling os_strtoll with \"%s\" and base 10, expected result 0 */
    str = "0x100";
    ll = os_strtoll(str, &ptr, 10);
    CU_ASSERT (ll == 0 && ptr && *ptr == 'x');

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_003j\n");
  #endif
    /* calling os_strtoll with \"%s\" and base 0, expected result 256 */
    str = "0x100g";
    ll = os_strtoll(str, &ptr, 0);
    CU_ASSERT (ll == 256 && ptr && *ptr == 'g');

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_004a\n");
  #endif
    str = "0100";
    ll = os_strtoll(str, NULL, 0);
    CU_ASSERT(ll == 64LL);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_004b\n");
  #endif
    str = "0100";
    ll = os_strtoll(str, NULL, 8);
    CU_ASSERT(ll == 64LL);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_004c\n");
  #endif
    str = "100";
    ll = os_strtoll(str, NULL, 8);
    CU_ASSERT(ll == 64LL);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_004d\n");
  #endif
    /* calling os_strtoll with \"%s\" and base 10, expected result 100 */
    str = "0100";
    ll = os_strtoll(str, &ptr, 10);
    CU_ASSERT(ll == 100);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_004e\n");
  #endif
    /* calling os_strtoll with \"%s\" and base 0, expected result 64 */
    str = "01008";
    ll = os_strtoll(str, &ptr, 8);
    CU_ASSERT(ll == 64LL && ptr && *ptr == '8');

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoll_004f\n");
  #endif
    str = "00001010";
    ll = os_strtoll(str, NULL, 2);
    CU_ASSERT(ll == 10LL);

  #if ENABLE_TRACING
    printf ("Ending tc_os_strtoll\n");
  #endif
}

static void tc_os_strtoull (void)
{
  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoull_001a\n");
  #endif
    str = "0xffffffffffffffff";
    ull = os_strtoull(str, NULL, 0);
    CU_ASSERT(ull == ullmax);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoull_001b\n");
  #endif
    str = "-1";
    ull = os_strtoull(str, NULL, 0);
    CU_ASSERT(ull == ullmax);

  #if ENABLE_TRACING
    printf ("Starting tc_os_strtoull_001c\n");
  #endif
    str = "-2";
    ull = os_strtoull(str, NULL, 0);
    CU_ASSERT(ull == (ullmax - 1));
    
  #if ENABLE_TRACING
    printf ("Ending tc_os_strtoull\n");
  #endif
}

static void tc_os_atoll (void)
{
  #if ENABLE_TRACING
    printf ("Starting tc_os_atoll_001\n");
  #endif
    str = "10";
    ll = os_atoll(str);
    CU_ASSERT(ll == 10);
    
  #if ENABLE_TRACING
    printf ("Ending tc_os_atoll\n");
  #endif
}

static void tc_os_atoull (void)
{
  #if ENABLE_TRACING
    printf ("Starting tc_os_atoull_001\n");
  #endif
    str = "10";
    ull = os_atoull(str);
    CU_ASSERT(ull == 10);
    
  #if ENABLE_TRACING
    printf ("Ending tc_os_atoull\n");
  #endif
}

static void tc_os_lltostr (void)
{
  #if ENABLE_TRACING
    printf ("Starting tc_os_lltostr_001\n");
  #endif
    ll = llmax;
    ptr = os_lltostr(ll, buf, 0, NULL);
    CU_ASSERT(ptr == NULL);

  #if ENABLE_TRACING
    printf ("Starting tc_os_lltostr_002\n");
  #endif
    /* calling os_lltostr with %lld with buffer size of 5, expected result \"5432\" */
    ll = 54321;
    ptr = os_lltostr(ll, buf, 5, NULL);
    CU_ASSERT(strcmp(ptr, "5432") == 0);

  #if ENABLE_TRACING
    printf ("Starting tc_os_lltostr_003a\n");
  #endif
    ll = llmax;
    ptr = os_lltostr(ll, buf, sizeof(buf), NULL);
    CU_ASSERT(strcmp(ptr, str_llmax) == 0);

  #if ENABLE_TRACING
    printf ("Starting tc_os_lltostr_003b\n");
  #endif
    ll = llmin;
    ptr = os_lltostr(ll, buf, sizeof(buf), NULL);
    CU_ASSERT(strcmp(ptr, str_llmin) == 0);

  #if ENABLE_TRACING
    printf ("Starting tc_os_lltostr_004\n");
  #endif
    ll = 1;
    ptr = os_lltostr(ll, buf, sizeof(buf), NULL);
    CU_ASSERT(strcmp(ptr, "1") == 0);

  #if ENABLE_TRACING
    printf ("Starting tc_os_lltostr_005\n");
  #endif
    ll = 0;
    ptr = os_lltostr(ll, buf, sizeof(buf), NULL);
    CU_ASSERT(strcmp(ptr, "0") == 0);

  #if ENABLE_TRACING
    printf ("Starting tc_os_lltostr_006\n");
  #endif
    ll = -1;
    ptr = os_lltostr(ll, buf, sizeof(buf), NULL);
    CU_ASSERT(strcmp(ptr, "-1") == 0);
    
  #if ENABLE_TRACING
    printf ("Ending tc_os_lltostr\n");
  #endif
}

static void tc_os_ulltostr (void)
{
  #if ENABLE_TRACING
    printf ("Starting tc_os_ulltostr_001\n");
  #endif
    ull = ullmax;
    ptr = os_ulltostr(ull, buf, sizeof(buf), NULL);
    CU_ASSERT(strcmp(ptr, str_ullmax) == 0);
    
  #if ENABLE_TRACING
    printf ("Starting tc_os_ulltostr_002\n");
  #endif
    ull = 0ULL;
    ptr = os_ulltostr(ull, buf, sizeof(buf), NULL);
    CU_ASSERT(strcmp(ptr, "0") == 0);
 
  #if ENABLE_TRACING
    printf ("Ending tc_os_ulltostr\n");
  #endif
}

int main (int argc, char *argv[])
{
    CU_pSuite suite;

    if (runner_init(argc, argv)){
        goto err_init;
    }

    if ((suite = CU_add_suite ("abstraction_strtoll", suite_abstraction_strtoll_init, suite_abstraction_strtoll_clean)) == NULL){
        goto err;
    }
    if (CU_add_test (suite, "tc_os_strtoll", tc_os_strtoll) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_strtoull", tc_os_strtoull) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_atoll", tc_os_atoll) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_atoull", tc_os_atoull) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_lltostr", tc_os_lltostr) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_ulltostr", tc_os_ulltostr) == NULL) {
        goto err;
    }
    runner_run();
err:
    runner_fini();
err_init:
    return CU_get_error();
}
