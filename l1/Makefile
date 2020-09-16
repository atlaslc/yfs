LAB=8
SOL=8
RPC=../rpc
CFLAGS =  -g -MD -Wall -I$(RPC) -DLAB=$(LAB) -DSOL=$(SOL) -D_FILE_OFFSET_BITS=64
CXX = g++

LOCK_SERVER = lock_server.o lock_smain.o rpc.o host.o chan.o
LOCK_TESTER = lock_tester.o lock_client.o rpc.o host.o chan.o
LOCK_DEMO = lock_demo.o lock_client.o rpc.o host.o chan.o

RPCTEST = rpctest.o rpc.o host.o chan.o

LOCK_SERVER_7 = $(LOCK_SERVER) rsm.o view.o
LOCK_TESTER_7 = $(LOCK_TESTER) rsm_client.o view.o
LOCK_DEMO_7 = $(LOCK_DEMO) rsm_client.o view.o

YFS_CLIENT = yfs_client.o extent_client.o
EXTENT_SERVER = extent_server.o extent_smain.o rpc.o host.o chan.o
FUSE2YFS = fuse.o rpc.o host.o chan.o

FUSEFLAGS = -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=25 -I/usr/local/include/fuse -I/usr/include/fuse
MACFLAGS = -D__FreeBSD__=10

all: lab1

lab1: lock_server lock_tester lock_demo rpctest

extent_server : $(EXTENT_SERVER)
	$(CXX) $(CFLAGS) -o extent_server $(EXTENT_SERVER) -lpthread

rpctest : $(RPCTEST)
	$(CXX) $(CFLAGS) -o rpctest $(RPCTEST) -lpthread

yfs_client : $(FUSE2YFS) $(YFS_CLIENT)
	if [ `uname` = "linux" -o `uname` = "Linux" -o `uname` = "linux-gnu" ]; then \
		$(CXX) $(CFLAGS) -o yfs_client $(FUSE2YFS) $(YFS_CLIENT) -L/usr/local/lib -lfuse -lpthread -lrt -ldl; \
	else \
		$(CXX) $(CFLAGS) -o yfs_client $(FUSE2YFS) $(YFS_CLIENT) -L/usr/local/lib -lfuse -lpthread; \
	fi

LS=$(LOCK_SERVER)
lock_server : $(LS)
	if [ `uname` = "SunOS" ]; then \
		$(CXX) $(CFLAGS) -o lock_server $(LS) -lpthread -lxnet; \
	else \
		$(CXX) $(CFLAGS) -o lock_server $(LS) -lpthread; \
	fi

LT=$(LOCK_TESTER)
lock_tester : $(LT)
	if [ `uname` = "SunOS" ]; then \
		$(CXX) $(CFLAGS) -o lock_tester $(LT) -lpthread -lxnet; \
	else \
		$(CXX) $(CFLAGS) -o lock_tester $(LT) -lpthread; \
	fi

LD=$(LOCK_DEMO)
lock_demo: $(LD)
	if [ `uname` = "SunOS" ]; then \
		$(CXX) $(CFLAGS) -o lock_demo $(LD) -lpthread -lxnet; \
	else \
		$(CXX) $(CFLAGS) -o lock_demo $(LD) -lpthread; \
	fi

rpc.o: $(RPC)/rpc.cc
	$(CXX) $(CFLAGS) -c $(RPC)/rpc.cc

rpctest.o: $(RPC)/rpctest.cc
	$(CXX) $(CFLAGS) -c $(RPC)/rpctest.cc

chan.o: $(RPC)/chan.cc
	$(CXX) $(CFLAGS) -c $(RPC)/chan.cc

host.o: $(RPC)/host.cc
	$(CXX) $(CFLAGS) -c $(RPC)/host.cc

%.o : %.cc
	$(CXX) -c $(CFLAGS) $<

fuse.o: fuse.cc
	if [ `uname` = "linux" -o `uname` = "Linux" -o `uname` = "linux-gnu" ]; then \
                $(CXX) -c $(CFLAGS) $(FUSEFLAGS) $<; \
        else \
                $(CXX) -c $(CFLAGS) $(FUSEFLAGS) $(MACFLAGS) $<; \
        fi       

LAB1_SRC =  lock_server.cc lock_smain.cc lock_tester.cc lock_client.cc lock_protocol.h lock_client.h lock_server.h lock_demo.cc Makefile

LAB2_SRC =  yfs_client.cc yfs_client.h extent_protocol.h extent_client.cc extent_client.h extent_server.cc extent_server.h extent_smain.cc fuse.cc start.sh stop.sh Makefile test-lab-2.pl

LAB3_SRC = test-lab-3.pl start.sh stop.sh

LAB4_SRC = test-lab-4-a.pl test-lab-4-b.c test-lab-4-c.c Makefile start.sh stop.sh

LAB5_SRC = lock_client_cache.h lock_client_cache.cc Makefile lock_tester.cc lock_server_cache.cc lock_server_cache.h

LAB7_SRC = Makefile rsm.cc rsm.h rsm_protocol.h rsm_client.cc rsm_client.h view.h view.cc rsm_tester.pl lock_demo.cc

LAB8_SRC = Makefile view.h view.cc rsm_tester.pl paxos_protocol.h paxos.h paxos.cc log.h log.cc rsm_state_transfer.h

l1:
	./mklab.pl 1 0 l1 $(LAB1_SRC)
l2:
	./mklab.pl 2 0 l2 $(LAB2_SRC)
l3:
	./mklab.pl 3 0 l3 $(LAB3_SRC)
l4:
	./mklab.pl 4 0 l4 $(LAB4_SRC)
l5:
	./mklab.pl 5 0 l5 $(LAB5_SRC)
l7:
	./mklab.pl 7 0 l7 $(LAB7_SRC)
l8:
	./mklab.pl 8 0 l8 $(LAB8_SRC)

clean :
	rm -f *.o *.d fuse2yfs extent_server yfs_client lock_server lock_tester lock_demo

-include *.d
