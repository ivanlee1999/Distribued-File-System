#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "mfs.h"
#include "udp.h"
#include "structure.h"

#include <time.h>
#include <stdlib.h>

int MIN_PORT = 20000;
int MAX_PORT = 40000;


// Bind random client port number
// int fd = UDP_Open(port_num);


#define BUFFER_SIZE (1000)

char* serverAddress;
int serverPort;
int sd;
int rc;

struct sockaddr_in addrSnd, addrRcv;


int sendRequest(Msg request, Msg* response, char* address, int port){
    
    
    rc = UDP_Write(sd, &addrSnd,(char*) &request, BUFFER_SIZE);

    if (rc < 0) {
        printf("client:: failed to send\n");
        exit(1);
    }
    // response->stat = malloc(sizeof(MFS_Stat_t));

    printf("waiting for response\n");
    rc = UDP_Read(sd, &addrRcv,(char*) response, BUFFER_SIZE);
    // printf("state size %d\n", response->stat.size);
    printf("client:: got reply [size:%d contents:(%s)\n", rc, (char*)response);
    return response->inum;
}



int MFS_Init(char *hostname, int port){
    
    srand(time(0));
    int port_num = (rand() % (MAX_PORT - MIN_PORT) + MIN_PORT);

    
    printf("open portnumbrt %d\n", port_num);
    sd = UDP_Open(port_num);
    rc = UDP_FillSockAddr(&addrSnd, hostname, port);
    if(rc < 0);
    
    // serverAddress = hostname;
    // serverPort = port;
    return 0;
}


int MFS_Lookup(int pinum, char* name){
    Msg request;
    Msg response;
    response.type = 9;
    response.inum = -5;
    request.requestType =2;
    request.inum = pinum;
    strncpy(request.name,name,28);
    //request.name = name;
    rc = sendRequest(request, &response,  serverAddress, serverPort);
    return rc;
}

int MFS_Stat(int inum, MFS_Stat_t *m){
    Msg request,response;
    response.type = 9;
    response.inum = -2;
    request.requestType = 3;
    request.inum = inum;
    // request.stat = m;
    rc = sendRequest(request, &response,  serverAddress, serverPort);
    m->size = response.stat.size;
    m->type = response.stat.type;
    return rc;
}

int MFS_Write(int inum, char *buffer, int offset, int nbytes){
    Msg request,response;
    response.type = 9;
    response.inum = -2;
    request.requestType= 4;
    //request.buffer = buffer;
    strncpy(request.buffer,buffer,sizeof(request.buffer));
    request.inum = inum;
    request.offset = offset;
    request.nbytes = nbytes;
    rc = sendRequest(request, &response,  serverAddress, serverPort);
    return rc;
}        //4

int MFS_Read(int inum, char *buffer, int offset, int nbytes){
    Msg request,response;
    response.type = 9;
    response.inum = -2;
    request.requestType= 5;
    //request.buffer = buffer;
    strncpy(request.buffer,buffer,sizeof(request.buffer));
    request.inum = inum;
    request.offset = offset;
    request.nbytes = nbytes;
    rc = sendRequest(request, &response,  serverAddress, serverPort);
    return rc;
}         //5

int MFS_Creat(int pinum, int type, char *name){
    Msg request,response;
    response.type = 9;
    response.inum = -2;
    request.requestType = 6;
    request.inum = pinum;
    request.type = type;
    //request.name =name;
    strncpy(request.name,name,28);
    rc = sendRequest(request, &response,  serverAddress, serverPort); 
    return rc;
}                       //6

int MFS_Unlink(int pinum, char *name){
    Msg request,response;
    response.type = 9;
    response.inum = -2;
    request.inum = pinum;
    request.requestType = 7;
    rc = sendRequest(request, &response,  serverAddress, serverPort);
    return rc;
}                                //7

int MFS_Shutdown(){
    Msg request;
    Msg response;
    response.type = 9;
    response.inum = -2;

    request.requestType = 8;
    rc = sendRequest(request, &response,  serverAddress, serverPort);
    return rc;
    
}
