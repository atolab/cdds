#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

/* We are deliberately testing some bad arguments that SAL will complain about.
 * So, silence SAL regarding these issues. */
#pragma warning(push)
#pragma warning(disable: 6387 28020)

/* Add --verbose command line argument to get the cr_log_info traces (if there are any). */

static dds_entity_t entity = -1;

#define cr_assert_status_eq(s1, s2, ...) cr_assert_eq(dds_err_nr(s1), s2, __VA_ARGS__)

void entity_creation()
{
    /* Use participant as entity in the tests. */
    entity = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(entity, 0, "dds_create_participant");
}

void entity_enabling()
{
    dds_return_t status;

    /* Check enabling with bad parameters. */
    status = dds_enable(0);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_enable (NULL)");

    /* Check actual enabling. */
    /* TODO: CHAM-96: Check enabling.
    status = dds_enable(&entity);
    cr_assert_status_eq(status, dds_err_nr(DDS_RETCODE_OK), "dds_enable (delayed enable)");
    */

    /* Check re-enabling (should be a noop). */
    status = dds_enable(entity);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_enable (already enabled)");
}

void entity_qos_get_set(dds_entity_t e, const char* info)
{
    dds_return_t status;
    dds_qos_t *qos = dds_qos_create();

    /* Get QoS. */
    status = dds_get_qos (entity, qos);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_get_qos(entity, qos) %s", info);

    /* Entity (partition) is enabled, so we shouldn't be able to set QoS. */
    /* Checking all QoS internals (also related to enabled/disabled) should be
     * done by a QoS test and specific 'child' entities. */
    status = dds_set_qos (entity, qos);
    cr_assert_status_eq(status, DDS_RETCODE_IMMUTABLE_POLICY, "dds_set_qos(entity, qos) %s", info);

    dds_qos_delete(qos);
}

void entity_qos()
{
    dds_return_t status;
    dds_qos_t *qos = dds_qos_create();

    /* Don't check inconsistent and immutable policies. That's a job
     * for the specific entity children, not for the generic part. */

    /* Check getting QoS with bad parameters. */
    status = dds_get_qos (0, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_qos(NULL, NULL)");
    status = dds_get_qos (entity, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_qos(entity, NULL)");
    status = dds_get_qos (0, qos);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_qos(NULL, qos)");

    /* Check setting QoS with bad parameters. */
    status = dds_set_qos (0, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_set_qos(NULL, NULL)");
    status = dds_set_qos (entity, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_set_qos(entity, NULL)");
    status = dds_set_qos (0, qos);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_set_qos(NULL, qos)");

    /* Check set/get with entity without initial qos. */
    entity_qos_get_set(entity, "{without initial qos}");

    /* Check set/get with entity with initial qos. */
    {
        dds_entity_t par = dds_create_participant (DDS_DOMAIN_DEFAULT, qos, NULL);
        entity_qos_get_set(par, "{with initial qos}");
        dds_delete(par);
    }

    /* Delete qos. */
    dds_qos_delete(qos);
}

void entity_listeners(void)
{
    dds_return_t status;
    dds_listener_t *l1 = dds_listener_create(NULL);
    dds_listener_t *l2 = dds_listener_create(NULL);
    void *cb1;
    void *cb2;

    /* Don't check actual workings of the listeners. That's a job
     * for the specific entity children, not for the generic part. */

    /* Set some random values for the l2 listener callbacks.
     * I know for sure that these will not be called within this test.
     * Otherwise, the following would let everything crash.
     * We just set them to know for sure that we got what we set. */
    dds_lset_liveliness_changed(l2,         (dds_on_liveliness_changed_fn)          1234);
    dds_lset_requested_deadline_missed(l2,  (dds_on_requested_deadline_missed_fn)   5678);
    dds_lset_requested_incompatible_qos(l2, (dds_on_requested_incompatible_qos_fn)  8765);
    dds_lset_publication_matched(l2,        (dds_on_publication_matched_fn)         4321);

    /* Check getting Listener with bad parameters. */
    status = dds_get_listener (0, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_listener(NULL, NULL)");
    status = dds_get_listener (entity, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_listener(entity, NULL)");
    status = dds_get_listener (0, l1);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_listener(NULL, listener)");

    /* Get Listener, which should be unset. */
    status = dds_get_listener (entity, l1);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_get_listener(entity, listener)");
    dds_lget_liveliness_changed (l1, (dds_on_liveliness_changed_fn*)&cb1);
    cr_assert_eq(cb1, DDS_LUNSET, "Listener not initialized to NULL");
    dds_lget_requested_deadline_missed (l1, (dds_on_requested_deadline_missed_fn*)&cb1);
    cr_assert_eq(cb1, DDS_LUNSET, "Listener not initialized to NULL");
    dds_lget_requested_incompatible_qos (l1, (dds_on_requested_incompatible_qos_fn*)&cb1);
    cr_assert_eq(cb1, DDS_LUNSET, "Listener not initialized to NULL");
    dds_lget_publication_matched (l1, (dds_on_publication_matched_fn*)&cb1);
    cr_assert_eq(cb1, DDS_LUNSET, "Listener not initialized to NULL");

    /* Check setting Listener with bad parameters. */
    status = dds_set_listener (0, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_set_listener(NULL, NULL)");
    status = dds_set_listener (0, l2);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_set_listener(NULL, listener)");

    /* Getting after setting should return set listener. */
    status = dds_set_listener (entity, l2);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_set_listener(entity, listener)");
    status = dds_get_listener (entity, l1);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_get_listener(entity, listener)");
    dds_lget_liveliness_changed (l1, (dds_on_liveliness_changed_fn*)&cb1);
    dds_lget_liveliness_changed (l2, (dds_on_liveliness_changed_fn*)&cb2);
    cr_assert_eq(cb1, cb2, "Listeners are not equal");
    dds_lget_requested_deadline_missed (l1, (dds_on_requested_deadline_missed_fn*)&cb1);
    dds_lget_requested_deadline_missed (l2, (dds_on_requested_deadline_missed_fn*)&cb2);
    cr_assert_eq(cb1, cb2, "Listeners are not equal");
    dds_lget_requested_incompatible_qos (l1, (dds_on_requested_incompatible_qos_fn*)&cb1);
    dds_lget_requested_incompatible_qos (l2, (dds_on_requested_incompatible_qos_fn*)&cb2);
    cr_assert_eq(cb1, cb2, "Listeners are not equal");
    dds_lget_publication_matched (l1, (dds_on_publication_matched_fn*)&cb1);
    dds_lget_publication_matched (l2, (dds_on_publication_matched_fn*)&cb2);
    cr_assert_eq(cb1, cb2, "Listeners are not equal");

    /* Reset listener. */
    status = dds_set_listener (entity, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_set_listener(entity, NULL)");
    status = dds_get_listener (entity, l2);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_get_listener(entity, listener)");
    dds_lget_liveliness_changed (l2, (dds_on_liveliness_changed_fn*)&cb2);
    cr_assert_eq(cb2, DDS_LUNSET, "Listener not reset");
    dds_lget_requested_deadline_missed (l2, (dds_on_requested_deadline_missed_fn*)&cb2);
    cr_assert_eq(cb2, DDS_LUNSET, "Listener not reset");
    dds_lget_requested_incompatible_qos (l2, (dds_on_requested_incompatible_qos_fn*)&cb2);
    cr_assert_eq(cb2, DDS_LUNSET, "Listener not reset");
    dds_lget_publication_matched (l2, (dds_on_publication_matched_fn*)&cb2);
    cr_assert_eq(cb2, DDS_LUNSET, "Listener not reset");

    dds_free(l2);
    dds_free(l1);
}

void entity_status(void)
{
    dds_return_t status;
    uint32_t s1 = 0;

    /* Don't check actual bad statuses. That's a job
     * for the specific entity children, not for the generic part. */

    /* Check getting Status with bad parameters. */
    status = dds_get_enabled_status (0, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_enabled_status(NULL, NULL)");
    status = dds_get_enabled_status (entity, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_enabled_status(entity, NULL)");
    status = dds_get_enabled_status (0, &s1);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_enabled_status(NULL, status)");

    /* Get Status, which should be 0 for a participant. */
    status = dds_get_enabled_status (entity, &s1);
    cr_assert_status_eq(status, DDS_RETCODE_ILLEGAL_OPERATION, "dds_get_enabled_status(entity, status)");
    cr_assert_eq(s1, 0, "Enabled status mask is not 0");

    /* Check setting Status with bad parameters. */
    status = dds_set_enabled_status (0, 0);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_set_enabled_status(NULL, 0)");

    /* I shouldn't be able to set statuses on a participant. */
    status = dds_set_enabled_status (entity, 0);
    cr_assert_status_eq(status, DDS_RETCODE_ILLEGAL_OPERATION, "dds_set_enabled_status(entity, 0)");
    status = dds_set_enabled_status (entity, DDS_DATA_AVAILABLE_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_ILLEGAL_OPERATION, "dds_set_enabled_status(entity, status)");

    /* Check getting Status changes with bad parameters. */
    status = dds_get_status_changes (0, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_status_changes(NULL, NULL)");
    status = dds_get_status_changes (entity, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_status_changes(entity, NULL)");
    status = dds_get_status_changes (0, &s1);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_status_changes(NULL, status)");
    status = dds_get_status_changes (entity, &s1);
    cr_assert_status_eq(status, DDS_RETCODE_ILLEGAL_OPERATION, "dds_get_status_changes(entity, status)");

    /* Status read and take shouldn't work either. */
    status = dds_read_status (0, &s1, 0);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_read_status(NULL, status, 0)");
    status = dds_read_status (entity, &s1, 0);
    cr_assert_status_eq(status, DDS_RETCODE_ILLEGAL_OPERATION, "dds_read_status(entity, status, 0)");
    status = dds_take_status (0, &s1, 0);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_take_status(NULL, status, 0)");
    status = dds_take_status (entity, &s1, 0);
    cr_assert_status_eq(status, DDS_RETCODE_ILLEGAL_OPERATION, "dds_take_status(entity, status, 0)");
}


void entity_handle(void)
{
    dds_return_t status;
    dds_instance_handle_t hdl;

    /* Don't check actual handle contents. That's a job
     * for the specific entity children, not for the generic part. */

    /* Check getting Handle with bad parameters. */
    status = dds_instancehandle_get (0, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_instancehandle_get(NULL, NULL)");
    status = dds_instancehandle_get (entity, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_instancehandle_get(entity, NULL)");
    status = dds_instancehandle_get (0, &hdl);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_instancehandle_get(NULL, handle)");

    /* Get Instance Handle, which should not be 0 for a participant. */
    status = dds_instancehandle_get (entity, &hdl);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_instancehandle_get(entity, handle)");
    cr_assert_neq(hdl, 0, "Entity instance handle is 0");
}

void entity_get_entities(void)
{
    dds_return_t status;
    dds_entity_t par;
    dds_entity_t child;

    /* ---------- Get Parent ------------ */

    /* Check getting Parent with bad parameters. */
    par = dds_get_parent (0);
    cr_assert_eq(dds_err_nr(par), DDS_RETCODE_BAD_PARAMETER, "Parent was returned (despite of bad parameter)");

    /* Get Parent, a participant doesn't have a parent. */
    par = dds_get_parent (entity);
    cr_assert_eq(dds_err_nr(par), DDS_RETCODE_ILLEGAL_OPERATION, "Parent was returned (despite of it being a participant)");

    /* ---------- Get Participant ------------ */

    /* Check getting Participant with bad parameters. */
    par = dds_get_participant (0);
    cr_assert_eq(dds_err_nr(par), DDS_RETCODE_BAD_PARAMETER, "Participant was returned (despite of bad parameter)");

    /* Get Participant, a participants' participant is itself. */
    par = dds_get_participant (entity);
    cr_assert_eq(par, entity, "Returned participant was not expected");

    /* ---------- Get Children ------------ */

    /* Check getting Children with bad parameters. */
    status = dds_get_children (0, &child, 1);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_children(NULL, child, 1)");
    status = dds_get_children (entity, NULL, 1);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_children(entity, NULL, 1)");
    status = dds_get_children (entity, &child, 0);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_children(entity, child, 0)");
    status = dds_get_children (0, NULL, 1);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_children(NULL, NULL, 1)");
    status = dds_get_children (0, &child, 0);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_children(NULL, child, 0)");

    /* Get Children, of which there are currently none. */
    status = dds_get_children (entity, NULL, 0);
    if (status > 0) {
        cr_assert("dds_get_children(entity, NULL, 0) un-expectantly found children");
    } else {
        cr_assert_eq(status, 0, "dds_get_children(entity, NULL, 0) failed");
    }
    status = dds_get_children (entity, &child, 1);
    if (status > 0) {
        cr_assert("dds_get_children(entity, child, 1) un-expectantly returned children");
    } else {
        cr_assert_eq(status, 0, "dds_get_children(entity, child, 1) failed");
    }
}

void entity_get_domainid(void)
{
    dds_return_t status;
    dds_domainid_t id;

    /* Check getting ID with bad parameters. */
    status = dds_get_domainid (0, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_domainid(NULL, NULL)");
    status = dds_get_domainid (entity, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_domainid(entity, NULL)");
    status = dds_get_domainid (0, &id);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_domainid(NULL, id)");

    /* Get and check the domain id. */
    status = dds_get_domainid (entity, &id);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_get_domainid(entity, id)");
    cr_assert_eq(id, 0, "Different domain_id was returned than expected");
}

void entity_deletion(void)
{
    dds_return_t status;
    status = dds_delete(0);
    cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_delete(NULL)");
    status = dds_delete(entity);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_delete(entity)");
    entity = 0;
}

Test(vddsc, entity_api)
{
    entity_creation();
    entity_enabling();
    entity_qos();
    entity_listeners();
    entity_status();
    entity_handle();
    entity_get_entities();
    entity_get_domainid();
    entity_deletion();

}

#pragma warning(pop)
