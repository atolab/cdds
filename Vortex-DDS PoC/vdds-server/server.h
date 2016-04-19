#ifndef VDDS_SERVER_H
#define VDDS_SERVER_H

#define VDDS_SOCKET_NAME "/tmp/vdds-server"

enum reqcode {
    VDDSREQ_HELLO,
    VDDSREQ_GOODBYE,
    VDDSREQ_ENTITY_DELETE,
    VDDSREQ_PARTICIPANT_CREATE,
    VDDSREQ_TOPIC_CREATE,
    VDDSREQ_WRITE_SET_BATCH,
    VDDSREQ_WRITE_FLUSH,
    VDDSREQ_PUBLISHER_CREATE,
    VDDSREQ_SUBSCRIBER_CREATE,
    VDDSREQ_WRITER_CREATE,
    VDDSREQ_READER_CREATE,
    VDDSREQ_WRITE,
    VDDSREQ_WRITE_ASYNC,
    VDDSREQ_TAKE,
    VDDSREQ_WAITSET_CREATE,
    VDDSREQ_WAITSET_DELETE,
    VDDSREQ_WAITSET_ATTACH,
    VDDSREQ_WAIT,
    VDDSREQ_READCONDITION_CREATE,
};

enum repcode {
    VDDSREP_NOTHING, /* just status (or not even that) */
    VDDSREP_ENTITY, /* entity as out parameter */
    VDDSREP_WAITSET,
    VDDSREP_CONDITION,
    VDDSREP_SAMPLES, /* samples returned by read/take */
    VDDSREP_WAITRESULT
};

struct reqhdr {
    enum reqcode code;
    union {
        struct {
            pid_t pid;
        } hello;
        struct {
            char dummy;
        } goodbye;
        struct {
            dds_entity_t entity;
        } entity_delete;
        struct {
            dds_domainid_t domain;
        } participant_create;
        struct {
            dds_entity_t pp;
        } topic_create;
        struct {
            bool enable;
        } write_set_batch;
        struct {
            dds_entity_t wr;
        } write_flush;
        struct {
            dds_entity_t pp;
        } publisher_create;
        struct {
            dds_entity_t pp;
        } subscriber_create;
        struct {
            dds_entity_t pp_or_pub;
            dds_entity_t topic;
        } writer_create;
        struct {
            dds_entity_t pp_or_sub;
            dds_entity_t topic;
        } reader_create;
        struct {
            dds_entity_t wr;
        } write;
        struct {
            dds_entity_t rd;
            uint32_t maxs;
            uint32_t mask;
        } take;
        struct {
            dds_waitset_t ws;
            size_t nxs;
            dds_time_t reltimeout;
        } wait;
        struct {
            dds_waitset_t ws;
        } waitset_delete;
        struct {
            dds_waitset_t ws;
            dds_condition_t cond;
            dds_attach_t x;
        } waitset_attach;
        struct {
            dds_entity_t e;
            uint32_t mask;
        } readcondition_create;
    } u;
};

struct rephdr {
    enum repcode code;
    int status;
    union {
        struct {
            dds_entity_t e;
        } entity;
        struct {
            dds_waitset_t ws;
        } waitset;
        struct {
            dds_condition_t cond;
        } condition;
    } u;
};

#endif