.PHONY: all clean

OPT = #
CC = clang
LD = clang
CFLAGS = -c -DLITE=1 $(OPT) -g -I$(PWD)/include
LDFLAGS = -g -Wall -L$(PWD)/gen -rpath $(PWD)/gen

all: gen/libvdds.dylib gen/libvdds-stubs.dylib gen/vdds-server gen/publisher gen/subscriber gen/rpc-publisher gen/rpc-subscriber gen/ping gen/pong gen/rpc-ping gen/rpc-pong
clean: ; rm -rf gen/*

LIBVDDS_DIRS := src/ddsi src/kernel src/util src/os src/os/darwin

LIBVDDS := $(notdir $(patsubst %.c, %, $(filter-out %_template.c, $(wildcard $(LIBVDDS_DIRS:%=%/*.c)))))

LIBVDDS_STUBS := $(patsubst %.c, %, $(notdir $(wildcard src/os/*.c src/os/darwin/*.c))) vdds-stubs dds_alloc dds_time dds_stream dds_key dds_err dds_qos q_bswap q_bswap_inlines q_md5 q_plist q_time q_misc q_osplser ddsi_ser

vpath %.c $(LIBVDDS_DIRS) vdds-server examples examples/generated rpc-examples

gen/libvdds.dylib: $(LIBVDDS:%=gen/%.o)
	$(LD) $(LDFLAGS) -dynamiclib -install_name @rpath/libvdds.dylib $^ -o $@

gen/libvdds-stubs.dylib: $(LIBVDDS_STUBS:%=gen/%.o)
	$(LD) $(LDFLAGS) -dynamiclib -install_name @rpath/libvdds-stubs.dylib $^ -o $@

gen/vdds-server: gen/server.o | gen/libvdds.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds

gen/publisher.o gen/subscriber.o gen/rpc-publisher.o gen/rpc-subscriber.o gen/ping.o gen/pong.o gen/rpc-ping.o gen/rpc-pong.o: CFLAGS += -I$(PWD)/examples/generated
gen/publisher.d gen/subscriber.d gen/rpc-publisher.d gen/rpc-subscriber.d gen/ping.d gen/pong.d gen/rpc-ping.d gen/rpc-pong.d: CFLAGS += -I$(PWD)/examples/generated

gen/publisher: gen/publisher.o gen/Throughput.o | gen/libvdds.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds

gen/subscriber: gen/subscriber.o gen/Throughput.o | gen/libvdds.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds

gen/rpc-publisher: gen/rpc-publisher.o gen/Throughput.o | gen/libvdds-stubs.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds-stubs

gen/rpc-subscriber: gen/rpc-subscriber.o gen/Throughput.o | gen/libvdds-stubs.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds-stubs

gen/ping: gen/ping.o gen/RoundTrip.o | gen/libvdds.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds

gen/pong: gen/pong.o gen/RoundTrip.o | gen/libvdds.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds

gen/rpc-ping: gen/rpc-ping.o gen/RoundTrip.o | gen/libvdds-stubs.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds-stubs

gen/rpc-pong: gen/rpc-pong.o gen/RoundTrip.o | gen/libvdds-stubs.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds-stubs

gen/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

gen/%.d: %.c
	$(CC) -M $(CFLAGS) $< -o $@

ifneq ($(MAKECMDGOALS),clean)
  -include $(LIBVDDS:%=gen/%.d) $(LIBVDDS_STUBS:%=gen/%.d) gen/server.d gen/publisher.d gen/Throuhgput.d gen/subscriber.d gen/rpc-publisher.d gen/rpc-subscriber.d gen/ping.d gen/RoundTrip.d gen/pong.d gen/rpc-ping.d gen/rpc-pong.d
endif
