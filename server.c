#include <stdio.h>
#include "udp.h"
#include "signal.h"
#include "structure.h"


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE (1000)

int sd;
int fd;

super_t superBlock;
int iMapStartPosition;
int dMapStartPosition;
int inodeStartPosition;
int dataStartPosition;

void intHandler(int dummy) {
    UDP_Close(sd);
    exit(130);
}



//These two functions take the pointer to the beginning of the inode or data block bitmap region and an integer that is the inode or data block number.
unsigned int get_bit(unsigned int *bitmap, int position) {
   int index = position / 32;
   int offset = 31 - (position % 32);
   return (bitmap[index] >> offset) & 0x1;
}

void set_bit(unsigned int *bitmap, int position) {
   int index = position / 32;
   int offset = 31 - (position % 32);
   bitmap[index] |=  0x1 << offset;
}
 

int lookup(int pinum, char *name){
    int pinumBlock = superBlock.inode_region_addr + (pinum * INODE_SIZE) / UFS_BLOCK_SIZE; 
    int pinumPosition = inodeStartPosition + pinum * INODE_SIZE;

    //check inode bitmap
    if(get_bit((unsigned int *) iMapStartPosition, pinumBlock) == 0) return-1;

    //check inode type
    inode_t pinode;
    //move read pointer to the position, SEEK_SET  means starting from the beginning
    lseek(fd, pinumPosition, SEEK_SET);
    read(fd, &pinode, sizeof(inode_t));
    if(pinode.type == UFS_REGULAR_FILE) return -1;

    //go through all 30 dir entries
    for(int i = 0; i< DIRECT_PTRS; i++){
        //not sure if it is block number or address
        unsigned int dirEntryBlock = pinode.direct[i];
        // read data block
        int dirEntryBlockAddr = dataStartPosition + dirEntryBlock * UFS_BLOCK_SIZE;
        lseek(ld, dirEntryBlockAddr, SEEK_SET);
        char* entryBlock = malloc(UFS_BLOCK_SIZE);
        read(fd, &entryBlock, UFS_BLOCK_SIZE);
        dir_ent_t* dirEntry;
        for(int j = 0; j < UFS_BLOCK_SIZE / sizeof(dir_ent_t); j++){
            dirEntry = entryBlock + j * sizeof(dir_ent_t);
            if(dirEntry->inum != -1){
                if(strcmp(dirEntry->name, name) == 0){
                    return dirEntry->inum;
                }
            }
        }
    }
    return -1;
}

    

int shutdown(){
    fsync(fd);
    exit(0);
}






int fsInit(char* img){
    if ((fd = open (img, O_RDONLY)) < 0)
    {  
        fprintf(stderr,"An error has occurred\n");
        return -1;
    }

    read(fd, &superBlock, sizeof(super_t));
    iMapStartPosition = superBlock.inode_bitmap_addr * UFS_BLOCK_SIZE;     //should be equal to 4096
    dMapStartPosition = superBlock.data_bitmap_addr * UFS_BLOCK_SIZE;
    inodeStartPosition = superBlock.inode_region_addr * UFS_BLOCK_SIZE;
    dataStartPosition = superBlock.data_region_addr * UFS_BLOCK_SIZE;
    return 0;
}


int startServer(int port, char* img){
    sd = UDP_Open(port);
    assert(sd > -1);
    char *src, *dst;
    struct stat statbuf;

    if(fsInit(img) < 0) return -1;

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