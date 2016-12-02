.PHONY: all clean

include config.mk

CPPFLAGS += -Iinclude
CPPFLAGS += -DDDSI_INCLUDE_SSM -DDDSI_INCLUDE_BANDWIDTH_LIMITING -DDDSI_INCLUDE_NETWORK_PARTITIONS -DDDSI_INCLUDE_NETWORK_CHANNELS

SHLIBS = vdds
EXES   = publisher subscriber ping pong ddpingpong
ifeq "$(OS)" "darwin"
SHLIBS += vdds-stubs
EXES   += vdds-server vdds-server2 ppss rpc-publisher rpc-subscriber rpc-ping rpc-pong rpc-pingpong
endif

all: $(SHLIBS:%=$(GEN)/$(LIBPRE)%$(SO)) $(EXES:%=$(GEN)/%$X)

clean:
	rm -rf $(GEN)/*

LIBVDDS_DIRS := src/ddsi src/kernel src/util src/os $(OSX:%=src/os/%)

vpath %.c $(LIBVDDS_DIRS) vdds-server examples examples/generated rpc-examples

LIBVDDS := $(notdir $(patsubst %.c, %, $(filter-out %_template.c, $(wildcard $(LIBVDDS_DIRS:%=%/*.c)))))
$(GEN)/$(LIBPRE)vdds$(SO): $(LIBVDDS:%=$(GEN)/%$O)
	$(make_shlib)

LIBVDDS_STUBS := $(patsubst %.c, %, $(notdir $(wildcard src/os/*.c $(OSX:%=src/os/%/*.c)))) vdds-stubs dds_alloc dds_time dds_stream dds_key dds_err dds_qos q_bswap q_bswap_inlines q_md5 q_plist q_time q_misc q_osplser ddsi_ser q_freelist sysdeps
$(GEN)/$(LIBPRE)vdds-stubs$(SO): $(LIBVDDS_STUBS:%=$(GEN)/%$O)
	$(make_shlib)

$(GEN)/vdds-server$X: $(GEN)/server$O | $(GEN)/$(LIBPRE)vdds$(SO)
	$(make_exe)

$(GEN)/vdds-server$X $(GEN)/vdds-server2$X: LDLIBS += -lvdds
$(GEN)/vdds-server2$X: LDFLAGS += -L/usr/local/lib -rpath /usr/local/lib
$(GEN)/vdds-server2$X: LDLIBS += -levent_pthreads -levent
$(GEN)/vdds-server2$X: CPPFLAGS += -I/usr/local/include
$(GEN)/vdds-server2$X: $(GEN)/server2$O | $(GEN)/$(LIBPRE)vdds$(SO)
	$(make_exe)

$(GEN)/publisher$O $(GEN)/subscriber$O $(GEN)/rpc-publisher$O $(GEN)/rpc-subscriber$O $(GEN)/ping$O $(GEN)/pong$O $(GEN)/ddpingpong$O $(GEN)/rpc-ping$O $(GEN)/rpc-pong$O $(GEN)/ppss$O: CPPFLAGS += -Iexamples/generated
$(GEN)/publisher.d $(GEN)/subscriber.d $(GEN)/rpc-publisher.d $(GEN)/rpc-subscriber.d $(GEN)/ping.d $(GEN)/pong.d $(GEN)/ddpingpong.d $(GEN)/rpc-ping.d $(GEN)/rpc-pong.d $(GEN)/ppss.d: CPPFLAGS += -Iexamples/generated

$(GEN)/ppss: LDLIBS += -lvdds
$(GEN)/ddpingpong: LDLIBS += -lvdds
$(GEN)/publisher $(GEN)/subscriber $(GEN)/ping $(GEN)/pong: LDLIBS += -lvdds
$(GEN)/rpc-publisher $(GEN)/rpc-subscriber $(GEN)/rpc-ping $(GEN)/rpc-pong: LDLIBS += -lvdds-stubs
$(GEN)/ppss$X: $(GEN)/ppss$O $(GEN)/Throughput$O | $(GEN)/$(LIBPRE)vdds$(SO)
	$(make_exe)
$(GEN)/publisher$X: $(GEN)/publisher$O $(GEN)/Throughput$O | $(GEN)/$(LIBPRE)vdds$(SO)
	$(make_exe)
$(GEN)/subscriber$X: $(GEN)/subscriber$O $(GEN)/Throughput$O | $(GEN)/$(LIBPRE)vdds$(SO)
	$(make_exe)
$(GEN)/rpc-publisher$X: $(GEN)/rpc-publisher$O $(GEN)/Throughput$O | $(GEN)/$(LIBPRE)vdds-stubs$(SO)
	$(make_exe)
$(GEN)/rpc-subscriber$X: $(GEN)/rpc-subscriber$O $(GEN)/Throughput$O | $(GEN)/$(LIBPRE)vdds-stubs$(SO)
	$(make_exe)
$(GEN)/ping$X: $(GEN)/ping$O $(GEN)/RoundTrip$O | $(GEN)/$(LIBPRE)vdds$(SO)
	$(make_exe)
$(GEN)/pong$X: $(GEN)/pong$O $(GEN)/RoundTrip$O | $(GEN)/$(LIBPRE)vdds$(SO)
	$(make_exe)
$(GEN)/ddpingpong$X: $(GEN)/ddpingpong$O $(GEN)/RoundTrip$O | $(GEN)/$(LIBPRE)vdds$(SO)
	$(make_exe)
$(GEN)/rpc-ping$X: $(GEN)/rpc-ping$O $(GEN)/RoundTrip$O | $(GEN)/$(LIBPRE)vdds-stubs$(SO)
	$(make_exe)
$(GEN)/rpc-pong$X: $(GEN)/rpc-pong$O $(GEN)/RoundTrip$O | $(GEN)/$(LIBPRE)vdds-stubs$(SO)
	$(make_exe)

$(GEN)/rpc-pingpong$X: LDLIBS += -lvdds
$(GEN)/rpc-pingpong$X: $(GEN)/rpc-ping$O $(GEN)/rpc-pong$O $(GEN)/rpc-pingpong$O $(GEN)/RoundTrip$O | $(GEN)/$(LIBPRE)vdds$(SO)
	$(make_exe)

$(GEN)/%$O: %.c
	$(CC) $(CPPFLAGS) $(OBJ_OFLAG)$@ -c $(call getabspath, $<)

$(GEN)/%.d: %.c
	$(make_dep)

ifneq ($(MAKECMDGOALS),clean)
  -include $(LIBVDDS:%=$(GEN)/%.d) $(LIBVDDS_STUBS:%=$(GEN)/%.d) $(GEN)/server.d $(GEN)/publisher.d $(GEN)/Throuhgput.d $(GEN)/subscriber.d $(GEN)/rpc-publisher.d $(GEN)/rpc-subscriber.d $(GEN)/ping.d $(GEN)/RoundTrip.d $(GEN)/pong.d $(GEN)ddpingpong.d $(GEN)/rpc-ping.d $(GEN)/rpc-pong.d
endif
