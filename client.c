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
        int rc = MFS_Lookup(atoi(argv[2]), "testdir");
        printf("return value: %d\n", rc);
    }

    if(strcmp(request, "3") == 0){
        printf("request stat\n");
        MFS_Stat_t* m = malloc(sizeof(MFS_Stat_t));
        int rc = MFS_Stat(atoi(argv[2]), m);
        printf("return value: %d\n", rc);
        printf("size: %d \n", m->size);
        printf("type: %d \n", m->type);
    }

    if(strcmp(request,"5") ==0){
        printf("request read\n");
        //MFS_Stat_t* m = malloc(sizeof(MFS_Stat_t));
        char* buffer = (char*)malloc(1);
        printf("buffer before : %#X \n", buffer);
        int rc = MFS_Read(atoi(argv[2]), buffer,0,1);
        printf("return value: %d\n", rc);
        printf("buffer content: %#X \n", buffer);
        //printf("type: %d \n", m->type);
    }

    if(strcmp(request, "9") == 0){

        printf("request unlink\n");
        //MFS_Stat_t* m = malloc(sizeof(MFS_Stat_t));
        int rc = MFS_Unlink(atoi(argv[2]),"testdir");
        printf("return value: %d\n", rc);
        // printf("size: %d \n", m->size);
        // printf("type: %d \n", m->type);

    }

    if(strcmp(request, "6") == 0){
        printf("request create\n");
        MFS_Creat(0, 0, "makedir");
        int rc = MFS_Lookup(0, "makedir");
        printf("return value: %d\n", rc);
        rc = MFS_Lookup(1, ".");
        printf("return value: %d\n", rc);
    }

    if(strcmp(request, "8") == 0){
        printf("request8\n");
        MFS_Shutdown();
    }
    if(strcmp(request, "0") == 0){
        printf("test empty\n");
        MFS_Creat(0, 0, "testdir");
        int rc = MFS_Lookup(0, "testdir");
        MFS_Creat(1, 1, "testfile");
        rc = MFS_Lookup(1, "testfile");
        printf("return value: %d\n", rc);
        rc = MFS_Unlink(0, "testdir");
        printf("return value: %d\n", rc);
        rc = MFS_Unlink(0, "testfile");
        printf("return value: %d\n", rc);
        rc = MFS_Unlink(0, "testdir");
        printf("return value: %d\n", rc);

    }

    
    return 0;
}

