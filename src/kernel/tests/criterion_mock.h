/*
 * Because debugging criterion is a big hassle, getting it out of the
 * loop during debugging can save quite a bit of time...
 *
 * Obviously we don't want to do that in the regular run, but it could
 * be helpful during debugging.
 *
 * Replace the criterion include(s) within your test with
 *      #include "criterion_mock.h"
 * This will circumvent criterion and the child process debug issues
 * all together (it can be hard to get it to work properly) and get one
 * nice single thread process with a main() function to easily set a
 * breakpoint at the start of the test.
 *      $ gdb ./<test_app>
          [ .. gdb stuff .. ]
          (gdb) break main
          (gdb) run
          [ .. hit breakpoint .. ]
          (gdb)
 * Enjoy your debugging in the now nice and un-complicated test process.
 */
#ifndef _CRITERION_MOCK_H_
#define _CRITERION_MOCK_H_


#include <math.h>
#include <assert.h>
#include "os/os.h"



/******************* MAIN WRAPPER ***************************/
/* This now only supports on Test() within a test file and
 * only the most simple variation at that. So, this wrapper
 * should become more complex to support those other Test()
 * calls as well... */
#define Test(s, t) int main (int argc, char *argv[])



/******************* LOGGING ***************************/
enum criterion_severity {
    CR_LOG_INFO,
    CR_LOG_WARNING,
    CR_LOG_ERROR,
};
#define cr_log(severity, msg, ...) printf(msg, __VA_ARGS__)
#define cr_log_info   printf
#define cr_log_warn   printf
#define cr_log_error  printf



/******************* BASE ASSERTS ***************************/
#define cr_assert_fail(FormatString, ...)           assert(false);
#define cr_assert(Condition, FormatString, ...)     assert(Condition);
#define cr_assert_not(Condition, FormatString, ...) assert(!Condition);

#define cr_expect_fail(FormatString, ...)    /* Just continue. */
#define cr_expect           cr_assert_not
#define cr_expect_not       cr_assert

#define cr_skip_test(FormatString, ...)    /* Mock not supported. */



/******************* VALUE ASSERTS ***************************/
#define cr_assert_eq(Actual, Expected, FormatString, ...)    assert(Actual == Expected);
#define cr_assert_neq(Actual, Unexpected, FormatString, ...) assert(Actual != Unexpected);
#define cr_assert_lt(Actual, Reference, FormatString, ...)   assert(Actual <  Reference);
#define cr_assert_leq(Actual, Reference, FormatString, ...)  assert(Actual <= Reference);
#define cr_assert_gt(Actual, Reference, FormatString, ...)   assert(Actual >  Reference);
#define cr_assert_geq(Actual, Reference, FormatString, ...)  assert(Actual >= Reference);

#define cr_expect_eq    cr_assert_neq
#define cr_expect_neq   cr_assert_eq
#define cr_expect_lt    cr_assert_geq
#define cr_expect_leq   cr_assert_gt
#define cr_expect_gt    cr_assert_leq
#define cr_expect_geq   cr_assert_lt



/******************* POINTER ASSERTS ***************************/
#define cr_assert_null(Value, FormatString, ...)     assert(Value == NULL);
#define cr_assert_not_null(Value, FormatString, ...) assert(Value != NULL);

#define cr_expect_null          cr_assert_not_null
#define cr_expect_not_null      cr_assert_null



/******************* FLOAT ASSERTS ***************************/
#define cr_assert_float_eq(Actual, Expected, Epsilon, FormatString, ...)    assert(fabs(Actual - Unexpected) > Epsilon);
#define cr_assert_float_neq(Actual, Unexpected, Epsilon, FormatString, ...) assert(fabs(Actual - Unexpected) < Epsilon);

#define cr_expect_float_eq      cr_assert_float_neq
#define cr_expect_float_neq     cr_assert_float_eq



/******************* STRING ASSERTS ***************************/
#define cr_assert_str_empty(Value, FormatString, ...)             assert(Value); assert(strlen(Value) == 0);
#define cr_assert_str_not_empty(Value, FormatString, ...)         assert(Value); assert(strlen(Value) > 0);
#define cr_assert_str_eq(Actual, Expected,   FormatString, ...)   assert(Actual); assert(Expected);   assert(strcmp(Actual, Reference) == 0);
#define cr_assert_str_neq(Actual, Unexpected, FormatString, ...)  assert(Actual); assert(Unexpected); assert(strcmp(Actual, Reference) != 0);
#define cr_assert_str_lt(Actual, Reference, FormatString, ...)    assert(Actual); assert(Reference);  assert(strcmp(Actual, Reference)  < 0);
#define cr_assert_str_leq(Actual, Reference, FormatString, ...)   assert(Actual); assert(Reference);  assert(strcmp(Actual, Reference) <= 0);
#define cr_assert_str_gt(Actual, Reference, FormatString, ...)    assert(Actual); assert(Reference);  assert(strcmp(Actual, Reference)  > 0);
#define cr_assert_str_geq(Actual, Reference, FormatString, ...)   assert(Actual); assert(Reference);  assert(strcmp(Actual, Reference) <= 0);

#define cr_expect_str_empty     cr_assert_str_not_empty
#define cr_expect_str_not_empty cr_assert_str_empty
#define cr_expect_str_neq       cr_assert_str_eq
#define cr_expect_str_eq        cr_assert_str_neq
#define cr_expect_str_lt        cr_assert_str_geq
#define cr_expect_str_leq       cr_assert_str_gt
#define cr_expect_str_gt        cr_assert_str_leq
#define cr_expect_str_geq       cr_assert_str_lt



/******************* MEMORY ***************************/
#define cr_malloc       os_malloc
//#define cr_calloc     os_calloc
#define cr_realloc      os_realloc
#define cr_free         os_free



#endif /* _CRITERION_MOCK_H_ */
