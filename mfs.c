#include "mfs.h"
#include "udp.h"
#include "structure.h"

#define BUFFER_SIZE (1000)

char* serverAddress;
int serverPort;




int sendRequest(Msg request, Msg response, char* address, int port){
    struct sockaddr_in addrSnd, addrRcv;
    int sd = UDP_Open(20000);
    int rc = UDP_FillSockAddr(&addrSnd, address, port);
    
    
    rc = UDP_Write(sd, &addrSnd, &request, BUFFER_SIZE);

    if (rc < 0) {
	printf("client:: failed to send\n");
	exit(1);
    }


    rc = UDP_Read(sd, &addrRcv, &response, BUFFER_SIZE);
    printf("client:: got reply [size:%d contents:(%s)\n", rc, &response);
    return 0;
}



int MFS_Init(char *hostname, int port){
    serverAddress = hostname;
    serverPort = port;
    return 0;
}





int MFS_Shutdown(){
    Msg request;
    Msg response;

    request.requestType = 8;
    int rc = sendRequest(request, response,  serverAddress, serverPort);
    return rc;
    
}
