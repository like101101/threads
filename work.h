#ifndef __WORK_H__
#define __WORK_H__

#include <stdint.h>
#include "now.h"

#define WORK_NOPLOOP_COUNT 40000

extern uint64_t WorkTSC;

inline  __attribute__((always_inline)) int 
Work_NOPLOOP()
{
  int i;
  for (i=0; i<WORK_NOPLOOP_COUNT;i++) {
    asm("nop");
  }
  return i;
}

inline  __attribute__((always_inline)) int
Work_4NOPLOOP()
{
  int i;
  for (i=0; i<(WORK_NOPLOOP_COUNT/4); i++) {
    asm("nop;nop;nop;nop");
  }
  return i;
}

inline  __attribute__((always_inline)) int 
Work_NULLLOOP()
{
  int i;
  for (i=0; i<(WORK_NOPLOOP_COUNT/4); i++) {
    asm("nop;nop;nop;nop");
  }
  return i;
}

inline  __attribute__((always_inline)) int doWork()
{
  int rc;
  uint64_t wstart, wend;
  wstart = now();
  rc = WORK();
  wend = now();
  WorkTSC += (wend - wstart);
}
  
#endif
