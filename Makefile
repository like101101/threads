CPUTYPE=x86-64
O=3

.PHONY: clean
CFLAGS := -g -march=$(CPUTYPE) -O$(O)

%.o: %.c
	gcc ${CFLAGS}  -c $< -o $@

%.o: %.s
	gcc ${CFLAGS}  -c $< -o $@ 

%.s: %.c
	gcc ${CFLAGS}  -S  $<

all: bm.NOPLOOP_LOCAL_WORK

bm.NOPLOOP_LOCAL_WORK: main.NOPLOOP_LOCAL_WORK.o work.o
	gcc ${CFLAGS} -DWORK=Work_NOPLOOP -DLOCAL_WORK $^ -o $@

work.o: work.c work.h now.h remote.h
	gcc ${CFLAGS} -c $<

main.NOPLOOP_LOCAL_WORK.o: main.c work.h now.h remote.h
	gcc ${CFLAGS} -c -DWORK=Work_NOPLOOP -DLOCAL_WORK $< -o $@


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
	rm -rf $(wildcard *.o bm_* bm.* threads threads2 threadsSilent threads2Silent)

distclean: clean
	rm -rf $(wildcard *.times)
