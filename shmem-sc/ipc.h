#ifndef __IPC_H__
#define __IPC_H__
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct signal {
  volatile int val;
} signal_t;

#define SHMEM_REGION_SIZE 512

void* ipc_connect_client();
void  ipc_close();
void* ipc_connect_server();

#endif