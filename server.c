#include <stdio.h>
#include "udp.h"
#include "signal.h"

#define BUFFER_SIZE (1000)

int sd;

void intHandler(int dummy) {
    UDP_Close(sd);
    exit(130);
}
 
// server code
int main(int argc, char *argv[]) {
    signal(SIGINT, intHandler);
    sd = UDP_Open(10000);
    assert(sd > -1);
    while (1) {
	struct sockaddr_in addr;
	char message[BUFFER_SIZE];
	printf("server:: waiting...\n");
	int rc = UDP_Read(sd, &addr, message, BUFFER_SIZE);
	printf("server:: read message [size:%d contents:(%s)]\n", rc, message);
	if (rc > 0) {
            char reply[BUFFER_SIZE];
            sprintf(reply, "goodbye CS537");
            rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
	    printf("server:: reply\n");
	} 
    }
    return 0; 
}
    


