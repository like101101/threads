#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sched.h>
#include <err.h>

#include "work.h"
#include "now.h"

#define USAGE "%s <iterations>\n"
#define VERBOSE

int pinCpu(int cpu)
{
  cpu_set_t  mask;
  CPU_ZERO(&mask);

  if (cpu == -1 ) {
    cpu = sched_getcpu();
    if (cpu == -1) {
      err(1, "could not determine current cpu");
    }
  }

  CPU_SET(cpu, &mask);
  if (sched_setaffinity(0, sizeof(mask), &mask) != 0) {
    err(1, "could not pin to cpu=%d",cpu);
  }
  
#ifdef VERBOSE
    fprintf(stderr, "PINNED TO CPU: %d\n", cpu);
#endif
    
}

int
main(int argc, char *argv[])
{

  register int iters;
  int cpu0, cpu1;
  uint64_t startTSC, endTSC, totalTSC;
  
  if (argc != 4) {
    fprintf(stderr, USAGE, argv[0]);
    return(-1);
  }
  
  iters = atoi(argv[1]);
  cpu0 = atoi(argv[2]);
  cpu1 = atoi(argv[3]);

  pinCpu(cpu0);
  
  startTSC = now();
  while (iters) {
#ifdef LOCAL_WORK
    doWork();
#elif LOCAL_WORK_WITH_REMOTE
#elif REMOTE_WORK
#endif
    iters--;
  }
  endTSC = now();

  totalTSC = endTSC - startTSC;
  fprintf(stderr, "%" PRIu64 " %" PRIu64 "\n", totalTSC, WorkTSC);
  return 0;
}  
