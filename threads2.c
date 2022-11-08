#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "now.h"

// getconf LEVEL1_DCACHE_LINESIZE
#define CACHE_LINE_SIZE 64
#define Q_LEN 1

const uint64_t DEFAULT_COUNT = 100000;
const int MSG_NUM_START = 0;
const int CLIENT_REQ_BASE = 0;
const int SERVER_REPLY_BASE = 100;

// Place all shared data on their own cache lines for
// good measure

// Each message is one cache line
union Message {
  char padding[CACHE_LINE_SIZE];
  struct Request {
    volatile int arg;
  } req;
  struct Reply {
    volatile int arg;
  } reply;
};

// Qinfo is one cache line that is an message index
union QInfo{
  int idx;
  char raw [CACHE_LINE_SIZE];
};

// Queue of messages index indicates next free message
// Two queues :
// Request Q: written by client read by server
// Reply Q: read by client written by server
struct MessageQ {
  union QInfo info;
  union Message messages[Q_LEN];
}  __attribute__ ((aligned (CACHE_LINE_SIZE))) reqQ =
  {
   .info.idx = MSG_NUM_START
  },
  __attribute__ ((aligned (CACHE_LINE_SIZE))) replyQ =
  {
   .info.idx = MSG_NUM_START				
  };

void printResult(uint64_t count, uint64_t start, uint64_t end)
{
  fprintf(stderr, "%" PRIu64 " %" PRIu64 "\n", count, end - start);
}

void * client (void *arg)
{
  uint64_t count = (uint64_t) arg;
  union QInfo __attribute__ ((aligned (CACHE_LINE_SIZE))) reqQinfo =
    {
     .idx = MSG_NUM_START
    };
  union QInfo __attribute__ ((aligned (CACHE_LINE_SIZE))) replyQinfo =
    {
     .idx = MSG_NUM_START
    };
  int myVal = CLIENT_REQ_BASE;

  uint64_t start = now();
  while (count) {
    // send request (copy volatile to nonvolatile local)
    //  we are single write so we know they will stay insync
    int curIdx = reqQ.info.idx;
    // add message data to queue
    reqQ.messages[curIdx%Q_LEN].req.arg = myVal;
    // publish message
    __sync_fetch_and_add(&(reqQ.info.idx),1);

    // spin for reply
    while (replyQ.info.idx == replyQinfo.idx) {}
    // process reply
#ifndef SILENT
    (void) !write(1, "Client has the ball\n",20);
#endif
    // we assume here only one reply added at a time!
    //   eg. no more writes to shared idx so we can make a nonvolatile copy
    replyQinfo.idx = replyQ.info.idx;
    assert(replyQ.messages[replyQinfo.idx%Q_LEN].reply.arg == (SERVER_REPLY_BASE + myVal));
    myVal++;
    count--;
  }
  uint64_t end = now();
  printResult(count, start, end);
  return NULL;
}
  
void * server (void *arg)
{
  union QInfo __attribute__ ((aligned (CACHE_LINE_SIZE))) reqQinfo =
    {
     .idx = MSG_NUM_START
    };
  
  int myVal = SERVER_REPLY_BASE;
  while (1) {
    // spin for request
    while (reqQ.info.idx == reqQinfo.idx) {}
    // we know things have changed
    // assume single request enqueued so we can safely make a nonvolatile copy
    reqQinfo.idx = reqQ.info.idx;
#ifndef SILENT
    (void) !write(1, "Server has the request\n",23);
#endif
    // process one message at a time 
    // doServerwork();
    assert(reqQ.messages[reqQinfo.idx%Q_LEN].req.arg == CLIENT_REQ_BASE + (myVal - SERVER_REPLY_BASE));

    // we are single writer so we can make nonvolatile copy
    int curIdx = replyQ.info.idx;
    // add message data to queue
    replyQ.messages[curIdx%Q_LEN].req.arg = myVal;
    // publish message
    __sync_fetch_and_add(&(replyQ.info.idx),1);

    myVal++;
  }
  return NULL;
}

#define USAGE "%s <count> <testcase>\n"

int main(int argc, char **argv)
{
#if 1
  register uint64_t count=DEFAULT_COUNT;
#else
  uint64_t count=DEFAULT_COUNT;
#endif
  int c;
  int rc = 0;
  
  pthread_t serverTid, clientTid;

  if (argc != 3) {
    fprintf(stderr, USAGE, argv[0]);
    return(-1);
  }
  
  count = atoll(argv[1]);
  c = atoi(argv[2]);
  
#ifdef VERBOSE
  printf("count:%" PRIu64 " &reqQ:%p sizeof(reqQ)=%ld &replyQ:%p sizeof(replyQ)=%ld\n",
	 count, 
	 &reqQ, sizeof(reqQ), &replyQ, sizeof(replyQ));
#endif

  switch (c) {
  case 1:
    {
      uint64_t start, end,  x = count;
      start = now();
      while (count) {
	count --;
      }
      end = now();
      printResult(x, start, end);
    }
    break;
  case 2:
    {
      uint64_t start, end, x = count;
      start = now();
      while (count) {
	asm volatile ("nop");
	count --;
      }
      end = now();
      printResult(x, start, end);
    }
    break;
  case 3:
    {
      uint64_t start, end, x = count;
      start = now();
      count = count >> 2;
      while (count) {
	asm volatile ("nop;nop;nop;nop");
	count--;
      }
      end = now();
      printResult(x, start, end);
    }
    break;
  case 4:
    {
      pthread_create(&serverTid, NULL, server, NULL);      
      client((void *)count);
    }
    break;
  default:
    fprintf(stderr, "%s: unknown test case %d (1-4)\n", argv[0], c);
    rc = -1;
  }
  return rc;
}
