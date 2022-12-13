#include <stdio.h>
#include "udp.h"
#include "signal.h"
#include "structure.h"
#include "ufs.h"
#define BUFFER_SIZE (1000)
#define BLOCK_SIZE (4096)

int sd;
int fd;
static super_t* super;
int nInodes;
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

int FS_Lookup(int pinum, char *name){

}

int FS_Stat(int inum, MFS_Stat_t *m){

}

int FS_Write(int inum, char *buffer, int offset, int nbytes){

}

int FS_Read(int inum, char *buffer, int offset, int nbytes){



}

int FS_Creat(int pinum, int type, char *name){

}

int FS_Unlink(int pinum, char *name){

}

int shutdown(){
    fsync(fd);
    exit(0);
}

int init(){
    //char* supBuffer = malloc(BUFFER_SIZE);  
    
}


int startServer(int port, char* img){
    sd = UDP_Open(port);
    assert(sd > -1);


    //mmap image to systen
    if ((fd = open (img, O_RDWR|O_CREAT, S_IRWXU)) < 0)
   {  
        fprintf(stderr,"An error has occurred\n");
        return -1;
   }



    //Read super block and initialize inode map, data map, etc
    super = malloc(sizeof(super_t));
    read(fd,super,sizeof(super_t));
    nInodes = (((super -> inode_bitmap_len)*BLOCK_SIZE)/sizeof(inode_t));

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
                FS_Lookup(request->inum, request->name);
                break;
            case 3:
                FS_Stat(request -> inum, request -> stat);
                break;
            case 4:
                FS_Write(request -> inum, request -> buffer, request->offset, request -> nbytes);
                break;
            case 5:
                FS_Read(request -> inum, request -> buffer, request->offset, request -> nbytes);
                break;
            case 6:
                FS_Creat( request -> inum, request -> type, request -> name);
                break;
            case 7:
                FS_Unlink(request -> inum, request -> name);
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