#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

// getconf LEVEL1_DCACHE_LINESIZE
#define CACHE_LINE_SIZE 64

const int CLIENT = 0;
const int CLIENT_REQ_BASE = 0;
const int SERVER = 1;
const int SERVER_REPLY_BASE = 100;

// Place all shared data on their own cache lines for
// good measure

// client write only
// server read only
union Message {
  char padding[CACHE_LINE_SIZE];
  struct Request {
    int arg;
  } req;
  // client read only
  // server write only
  struct Reply {
    int arg;
  } reply;
} __attribute__ ((aligned (CACHE_LINE_SIZE))) req =
  { .req.arg = CLIENT_REQ_BASE },
  __attribute__ ((aligned (CACHE_LINE_SIZE))) reply =
  { .reply.arg = SERVER_REPLY_BASE };

// read/write shared by both client and server
union SharedLine {
  char padding[CACHE_LINE_SIZE];
  volatile int theBall;
} __attribute__ ((aligned (CACHE_LINE_SIZE))) shared = { .theBall = CLIENT };

// use of compare and swap is a little overkill but I would
// start with this before doing further optimizations to
// confirm correctness and baseline performance

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

int main(int argc, char **argv)
{
  pthread_t serverTid, clientTid;

  printf("&req:%p sizeof(req)=%ld &reply:%p sizeof(reply)=%ld\n",
	 &req, sizeof(req), &reply, sizeof(reply));
  
  pthread_create(&serverTid, NULL, server, NULL);

  client(NULL);

  return 0;
}
