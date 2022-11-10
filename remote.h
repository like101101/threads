#ifndef __REMOTE_H__
#define __REMOTE_H__


// getconf LEVEL1_DCACHE_LINESIZE
#define CACHE_LINE_SIZE 64
#define Q_LEN 1

const int CLIENT = 0;
const int CLIENT_REQ_BASE = 0;
const int SERVER = 1;
const int SERVER_REPLY_BASE = 100;

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

#ifdef REMOTE_READ_SHARED

// Qinfo is one cache line that is an message index
union QInfo{
  volatile int idx;
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

static void * client (void *arg)
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
  
static void * server (void *arg)
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

#elif REMOTE_RW_SHARED

// read/write shared by both client and server
union SharedLine {
  char padding[CACHE_LINE_SIZE];
  volatile int theBall;
} __attribute__ ((aligned (CACHE_LINE_SIZE))) shared = { .theBall = CLIENT };

void * client (void *arg)
{
  int myVal = CLIENT_REQ_BASE;

  while (1) {
    while (shared.theBall != CLIENT); {}
#ifndef SILENT
    (void) !write(1, "Client has the ball\n",20);
#endif
    assert(reply.reply.arg == (SERVER_REPLY_BASE + myVal));
    req.req.arg = myVal;
    __sync_bool_compare_and_swap(&shared.theBall, CLIENT, SERVER);
    myVal++;
  }
  return NULL;
}

void * server (void *arg)
{
  int myVal = SERVER_REPLY_BASE;

  while (1) {
    while (shared.theBall != SERVER);
#ifndef SILENT
    (void) !write(1, "Server has the ball\n",20);
#endif
    assert(req.req.arg == CLIENT_REQ_BASE + (myVal - SERVER_REPLY_BASE));
    myVal++;
    reply.reply.arg = myVal;
    __sync_bool_compare_and_swap(&shared.theBall, SERVER, CLIENT);
  }
  return NULL;
}

#endif

#endif
