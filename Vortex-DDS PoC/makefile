.PHONY: all clean

OPT = #
CC = clang
LD = clang
CFLAGS = -c -DLITE=1 $(OPT) -g -I"$(PWD)/src/include" -I"$(PWD)/src/include/dds" -I"$(PWD)/src/ddsi/include" -I"$(PWD)/src/kernel/include" -I"$(PWD)/src/os/include" -I"$(PWD)/src/util/include" -I"$(PWD)/src/os/darwin/include" -I"$(PWD)/examples/generated"
LDFLAGS = -g -Wall -L"$(PWD)/gen" -rpath "$(PWD)/gen"

all: gen/libvdds.dylib gen/libvdds-stubs.dylib gen/vdds-server gen/publisher gen/subscriber gen/rpc-publisher gen/rpc-subscriber
clean: ; rm -rf gen/*

LIBVDDS_DIRS := src/ddsi/code src/kernel/code src/os/code src/os/darwin/code src/util/code

LIBVDDS := $(notdir $(patsubst %.c, %, $(filter-out %_template.c, $(wildcard $(LIBVDDS_DIRS:%=%/*.c)))))

LIBVDDS_STUBS := $(patsubst %.c, %, $(notdir $(wildcard src/os/code/*.c src/os/darwin/code/*.c))) vdds-stubs dds_alloc dds_time dds_stream dds_key dds_err dds_qos q_bswap q_bswap_inlines q_md5 q_plist q_time q_misc q_osplser ddsi_ser

vpath %.c $(LIBVDDS_DIRS) vdds-server examples examples/generated rpc-examples

gen/libvdds.dylib: $(LIBVDDS:%=gen/%.o)
	$(LD) $(LDFLAGS) -dynamiclib -install_name @rpath/libvdds.dylib $^ -o $@

gen/libvdds-stubs.dylib: $(LIBVDDS_STUBS:%=gen/%.o)
	$(LD) $(LDFLAGS) -dynamiclib -install_name @rpath/libvdds-stubs.dylib $^ -o $@

gen/vdds-server: gen/server.o | gen/libvdds.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds

gen/publisher: gen/publisher.o gen/Throughput.o | gen/libvdds.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds

gen/subscriber: gen/subscriber.o gen/Throughput.o | gen/libvdds.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds

gen/rpc-publisher: gen/rpc-publisher.o gen/Throughput.o | gen/libvdds-stubs.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds-stubs

gen/rpc-subscriber: gen/rpc-subscriber.o gen/Throughput.o | gen/libvdds-stubs.dylib
	$(LD) $(LDFLAGS) $^ -o $@ -lvdds-stubs

gen/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
