CPUTYPE=x86-64
O=3
.PHONY: clean
DFLAGS := -g -march=$(CPUTYPE) -O$(O) -lrt

all: ipc_client independent_client pingpong_client server pingpong_server

ipc_client: client.c ipc.c
	gcc ${LDFLAGS} $^ -o $@ -D IPC

independent_client: client.c ipc.c
	gcc ${LDFLAGS} $^ -o $@ -D CPUWORK

pingpong_client: client.c ipc.c
	gcc ${LDFLAGS} $^ -o $@ -D CPUWORK -D IPC

pingpong_server: server.c ipc.c
	gcc ${LDFLAGS} $^ -o $@

server: server.c ipc.c
	gcc ${LDFLAGS} $^ -o $@ -D CPUWORK

clean:
	rm -rf $(wildcard *.o *.s *.txt ipc_client independent_client pingpong_client server pingpong_server)
