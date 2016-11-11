.PHONY: all clean

include config.mk

CPPFLAGS += -Iinclude
CPPFLAGS += -DDDSI_INCLUDE_SSM -DDDSI_INCLUDE_BANDWIDTH_LIMITING -DDDSI_INCLUDE_NETWORK_PARTITIONS -DDDSI_INCLUDE_NETWORK_CHANNELS

SHLIBS = vdds vdds-stubs
EXES   = vdds-server vdds-server2 ppss publisher subscriber rpc-publisher rpc-subscriber ping pong rpc-ping rpc-pong rpc-pingpong

all: $(SHLIBS:%=gen/$(LIBPRE)%$(SO)) $(EXES:%=gen/%$X)

clean:
	rm -rf gen/*

LIBVDDS_DIRS := src/ddsi src/kernel src/util src/os src/os/$(OS)

vpath %.c $(LIBVDDS_DIRS) vdds-server examples examples/generated rpc-examples

LIBVDDS := $(notdir $(patsubst %.c, %, $(filter-out %_template.c, $(wildcard $(LIBVDDS_DIRS:%=%/*.c)))))
gen/$(LIBPRE)vdds$(SO): $(LIBVDDS:%=gen/%$O)
	$(make_shlib)

LIBVDDS_STUBS := $(patsubst %.c, %, $(notdir $(wildcard src/os/*.c src/os/$(OS)/*.c))) vdds-stubs dds_alloc dds_time dds_stream dds_key dds_err dds_qos q_bswap q_bswap_inlines q_md5 q_plist q_time q_misc q_osplser ddsi_ser
gen/$(LIBPRE)vdds-stubs$(SO): $(LIBVDDS_STUBS:%=gen/%$O)
	$(make_shlib)

gen/vdds-server$X: gen/server$O | gen/$(LIBPRE)vdds$(SO)
	$(make_exe)

gen/vdds-server$X gen/vdds-server2$X: LDLIBS += -lvdds
gen/vdds-server2$X: LDFLAGS += -L/usr/local/lib -rpath /usr/local/lib
gen/vdds-server2$X: LDLIBS += -levent_pthreads -levent
gen/vdds-server2$X: CPPFLAGS += -I/usr/local/include
gen/vdds-server2$X: gen/server2$O | gen/$(LIBPRE)vdds$(SO)
	$(make_exe)

gen/publisher$O gen/subscriber$O gen/rpc-publisher$O gen/rpc-subscriber$O gen/ping$O gen/pong$O gen/rpc-ping$O gen/rpc-pong$O gen/ppss$O: CPPFLAGS += -Iexamples/generated
gen/publisher.d gen/subscriber.d gen/rpc-publisher.d gen/rpc-subscriber.d gen/ping.d gen/pong.d gen/rpc-ping.d gen/rpc-pong.d gen/ppss.d: CPPFLAGS += -Iexamples/generated

gen/ppss: LDLIBS += -lvdds
gen/publisher gen/subscriber gen/ping gen/pong: LDLIBS += -lvdds
gen/rpc-publisher gen/rpc-subscriber gen/rpc-ping gen/rpc-pong: LDLIBS += -lvdds-stubs
gen/ppss$X: gen/ppss$O gen/Throughput$O | gen/$(LIBPRE)vdds$(SO)
	$(make_exe)
gen/publisher$X: gen/publisher$O gen/Throughput$O | gen/$(LIBPRE)vdds$(SO)
	$(make_exe)
gen/subscriber$X: gen/subscriber$O gen/Throughput$O | gen/$(LIBPRE)vdds$(SO)
	$(make_exe)
gen/rpc-publisher$X: gen/rpc-publisher$O gen/Throughput$O | gen/$(LIBPRE)vdds-stubs$(SO)
	$(make_exe)
gen/rpc-subscriber$X: gen/rpc-subscriber$O gen/Throughput$O | gen/$(LIBPRE)vdds-stubs$(SO)
	$(make_exe)
gen/ping$X: gen/ping$O gen/RoundTrip$O | gen/$(LIBPRE)vdds$(SO)
	$(make_exe)
gen/pong$X: gen/pong$O gen/RoundTrip$O | gen/$(LIBPRE)vdds$(SO)
	$(make_exe)
gen/rpc-ping$X: gen/rpc-ping$O gen/RoundTrip$O | gen/$(LIBPRE)vdds-stubs$(SO)
	$(make_exe)
gen/rpc-pong$X: gen/rpc-pong$O gen/RoundTrip$O | gen/$(LIBPRE)vdds-stubs$(SO)
	$(make_exe)

gen/rpc-pingpong$X: LDLIBS += -lvdds
gen/rpc-pingpong$X: gen/rpc-ping$O gen/rpc-pong$O gen/rpc-pingpong$O gen/RoundTrip$O | gen/$(LIBPRE)vdds$(SO)
	$(make_exe)

gen/%$O: %.c
	$(CC) $(CPPFLAGS) $(OBJ_OFLAG)$@ -c $(abspath $<)

gen/%.d: %.c
	$(make_dep)

ifneq ($(MAKECMDGOALS),clean)
  -include $(LIBVDDS:%=gen/%.d) $(LIBVDDS_STUBS:%=gen/%.d) gen/server.d gen/publisher.d gen/Throuhgput.d gen/subscriber.d gen/rpc-publisher.d gen/rpc-subscriber.d gen/ping.d gen/RoundTrip.d gen/pong.d gen/rpc-ping.d gen/rpc-pong.d
endif
