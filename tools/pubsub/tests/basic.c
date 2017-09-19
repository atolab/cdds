#include <criterion/criterion.h>
#include <criterion/logging.h>

#define MAIN test_main
#include "../pubsub.c"

Test(tools_pubsub, main) {
    char *argv[] = {"pubsub", "-T", "pubsubTestTopic", "-K", "KS", "-w1", "-D", "0.3", "-q", "t:d=t,r=r", "pubsub_partition"};
    int argc = sizeof(argv) / sizeof(char*);

    cr_log_info("Starting pubsub basic test");
    int result = MAIN(argc, argv);
    if (result != 0)
        printf("exitcode was %d\n", result);
    cr_assert_eq(result, 0, "pubsub exited non-zero");
    cr_log_info("Stopping pubsub basic test");
}
