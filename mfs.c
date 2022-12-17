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

struct sockaddr_in addrSnd, addrRcv;


int sendRequest(Msg request, Msg response, char* address, int port){
    
    int rc;
    rc = UDP_Write(sd, &addrSnd,(char*) &request, BUFFER_SIZE);

    if (rc < 0) {
        printf("client:: failed to send\n");
        exit(1);
    }

    printf("waiting for response\n");
    rc = UDP_Read(sd, &addrRcv,(char*) &response, BUFFER_SIZE);
    printf("client:: got reply [size:%d contents:(%s)\n", rc, (char*)&response);
    return 0;
}



int MFS_Init(char *hostname, int port){
    
    srand(time(0));
    int port_num = (rand() % (MAX_PORT - MIN_PORT) + MIN_PORT);

    
    printf("open portnumbrt %d\n", port_num);
    sd = UDP_Open(port_num);
    int rc = UDP_FillSockAddr(&addrSnd, hostname, port);
    if(rc < 0);
    
    // serverAddress = hostname;
    // serverPort = port;
    return 0;
}


int MFS_Lookup(int pinum, char* name){
    Msg request,response;
    response.type = 9;
    request.requestType =2;
    request.inum = pinum;
    strncpy(request.name,name,28);
    //request.name = name;
    int rc = sendRequest(request, response,  serverAddress, serverPort);
    return rc;
}

int MFS_Stat(int inum, MFS_Stat_t *m){
    Msg request,response;
    response.type = 9;
    request.requestType = 3;
    request.inum = inum;
    request.stat = m;
    int rc = sendRequest(request, response,  serverAddress, serverPort);
    return rc;
}

int MFS_Write(int inum, char *buffer, int offset, int nbytes){
    Msg request,response;
    response.type = 9;
    request.requestType= 4;
    //request.buffer = buffer;
    strncpy(request.buffer,buffer,sizeof(request.buffer));
    request.inum = inum;
    request.offset = offset;
    request.nbytes = nbytes;
    int rc = sendRequest(request, response,  serverAddress, serverPort);
    return rc;
}        //4

int MFS_Read(int inum, char *buffer, int offset, int nbytes){
    Msg request,response;
    response.type = 9;
    request.requestType= 5;
    //request.buffer = buffer;
    strncpy(request.buffer,buffer,sizeof(request.buffer));
    request.inum = inum;
    request.offset = offset;
    request.nbytes = nbytes;
    int rc = sendRequest(request, response,  serverAddress, serverPort);
    return rc;
}         //5

int MFS_Creat(int pinum, int type, char *name){
    Msg request,response;
    response.type = 9;
    request.requestType = 6;
    request.inum = pinum;
    request.type = type;
    //request.name =name;
    strncpy(request.name,name,28);
    int rc = sendRequest(request, response,  serverAddress, serverPort); 
    return rc;
}                       //6

int MFS_Unlink(int pinum, char *name){
    Msg request,response;
    response.type = 9;
    request.inum = pinum;
    request.requestType = 7;
    int rc = sendRequest(request, response,  serverAddress, serverPort);
    return rc;
}                                //7

int MFS_Shutdown(){
    Msg request;
    Msg response;
    response.type = 9;

    request.requestType = 8;
    int rc = sendRequest(request, response,  serverAddress, serverPort);
    return rc;
    
}
