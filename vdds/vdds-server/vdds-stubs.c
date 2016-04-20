#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <execinfo.h>
#include <inttypes.h>

#include "os.h"
#include "dds.h"
#include "dds_public_log.h"
#include "q_config.h"
#include "q_globals.h"
#include "q_xmsg.h"
#include "q_log.h"
#include "q_unused.h"
#include "ddsi_ser.h"
#include "dds_stream.h"
#include "q_osplser.h"
#include "server.h"

struct config config;
struct q_globals gv;

static FILE *fp;
static struct sertopic *sertopic;

int nn_vlog(UNUSED_ARG(logcat_t cat), UNUSED_ARG(const char *fmt), UNUSED_ARG(va_list ap)) { return 0; }
int nn_log(UNUSED_ARG(logcat_t cat), UNUSED_ARG(const char *fmt), ...) { return 0; }
int nn_trace(UNUSED_ARG(const char *fmt), ...) { return 0; }
void *nn_xmsg_addpar(UNUSED_ARG(struct nn_xmsg *m), UNUSED_ARG(unsigned pid), UNUSED_ARG(size_t len)) { return 0; }
void nn_xmsg_addpar_string(UNUSED_ARG(struct nn_xmsg *m), UNUSED_ARG(unsigned pid), UNUSED_ARG(const char *str)) { }
void nn_xmsg_addpar_octetseq(UNUSED_ARG(struct nn_xmsg *m), UNUSED_ARG(unsigned pid), UNUSED_ARG(const nn_octetseq_t *oseq)) { }
void nn_xmsg_addpar_stringseq(UNUSED_ARG(struct nn_xmsg *m), UNUSED_ARG(unsigned pid), UNUSED_ARG(const nn_stringseq_t *sseq)) { }
void nn_xmsg_addpar_guid(UNUSED_ARG(struct nn_xmsg *m), UNUSED_ARG(unsigned pid), UNUSED_ARG(const nn_guid_t *guid)) { }
void nn_xmsg_addpar_BE4u(UNUSED_ARG(struct nn_xmsg *m), UNUSED_ARG(unsigned pid), UNUSED_ARG(unsigned x)) { }
void nn_xmsg_addpar_reliability(UNUSED_ARG(struct nn_xmsg *m), UNUSED_ARG(unsigned pid), UNUSED_ARG(const struct nn_reliability_qospolicy *rq)) { }
void nn_xmsg_addpar_share(UNUSED_ARG(struct nn_xmsg *m), UNUSED_ARG(unsigned pid), UNUSED_ARG(const struct nn_share_qospolicy *rq)) { }
void nn_xmsg_addpar_subscription_keys(UNUSED_ARG(struct nn_xmsg *m), UNUSED_ARG(unsigned pid), UNUSED_ARG(const struct nn_subscription_keys_qospolicy *rq)) { }
void nn_xmsg_addpar_parvinfo(UNUSED_ARG(struct nn_xmsg *m), UNUSED_ARG(unsigned pid), UNUSED_ARG(const struct nn_prismtech_participant_version_info *pvi)) { }
void nn_xmsg_addpar_eotinfo(UNUSED_ARG(struct nn_xmsg *m), UNUSED_ARG(unsigned pid), UNUSED_ARG(const struct nn_prismtech_eotinfo *txnid)) { }

void dds_log_info(const char *fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    vfprintf (stderr, fmt, args);
    va_end (args);
}

int fwr_blob(FILE *fp, const void *xs, size_t n)
{
    if (fwrite(&n, sizeof(n), 1, fp) != 1) {
        return -1;
    }
    return fwrite(xs, n, 1, fp) == 1 ? 1 : -1;
}

int fwr_string(FILE *fp, const char *str)
{
    size_t n = strlen(str) + 1;
    return fwr_blob(fp, str, n);
}

int simple_request(FILE *fp, const struct reqhdr *req)
{
    //printf("simple_request %d\n", (int) req->code);
    if (fwrite(req, sizeof(*req), 1, fp) != 1 || fflush(fp) != 0) {
        return -1;
    } else {
        return 1;
    }
}

int simple_reply(FILE *fp, struct rephdr *rep)
{
    return fread(rep, sizeof(*rep), 1, fp) == 1 ? 1 : -1;
}

int simple(FILE *fp, const struct reqhdr *req, struct rephdr *rep)
{
    int rc;
    if ((rc = simple_request(fp, req)) < 0) {
        return rc;
    }
    return simple_reply(fp, rep);
}

int dds_init(int argc, char ** argv)
{
    struct sockaddr_un address;
    struct reqhdr hello;
    int fd;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, VDDS_SOCKET_NAME);
    if ((fd = socket(PF_LOCAL, SOCK_STREAM, 0)) == -1)
    {
        perror ("socket");
        return -DDS_RETCODE_ERROR;
    }
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    if (connect(fd, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        if (errno != ENOENT && errno != ECONNREFUSED) {
            perror("connect");
        }
        goto err;
    }
    gv.serpool = ddsi_serstatepool_new();
    hello.code = VDDSREQ_HELLO;
    hello.u.hello.pid = getpid();
    fp = fdopen(fd, "a+b");
    if (simple_request(fp, &hello) < 0) {
        goto err;
    }
    return DDS_RETCODE_OK;
err:
    close(fd);
    fd = -1;
    return -DDS_RETCODE_ERROR;

}

void dds_fini(void)
{
    struct reqhdr goodbye;
    goodbye.code = VDDSREQ_GOODBYE;
    simple_request(fp, &goodbye);
    ddsi_serstatepool_free(gv.serpool);
    fclose(fp);
    fp = NULL;
}

int dds_participant_create(dds_entity_t *pp, const dds_domainid_t domain, const dds_qos_t *qos, const dds_participantlistener_t *listener)
{
    struct reqhdr req;
    struct rephdr rep;
    assert(qos == NULL);
    assert(listener == NULL);
    req.code = VDDSREQ_PARTICIPANT_CREATE;
    req.u.participant_create.domain = domain;
    if (simple(fp, &req, &rep) < 0) {
        return -DDS_RETCODE_ERROR;
    }
    *pp = rep.u.entity.e;
    return rep.status;
}

void dds_entity_delete(dds_entity_t e)
{
    struct reqhdr req;
    struct rephdr rep;
    req.code = VDDSREQ_ENTITY_DELETE;
    req.u.entity_delete.entity = e;
    (void)simple(fp, &req, &rep);
}

struct sertopic *make_sertopic(const char *name, const char *typename, const dds_topic_descriptor_t *desc)
{
    static uint32_t next_topicid;
    struct sertopic *st;
    char *key;
    key = dds_alloc(strlen(name) + 1 + strlen(typename) + 1);
    sprintf(key, "%s/%s", name, typename);
    st = dds_alloc(sizeof (*st));
    st->type = (void*) desc;
    os_atomic_st32 (&st->refcount, 1);
    st->status_cb = 0;
    st->status_cb_entity = 0;
    st->name_typename = key;
    st->name = dds_alloc(strlen (name) + 1);
    strcpy (st->name, name);
    st->typename = dds_alloc(strlen (typename) + 1);
    strcpy (st->typename, typename);
    st->nkeys = desc->m_nkeys;
    st->keys = desc->m_keys;
    st->id = next_topicid++;
    st->hash = (st->id * UINT64_C (12844332200329132887)) >> 32;
    /* Check if topic cannot be optimised (memcpy marshal) */
    if ((desc->m_flagset & DDS_TOPIC_NO_OPTIMIZE) == 0)
    {
        st->opt_size = dds_stream_check_optimize(desc);
    }
    return st;
}

int dds_topic_create(dds_entity_t pp, dds_entity_t *topic, const dds_topic_descriptor_t *descriptor, const char *name, const dds_qos_t *qos, const dds_topiclistener_t *listener)
{
    struct reqhdr req;
    struct rephdr rep;
    assert(qos == NULL);
    assert(listener == NULL);
    assert(sertopic == NULL);
    assert(descriptor->m_nkeys == 0);
    sertopic = make_sertopic(name, descriptor->m_typename, descriptor);
    req.code = VDDSREQ_TOPIC_CREATE;
    req.u.topic_create.pp = pp;
    if (fwrite(&req, sizeof(req), 1, fp) != 1 ||
        fwr_blob(fp, descriptor, sizeof(*descriptor)) < 0 ||
        fwr_string(fp, descriptor->m_typename) < 0 ||
        fwr_blob(fp, descriptor->m_ops, descriptor->m_nops * sizeof(*descriptor->m_ops)) < 0 ||
        fwr_string(fp, descriptor->m_meta) < 0 ||
        fwr_string(fp, name) < 0 ||
        fflush(fp) != 0) {
        return -DDS_RETCODE_ERROR;
    }
    if (simple_reply(fp, &rep) < 0) {
        return -DDS_RETCODE_ERROR;
    }
    *topic = rep.u.entity.e;
    return rep.status;
}

void dds_write_set_batch(bool enable)
{
    struct reqhdr req;
    struct rephdr rep;
    req.code = VDDSREQ_WRITE_SET_BATCH;
    req.u.write_set_batch.enable = enable;
    (void)simple(fp, &req, &rep);
}

int dds_write_impl(dds_entity_t wr, const void *data, bool sync)
{
    struct reqhdr req;
    struct rephdr rep;
    size_t sz;
    serdata_t serdata;
    serdata = serialize(gv.serpool, sertopic, data);
    sz = ddsi_serdata_size(serdata) - sizeof (struct CDRHeader);
    req.code = sync ? VDDSREQ_WRITE : VDDSREQ_WRITE_ASYNC;
    req.u.write.wr = wr;
    if (fwrite(&req, sizeof(req), 1, fp) != 1 || fwr_blob(fp, serdata->data, sz) < 0 || fflush(fp) != 0) {
        ddsi_serdata_unref(serdata);
        return -DDS_RETCODE_ERROR;
    }
    ddsi_serdata_unref(serdata);
    rep.status = DDS_RETCODE_OK;
    if (sync && simple_reply(fp, &rep) < 0) {
        return -DDS_RETCODE_ERROR;
    }
    return rep.status;
}

int dds_write(dds_entity_t wr, const void *data)
{
    return dds_write_impl(wr, data, true);
}

int dds_write_async(dds_entity_t wr, const void *data)
{
    return dds_write_impl(wr, data, false);
}

void dds_write_flush(dds_entity_t wr)
{
    struct reqhdr req;
    struct rephdr rep;
    req.code = VDDSREQ_WRITE_FLUSH;
    req.u.write_flush.wr = wr;
    (void)simple(fp, &req, &rep);
}

int dds_publisher_create(dds_entity_t pp, dds_entity_t *publisher, const dds_qos_t *qos, const dds_publisherlistener_t *listener)
{
    struct reqhdr req;
    struct rephdr rep;
    assert(listener == NULL);
    req.code = VDDSREQ_PUBLISHER_CREATE;
    req.u.publisher_create.pp = pp;
    if (fwrite(&req, sizeof(req), 1, fp) != 1 || fwr_string(fp, qos->partition.strs[0]) < 0 || fflush(fp) != 0) {
        return -DDS_RETCODE_ERROR;
    }
    if (simple_reply(fp, &rep) < 0) {
        return -DDS_RETCODE_ERROR;
    }
    *publisher = rep.u.entity.e;
    return rep.status;
}

int dds_subscriber_create(dds_entity_t pp, dds_entity_t *subscriber, const dds_qos_t *qos, const dds_subscriberlistener_t *listener)
{
    struct reqhdr req;
    struct rephdr rep;
    assert(listener == NULL);
    req.code = VDDSREQ_SUBSCRIBER_CREATE;
    req.u.subscriber_create.pp = pp;
    if (fwrite(&req, sizeof(req), 1, fp) != 1 || fwr_string(fp, qos->partition.strs[0]) < 0 || fflush(fp) != 0) {
        return -DDS_RETCODE_ERROR;
    }
    if (simple_reply(fp, &rep) < 0) {
        return -DDS_RETCODE_ERROR;
    }
    *subscriber = rep.u.entity.e;
    return rep.status;
}

int dds_instance_dispose(dds_entity_t wr, const void *data)
{
    return -DDS_RETCODE_ERROR;
}

int dds_writer_create(dds_entity_t pp_or_pub, dds_entity_t *writer, dds_entity_t topic, const dds_qos_t *qos, const dds_writerlistener_t *listener)
{
    struct reqhdr req;
    struct rephdr rep;
    assert(listener == NULL);
    req.code = VDDSREQ_WRITER_CREATE;
    req.u.writer_create.pp_or_pub = pp_or_pub;
    req.u.writer_create.topic = topic;
    if (fwrite(&req, sizeof(req), 1, fp) != 1 || fwr_blob(fp, qos, sizeof(*qos)) < 0 || fflush(fp) != 0) {
        return -DDS_RETCODE_ERROR;
    }
    if (simple_reply(fp, &rep) < 0) {
        return -DDS_RETCODE_ERROR;
    }
    *writer = rep.u.entity.e;
    return rep.status;
}

int dds_reader_create(dds_entity_t pp_or_sub, dds_entity_t *reader, dds_entity_t topic, const dds_qos_t *qos, const dds_readerlistener_t *listener)
{
    struct reqhdr req;
    struct rephdr rep;
    assert(listener == NULL);
    req.code = VDDSREQ_READER_CREATE;
    req.u.reader_create.pp_or_sub = pp_or_sub;
    req.u.reader_create.topic = topic;
    if (fwrite(&req, sizeof(req), 1, fp) != 1 || fwr_blob(fp, qos, sizeof(*qos)) < 0 || fflush(fp) != 0) {
        return -DDS_RETCODE_ERROR;
    }
    if (simple_reply(fp, &rep) < 0) {
        return -DDS_RETCODE_ERROR;
    }
    *reader = rep.u.entity.e;
    return rep.status;
}

int frd_blob(FILE *fp, size_t *sz, void **blob)
{
    if (fread(sz, sizeof(*sz), 1, fp) != 1) {
        return -1;
    }
    *blob = os_malloc(*sz);
    if (fread(*blob, *sz, 1, fp) != 1) {
        os_free(*blob);
        return -1;
    }
    return 0;
}

int dds_take(dds_entity_t rd, void ** buf, uint32_t maxs, dds_sample_info_t * si, uint32_t mask)
{
    struct reqhdr req;
    struct rephdr rep;
    int i;
    assert(buf);
    assert(maxs > 0);
    assert(si);
    req.code = VDDSREQ_TAKE;
    req.u.take.rd = rd;
    req.u.take.maxs = maxs;
    req.u.take.mask = mask;
    if (simple(fp, &req, &rep) < 0) {
        return -DDS_RETCODE_ERROR;
    }
    assert((uint32_t)rep.status <= maxs);
    if (rep.status > 0) {
        char *blob = NULL;
        size_t blobsz = 0;
        size_t sz;
        if (fread(si, rep.status * sizeof(*si), 1, fp) != 1) {
            return -DDS_RETCODE_ERROR;
        }
        for (i = 0; i < rep.status; i++) {
            dds_stream_t istr;
            if (fread(&sz, sizeof(sz), 1, fp) != 1) {
                return -DDS_RETCODE_ERROR;
            }
            if (sz > blobsz) {
                blobsz = (sz + 4095) & ~(size_t)4095;
                blob = os_realloc(blob, blobsz);
            }
            if (fread(blob, sz, 1, fp) != 1) {
                return -DDS_RETCODE_ERROR;
            }
            istr.m_failed = false;
            istr.m_buffer.p8 = (uint8_t*)blob + sizeof(struct CDRHeader);
            istr.m_size = sz - sizeof(struct CDRHeader);
            istr.m_index = 0;
            istr.m_endian = (((struct CDRHeader *)blob)->identifier == CDR_LE);
            dds_stream_read_sample (&istr, buf[i], sertopic);
        }
        os_free(blob);
    }
    return rep.status;
}

dds_waitset_t dds_waitset_create (void)
{
    struct reqhdr req;
    struct rephdr rep;
    req.code = VDDSREQ_WAITSET_CREATE;
    if (simple(fp, &req, &rep) < 0) {
        return NULL;
    }
    return rep.u.waitset.ws;
}

int dds_waitset_delete (dds_waitset_t ws)
{
    struct reqhdr req;
    struct rephdr rep;
    req.code = VDDSREQ_WAITSET_DELETE;
    req.u.waitset_delete.ws = ws;
    if (simple(fp, &req, &rep) < 0) {
        return -DDS_RETCODE_ERROR;
    }
    return rep.status;
}

int dds_waitset_attach (dds_waitset_t ws, dds_condition_t cond, dds_attach_t x)
{
    struct reqhdr req;
    struct rephdr rep;
    req.code = VDDSREQ_WAITSET_ATTACH;
    req.u.waitset_attach.ws = ws;
    req.u.waitset_attach.cond = cond;
    req.u.waitset_attach.x = x;
    if (simple(fp, &req, &rep) < 0) {
        return -DDS_RETCODE_ERROR;
    }
    return rep.status;
}

int dds_waitset_wait (dds_waitset_t ws, dds_attach_t *xs, size_t nxs, dds_time_t reltimeout)
{
    struct reqhdr req;
    struct rephdr rep;
    req.code = VDDSREQ_WAIT;
    req.u.wait.ws = ws;
    req.u.wait.nxs = nxs;
    req.u.wait.reltimeout = reltimeout;
    if (simple(fp, &req, &rep) < 0) {
        return -DDS_RETCODE_ERROR;
    }
    assert(rep.status <= (int)nxs);
    if (rep.status > 0) {
        if (fread(xs, rep.status * sizeof(xs[0]), 1, fp) != 1) {
            return -DDS_RETCODE_ERROR;
        }
    }
    return rep.status;
}

dds_condition_t dds_readcondition_create (dds_entity_t e, uint32_t mask)
{
    struct reqhdr req;
    struct rephdr rep;
    req.code = VDDSREQ_READCONDITION_CREATE;
    req.u.readcondition_create.e = e;
    req.u.readcondition_create.mask = mask;
    if (simple(fp, &req, &rep) < 0) {
        return NULL;
    }
    return rep.u.condition.cond;
}


