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

#include "os/os.h"
#include "dds.h"
#include "server.h"
#include "kernel/dds_init.h"
#include "kernel/dds_rhc.h"
#include "ddsi/q_thread.h"
#include "ddsi/ddsi_ser.h"

#define LOCKFILE_NAME "/tmp/vdds-server-lock"

#define IOBUF_SIZE 4096
struct iobuf1 {
    size_t rp, wp;
    char buf[4096];
};

struct iobuf {
    int fd;
    struct iobuf1 r, w;
};

int ibread(struct iobuf *b, void *dst, size_t n)
{
    size_t av = b->r.wp - b->r.rp;
    if (av == 0) {
        ssize_t nrd;
        if ((nrd = read(b->fd, b->r.buf, sizeof(b->r.buf))) <= 0) {
            return -1;
        }
        b->r.rp = 0;
        b->r.wp = (size_t)nrd;
        av = b->r.wp - b->r.rp;
    }
    if (n <= av) {
        memcpy(dst, b->r.buf + b->r.rp, n);
        b->r.rp += n;
        return 1;
    } else {
        memcpy(dst, b->r.buf + b->r.rp, av);
        b->r.rp += av;
        return ibread(b, (char *)dst + av, n - av);
    }
}

int ibavail(const struct iobuf *b)
{
    return b->r.rp < b->r.wp;
}

int ibwrite(struct iobuf *b, const void *src, size_t n)
{
    size_t av = sizeof(b->w.buf) - b->w.wp;
    if (av == 0) {
        ssize_t nwr;
        if ((nwr = write(b->fd, b->w.buf + b->w.rp, b->w.wp - b->w.rp)) <= 0) {
            return -1;
        }
        b->w.rp += (size_t)nwr;
        if (b->w.rp < b->w.wp) {
            memmove(b->w.buf, b->w.buf + b->w.rp, b->w.wp - b->w.rp);
            b->w.wp -= b->w.rp;
            b->w.rp = 0;
        } else {
            b->w.rp = b->w.wp = 0;
        }
        av = sizeof(b->w.buf) - b->w.wp;
    }
    if (n <= av) {
        memcpy(b->w.buf + b->w.wp, src, n);
        b->w.wp += n;
        return 1;
    } else {
        memcpy(b->w.buf + b->w.wp, src, av);
        b->w.wp += av;
        return ibwrite(b, (const char *)src + av, n - av);
    }
}

int ibflush(struct iobuf *b)
{
    while (b->w.rp < b->w.wp) {
        ssize_t nwr;
        if ((nwr = write(b->fd, b->w.buf + b->w.rp, b->w.wp - b->w.rp)) <= 0) {
            return -1;
        }
        b->w.rp += (size_t)nwr;
    }
    b->w.rp = b->w.wp = 0;
    return 1;
}

struct client {
    struct iobuf ib;
    pid_t pid;
    dds_entity_t ppant;
    int (*dispatch) (struct client *cl, const struct reqhdr *req);
    struct thread_state1 *server_tid;
    struct client *next;
};

sig_atomic_t terminate = 0;
os_mutex clients_lock;
os_cond clients_cond;
struct client *clients = NULL;
struct client *dead_clients = NULL;

void sigh(int sig __attribute__ ((unused)))
{
    terminate = 1;
}

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

#if 0
int make_shmem(void **paddr, size_t size)
{
    char template[] = "/tmp/vdds.XXXXXX";
    struct stat statbuf;
    int fd;
    void *addr;
    if ((fd = mkstemp(template)) == -1) {
        perror("mkstemp");
        goto err_mkstemp;
    }
    if (unlink(template) == -1) {
        perror("unlink");
        goto err_unlink;
    }
    if (fstat(fd, &statbuf) == -1) {
        perror("fstat");
        goto err_fstat;
    }
    if (statbuf.st_nlink != 0) {
        fprintf(stderr, "Filesystem sill has links to the inode? Someone must be playing games\n");
        goto err_fstat;
    }
    if ((ftruncate(fd, size)) == -1) {
        perror("ftruncate");
        goto err_ftruncate;
    }
    addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == (void *) -1) {
        perror("mmap");
        goto err_mmap;
    }
    *paddr = addr;
    return fd;
err_mmap:
err_ftruncate:
err_fstat:
err_unlink:
    close(fd);
err_mkstemp:
    *paddr = NULL;
    return -1;
}
#endif

int reply(struct client *cl, const struct rephdr *rep)
{
    return ibwrite(&cl->ib, rep, sizeof(*rep));
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
        printf("sock%d: pid %d\n", cl->ib.fd, (int)cl->pid);
        return 0;
    } else {
        return -1;
    }
}

int rd_blob(struct client *cl, size_t *sz, void **blob)
{
    if (ibread(&cl->ib, sz, sizeof(*sz)) < 0) {
        return -1;
    }
    *blob = os_malloc(*sz);
    if (ibread(&cl->ib, *blob, *sz) < 0) {
        os_free(*blob);
        return -1;
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
    dds_delete(req->u.entity_delete.entity);
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
        rep.status = dds_create_subscriber (req->u.subscriber_create.pp, &rep.u.entity.e, qos, NULL);
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
        if (ibwrite(&cl->ib, si, sizeof(*si) * rep.status) < 0) {
            goto out;
        }
        while (i < rep.status) {
            size_t sz = ddsi_serdata_size(buf[i]);
            if (ibwrite(&cl->ib, &sz, sizeof(sz)) < 0 || ibwrite(&cl->ib, &buf[i]->hdr, sz) < 0) {
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
    return rc;
}

#define WAITSET_MAX 64
struct dds_waitset_cont {
  int trp_fd;
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
    assert(c->trp_fd >= 0);
    c->trp_hdr.status = ret;
    if (write(c->trp_fd, &c->trp_hdr, sizeof(c->trp_hdr) + ret * sizeof(c->trp_xs[0])) != sizeof(c->trp_hdr) + ret * sizeof(c->trp_xs[0])) {
        abort();
    }
    c->trp_fd = -1;
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
    c->trp_fd = -1;
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
    assert(c->trp_fd == -1);
    c->trp_fd = cl->ib.fd;
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
    //printf("%d\n", (int)req->code);
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

int serve_one(struct client *cl)
{
    struct reqhdr req;
    if (ibread(&cl->ib, &req, sizeof(req)) > 0) {
        int rc;
        if ((rc = cl->dispatch(cl, &req)) <= 0) {
            if (rc < 0) {
                printf("sock%d: protocol error\n", cl->ib.fd);
                goto error;
            } else if (rc == 0) {
                cl->dispatch = (cl->pid == -1) ? dispatch_unbound : dispatch_bound;
            }
        }
        ibflush(&cl->ib);
    }
    return 1;
error:
    printf("sock%d: disconnect\n", cl->ib.fd);
    if (cl->ppant != DDS_HANDLE_NIL) {
        dds_delete(cl->ppant);
    }

    {
        struct client **ptr, *cur;
        os_mutexLock(&clients_lock);
        for (ptr = &clients, cur = clients; cur != cl; ptr = &cur->next, cur = cur->next)
            ;
        *ptr = cl->next;
        cl->next = dead_clients;
        dead_clients = cl;
        os_mutexUnlock(&clients_lock);
    }
    return -1;
}

void *server(void *vclient)
{
    struct client *cl = vclient;
    while (!terminate && serve_one(cl) > 0)
        ;
    return NULL;
}

void reap_dead_client(struct client *cl)
{
    (void)join_thread(cl->server_tid, NULL);
    close(cl->ib.fd);
    os_free(cl);
}

void reap_dead_clients_unlocked(void)
{
    while (dead_clients) {
        struct client *cl = dead_clients;
        dead_clients = cl->next;
        reap_dead_client(cl);
    }
}

void reap_dead_clients(void)
{
    os_mutexLock(&clients_lock);
    reap_dead_clients_unlocked();
    os_mutexUnlock(&clients_lock);
}

#define MAX_CLIENTS 16

int main(int argc, char **argv)
{
    struct sockaddr_un addr;
    int sock, opt;
    struct pollfd pollfd[1 + MAX_CLIENTS];
    int single = 0;
    int ncl = 0;
    int pending = 0;
    struct client *ix2cl[1 + MAX_CLIENTS];

    while ((opt = getopt(argc, argv, "1")) != EOF) {
        switch (opt) {
            case '1':
                single = 1;
                break;
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
    signal(SIGINT, sigh);
    signal(SIGTERM, sigh);

    dds_init(argc, argv);
    dds_init_impl(DDS_DOMAIN_DEFAULT);
    os_mutexInit(&clients_lock);
    os_condInit(&clients_cond, &clients_lock);

    pollfd[0].fd = sock;
    pollfd[0].events = POLLIN;
    printf("listening ...\n");
    fflush(stdout);
    while (!terminate) {
        int res;
        reap_dead_clients();
        if ((res = poll(pollfd, 1 + ncl, pending ? 0 : -1)) == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("poll");
                goto err_poll;
            }
        }

        if (pollfd[0].revents & POLLIN) {
            struct sockaddr_un addr;
            socklen_t addrlen;
            char tname[32];
            struct client *cl;
            int s;
            if ((s = accept(sock, (struct sockaddr *)&addr, &addrlen)) == -1) {
                perror("accept");
                goto err_accept;
            }
            snprintf(tname, sizeof(tname), "sock%d", s);

            cl = os_malloc(sizeof(*cl));
            cl->pid = -1;
            cl->ppant = DDS_HANDLE_NIL;
            cl->dispatch = dispatch_unbound;
            cl->ib.fd = s;
            cl->ib.r.rp = cl->ib.r.wp = 0;
            cl->ib.w.rp = cl->ib.w.wp = 0;
            os_mutexLock(&clients_lock);
            cl->next = clients;
            clients = cl;
            os_mutexUnlock(&clients_lock);

            printf("%s: accepted client %p\n", tname, cl);
            if (!single) {
                cl->server_tid = create_thread(tname, server, cl);
            } else {
                ++ncl;
                pollfd[ncl].fd = s;
                pollfd[ncl].events = POLLIN | POLLHUP;
                ix2cl[ncl] = cl;
            }
        }

        if (single)
        {
            int i = 1;
            pending = 0;
            while (i <= ncl) {
                if (!(pollfd[i].revents & (POLLIN | POLLHUP)) && !ibavail(&ix2cl[i]->ib)) {
                    ++i;
                } else if (serve_one(ix2cl[i]) >= 0) {
                    pending += ibavail(&ix2cl[i]->ib);
                    ++i;
                } else {
                    if (i < ncl) {
                        ix2cl[i] = ix2cl[ncl];
                        pollfd[i] = pollfd[ncl];
                    }
                    --ncl;
                }
            }
        }
    }

    terminate = 1;
    os_mutexLock(&clients_lock);
    while (clients) {
        os_condWait(&clients_cond, &clients_lock);
        reap_dead_clients_unlocked();
    }
    os_mutexUnlock(&clients_lock);

    os_condDestroy(&clients_cond);
    os_mutexDestroy(&clients_lock);
    dds_fini();
    unlink(VDDS_SOCKET_NAME);
    close(sock);
    rm_lock_file();
    return 0;

err_accept:
err_poll:
err_listen:
    unlink(VDDS_SOCKET_NAME);
err_bind:
    close(sock);
err_socket:
    rm_lock_file();
    return 2;
}
