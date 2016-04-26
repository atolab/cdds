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

#include <event2/event.h>
#include <event2/thread.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "os/os.h"
#include "dds.h"
#include "server.h"
#include "kernel/dds_init.h"
#include "kernel/dds_rhc.h"
#include "ddsi/q_thread.h"
#include "ddsi/ddsi_ser.h"

#define LOCKFILE_NAME "/tmp/vdds-server-lock"

struct client {
    struct bufferevent *bev;
    int (*dispatch) (struct client *cl, const struct reqhdr *req);
    pid_t pid;
    dds_entity_t ppant;
};

struct admin {
    char dummy;
};

int grab_lock_file(void)
{
    pid_t pid = getpid();
    uid_t uid = getuid();
    char src[32], xsrc[32];
    ssize_t xsrclen;
    int p;
    long xpid, xuid;
    struct stat statbuf;
    assert(sizeof(long) >= sizeof(uid_t));
    assert(sizeof(long) >= sizeof(pid_t));
    sprintf(src, "%ld:%ld", (long) uid, (long) pid);
retry:
    if (symlink(src, LOCKFILE_NAME) == 0)
        return 0;
    if (errno != EEXIST) {
        perror("symlink");
        return -1;
    }
    if (lstat(LOCKFILE_NAME, &statbuf) == -1) {
        if (errno == EEXIST)
            goto retry;
        perror("lstat");
        return -1;
    }
    if ((xsrclen = readlink(LOCKFILE_NAME, xsrc, sizeof(xsrc) - 1)) == -1) {
        if (errno == ENOENT) {
            goto retry;
        }
        perror("readlink");
        return -1;
    }
    xsrc[xsrclen] = 0;
    if (sscanf(xsrc, "%ld:%ld%n", &xuid, &xpid, &p) != 2 || xsrc[p] != 0 || xpid <= 0 || xuid != (long)statbuf.st_uid) {
        fprintf(stderr, "%s: has unexpected contents (%s)\n", LOCKFILE_NAME, xsrc);
        return -1;
    }
    if (xuid != (long)uid) {
        fprintf(stderr, "%s: different user\n", LOCKFILE_NAME);
        return -1;
    }
    if (kill((pid_t)xpid, 0) == -1 && errno == ESRCH) {
        fprintf(stderr, "removing %s, %s and retrying\n", VDDS_SOCKET_NAME, LOCKFILE_NAME);
        unlink(VDDS_SOCKET_NAME);
        unlink(LOCKFILE_NAME);
        goto retry;
    }
    fprintf(stderr, "lkst appears to be running already\n");
    return -1;
}

int rm_lock_file(void)
{
    unlink(LOCKFILE_NAME);
    return 0;
}

int reply(struct client *cl, const struct rephdr *rep)
{
    if (bufferevent_write(cl->bev, rep, sizeof(*rep)) == -1) {
        abort();
    }
    bufferevent_flush(cl->bev, EV_WRITE, BEV_FLUSH);
    return 1;
}

static int protocol_error(int rc)
{
    return rc;
}

int dispatch_unbound(struct client *cl, const struct reqhdr *req)
{
    if (req->code == VDDSREQ_HELLO) {
        /* should use credential-passing instead, I know ... */
        if (req->u.hello.pid <= 0) {
            return -1;
        }
        cl->pid = req->u.hello.pid;
        printf("sock%d: pid %d\n", bufferevent_getfd(cl->bev), (int)cl->pid);
        return 0;
    } else {
        return -1;
    }
}

int rd_blob(struct client *cl, size_t *sz, void **blob)
{
    if (bufferevent_read(cl->bev, sz, sizeof(*sz)) != sizeof(*sz)) {
        abort();
    }
    *blob = os_malloc(*sz);
    if (bufferevent_read(cl->bev, *blob, *sz) != *sz) {
        os_free(*blob);
        abort();
    }
    return 0;
}

int rd_string(struct client *cl, char **str)
{
    size_t sz;
    void *blob;
    if (rd_blob(cl, &sz, &blob) < 0) {
        return -1;
    }
    *str = blob;
    return 0;
}

int db_entity_delete(struct client *cl, const struct reqhdr *req)
{
    struct rephdr rep;
    dds_entity_delete(req->u.entity_delete.entity);
    rep.code = VDDSREP_NOTHING;
    rep.status = DDS_RETCODE_OK;
    return reply(cl, &rep);
}

int db_participant_create(struct client *cl, const struct reqhdr *req)
{
    struct rephdr rep;
    rep.code = VDDSREP_ENTITY;
    rep.status = dds_participant_create(&rep.u.entity.e, req->u.participant_create.domain, NULL, NULL);
    return reply(cl, &rep);
}

int db_topic_create(struct client *cl, const struct reqhdr *req)
{
    struct rephdr rep;
    dds_topic_descriptor_t *desc;
    size_t sz;
    char *name;
    void *blob;
    int rc;
    if ((rc = rd_blob(cl, &sz, &blob)) < 0) {
        return protocol_error(rc);
    }
    desc = blob;
    if ((rc = rd_string(cl, (char **)&desc->m_typename)) < 0) {
        return protocol_error(rc);
    }
    if ((rc = rd_blob(cl, &sz, &blob)) < 0) {
        return protocol_error(rc);
    }
    desc->m_ops = blob;
    if ((rc = rd_string(cl, (char **)&desc->m_meta)) < 0) {
        return protocol_error(rc);
    }
    if ((rc = rd_string(cl, &name)) < 0) {
        return protocol_error(rc);
    }
    rep.code = VDDSREP_ENTITY;
    if ((rep.status = dds_topic_create(req->u.topic_create.pp, &rep.u.entity.e, desc, name, NULL, NULL)) != DDS_RETCODE_OK) {
        if ((rep.u.entity.e = dds_topic_find(req->u.topic_create.pp, name)) != DDS_HANDLE_NIL) {
            rep.status = DDS_RETCODE_OK;
        }
    }
    os_free(name);
    return reply(cl, &rep);
}

int db_write_set_batch(struct client *cl, const struct reqhdr *req)
{
    struct rephdr rep;
    dds_write_set_batch(req->u.write_set_batch.enable);
    rep.code = VDDSREP_NOTHING;
    rep.status = DDS_RETCODE_OK;
    return reply(cl, &rep);
}

int db_write_flush(struct client *cl, const struct reqhdr *req)
{
    struct rephdr rep;
    dds_write_flush(req->u.write_flush.wr);
    rep.code = VDDSREP_NOTHING;
    rep.status = DDS_RETCODE_OK;
    return reply(cl, &rep);
}

int db_publisher_subscriber_create(struct client *cl, const struct reqhdr *req)
{
    struct rephdr rep;
    char *partname;
    const char *parts[1];
    int rc;
    dds_qos_t *qos;
    if ((rc = rd_string(cl, &partname)) < 0) {
        return protocol_error(rc);
    }
    qos = dds_qos_create ();
    parts[0] = partname;
    dds_qset_partition (qos, 1, parts);
    rep.code = VDDSREP_ENTITY;
    if (req->code == VDDSREQ_PUBLISHER_CREATE) {
        rep.status = dds_publisher_create (req->u.publisher_create.pp, &rep.u.entity.e, qos, NULL);
    } else {
        assert(req->code == VDDSREQ_SUBSCRIBER_CREATE);
        rep.status = dds_subscriber_create (req->u.subscriber_create.pp, &rep.u.entity.e, qos, NULL);
    }
    dds_qos_delete (qos);
    os_free(partname);
    return reply(cl, &rep);
}

int db_writer_reader_create(struct client *cl, const struct reqhdr *req)
{
    struct rephdr rep;
    size_t sz;
    void *blob;
    int rc;
    if ((rc = rd_blob(cl, &sz, &blob)) < 0) {
        return protocol_error(rc);
    }
    rep.code = VDDSREP_ENTITY;
    if (req->code == VDDSREQ_WRITER_CREATE) {
        rep.status = dds_writer_create(req->u.writer_create.pp_or_pub, &rep.u.entity.e, req->u.writer_create.topic, blob, NULL);
    } else {
        assert(req->code == VDDSREQ_READER_CREATE);
        rep.status = dds_reader_create(req->u.reader_create.pp_or_sub, &rep.u.entity.e, req->u.writer_create.topic, blob, NULL);
    }
    os_free(blob);
    return reply(cl, &rep);
}

int db_write_maybe_async(struct client *cl, const struct reqhdr *req)
{
    struct rephdr rep;
    size_t sz;
    void *blob;
    int rc;
    /* if blob entirely contained in rawbuf, can do without the malloc */
    if ((rc = rd_blob(cl, &sz, &blob)) < 0) {
        return protocol_error(rc);
    }
    rep.code = VDDSREP_NOTHING;
    rep.status = dds_writecdr(req->u.write.wr, blob, sz);
    os_free(blob);
    if (req->code == VDDSREQ_WRITE) {
        return reply(cl, &rep);
    } else {
        assert(req->code == VDDSREQ_WRITE_ASYNC);
        return 1;
    }
}

int db_take(struct client *cl, const struct reqhdr *req)
{
    struct rephdr rep;
    struct serdata **buf = os_malloc(req->u.take.maxs * sizeof(*buf));
    dds_sample_info_t *si = os_malloc(req->u.take.maxs * sizeof(*si));
    int rc;
    int i = 0;
    rep.code = VDDSREP_SAMPLES;
    rep.status = dds_takecdr(req->u.take.rd, buf, req->u.take.maxs, si, req->u.take.mask);
    if ((rc = reply(cl, &rep)) < 0) {
        goto out;
    }
    if (rep.status > 0) {
        if (bufferevent_write(cl->bev, si, sizeof(*si) * rep.status) == -1) {
            rc = -1;
            goto out;
        }
        while (i < rep.status) {
            size_t sz = ddsi_serdata_size(buf[i]);
            if (bufferevent_write(cl->bev, &sz, sizeof(sz)) == -1 ||
                bufferevent_write(cl->bev, &buf[i]->hdr, sz) == -1 ||
                bufferevent_flush(cl->bev, EV_WRITE, BEV_FLUSH) < 0) {
                rc = -1;
                goto out;
            }
            ddsi_serdata_unref(buf[i]);
            i++;
        }
    }
    rc = 1;
out:
    while (i < rep.status) {
        ddsi_serdata_unref(buf[i++]);
    }
    os_free(buf);
    os_free(si);
    if (rc < 0) abort();
    return rc;
}

#define WAITSET_MAX 64
struct dds_waitset_cont {
    struct bufferevent *bev;
    /* memory layout of trp_hdr + trp_xs assumed to match excepted reply in client */
    struct {
        int code; /* = 5, see server.h */
        int status;
        void *pad; /* entity pointer */
    } trp_hdr;
    dds_attach_t trp_xs[WAITSET_MAX];
};

static void waitset_cont(dds_waitset_t ws, void *arg, int ret)
{
    struct dds_waitset_cont *c = arg;
    assert(ret >= 0);
    assert(c->bev != NULL);
    c->trp_hdr.status = ret;
    if (bufferevent_write(c->bev, &c->trp_hdr, sizeof(c->trp_hdr) + ret * sizeof(c->trp_xs[0])) == -1) {
        abort();
    } else {
        bufferevent_flush(c->bev, EV_WRITE, BEV_FLUSH);
    }
    c->bev = NULL;
}

static void waitset_block(dds_waitset_t ws, void *arg, dds_time_t abstimeout)
{
    (void)ws;
    (void)arg;
    (void)abstimeout;
}

int db_waitset_create(struct client *cl, const struct reqhdr *req)
{
    struct rephdr rep;
    struct dds_waitset_cont *c;
    rep.code = VDDSREP_WAITSET;
    rep.status = 0;
    rep.u.waitset.ws = dds_waitset_create_cont(waitset_block, waitset_cont, sizeof(struct dds_waitset_cont));
    c = dds_waitset_get_cont(rep.u.waitset.ws);
    c->bev = NULL;
    c->trp_hdr.code = 5;
    return reply(cl, &rep);
}

int db_waitset_delete(struct client *cl, const struct reqhdr *req)
{
    struct rephdr rep;
    rep.code = VDDSREP_NOTHING;
    rep.status = dds_waitset_delete(req->u.waitset_delete.ws);
    return reply(cl, &rep);
}

int db_waitset_attach(struct client *cl, const struct reqhdr *req)
{
    struct rephdr rep;
    rep.code = VDDSREP_NOTHING;
    rep.status = dds_waitset_attach(req->u.waitset_attach.ws, req->u.waitset_attach.cond, req->u.waitset_attach.x);
    return reply(cl, &rep);
}

int db_waitset_wait(struct client *cl, const struct reqhdr *req)
{
    struct dds_waitset_cont *c;
    c = dds_waitset_get_cont(req->u.wait.ws);
    assert(req->u.wait.nxs <= WAITSET_MAX);
    assert(c->bev == NULL);
    c->bev = cl->bev;
    dds_waitset_wait(req->u.wait.ws, c->trp_xs, req->u.wait.nxs, req->u.wait.reltimeout);
    return 1;
}

int db_readcondition_create(struct client *cl, const struct reqhdr *req)
{
    struct rephdr rep;
    rep.code = VDDSREP_CONDITION;
    rep.status = 0;
    rep.u.condition.cond = dds_readcondition_create(req->u.readcondition_create.e, req->u.readcondition_create.mask);
    return reply(cl, &rep);
}

int dispatch_bound(struct client *cl, const struct reqhdr *req)
{
    switch (req->code) {
        case VDDSREQ_HELLO:
            return protocol_error(-1);
        case VDDSREQ_GOODBYE:
            cl->pid = -1;
            return 0;
        case VDDSREQ_ENTITY_DELETE:
            return db_entity_delete(cl, req);
        case VDDSREQ_PARTICIPANT_CREATE:
            return db_participant_create(cl, req);
        case VDDSREQ_TOPIC_CREATE:
            return db_topic_create(cl, req);
        case VDDSREQ_WRITE_SET_BATCH:
            return db_write_set_batch(cl, req);
        case VDDSREQ_WRITE_FLUSH:
            return db_write_flush(cl, req);
        case VDDSREQ_PUBLISHER_CREATE:
        case VDDSREQ_SUBSCRIBER_CREATE:
            return db_publisher_subscriber_create(cl, req);
        case VDDSREQ_WRITER_CREATE:
        case VDDSREQ_READER_CREATE:
            return db_writer_reader_create(cl, req);
        case VDDSREQ_WRITE:
        case VDDSREQ_WRITE_ASYNC:
            return db_write_maybe_async(cl, req);
        case VDDSREQ_TAKE:
            return db_take(cl, req);
        case VDDSREQ_WAITSET_CREATE:
            return db_waitset_create(cl, req);
        case VDDSREQ_WAITSET_DELETE:
            return db_waitset_delete(cl, req);
        case VDDSREQ_WAITSET_ATTACH:
            return db_waitset_attach(cl, req);
        case VDDSREQ_WAIT:
            return db_waitset_wait(cl, req);
        case VDDSREQ_READCONDITION_CREATE:
            return db_readcondition_create(cl, req);
    }
    return protocol_error(-1);
}

void cb_handle_req(struct bufferevent *bev, void *arg)
{
    struct client * const cl = arg;
    struct evbuffer * const input = bufferevent_get_input(bev);
    unsigned char *mem;
    struct reqhdr req;
    //printf("handle_req\n");
    while ((mem = evbuffer_pullup(input, sizeof(req))) != NULL) {
        int rc;
        req = *(struct reqhdr *)mem;
        (void)evbuffer_drain(input, sizeof(req));
        //printf("%d\n", (int)req.code);
        if ((rc = cl->dispatch(cl, &req)) <= 0) {
            if (rc < 0) {
                printf("sock%d: protocol error\n", bufferevent_getfd(cl->bev));
                bufferevent_free(bev);
                return;
            } else if (rc == 0) {
                cl->dispatch = (cl->pid == -1) ? dispatch_unbound : dispatch_bound;
            }
        }
        bufferevent_flush(bev, EV_WRITE, BEV_FLUSH);
    }
}

void cb_handle_resp(struct bufferevent *bev, void *arg)
{
    //printf("handle_resp\n");
}

void cb_handle_event(struct bufferevent *bev, short events, void *arg)
{
    struct client * const cl = arg;
    printf("handle_event\n");
    if (events & BEV_EVENT_ERROR)
        perror("Error from bufferevent");
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        printf("sock%d: disconnect\n", bufferevent_getfd(cl->bev));
        if (cl->ppant != DDS_HANDLE_NIL) {
            dds_entity_delete(cl->ppant);
        }
        os_free(cl);
        bufferevent_free(bev);
    }
}

void cb_connect(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *addr, int len, void *arg)
{
    struct event_base * const base = evconnlistener_get_base(listener);
    struct bufferevent *bev;
    char tname[32];
    struct client *cl;
    snprintf(tname, sizeof(tname), "sock%d", fd);

    bev = bufferevent_socket_new (base, fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
    if (bev == NULL) {
        abort();
    }

    cl = os_malloc(sizeof(*cl));
    cl->pid = -1;
    cl->ppant = DDS_HANDLE_NIL;
    cl->dispatch = dispatch_unbound;
    cl->bev = bev;

    bufferevent_setcb(bev, cb_handle_req, cb_handle_resp, cb_handle_event, cl);
    if (bufferevent_enable(bev, EV_READ|EV_WRITE) != 0) {
        abort();
    }

    printf("%s: accepted client %p\n", tname, cl);
}

void cb_connect_err(struct evconnlistener *listener, void *arg)
{
    struct event_base * const base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. Shutting down.\n", err, evutil_socket_error_to_string(err));
    event_base_loopexit(base, NULL);
}

int main(int argc, char **argv)
{
    struct sockaddr_un addr;
    int sock, opt;
    struct event_base *evbase;
    struct evconnlistener *evconn;
    struct admin adm;
    int exitcode = EXIT_FAILURE;

    memset(&adm, 0, sizeof(adm));

    //event_enable_debug_mode();

    {
        int rc;
#if defined EVTHREAD_USE_WINDOWS_THREADS_IMPLEMENTED
        rc = evthread_use_windows_threads();
#elif defined EVTHREAD_USE_PTHREADS_IMPLEMENTED
        rc = evthread_use_pthreads();
#else
#error "no libevent threading support"
#endif
        if (rc == -1) {
            fprintf(stderr, "libevent thread support init failed\n");
            return 1;
        }
    }

    while ((opt = getopt(argc, argv, "")) != EOF) {
        switch (opt) {
            default:
                fprintf(stderr, "usage: %s\n", argv[0]);
                return 1;
        }
    }
    umask(077);

    if (grab_lock_file() == -1) {
        return 2;
    }
    if ((sock = socket(PF_LOCAL, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        goto err_socket;
    }
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, VDDS_SOCKET_NAME);
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind");
        goto err_bind;
    }
    if (listen(sock, 0) == -1) {
        perror("listen");
        goto err_listen;
    }
    evutil_make_socket_nonblocking(sock);

    dds_init(argc, argv);
    dds_init_impl(DDS_DOMAIN_DEFAULT);

    if ((evbase = event_base_new()) == NULL) {
        goto err_event_base;
    }

    evconn = evconnlistener_new(evbase, cb_connect, &adm, 0, 0, sock);
    evconnlistener_set_error_cb(evconn, cb_connect_err);
    event_base_loop(evbase, 0);
    exitcode = 0;

    event_base_free(evbase);
err_event_base:
    dds_fini();
err_listen:
    unlink(VDDS_SOCKET_NAME);
err_bind:
    close(sock);
err_socket:
    rm_lock_file();
#if LIBEVENT_VERSION_NUMBER >= 0x2010100
    libevent_global_shutdown();
#endif
    return exitcode;
}
