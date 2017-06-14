#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

#define cr_assert_status_eq(s1, s2, ...) cr_assert_eq(dds_err_nr(s1), s2, __VA_ARGS__)

/* Dummy callback */
static void data_available_cb(dds_entity_t reader, void* arg) {}


void publisher_creation(void)
{

  const char *singlePartitions[] = { "partition" };
  const char *multiplePartitions[] = { "partition1", "partition2" };
  const char *duplicatePartitions[] = { "partition", "partition" };

  dds_entity_t participant;
  dds_entity_t publisher, publisher1;
  dds_listener_t *listener;
  dds_qos_t *qos;

  /* Use NULL participant */
  publisher = dds_create_publisher(NULL, NULL, NULL);
  cr_assert_eq(publisher, NULL, "dds_create_publisher(NULL,NULL,NULL)");

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_neq(participant, NULL, "dds_create_participant(DDS_DOMAIN_DEFAULT,NULL,NULL)");

  /* Use non-null participant */
  publisher = dds_create_publisher(participant, NULL, NULL);
  cr_assert_neq(publisher, NULL, "dds_create_publisher(participant,NULL,NULL)");
  dds_delete(publisher);

  /* Use entity that is not a participant */
  publisher1 = dds_create_publisher(publisher, NULL, NULL);
  cr_assert_eq(publisher1, NULL, "dds_create_publisher(publisher,NULL,NULL)");

  /* Create a non-null qos */
  qos = dds_qos_create();
  cr_assert_neq(qos, NULL, "dds_qos_create()");

  /* Use qos without partition; in that case the default partition should be used */
  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_neq(publisher, NULL, "dds_create_publisher(participant,qos,NULL) where qos with default partition");
  dds_delete(publisher);

  /* Use qos with single partition */
  dds_qset_partition (qos, 1, singlePartitions);
  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_neq(publisher, NULL, "dds_create_publisher(participant,qos,NULL) where qos with single partition");
  dds_delete(publisher);

  /* Use qos with multiple partitions */
  dds_qset_partition (qos, 2, multiplePartitions);
  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_neq(publisher, NULL, "dds_create_publisher(participant,qos,NULL) where qos with multiple partitions");
  dds_delete(publisher);

  /* Use qos with multiple partitions */
  dds_qset_partition (qos, 2, duplicatePartitions);
  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_neq(publisher, NULL, "dds_create_publisher(participant,qos,NULL) where qos with duplicate partitions");
  dds_delete(publisher);

  /* Use listener(NULL) */
  listener = dds_listener_create(NULL);
  cr_assert_neq(listener, NULL, "dds_listener_create(NULL)");
  publisher = dds_create_publisher(participant, NULL, listener);
  cr_assert_neq(publisher, NULL, "dds_create_publisher(participant,NULL,listener(NULL))");
  dds_delete(publisher);

  dds_listener_reset(listener);

  /* Use listener for data_available */
  dds_lset_data_available(listener, NULL);
  publisher = dds_create_publisher(participant, NULL, listener);
  cr_assert_neq(publisher, NULL, "dds_create_publisher(participant,NULL,listener) with dds_lset_data_available(listener, NULL)");
  dds_delete(publisher);

  dds_listener_reset(listener);

  /* Use DDS_LUNSET for data_available */
  dds_lset_data_available(listener, DDS_LUNSET);
  publisher = dds_create_publisher(participant, NULL, listener);
  cr_assert_neq(publisher, NULL, "dds_create_publisher(participant,NULL,listener) with dds_lset_data_available(listener, DDS_LUNSET)");
  dds_delete(publisher);

  dds_listener_reset(listener);

  /* Use callback for data_available */
  dds_lset_data_available(listener, data_available_cb);
  publisher = dds_create_publisher(participant, NULL, listener);
  cr_assert_neq(publisher, NULL, "dds_create_publisher(participant,NULL,listener) with dds_lset_data_available(listener, data_available_cb)");
  dds_delete(publisher);

  /* Use both qos setting and callback listener */
  dds_lset_data_available(listener, data_available_cb);
  publisher = dds_create_publisher(participant, qos, listener);
  cr_assert_neq(publisher, NULL, "dds_create_publisher(participant,qos,listener) with dds_lset_data_available(listener, data_available_cb)");
  dds_delete(publisher);

  dds_listener_delete(listener);
  dds_qos_delete(qos);
  dds_delete (participant);
}


void publisher_suspend_resume(void)
{

  dds_entity_t participant, publisher;
  dds_return_t status;

  /* Suspend a NULL publisher */
  status = dds_suspend(NULL);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_suspend(NULL)");

  /* Resume a NULL publisher */
  status = dds_resume(NULL);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_resume(NULL)");

  /* Uae dds_suspend on something else than a publisher */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_neq(participant, NULL, "dds_create_participant(DDS_DOMAIN_DEFAULT,NULL,NULL)");
  status = dds_suspend(participant);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_suspend(participant)");

  /* Use dds_resume on something else than a publisher */
  status = dds_resume(participant);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_resume(participant)");

  /* Use dds_resume without calling dds_suspend */
  publisher = dds_create_publisher(participant, NULL, NULL);
  cr_assert_neq(publisher, NULL, "dds_create_publisher(participant,NULL,NULL)");
  status = dds_resume(publisher);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_resume(publisher) without prior suspend");

  /* Use dds_suspend on non-null publisher */
  status = dds_suspend(publisher);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_suspend(publisher)");

  /* Use dds_resume on non-null publisher */
  status = dds_resume(publisher);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_resume(publisher)");

  dds_delete(publisher);
  dds_delete(participant);

  return;
}

void publisher_wait_for_acks(void)
{
  dds_entity_t participant, publisher;
  dds_return_t status;
  dds_duration_t zeroSec = ((dds_duration_t)DDS_SECS(0));
  dds_duration_t oneSec = ((dds_duration_t)DDS_SECS(1));
  dds_duration_t minusOneSec = ((dds_duration_t)DDS_SECS(-1));

  /* Wait_for_acks on NULL publisher or writer and minusOneSec timeout */
  status = dds_wait_for_acks(NULL, minusOneSec);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(NULL,-1)");

  /* Wait_for_acks on NULL publisher or writer and zeroSec timeout */
  status = dds_wait_for_acks(NULL, zeroSec);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(NULL,0)");

  /* wait_for_acks on NULL publisher or writer and oneSec timeout */
  status = dds_wait_for_acks(NULL, oneSec);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(NULL,1)");

  /* wait_for_acks on NULL publisher or writer and DDS_INFINITE timeout */
  status = dds_wait_for_acks(NULL, DDS_INFINITY);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(NULL,DDS_INFINITY)");

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_neq(participant, NULL, "dds_create_participant(DDS_DOMAIN_DEFAULT,NULL,NULL)");

  /* Wait_for_acks on participant and minusOneSec timeout */
  status = dds_wait_for_acks(participant, minusOneSec);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(participant,-1)");

  /* Wait_for_acks on participant and zeroSec timeout */
  status = dds_wait_for_acks(participant, zeroSec);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(participant,0)");

  /* Wait_for_acks on participant and oneSec timeout */
  status = dds_wait_for_acks(participant, oneSec);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(participant,1)");

  /* Wait_for_acks on participant and DDS_INFINITE timeout */
  status = dds_wait_for_acks(participant, DDS_INFINITY);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(participant,DDS_INFINITY)");

  publisher = dds_create_publisher(participant, NULL, NULL);
  cr_assert_neq(publisher, NULL, "dds_create_publisher(participant,NULL,NULL)");

  /* Wait_for_acks on publisher and minusOneSec timeout */
  status = dds_wait_for_acks(publisher, minusOneSec);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(publisher,-1)");

  /* Wait_for_acks on publisher and zeroSec timeout */
  status = dds_wait_for_acks(publisher, zeroSec);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(publisher,0)");

  /* Wait_for_acks on publisher and oneSec timeout */
  status = dds_wait_for_acks(publisher, oneSec);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(publisher,1)");

  /* Wait_for_acks on publisher and DDS_INFINITE timeout */
  status = dds_wait_for_acks(publisher, DDS_INFINITY);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(publisher,DDS_INFINITY)");

  /* TODO: create tests by calling dds_qwait_for_acks on writers */

  status = dds_suspend(publisher);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_suspend(publisher)");

  /* Wait_for_acks on suspended publisher and minusOneSec timeout */
  status = dds_wait_for_acks(publisher, minusOneSec);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(suspended_publisher,-1)");

  /* Wait_for_acks on suspended publisher and zeroSec timeout */
  status = dds_wait_for_acks(publisher, zeroSec);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(suspended_publisher,0)");

  /* Wait_for_acks on suspended publisher and oneSec timeout */
  status = dds_wait_for_acks(publisher, oneSec);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(suspended_publisher,1)");

  /* Wait_for_acks on suspended publisher and DDS_INFINITE timeout */
  status = dds_wait_for_acks(publisher, DDS_INFINITY);
  cr_assert_status_eq(status, DDS_RETCODE_UNSUPPORTED, "dds_wait_for_acks(suspended_publisher,DDS_INFINITY)");

  dds_delete(publisher);
  dds_delete(participant);

  return;
}

void publisher_coherency(void)
{
  return;
}


Test(vddsc, publisher)
{
    publisher_creation();
    publisher_suspend_resume();
    publisher_wait_for_acks();
    publisher_coherency();
}

