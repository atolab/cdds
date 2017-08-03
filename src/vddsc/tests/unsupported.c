#include <stdio.h>
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <criterion/parameterized.h>

#include "dds.h"
#include "RoundTrip.h"

static dds_entity_t e[7];

#define PAR (0)
#define TOP (1)
#define PUB (2)
#define WRI (3)
#define SUB (4)
#define REA (5)
#define BAD (6)

static const char *entity_kind_str(dds_entity_t e) {
    if(e <= 0) {
        return "(ERROR)";
    }
    switch(e & DDS_ENTITY_KIND_MASK) {
        case DDS_KIND_TOPIC:        return "Topic";
        case DDS_KIND_PARTICIPANT:  return "Participant";
        case DDS_KIND_READER:       return "Reader";
        case DDS_KIND_WRITER:       return "Writer";
        case DDS_KIND_SUBSCRIBER:   return "Subscriber";
        case DDS_KIND_PUBLISHER:    return "Publisher";
        case DDS_KIND_COND_READ:    return "ReadCondition";
        case DDS_KIND_COND_QUERY:   return "QueryCondition";
        case DDS_KIND_WAITSET:      return "WaitSet";
        default:                    return "(INVALID_ENTITY)";
    }
}

static void
setup(void)
{
    e[PAR] = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(e[PAR], 0);
    e[TOP] = dds_create_topic(e[PAR], &RoundTripModule_DataType_desc, "RoundTrip", NULL, NULL);
    cr_assert_gt(e[TOP], 0);
    e[PUB] = dds_create_publisher(e[PAR], NULL, NULL);
    cr_assert_gt(e[PUB], 0);
    e[WRI] = dds_create_writer(e[PUB], e[TOP], NULL, NULL);
    cr_assert_gt(e[WRI], 0);
    e[SUB] = dds_create_subscriber(e[PAR], NULL, NULL);
    cr_assert_gt(e[SUB], 0);
    e[REA] = dds_create_reader(e[SUB], e[TOP], NULL, NULL);
    cr_assert_gt(e[REA], 0);
    e[BAD] = 1;
}

static void
teardown(void)
{
    for(unsigned i = (sizeof e / sizeof *e); i > 0; i--) {
        dds_delete(e[i - 1]);
    }
}

/*************************************************************************************************/
struct index_result {
    unsigned index;
    dds_return_t exp_res;
};

/*************************************************************************************************/
ParameterizedTestParameters(vddsc_unsupported, dds_begin_end_coherent) {
    /* The parameters seem to be initialized before spawning children,
     * so it makes no sense to try and store anything dynamic here. */
    static struct index_result pars[] = {
       {PUB, dds_err_nr(DDS_RETCODE_UNSUPPORTED)},
       {WRI, dds_err_nr(DDS_RETCODE_UNSUPPORTED)},
       {SUB, dds_err_nr(DDS_RETCODE_UNSUPPORTED)},
       {REA, dds_err_nr(DDS_RETCODE_UNSUPPORTED)},
       {BAD, dds_err_nr(DDS_RETCODE_BAD_PARAMETER)}
    };

    return cr_make_param_array(struct index_result, pars, sizeof pars / sizeof *pars);
};

ParameterizedTest(struct index_result *par, vddsc_unsupported, dds_begin_end_coherent, .init = setup, .fini = teardown)
{
    dds_return_t result;
    result = dds_begin_coherent(e[par->index]);
    cr_expect_eq(dds_err_nr(result), dds_err_nr(par->exp_res), "Unexpected return code %d \"%s\" (expected %d \"%s\") from dds_begin_coherent(%s): (%d)", dds_err_nr(result), dds_err_str(result), par->exp_res, dds_err_str(par->exp_res), entity_kind_str(e[par->index]), result);

    result = dds_end_coherent(e[par->index]);
    cr_expect_eq(dds_err_nr(result), dds_err_nr(par->exp_res), "Unexpected return code %d \"%s\" (expected %d \"%s\") from dds_end_coherent(%s): (%d)", dds_err_nr(result), dds_err_str(result), par->exp_res, dds_err_str(par->exp_res), entity_kind_str(e[par->index]), result);
}

/*************************************************************************************************/
ParameterizedTestParameters(vddsc_unsupported, dds_wait_for_acks) {
    static struct index_result pars[] = {
       {PUB, dds_err_nr(DDS_RETCODE_UNSUPPORTED)},
       {WRI, dds_err_nr(DDS_RETCODE_UNSUPPORTED)},
       {BAD, dds_err_nr(DDS_RETCODE_BAD_PARAMETER)}
    };

    return cr_make_param_array(struct index_result, pars, sizeof pars / sizeof *pars);
};

ParameterizedTest(struct index_result *par, vddsc_unsupported, dds_wait_for_acks, .init = setup, .fini = teardown)
{
    dds_return_t result;
    result = dds_wait_for_acks(e[par->index], 0);
    cr_expect_eq(dds_err_nr(result), dds_err_nr(par->exp_res), "Unexpected return code %d \"%s\" (expected %d \"%s\") from dds_wait_for_acks(%s, 0): (%d)", dds_err_nr(result), dds_err_str(result), par->exp_res, dds_err_str(par->exp_res), entity_kind_str(e[par->index]), result);
}

/*************************************************************************************************/
ParameterizedTestParameters(vddsc_unsupported, dds_suspend_resume) {
    static struct index_result pars[] = {
       {PUB, dds_err_nr(DDS_RETCODE_UNSUPPORTED)},
       {WRI, dds_err_nr(DDS_RETCODE_BAD_PARAMETER)},
       {BAD, dds_err_nr(DDS_RETCODE_BAD_PARAMETER)}
    };

    return cr_make_param_array(struct index_result, pars, sizeof pars / sizeof *pars);
};

ParameterizedTest(struct index_result *par, vddsc_unsupported, dds_suspend_resume, .init = setup, .fini = teardown)
{
    dds_return_t result;
    result = dds_suspend(e[par->index]);
    cr_expect_eq(dds_err_nr(result), dds_err_nr(par->exp_res), "Unexpected return code %d \"%s\" (expected %d \"%s\") from dds_suspend(%s): (%d)", dds_err_nr(result), dds_err_str(result), par->exp_res, dds_err_str(par->exp_res), entity_kind_str(e[par->index]), result);

    result = dds_resume(e[par->index]);
    cr_expect_eq(dds_err_nr(result), dds_err_nr(par->exp_res), "Unexpected return code %d \"%s\" (expected %d \"%s\") from dds_resume(%s): (%d)", dds_err_nr(result), dds_err_str(result), par->exp_res, dds_err_str(par->exp_res), entity_kind_str(e[par->index]), result);
}
