#define RTLD_DEFAULT	((void *) 0)

//#include <dlfcn.h>
#include <time.h>
#include <string.h>
//#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include <inttypes.h>
#include <signal.h>
#include <unistd.h>
#include <stdarg.h>
#include <math.h>

#include "testtype.h"
#include "common.h"
//#include "porting.h"
#include "os/os.h"

//#define DEBUG
#ifdef DEBUG
#define PRINTD printf
#else
#define PRINTD(...)
#endif

#ifndef DDS_DOMAIN_DEFAULT /* pre-6.0 */
#define DDS_DOMAIN_DEFAULT 0
#endif

enum qostype {
  QT_TOPIC,
  QT_PUBLISHER,
  QT_SUBSCRIBER,
  QT_READER,
  QT_WRITER
};

struct qos {
  enum qostype qt;
  union {
    struct {
    	dds_qos_t *q;
    } topic;
    struct {
    	dds_qos_t *q;
    } pub;
    struct {
    	dds_qos_t *q;
    } sub;
    struct {
    	dds_entity_t t;
    	dds_entity_t s;
    	dds_qos_t *q;
    } rd;
    struct {
    	dds_entity_t t;
    	dds_entity_t p;
    	dds_qos_t *q;
    } wr;
  } u;
};

dds_entity_t dp = 0;
dds_entity_t qosprov = 0;
const dds_topic_descriptor_t *ts_KeyedSeq;
const dds_topic_descriptor_t *ts_Keyed32;
const dds_topic_descriptor_t *ts_Keyed64;
const dds_topic_descriptor_t *ts_Keyed128;
const dds_topic_descriptor_t *ts_Keyed256;
const dds_topic_descriptor_t *ts_OneULong;

const char *saved_argv0;

unsigned long long nowll (void)
{
  os_time t = os_timeGet ();
  return (unsigned long long) (t.tv_sec * 1000000000ll + t.tv_nsec);
}

////void nowll_as_ddstime (DDS_Time_t *t)
////{
////  os_time ost = os_timeGet ();
////  t->sec = ost.tv_sec;
////  t->nanosec = (DDS_unsigned_long) ost.tv_nsec;
////}
//
//void bindelta (unsigned long long *bins, unsigned long long d, unsigned repeat)
//{
//  int bin = 0;
//  while (d)
//  {
//    bin++;
//    d >>= 1;
//  }
//  bins[bin] += repeat;
//}
//
//void binprint (unsigned long long *bins, unsigned long long telapsed)
//{
//  unsigned long long n;
//  unsigned i, minbin = BINS_LENGTH- 1, maxbin = 0;
//  n = 0;
//  for (i = 0; i < BINS_LENGTH; i++)
//  {
//    n += bins[i];
//    if (bins[i] && i < minbin)
//      minbin = i;
//    if (bins[i] && i > maxbin)
//      maxbin = i;
//  }
//  printf ("< 2**n | %llu in %.06fs avg %.1f/s\n", n, telapsed * 1e-9, n / (telapsed * 1e-9));
//  for (i = minbin; i <= maxbin; i++)
//  {
//    static const char ats[] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
//    double pct = 100.0 * (double) bins[i] / n;
//    int nats = (int) ((pct / 100.0) * (sizeof (ats) - 1));
//    printf ("%2d: %6.2f%% %*.*s\n", i, pct, nats, nats, ats);
//  }
//}

struct hist {
  unsigned nbins;
  uint64_t binwidth;
  uint64_t bin0; /* bins are [bin0,bin0+binwidth),[bin0+binwidth,bin0+2*binwidth) */
  uint64_t binN; /* bin0 + nbins*binwidth */
  uint64_t min, max; /* min and max observed since last reset */
  uint64_t under, over; /* < bin0, >= binN */
  uint64_t bins[];
};

struct hist *hist_new (unsigned nbins, uint64_t binwidth, uint64_t bin0)
{
  struct hist *h = os_malloc (sizeof (*h) + nbins * sizeof (*h->bins));
  h->nbins = nbins;
  h->binwidth = binwidth;
  h->bin0 = bin0;
  h->binN = h->bin0 + h->nbins * h->binwidth;
  hist_reset (h);
  return h;
}

void hist_free (struct hist *h)
{
  os_free (h);
}

void hist_reset_minmax (struct hist *h)
{
  h->min = UINT64_MAX;
  h->max = 0;
}

void hist_reset (struct hist *h)
{
  hist_reset_minmax (h);
  h->under = 0;
  h->over = 0;
  memset (h->bins, 0, h->nbins * sizeof (*h->bins));
}

void hist_record (struct hist *h, uint64_t x, unsigned weight)
{
  if (x < h->min)
    h->min = x;
  if (x > h->max)
    h->max = x;
  if (x < h->bin0)
    h->under += weight;
  else if (x >= h->binN)
    h->over += weight;
  else
    h->bins[(x - h->bin0) / h->binwidth] += weight;
}

static void xsnprintf (char *buf, size_t bufsz, size_t *p, const char *fmt, ...)
{
  if (*p < bufsz)
  {
    int n;
    va_list ap;
    va_start(ap, fmt);
    n = os_vsnprintf(buf + *p, bufsz - *p, fmt, ap);
    va_end(ap);
    *p += (size_t)n;
  }
}

void hist_print (struct hist *h, uint64_t dt, int reset)
{
//  char l[h->nbins + 200];
  char *l = (char *) os_malloc(sizeof(char) * (h->nbins + 200));  //variable size array malloc
  char *hist = (char *) os_malloc(sizeof(char) * (h->nbins + 1)); //variable size array malloc
//  char hist[h->nbins+1];
  double dt_s = dt / 1e9, avg;
  uint64_t peak = 0, cnt = h->under + h->over;
  size_t p = 0;
  hist[h->nbins] = 0;
  for (unsigned i = 0; i < h->nbins; i++)
  {
    cnt += h->bins[i];
    if (h->bins[i] > peak)
      peak = h->bins[i];
  }

  const uint64_t p1 = peak / 100;
  const uint64_t p10 = peak / 10;
  const uint64_t p20 = 1 * peak / 5;
  const uint64_t p40 = 2 * peak / 5;
  const uint64_t p60 = 3 * peak / 5;
  const uint64_t p80 = 4 * peak / 5;
  for (unsigned i = 0; i < h->nbins; i++)
  {
    if (h->bins[i] == 0) hist[i] = ' ';
    else if (h->bins[i] <= p1) hist[i] = '.';
    else if (h->bins[i] <= p10) hist[i] = '_';
    else if (h->bins[i] <= p20) hist[i] = '-';
    else if (h->bins[i] <= p40) hist[i] = '=';
    else if (h->bins[i] <= p60) hist[i] = 'x';
    else if (h->bins[i] <= p80) hist[i] = 'X';
    else hist[i] = '@';
  }

  avg = cnt / dt_s;
  if (avg < 999.5)
    xsnprintf (l, sizeof(l), &p, "%5.3g", avg);
  else if (avg < 1e6)
    xsnprintf (l, sizeof(l), &p, "%4.3gk", avg / 1e3);
  else
    xsnprintf (l, sizeof(l), &p, "%4.3gM", avg / 1e6);
  xsnprintf (l, sizeof(l), &p, "/s (");

  if (cnt < (uint64_t) 10e3)
    xsnprintf (l, sizeof(l), &p, "%5"PRIu64" ", cnt);
  else if (cnt < (uint64_t) 1e6)
    xsnprintf (l, sizeof(l), &p, "%5.1fk", cnt / 1e3);
  else
    xsnprintf (l, sizeof(l), &p, "%5.1fM", cnt / 1e6);

  xsnprintf (l, sizeof(l), &p, " in %.1fs) ", dt_s);

  if (h->min == UINT64_MAX)
    xsnprintf (l, sizeof(l), &p, " inf ");
  else if (h->min < 1000)
    xsnprintf (l, sizeof(l), &p, "%3"PRIu64"n ", h->min);
  else if (h->min + 500 < 1000000)
    xsnprintf (l, sizeof(l), &p, "%3"PRIu64"u ", (h->min + 500) / 1000);
  else if (h->min + 500000 < 1000000000)
    xsnprintf (l, sizeof(l), &p, "%3"PRIu64"m ", (h->min + 500000) / 1000000);
  else
    xsnprintf (l, sizeof(l), &p, "%3"PRIu64"s ", (h->min + 500000000) / 1000000000);

  if (h->bin0 > 0)
  {
    int pct = (cnt == 0) ? 0 : 100 * (int) ((h->under + cnt/2) / cnt);
    xsnprintf (l, sizeof(l), &p, "%3d%% ", pct);
  }

  {
    int pct = (cnt == 0) ? 0 : 100 * (int) ((h->over + cnt/2) / cnt);
    xsnprintf (l, sizeof(l), &p, "|%s| %3d%%", hist, pct);
  }

  if (h->max < 1000)
    xsnprintf (l, sizeof(l), &p, " %3"PRIu64"n", h->max);
  else if (h->max + 500 < 1000000)
    xsnprintf (l, sizeof(l), &p, " %3"PRIu64"u", (h->max + 500) / 1000);
  else if (h->max + 500000 < 1000000000)
    xsnprintf (l, sizeof(l), &p, " %3"PRIu64"m", (h->max + 500000) / 1000000);
  else
    xsnprintf (l, sizeof(l), &p, " %3"PRIu64"s", (h->max + 500000000) / 1000000000);

  (void) p;
  puts (l);
  os_free(l);
  os_free(hist);
  if (reset)
    hist_reset (h);
}

void error (const char *fmt, ...)
{
  va_list ap;
  fprintf (stderr, "%s: error: ", saved_argv0);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  exit (2);
}

const char *dds_strerror (int code)
{
  switch (code)
  {
    case DDS_RETCODE_OK: return "ok";
    case DDS_RETCODE_ERROR: return "error";
    case DDS_RETCODE_UNSUPPORTED: return "unsupported";
    case DDS_RETCODE_BAD_PARAMETER: return "bad parameter";
    case DDS_RETCODE_PRECONDITION_NOT_MET: return "precondition not met";
    case DDS_RETCODE_OUT_OF_RESOURCES: return "out of resources";
    case DDS_RETCODE_NOT_ENABLED: return "not enabled";
    case DDS_RETCODE_IMMUTABLE_POLICY: return "immutable policy";
    case DDS_RETCODE_INCONSISTENT_POLICY: return "inconsistent policy";
    case DDS_RETCODE_ALREADY_DELETED: return "already deleted";
    case DDS_RETCODE_TIMEOUT: return "timeout";
    case DDS_RETCODE_NO_DATA: return "no data";
    case DDS_RETCODE_ILLEGAL_OPERATION: return "illegal operation";
    default: return "(undef)";
  }
}

void save_argv0 (const char *argv0)
{
  saved_argv0 = argv0;
}

int common_init (const char *argv0)
{
	save_argv0 (argv0);
	PRINTD("common_init: Before creating domain participant=%p\n",dp);
	dp = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL); // changed method name
//	DDS_ENTITY_CHECK (dp, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

	PRINTD("common_init: Domain participant=%p created\n",dp);

	ts_KeyedSeq = &KeyedSeq_desc;
	ts_Keyed32 = &Keyed32_desc;
	ts_Keyed64 = &Keyed64_desc;
	ts_Keyed128 = &Keyed128_desc;
	ts_Keyed256 = &Keyed256_desc;
	ts_OneULong = &OneULong_desc;
	return 0;
}

void common_fini (void)
{
	dds_delete(qosprov); //Todo: method name changed
	PRINTD("common_fini: Deleting domain participant=%p\n",dp);
	dds_delete(dp);
	PRINTD("common_fini: Domain participant=%p deleted\n",dp);
}

int change_publisher_partitions (dds_entity_t pub, unsigned npartitions, const char *partitions[])
{
	dds_qos_t *qos;
	int rc;

	if ((qos = dds_qos_create()) == NULL)
		return DDS_RETCODE_OUT_OF_RESOURCES;

	rc = dds_get_qos(pub, qos);	//Todo: Changed mathod signature.
	dds_qset_partition(qos, npartitions, partitions);
	rc = dds_set_qos(pub, qos);
	dds_qos_delete(qos);
	return rc;
}

int change_subscriber_partitions (dds_entity_t sub, unsigned npartitions, const char *partitions[])
{
  dds_qos_t *qos;
  int rc;

  if ((qos = dds_qos_create()) == NULL)
    return DDS_RETCODE_OUT_OF_RESOURCES;

  rc = dds_get_qos(sub, qos);
  dds_qset_partition(qos, npartitions, partitions);
  rc = dds_set_qos (sub, qos);
  dds_qos_delete(qos);
  return rc;
}

static dds_qos_t *get_topic_qos (dds_entity_t t)
{
  dds_qos_t *tQos = dds_qos_create();
  if ((tQos) == NULL)
      error ("get_topic_qos: dds_qos_create\n");
  int rc = dds_get_qos(t, tQos);
  if(tQos == NULL)
    error ("dds_qos_get_topic_qos\n");
  return tQos;
}

struct qos *new_tqos (void)
{
  struct qos *a;
  if ((a = os_malloc (sizeof (*a))) == NULL)
    error ("new_tqos: os_malloc\n");
  a->qt = QT_TOPIC;
  a->u.topic.q = dds_qos_create();
  if ((a->u.topic.q) == NULL)
    error ("new_tqos: dds_qos_create\n");
//  dds_get_default_topic_qos(a->u.topic.q); //Todo: Removed from ddsv2 since it cant be set anyway. an empty qos behaves the same by definition.

  /* Not all defaults are those of DCPS: */
  dds_qset_reliability(a->u.topic.q, DDS_RELIABILITY_RELIABLE, DDS_SECS(1));
  dds_qset_destination_order(a->u.topic.q, DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP);
  return a;
}

struct qos *new_pubqos (void)
{
  struct qos *a;
  if ((a = os_malloc (sizeof (*a))) == NULL)
    error ("new_pubqos: os_malloc\n");
  a->qt = QT_PUBLISHER;
  a->u.pub.q = dds_qos_create();
  if ((a->u.pub.q) == NULL)
    error ("new_pubqos: dds_qos_create\n");
//  dds_get_default_publisher_qos(a->u.pub.q); //Todo: Removed from ddsv2 since it cant be set anyway. an empty qos behaves the same by definition.
  return a;
}

struct qos *new_subqos (void)
{
  struct qos *a;
  if ((a = os_malloc (sizeof (*a))) == NULL)
    error ("new_subqos: os_malloc\n");
  a->qt = QT_SUBSCRIBER;
  a->u.sub.q = dds_qos_create();
  if ((a->u.sub.q) == NULL)
      error ("new_subqos: dds_qos_create\n");
//  dds_get_default_subscriber_qos(a->u.sub.q);//Todo: Removed from ddsv2 since it cant be set anyway. an empty qos behaves the same by definition.
  return a;
}

dds_qos_t *new_subqosNew (void)
{
  dds_qos_t *qosAll = dds_qos_create();
//  dds_get_default_subscriber_qos(qosAll);//Todo: Removed from ddsv2 since it cant be set anyway. an empty qos behaves the same by definition.
  return qosAll;
}

struct qos *new_rdqos (dds_entity_t s, dds_entity_t t)
{
	dds_qos_t *tQos = get_topic_qos (t);
	struct qos *a;
	if ((a = os_malloc (sizeof (*a))) == NULL)
		error ("new_rdqos: os_malloc\n");
	a->qt = QT_READER;
	a->u.rd.t = t;
	a->u.rd.s = s;
	a->u.rd.q = dds_qos_create();
	if ((a->u.rd.q) == NULL)
		error ("new_rdqos: dds_qos_create\n");
//	dds_get_default_reader_qos(a->u.rd.q);//Todo: Removed from ddsv2 since it cant be set anyway. an empty qos behaves the same by definition.
	if(tQos != NULL)
		dds_qos_copy(a->u.rd.q, tQos);
	dds_qos_delete(tQos);
	return a;
}

struct qos *new_wrqos (dds_entity_t p, dds_entity_t t)
{
	dds_qos_t *tQos = get_topic_qos (t);
	struct qos *a;
	if ((a = os_malloc (sizeof (*a))) == NULL)
		error ("new_wrqos: os_malloc\n");
	a->qt = QT_WRITER;
	a->u.wr.t = t;
	a->u.wr.p = p;
	a->u.wr.q = dds_qos_create();
	if ((a->u.wr.q) == NULL)
		error ("new_wrqos: dds_qos_create\n");
//	dds_get_default_writer_qos(a->u.wr.q);//Todo: Removed from ddsv2 since it cant be set anyway. an empty qos behaves the same by definition.
	if(tQos != NULL)
		dds_qos_copy(a->u.wr.q, tQos);
	dds_qos_delete(tQos);

	/* Not all defaults are those of DCPS: */
	dds_qset_writer_data_lifecycle(a->u.wr.q, false);
	return a;
}

void free_qos (struct qos *a)
{
  switch (a->qt)
  {
    case QT_TOPIC: dds_qos_delete (a->u.topic.q); break;
    case QT_PUBLISHER: dds_qos_delete (a->u.pub.q); break;
    case QT_SUBSCRIBER: dds_qos_delete (a->u.sub.q); break;
    case QT_READER: dds_qos_delete (a->u.rd.q); break;
    case QT_WRITER: dds_qos_delete (a->u.rd.q); break;
  }
  os_free (a);
}

dds_entity_t new_topic (const char *name, const dds_topic_descriptor_t *topicDesc, const struct qos *a)
{
	dds_entity_t tp;
//	int ret = 0;
	if (a->qt != QT_TOPIC)
		error ("new_topic called with non-topic qos\n");

	tp = dds_create_topic(dp, topicDesc, name, a->u.topic.q, NULL);
//	errorMsg(ret, "new_topic: dds_topic_create");
	printf("topic create: return number: %d\n str: %s\nreturn code: %d\n", tp, dds_err_str(tp), dds_err_nr(tp));

	return tp;
}

dds_entity_t new_topic_KeyedSeq (const char *name, const struct qos *a)
{
  return new_topic (name, ts_KeyedSeq, a);
}

dds_entity_t new_topic_Keyed32 (const char *name, const struct qos *a)
{
  return new_topic (name, ts_Keyed32, a);
}

dds_entity_t new_topic_Keyed64 (const char *name, const struct qos *a)
{
  return new_topic (name, ts_Keyed64, a);
}

dds_entity_t new_topic_Keyed128 (const char *name, const struct qos *a)
{
  return new_topic (name, ts_Keyed128, a);
}

dds_entity_t new_topic_Keyed256 (const char *name, const struct qos *a)
{
  return new_topic (name, ts_Keyed256, a);
}

dds_entity_t new_topic_OneULong (const char *name, const struct qos *a)
{
  return new_topic (name, ts_OneULong, a);
}

dds_entity_t new_publisher (const struct qos *a, unsigned npartitions, const char **partitions)
{
	dds_qos_t *pQos = dds_qos_create();
	dds_entity_t p = 0;
	if (pQos == NULL)
		error ("new_publisher: dds_qos_create\n");

	if (a == NULL) {
//		dds_get_default_publisher_qos(pQos);//Todo: Removed from ddsv2 since it cant be set anyway. an empty qos behaves the same by definition.
		dds_qset_partition(pQos, npartitions, partitions);
		p = dds_create_publisher(dp, pQos, NULL); // Todo: Changed method name
//		DDS_ENTITY_CHECK (p, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
	} else {
		if (a->qt != QT_PUBLISHER)
		  error ("new_topic called with non-publisher qos\n");
		dds_qset_partition(a->u.pub.q, npartitions, partitions);
		p = dds_create_publisher(dp, a->u.pub.q, NULL); // Todo: Changed method name
//		DDS_ENTITY_CHECK (p, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
	}

	dds_qos_delete(pQos);
	return p;
}

dds_entity_t new_subscriber (const struct qos *a, unsigned npartitions, const char **partitions)
{
	int ret = 0;
	dds_qos_t *sQos = dds_qos_create();
	dds_entity_t s = 0;
	if (sQos == NULL)
	  error ("new_subscriber: dds_qos_create\n");

	if (a == NULL) {
//		dds_get_default_subscriber_qos(sQos);//Todo: Removed from ddsv2 since it cant be set anyway. an empty qos behaves the same by definition.
		dds_qset_partition(sQos, npartitions, partitions);
		s = dds_create_subscriber(dp, sQos, NULL);
		errorMsg(ret, "new_subscriber: dds_subscriber_create\n");
	} else {
		if (a->qt != QT_SUBSCRIBER)
		  error ("new_topic called with non-subscriber qos\n");
		dds_qset_partition(a->u.sub.q, npartitions, partitions);
		s = dds_create_subscriber(dp, a->u.sub.q, NULL);
		errorMsg(ret, "new_subscriber: dds_subscriber_create\n");
	}
	dds_qos_delete(sQos);
	return s;
}

dds_entity_t new_datawriter_listener (const struct qos *a, const dds_listener_t *l)
{
	dds_entity_t wr;
	if (a->qt != QT_WRITER) {
		error ("new_datawriter called with non-writer qos\n");
	}
	if ((wr = dds_create_writer(a->u.wr.p, a->u.wr.t, a->u.wr.q, l)) < DDS_RETCODE_OK) {
		error ("dds_writer_create failed with value %d: %s\n",dds_err_nr(wr),dds_err_str(wr));
	}
	return wr;
}

dds_entity_t new_datawriter (const struct qos *a)
{
  return new_datawriter_listener (a, NULL);
}

dds_entity_t new_datareader_listener (const struct qos *a, const dds_listener_t *l)
{
	int ret;
	dds_entity_t rd = 0;
	if (a->qt != QT_READER)
		error ("new_datareader called with non-reader qos\n");
	if((rd = dds_create_reader(a->u.rd.s, a->u.rd.t, a->u.rd.q, l)) < DDS_RETCODE_OK) { //Todo: signature changed
		error ("dds_reader_create failed with value %d: %s\n",dds_err_nr(rd),dds_err_str(rd));
	}
	return rd;
}

dds_entity_t new_datareader (const struct qos *a)
{
  return new_datareader_listener (a, NULL);
}

static void inapplicable_qos(const struct qos *a, const char *n)
{
  const char *en = "?";
  switch (a->qt)
  {
    case QT_TOPIC: en = "topic"; break;
    case QT_PUBLISHER: en = "publisher"; break;
    case QT_SUBSCRIBER: en = "subscriber"; break;
    case QT_WRITER: en = "writer"; break;
    case QT_READER: en = "reader"; break;
  }
  fprintf(stderr, "warning: %s entity ignoring inapplicable QoS \"%s\"\n", en, n);
}

#define get_qos_TRW(a, n) (((a)->qt == QT_TOPIC) ? (a)->u.topic.q : ((a)->qt == QT_READER) ? (a)->u.rd.q : ((a)->qt == QT_WRITER) ? (a)->u.wr.q : (inapplicable_qos((a), n), NULL))
#define get_qos_TW(a, n) (((a)->qt == QT_TOPIC) ? (a)->u.topic.q : ((a)->qt == QT_WRITER) ? (a)->u.wr.q : (inapplicable_qos((a), n), NULL))
#define get_qos_RW(a, n) (((a)->qt == QT_READER) ? (a)->u.rd.q : ((a)->qt == QT_WRITER) ? (a)->u.wr.q : (inapplicable_qos((a), n), NULL))
#define get_qos_T(a, n) (((a)->qt != QT_TOPIC) ? (inapplicable_qos((a), n), NULL) : (a)->u.topic.q)
#define get_qos_R(a, n) (((a)->qt != QT_READER) ? (inapplicable_qos((a), n), NULL) : (a)->u.rd.q)
#define get_qos_W(a, n) (((a)->qt != QT_WRITER) ? (inapplicable_qos((a), n), NULL) : (a)->u.wr.q)
#define get_qos_PS(a, n) (((a)->qt == QT_PUBLISHER) ? (a)->u.pub.q : ((a)->qt == QT_SUBSCRIBER) ? (a)->u.sub.q : (inapplicable_qos((a), n), NULL))

const dds_qos_t *qos_datawriter(const struct qos *a)
{
  return a->qt == QT_WRITER ? a->u.wr.q : NULL;
}

void qos_durability (struct qos *a, const char *arg)
{
	dds_qos_t *qp = get_qos_TRW(a, "durability");
	if (qp == NULL)
		return;
	if (strcmp (arg, "v") == 0)
		dds_qset_durability(qp, DDS_DURABILITY_VOLATILE);
	else if (strcmp (arg, "tl") == 0)
		dds_qset_durability(qp, DDS_DURABILITY_TRANSIENT_LOCAL);
	else if (strcmp (arg, "t") == 0)
		dds_qset_durability(qp, DDS_DURABILITY_TRANSIENT);
	else if (strcmp (arg, "p") == 0)
		dds_qset_durability(qp, DDS_DURABILITY_PERSISTENT);
	else
		error ("durability qos: %s: invalid\n", arg);
}

char* enumValue(const struct qos *a) {
	if(a->qt==QT_SUBSCRIBER)
		return "SUBSCRIBER";
	else if(a->qt==QT_PUBLISHER)
		return "PUBLISHER";
	else if(a->qt==QT_TOPIC)
		return "TOPIC";
	else if(a->qt==QT_READER)
		return "READER";
	else if(a->qt==QT_WRITER)
		return "WRITER";
	else return "DEFAULT";
}

void qos_history (struct qos *a, const char *arg)
{
	dds_qos_t *qp = get_qos_TRW(a, "history");
	int hist_depth, pos;
	if (qp == NULL)
		return;
	if (strcmp (arg, "all") == 0)
	{
		dds_qset_history(qp, DDS_HISTORY_KEEP_ALL, DDS_LENGTH_UNLIMITED);
		PRINTD("%s: qos: History keep all\n\n",enumValue(a));
	}
	else if (sscanf (arg, "%d%n", &hist_depth, &pos) == 1 && arg[pos] == 0 && hist_depth > 0)
	{
		dds_qset_history(qp, DDS_HISTORY_KEEP_LAST, hist_depth);
	}
	else
	{
		error ("history qos: %s: invalid\n", arg);
	}
}

void qos_destination_order (struct qos *a, const char *arg)
{
	dds_qos_t *qp = get_qos_TRW(a, "destination_order");
	if (qp == NULL)
		return;
	if (strcmp (arg, "r") == 0)
		dds_qset_destination_order(qp, DDS_DESTINATIONORDER_BY_RECEPTION_TIMESTAMP);
	else if (strcmp (arg, "s") == 0){
		dds_qset_destination_order(qp, DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP);
		PRINTD("%s: dds_qset_dest_order\n\n",enumValue(a));
	}
	else
		error ("destination order qos: %s: invalid\n", arg);
}

void qos_ownership (struct qos *a, const char *arg)
{
	dds_qos_t *qp = get_qos_TRW(a, "ownership");
	int strength, pos;
	if (qp == NULL)
		return;
	if (strcmp (arg, "s") == 0)
		dds_qset_ownership(qp, DDS_OWNERSHIP_SHARED);
	else if (strcmp (arg, "x") == 0) {
		dds_qset_ownership(qp, DDS_OWNERSHIP_EXCLUSIVE);
	}
	else if (sscanf (arg, "x:%d%n", &strength, &pos) == 1 && arg[pos] == 0)
	{
		dds_qos_t *qps = get_qos_W(a, "ownership_strength");
		dds_qset_ownership(qp, DDS_OWNERSHIP_EXCLUSIVE);
		if(qps) dds_qset_ownership_strength(qps, strength);
	}
	else
	{
		error ("ownership qos: %s invalid\n", arg);
	}
}

void qos_transport_priority (struct qos *a, const char *arg)
{
	dds_qos_t *qp = get_qos_W(a, "transport_priority");
	int pos;
	int value;
	if (qp == NULL)
		return;
	if (sscanf (arg, "%d%n", &value, &pos) != 1 || arg[pos] != 0)
		error ("transport_priority qos: %s invalid\n", arg);
	dds_qset_transport_priority(qp, value);
	PRINTD("%s: %d: dds_qset_transport_priority\n\n",enumValue(a),value);
}

static unsigned char gethexchar (const char **str)
{
  unsigned char v = 0;
  int empty = 1;
  while (**str)
  {
    switch (**str)
    {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        v = 16 * v + (unsigned char) **str - '0';
        (*str)++;
        break;
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        v = 16 * v + (unsigned char) **str - 'a' + 10;
        (*str)++;
        break;
      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        v = 16 * v + (unsigned char) **str - 'A' + 10;
        (*str)++;
        break;
      default:
        if (empty)
          error ("empty \\x escape");
        goto done;
    }
    empty = 0;
  }
 done:
  return v;
}

static unsigned char getoctchar (const char **str)
{
  unsigned char v = 0;
  int nseen = 0;
  while (**str && nseen < 3)
  {
    if (**str >= '0' && **str <= '7')
    {
      v = 8 * v + (unsigned char) **str - '0';
      (*str)++;
      nseen++;
    }
    else
    {
      if (nseen == 0)
        error ("empty \\ooo escape");
      break;
    }
  }
  return v;
}

static void *unescape (const char *str, size_t *len)
{
  /* results in a blob without explicit terminator, i.e., can't get
     any longer than strlen(str) */
  unsigned char *x = os_malloc (strlen (str)), *p = x;
  while (*str)
  {
    if (*str != '\\')
      *p++ = (unsigned char) *str++;
    else
    {
      str++;
      switch (*str)
      {
        case '\\': case ',': case '\'': case '"': case '?':
          *p++ = (unsigned char) *str;
          str++;
          break;
        case 'x':
          str++;
          *p++ = gethexchar (&str);
          break;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
          *p++ = getoctchar (&str);
          break;
        case 'a': *p++ = '\a'; str++; break;
        case 'b': *p++ = '\b'; str++; break;
        case 'f': *p++ = '\f'; str++; break;
        case 'n': *p++ = '\n'; str++; break;
        case 'r': *p++ = '\r'; str++; break;
        case 't': *p++ = '\t'; str++; break;
        case 'v': *p++ = '\v'; str++; break;
        case 'e': *p++ = 0x1b; str++; break;
        default:
          error ("invalid escape string: %s\n", str);
          break;
      }
    }
  }
  *len = (size_t) (p - x);
  return x;
}

void qos_user_data (struct qos *a, const char *arg)
{
	dds_qos_t *qp = get_qos_RW(a, "user_data");
	size_t len;
	if (qp == NULL)
		return;

	void *unesc = unescape (arg, &len);
	if(len==0) {
	  dds_qset_userdata(qp,NULL,0);
	} else {
	  dds_qset_userdata(qp, unesc, len);
	}

	os_free (unesc);
}

int double_to_dds_duration (dds_duration_t *dd, double d)
{
	if (d < 0)
	  		return -1;
	double nanosec = d * 1e9;  //Todo: Changed this double to dds duration. Previously it was using floor func.
	if(nanosec > INT64_MAX) {
		*dd = DDS_INFINITY;
	} else {
		*dd = (int64_t) nanosec;
	}
    return 0;
}

void set_infinite_dds_duration (dds_duration_t *dd)
{
	*dd = DDS_INFINITY;
}

void qos_reliability (struct qos *a, const char *arg)
{
	dds_qos_t *qp = get_qos_TRW(a, "reliability");
	const char *argp = arg;
	dds_duration_t max_block_t = DDS_MSECS(100);
	PRINTD("duration: %"PRId64 " \n\n", max_block_t);

	if (qp == NULL)
		return;

	switch (*argp++)
	{
		case 'n':
			dds_qset_reliability(qp, DDS_RELIABILITY_BEST_EFFORT, max_block_t);
			PRINTD("max_block_t: %"PRId64 " case: n \n\n", max_block_t);
			break;
		case 'y':
			if(*argp == ':') {
				double max_blocking_time;
				int pos;
				if (strcmp (argp, ":inf") == 0)
				{
				  set_infinite_dds_duration (&max_block_t);
				  argp += 4;
				}
				else if (sscanf (argp, ":%lf%n", &max_blocking_time, &pos) == 1 && argp[pos] == 0)
				{
				  if (max_blocking_time <= 0 || double_to_dds_duration (&max_block_t, max_blocking_time) < 0)
					error ("reliability qos: %s: max blocking time out of range\n", arg);
				  argp += pos;
				}
				else
				{
				  error ("reliability qos: %s: invalid max_blocking_time\n", arg);
				}
			}
			dds_qset_reliability(qp, DDS_RELIABILITY_RELIABLE, max_block_t);
			PRINTD("max_block_t: %"PRId64 " case: y \n\n", max_block_t);
			break;
		case 's':
			fprintf(stderr, "warning: %s entity ignoring inapplicable QoS \"synchronous reliability\"\n", enumValue(a));
//		  	qp->synchronous = 1;
			break;
		default:
		  error ("reliability qos: %s: invalid\n", arg);
	}
	if (*argp != 0)
	{
		error ("reliability qos: %s: invalid\n", arg);
	}
}

void qos_liveliness (struct qos *a, const char *arg)
{
	dds_duration_t dd = 0;
	dds_qos_t *qp = get_qos_TRW(a, "liveliness");
	double lease_duration;
	int pos;

	if (qp == NULL)
	return;

	if (strcmp (arg, "a") == 0)
	{
		PRINTD("Inside qos_liveliness_automatic\n\n");
		dds_qset_liveliness(qp, DDS_LIVELINESS_AUTOMATIC, DDS_INFINITY);
	}
	else if (sscanf (arg, "p:%lf%n", &lease_duration, &pos) == 1 && arg[pos] == 0)
	{
		if (lease_duration <= 0 || double_to_dds_duration (&dd, lease_duration) < 0)
		  error ("liveliness qos: %s: lease duration out of range\n", arg);
		dds_qset_liveliness(qp, DDS_LIVELINESS_MANUAL_BY_PARTICIPANT, lease_duration);
	}
	else if (sscanf (arg, "w:%lf%n", &lease_duration, &pos) == 1 && arg[pos] == 0)
	{
		if (lease_duration <= 0 || double_to_dds_duration (&dd, lease_duration) < 0)
		  error ("liveliness qos: %s: lease duration out of range\n", arg);
		dds_qset_liveliness(qp, DDS_LIVELINESS_MANUAL_BY_TOPIC, lease_duration);
	}
	else
	{
		error ("liveliness qos: %s: invalid\n", arg);
	}
}

static void qos_simple_duration (dds_duration_t *dd, const char *name, const char *arg)
{
  double duration;
  int pos;
  if (strcmp (arg, "inf") == 0)
    set_infinite_dds_duration (dd);
  else if (sscanf (arg, "%lf%n", &duration, &pos) == 1 && arg[pos] == 0)
  {
    if (double_to_dds_duration (dd, duration) < 0)
      error ("%s qos: %s: duration invalid\n", name, arg);
  }
  else
  {
    error ("%s qos: %s: invalid\n", name, arg);
  }
}

void qos_latency_budget (struct qos *a, const char *arg)
{
	dds_qos_t *qp = get_qos_TRW(a, "latency_budget");
	dds_duration_t duration = 0;
	if (qp == NULL)
		return;
	qos_simple_duration (&duration, "latency_budget", arg);
	PRINTD("Duration: %" PRId64 " \n\n",duration);
	dds_qset_latency_budget(qp, duration);
}

void qos_deadline (struct qos *a, const char *arg)
{
  dds_qos_t *qp = get_qos_TRW(a, "deadline");
  dds_duration_t deadline = 0;
  if (qp == NULL)
    return;
  qos_simple_duration (&deadline, "deadline", arg);
  PRINTD("Deadline: %"PRId64"\n\n",deadline);
  dds_qset_deadline(qp, deadline);
}

void qos_lifespan (struct qos *a, const char *arg)
{
	dds_qos_t *qp = get_qos_TW(a, "lifespan");
	dds_duration_t duration = 0;
	if (qp == NULL)
		return;
	qos_simple_duration (&duration, "lifespan", arg);
	PRINTD("Duration: %" PRId64 " \n\n",duration);
	dds_qset_lifespan(qp, duration);
}

static int one_resource_limit (int32_t *val, const char **arg)
{
  int pos;
  if (strncmp (*arg, "inf", 3) == 0)
  {
    *val = DDS_LENGTH_UNLIMITED;
    (*arg) += 3;
    return 1;
  }
  else if (sscanf (*arg, "%d%n", val, &pos) == 1)
  {
    (*arg) += pos;
    return 1;
  }
  else
  {
    return 0;
  }
}

void qos_resource_limits (struct qos *a, const char *arg)
{
	dds_qos_t *qp = get_qos_TRW(a, "resource_limits");
	const char *argp = arg;
	int32_t max_samples = 0;
	int32_t max_instances = 0;
	int32_t max_samples_per_instance = 0;
	if (qp == NULL)
		return;

	if (!one_resource_limit (&max_samples, &argp))
		goto err;
	if (*argp++ != '/')
		goto err;
	if (!one_resource_limit (&max_instances, &argp))
		goto err;
	if (*argp++ != '/')
		goto err;
	if (!one_resource_limit (&max_samples_per_instance, &argp))
		goto err;

	dds_qset_resource_limits(qp, max_samples, max_instances, max_samples_per_instance);
	PRINTD("%s: max_samples: %"PRId32" max_ins: %"PRId32" max_s_ps:%"PRId32" \n\n",enumValue(a),max_samples,max_instances,max_samples_per_instance);

	if (*argp != 0)
		goto err;
		return;

	err:
	error ("resource limits qos: %s: invalid\n", arg);
}

void qos_durability_service (struct qos *a, const char *arg)
{
	dds_qos_t *qp = get_qos_T(a, "durability_service");
	const char *argp = arg;
	double service_cleanup_delay_t;
	int pos, hist_depth;
	dds_duration_t service_cleanup_delay = 0;
	dds_history_kind_t history_kind = DDS_HISTORY_KEEP_LAST;
	int32_t history_depth = 1;
	int32_t max_samples = DDS_LENGTH_UNLIMITED;
	int32_t max_instances = DDS_LENGTH_UNLIMITED;
	int32_t max_samples_per_instance = DDS_LENGTH_UNLIMITED;

	if (qp == NULL)
		return;

	argp = arg;
	if (strncmp (argp, "inf", 3) == 0) {
		set_infinite_dds_duration (&service_cleanup_delay);
		pos = 3;
	} else if (sscanf (argp, "%lf%n", &service_cleanup_delay_t, &pos) == 1) {
		if (service_cleanup_delay_t < 0 || double_to_dds_duration (&service_cleanup_delay, service_cleanup_delay_t) < 0)
			error ("durability service qos: %s: service cleanup delay out of range\n", arg);
	} else {
		goto err;
	}

	if (argp[pos] == 0) {
		dds_qset_durability_service(qp, service_cleanup_delay, history_kind, history_depth, max_samples, max_instances, max_samples_per_instance);
		return;
	} else if (argp[pos] != '/') goto err;
	argp += pos + 1;

	if (strncmp (argp, "all", 3) == 0) {
		history_kind = DDS_HISTORY_KEEP_ALL;
		pos = 3;
	} else if (sscanf (argp, "%d%n", &hist_depth, &pos) == 1 && hist_depth > 0) {
		history_depth = hist_depth;
	} else {
		goto err;
	}

	if (argp[pos] == 0) {
		dds_qset_durability_service(qp, service_cleanup_delay, history_kind, history_depth, max_samples, max_instances, max_samples_per_instance);
		return;
	} else if (argp[pos] != '/') goto err;
	argp += pos + 1;

	if (!one_resource_limit (&max_samples, &argp))
		goto err;
	if (*argp++ != '/')
		goto err;
	if (!one_resource_limit (&max_instances, &argp))
		goto err;
	if (*argp++ != '/')
		goto err;
	if (!one_resource_limit (&max_samples_per_instance, &argp))
		goto err;

	dds_qset_durability_service(qp, service_cleanup_delay, history_kind, history_depth, max_samples, max_instances, max_samples_per_instance);
	PRINTD("%s: cleanUpdelay: %" PRId64" depth: %" PRId32" max_samples: %"PRId32" max_ins: %"PRId32" max_s_ps: %"PRId32" \n\n",enumValue(a),service_cleanup_delay,history_depth,max_samples,max_instances,max_samples_per_instance);

	if (*argp != 0)
		goto err;
	return;

	err:
	error ("resource limits qos: %s: invalid\n", arg);
}

void qos_presentation (struct qos *a, const char *arg)
{
	dds_qos_t *qp = get_qos_PS(a, "presentation");
	if (qp == NULL)
		return;
	if (strcmp (arg, "i") == 0) {
		dds_qset_presentation(qp, DDS_PRESENTATION_INSTANCE, 0, 0);
	} else if (strcmp (arg, "t") == 0) {
		dds_qset_presentation(qp, DDS_PRESENTATION_TOPIC, 1, 0);
	} else if (strcmp (arg, "g") == 0) {
		dds_qset_presentation(qp, DDS_PRESENTATION_GROUP, 1, 0);
		PRINTD("%s : dds_qset_presentation_group\n\n",enumValue(a));
	} else {
		error ("presentation qos: %s: invalid\n", arg);
	}
}

void qos_autodispose_unregistered_instances (struct qos *a, const char *arg)
{
	dds_qos_t *qp = get_qos_W(a, "writer_data_lifecycle");
	if (qp == NULL)
		return;
	if (strcmp (arg, "n") == 0)
	  dds_qset_writer_data_lifecycle(qp, false);
	else if (strcmp (arg, "y") == 0)
	  dds_qset_writer_data_lifecycle(qp, true);
	else
		error ("autodispose_unregistered_instances qos: %s: invalid\n", arg);
	PRINTD("%s: %s : qos_autodispose_unregistered_instances\n\n",enumValue(a),arg);
}

//static unsigned split_string (char ***xs, const char *in, const char *sep)
//{
//  char *incopy = os_strdup (in), *cursor = incopy, *tok;
//  unsigned n = 0;
//  *xs = NULL;
//  while ((tok = os_strsep (&cursor, sep)) != NULL)
//  {
//    *xs = realloc (*xs, (n+1) * sizeof (**xs));
//    (*xs)[n++] = tok;
//  }
//  return n;
//}

void qos_subscription_keys (struct qos *a, const char *arg)
{
	fprintf(stderr, "warning: %s entity ignoring inapplicable QoS \"Subscription Key\"\n", enumValue(a));
	return;
}

const char *qos_arg_usagestr = "\
QOS (not all are universally applicable):\n\
  A={a|p:S|w:S}   liveliness (automatic, participant or writer, S in seconds)\n\
  d={v|tl|t|p}    durability (default: v)\n\
  D=P             deadline P in seconds (default: inf)\n\
  k={all|N}       KEEP_ALL or KEEP_LAST N\n\
  l=D             latency budget in seconds (default: 0)\n\
  L=D             lifespan in seconds (default: inf)\n\
  o=[r|s]         order by reception or source timestamp (default: s)\n\
  O=[s|x[:S]]     ownership: shared or exclusive, strength S (default: s)\n\
  p=PRIO          transport priority (default: 0)\n\
  P={i|t|g}       instance, or {topic|group} + coherent updates\n\
  r={y[:T]|s[:T]|n}  reliability, T is max blocking time in seconds,\n\
                  s is reliable+synchronous (default: y:1)\n\
  R=S/I/SpI       resource limits (samples, insts, S/I; default: inf/inf/inf)\n\
  S=C[/H[/S/I/SpI]] durability_service (cleanup delay, history, resource limits)\n\
  u={y|n}         autodispose unregistered instances (default: n)\n\
  U=TEXT          set user_data to TEXT\n\
  V=K0:K1:K2      set subscription keys\n\
";

void set_qosprovider (const char *arg)
{
//  int result = DDS_RETCODE_OK;
  const char *p = strchr (arg, ',');
  const char *xs = strstr (arg, "://");
  char *profile = NULL;
  const char *uri;
  if (p == NULL || xs == NULL || p >= xs)
    uri = arg;
  else {
    uri = p+1;
    profile = os_strdup(arg);
    profile[p-arg] = 0;
  }
  PRINTD("set_qosprovider:: uri: %s, profile: %s\n",uri,profile);

//  if((result = dds_qosprovider_create(&qosprov, uri, profile)) != DDS_RETCODE_OK)	//Todo: There is no qosprovider_create in ddsi
//	  error("dds_qosprovider_create(%s,%s) failed\n", uri, profile ? profile : "(null)");
  dds_string_free((char *) arg);
  dds_string_free((char *) p);
  dds_string_free((char *) xs);
  dds_string_free((char *) uri);
  os_free(profile);
}

void setqos_from_args (struct qos *q, int n, const char *args[])
{
  int i;
  for (i = 0; i < n; i++)
  {
    char *args_copy = os_strdup (args[i]), *cursor = args_copy;
    const char *arg;
    while ((arg = os_strsep (&cursor, ",")) != NULL)
    {
      if (arg[0] && arg[1] == '=') {
        const char *a = arg + 2;
        switch (arg[0]) {
          case 'A': qos_liveliness (q, a); break;
          case 'd': qos_durability (q, a); break;
          case 'D': qos_deadline (q, a); break;
          case 'k': qos_history (q, a); break;
          case 'l': qos_latency_budget (q, a); break;
          case 'L': qos_lifespan (q, a); break;
          case 'o': qos_destination_order (q, a); break;
          case 'O': qos_ownership (q, a); break;
          case 'p': qos_transport_priority (q, a); break;
          case 'P': qos_presentation (q, a); break;
          case 'r': qos_reliability (q, a); break;
          case 'R': qos_resource_limits (q, a); break;
          case 'S': qos_durability_service (q, a); break;
          case 'u': qos_autodispose_unregistered_instances (q, a); break;
          case 'U': qos_user_data (q, a); break;
          case 'V': qos_subscription_keys (q, a); break;
          default:
            fprintf (stderr, "%s: unknown QoS\n", arg);
            exit (1);
        }
      } else if (!qosprov) {
        fprintf (stderr, "QoS specification %s requires a QoS provider but none set\n", arg);
        exit (1);
      } else {
    	  printf("Qos provider not supported\n"); //Todo: Commentted qos provider. Could not find in dds.h. Fix required.
//        int result;
//        if (*arg == 0)
//          arg = NULL;
//        switch (q->qt) {
//          case QT_TOPIC:
//            if ((result = dds_qosprovider_get_topic_qos(qosprov, q->u.topic.q, arg)) != DDS_RETCODE_OK)
//              error ("dds_qosprovider_get_topic_qos(%s): error %d (%s)\n", arg, (int) result, dds_strerror(result));
//            break;
//          case QT_PUBLISHER:
//            if ((result = dds_qosprovider_get_publisher_qos(qosprov, q->u.pub.q, arg)) != DDS_RETCODE_OK)
//              error ("dds_qosprovider_get_publisher_qos(%s): error %d (%s)\n", arg, (int) result, dds_strerror(result));
//            break;
//          case QT_SUBSCRIBER:
//            if ((result = dds_qosprovider_get_subscriber_qos(qosprov, q->u.sub.q, arg)) != DDS_RETCODE_OK)
//              error ("dds_qosprovider_get_subscriber_qos(%s): error %d (%s)\n", arg, (int) result, dds_strerror(result));
//            break;
//          case QT_WRITER:
//            if ((result = dds_qosprovider_get_writer_qos(qosprov, q->u.wr.q, arg)) != DDS_RETCODE_OK)
//              error ("dds_qosprovider_get_writer_qos(%s): error %d (%s)\n", arg, (int) result, dds_strerror(result));
//            break;
//          case QT_READER:
//            if ((result = dds_qosprovider_get_reader_qos(qosprov, q->u.rd.q, arg)) != DDS_RETCODE_OK)
//              error ("dds_qosprovider_get_reader_qos(%s): error %d (%s)\n", arg, (int) result, dds_strerror(result));
//            break;
//        }
      }
    }
    os_free((char *)arg);
    os_free (args_copy);
  }
}

void errorMsg(int value, char* msg) {
	if(value != 0 ) {
		printf("Function %s failed with value %d\n", msg, dds_err_nr(value)); //Todo: Changed dds_err_no(value)
		exit(value);
	}
}


