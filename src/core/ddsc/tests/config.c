#include "ddsc/dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "os/os.h"
#include "config_env.h"

#define FORCE_ENV

#define CONFIG_ENV_VAR_SIZE (1000)

#define URI_VARIABLE "VORTEXDDS_URI"
#define MAX_PARTICIPANTS_VARIABLE "MAX_PARTICIPANTS"

static void config__check_env(
    _In_z_ const char * env_variable,
    _In_z_ const char * expected_value)
{
    const char * env_uri = os_getenv(env_variable);

    cr_assert_not_null(env_uri, "Environment variable '%s' isn't set. This needs to be set to '%s' for this test to run.", env_variable, expected_value);
    cr_assert_str_eq(env_uri, expected_value, "Environment variable '%s' has an unexpected value: '%s' (expected: '%s')", env_variable, env_uri, expected_value);
}


static void config__check_set_env(
    _In_z_ const char * env_variable,
    _In_z_ const char * expected_value,
    _Out_  char * static_bfr,
    _In_   int static_bfr_size)
{
#ifdef FORCE_ENV
    bool env_ok;
    const char * env_uri = os_getenv(env_variable);
    const char * const env_not_set = "Environment variable '%s' isn't set. This needs to be set to '%s' for this test to run.";
    const char * const env_not_as_expected = "Environment variable '%s' has an unexpected value: '%s' (expected: '%s')";

    if ( env_uri == NULL ) {
        cr_log_info(env_not_set, env_variable, expected_value);
        env_ok = false;
    } else if ( strncmp(env_uri, expected_value, strlen(expected_value)) != 0 ) {
        cr_log_info(env_not_as_expected, env_variable, env_uri, expected_value);
        env_ok = false;
    } else {
        env_ok = true;
    }

    if ( !env_ok ) {
        os_result r;

        (void) snprintf(static_bfr, static_bfr_size, "%s=%s", env_variable, expected_value);

        r = os_putenv(static_bfr);
        cr_assert_eq(r, os_resultSuccess, "Invoking os_putenv(\"%s\") failed", static_bfr);
        cr_log_warn("Environment variable '%s' set to expected value '%s'", env_variable, expected_value);

        config__check_env(env_variable, expected_value);
    }
#else
    config__check_env(env_variable, expected_value);
#endif /* FORCE_ENV */
}

Test(ddsc_config, simple_udp, .init = os_osInit, .fini = os_osExit) {

    static char uri[CONFIG_ENV_VAR_SIZE];
    static char par[CONFIG_ENV_VAR_SIZE];
    dds_entity_t participant;

    config__check_set_env(URI_VARIABLE, CONFIG_ENV_SIMPLE_UDP, uri, CONFIG_ENV_VAR_SIZE);
    config__check_set_env(MAX_PARTICIPANTS_VARIABLE, CONFIG_ENV_MAX_PARTICIPANTS, par, CONFIG_ENV_VAR_SIZE);

    participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);

    cr_assert_gt(participant, 0, "dds_create_participant");

    dds_delete(participant);
}

Test(ddsc_config, invalid, .init = os_osInit, .fini = os_osExit) {
    static char uri[CONFIG_ENV_VAR_SIZE];
    dds_entity_t participant;

    config__check_set_env(URI_VARIABLE, CONFIG_ENV_INVALID_CONF, uri, CONFIG_ENV_VAR_SIZE);

    /* This would crash (CHAM-540). */
    participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    /* When we get here, then at least it didn't crash. See if we have a proper error. */
    cr_assert_lt(participant, 0, "dds_create_participant");

    dds_delete(participant);
}
