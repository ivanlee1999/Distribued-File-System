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

    // int sd = UDP_Open(20000);
    // int rc = UDP_FillSockAddr(&addrSnd, "localhost", 2343);
    char* request = argv[1];
    MFS_Init("localhost", 2343);
    
    if(strcmp(request, "1") == 0){
        printf("request1\n");
        MFS_Init("localhost", 2343);
    }
    if(strcmp(request, "2") == 0){
        printf("request lookup\n");
        int rc = MFS_Lookup(10, "test");
        printf("return value: %d\n", rc);
    }
    if(strcmp(request, "3") == 0){
        printf("request stat\n");
        MFS_Stat_t* m = malloc(sizeof(MFS_Stat_t));
        int rc = MFS_Stat(20, m);
        printf("return value: %d\n", rc);
        printf("size: %d \n", m->size);
        printf("type: %d \n", m->type);
    }
    if(strcmp(request, "6") == 0){
        printf("request create\n");
        MFS_Creat(0, 1, "test");
        int rc = MFS_Lookup(0, "test");
        printf("return value: %d\n", rc);
    }
    if(strcmp(request, "8") == 0){
        printf("request8\n");
        MFS_Shutdown();
    }
    
    return 0;
}

