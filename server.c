#include <stdio.h>
#include "udp.h"
#include "signal.h"
#include "structure.h"

#define BUFFER_SIZE (1000)

int sd;
int fd;

void intHandler(int dummy) {
    UDP_Close(sd);
    exit(130);
}
 
// server code
// int main(int argc, char *argv[]) {
//     signal(SIGINT, intHandler);
//     sd = UDP_Open(10000);
//     assert(sd > -1);
//     while (1) {
// 	struct sockaddr_in addr;
// 	Msg* request = malloc(sizeof(Msg));
// 	printf("server:: waiting...\n");
// 	int rc = UDP_Read(sd, &addr, request, BUFFER_SIZE);
// 	printf("server:: read message [size:%d contents:(%s)]\n", rc, request);



// 	if (rc > 0) {
//             char reply[BUFFER_SIZE];
//             sprintf(reply, "goodbye CS537");
//             rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
// 	    printf("server:: reply\n");
// 	} 
//     }
//     return 0; 
// }


int lookup(int pinum, char *name){

}

    

int shutdown(){
    fsync(fd);
    exit(0);
}


int startServer(int port, char* img){
    sd = UDP_Open(port);
    assert(sd > -1);


    //mmap image to systen
    if ((fd = open (img, O_RDONLY)) < 0)
   {  
        fprintf(stderr,"An error has occurred\n");
        return -1;
   }
   









    while (1) {
        struct sockaddr_in addr;
        Msg* request = malloc(sizeof(Msg));
        printf("server:: waiting...\n");
        int rc = UDP_Read(sd, &addr, request, BUFFER_SIZE);
        printf("server:: read message [size:%d contents:(%s)]\n", rc, request);
        if (rc > 0) {
            switch (request->requestType)
            {
            case 1:
                break;
            case 2:
                lookup(request->inum, request->name);
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
                break;
            case 6:
                break;
            case 7:
                break;
            case 8:
                shutdown();
                break;
            default:
                break;
            }
        } 
    }
    return 0; 
}

int main(int argc, char *argv[]) {
    if (argc != 3)
		perror("Error : Incorrect arguments\n");

	startServer(atoi(argv[1]), argv[2]);
}