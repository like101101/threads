CPUTYPE=x86-64
O=3
.PHONY: clean
CFLAGS := -g -march=$(CPUTYPE) -O$(O)

%.o: %.s
	gcc ${CFLAGS}  -c $< -o $@ 

%.s: %.c
	gcc ${CFLAGS} -masm=intel -S  $<

all:threadsSilent threads2Silent

threads.o: threads.s
threads2.o: threads2.s

threads: threads.o
	gcc ${LDFLAGS} threads.o -o $@ -lpthread

threads2: threads2.o
	gcc ${LDFLAGS} threads2.o -o $@ -lpthread



threadsSilent: CFLAGS+=-DSILENT
threadsSilent: threads.o 
	gcc ${LDFLAGS} threads.o -o $@ -lpthread

threads2Silent: CFLAGS=-DSILENT
threads2Silent: threads2.o 
	gcc ${LDFLAGS} threads2.o -o $@ -lpthread

clean:
	rm -rf $(wildcard *.o threads.s threads2.s threads threadsSilent threads2 threads2Silent)

distclean: clean
	rm -rf $(wildcard *.times)
