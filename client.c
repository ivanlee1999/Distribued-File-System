#include <stdio.h>
#include "udp.h"
#include "mfs.c"
#include <time.h>
#include <stdlib.h>
#define BUFFER_SIZE (1000)





// client code
int main(int argc, char *argv[]) {
    printf("compile finished\n");
    struct sockaddr_in addrSnd, addrRcv;

    int sd = UDP_Open(20000);
    int rc = UDP_FillSockAddr(&addrSnd, "localhost", 2343);
    char* request = argv[1];
    MFS_Init("localhost", 2343);
    
    if(strcmp(request, "1") == 0){
        printf("request1\n");
        MFS_Init("localhost", 2343);
    }
    if(strcmp(request, "2") == 0){
        printf("request2\n");
        MFS_Lookup(2, "test");
    }
    if(strcmp(request, "3") == 0){
        printf("request3\n");
        MFS_Stat_t* m = malloc(sizeof(MFS_Stat_t));
        MFS_Stat(3, m);
        printf("size: %d \n", m->size);
        printf("type: %d \n", m->type);
    }
    if(strcmp(request, "8") == 0){
        printf("request8\n");
        MFS_Shutdown();
    }
    
    return 0;
}

