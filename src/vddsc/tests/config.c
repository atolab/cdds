#include "dds.h"
#include <criterion/criterion.h>
#include "os/os.h"
#include "config_env.h"

#define URI_VARIABLE "VORTEXDDS_URI"
#define MAX_PARTICIPANTS_VARIABLE "MAX_PARTICIPANTS"

static void config__check_env(
    _In_z_ const char * env_variable,
    _In_z_ const char * expected_value)
{
    const char * env_uri = os_getenv(env_variable);
    cr_assert_not_null(env_uri, "Environment variable %s isn't set. This needs to be set to %s for this test to run.", env_variable, expected_value);
    cr_assert_str_eq(env_uri, expected_value, "Environment variable %s has an unexpected value: '%s' (expected: '%s')", env_variable, env_uri, expected_value);
}

Test(vddsc_config, simple_udp, .init = os_osInit, .fini = os_osExit) {

    dds_entity_t participant;

    config__check_env(URI_VARIABLE, CONFIG_ENV_SIMPLE_UDP);
    config__check_env(MAX_PARTICIPANTS_VARIABLE, CONFIG_ENV_MAX_PARTICIPANTS);

    participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);

    cr_assert_gt(participant, 0, "dds_create_participant");

    dds_delete(participant);
}
