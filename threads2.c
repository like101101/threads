#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

// getconf LEVEL1_DCACHE_LINESIZE
#define CACHE_LINE_SIZE 64
#define Q_LEN 1

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


void * client (void *arg)
{
  union QInfo __attribute__ ((aligned (CACHE_LINE_SIZE))) reqQinfo =
    {
     .idx = MSG_NUM_START
    };
  union QInfo __attribute__ ((aligned (CACHE_LINE_SIZE))) replyQinfo =
    {
     .idx = MSG_NUM_START
    };
  int myVal = CLIENT_REQ_BASE;
  
  while (1) {
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
  }
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

int main(int argc, char **argv)
{
  pthread_t serverTid, clientTid;

  printf("&reqQ:%p sizeof(reqQ)=%ld &replyQ:%p sizeof(replyQ)=%ld\n",
	 &reqQ, sizeof(reqQ), &replyQ, sizeof(replyQ));
  
  pthread_create(&serverTid, NULL, server, NULL);

  client(NULL);

  return 0;
}
