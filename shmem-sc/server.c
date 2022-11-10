#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ipc.h"

#define WRITE_LOC "/dev/null"
#define WRITE_BUF "0000000000"
#define BUF_LEN 10

/*
Server Implementaion
*/

#define USAGE "%s <iterations>\n"


int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, USAGE, argv[0]);
        return(-1);
    }

    int iterations = atoi(argv[1]);

    void* shared_memory = ipc_connect_server();
    if (shared_memory == NULL) {
        fprintf(stderr, "fail to open shared memory");
        return(-1);
    }

    FILE* fp = fopen(WRITE_LOC, "w");
    int fd = fileno(fp);

    register int local_signal = 1;

    signal_t* shared_signal = (signal_t*) shared_memory;

    while (iterations){

        //spin for request
        while (local_signal != shared_signal->val);

        #ifdef CPUWORK

        //dowork
        write(fd, WRITE_BUF, BUF_LEN);
        #endif

        __sync_fetch_and_sub(&(shared_signal->val),1);
        iterations --;
    }

    //clean up
    close(fd);
    ipc_close();
    return 0;

}
