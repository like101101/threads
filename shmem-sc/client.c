#include "ipc.h"
#include <time.h>

#define WRITE_LOC "/dev/null"
#define WRITE_BUF "0000000000"
#define BUF_LEN 10

/*
Client Implementaion
*/

#define USAGE "%s <iterations>\n"


int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, USAGE, argv[0]);
        return(-1);
    }

    int iterations = atoi(argv[1]);
    clock_t start, end;
    double cpu_time_used;

    FILE* fp = fopen(WRITE_LOC, "w");
    int fd = fileno(fp);

    #ifdef IPC
    void* shared_memory = ipc_connect_client();
    if (shared_memory == NULL) {
        fprintf(stderr, "fail to open shared memory");
        return(-1);
    }

    register int local_signal = 0;

    signal_t* shared_signal = (signal_t*) shared_memory;
    #endif

    start = clock();
    while (iterations){

        #ifdef IPC
        // send the request
        __sync_fetch_and_add(&(shared_signal->val),1);
        #endif

        //do some work
        #ifdef CPUWORK
        write(fd, WRITE_BUF, BUF_LEN);
        #endif

        #ifdef IPC
        //spin for back
        while (local_signal != shared_signal->val);
        #endif

        iterations --;
    }
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  	printf("%f", cpu_time_used);
    close(fd);
    
    // Cleanup
    #ifdef IPC
	ipc_close();
    #endif
    return 0;
}
