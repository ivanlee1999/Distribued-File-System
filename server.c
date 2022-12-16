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
struct sockaddr_in *addr;

super_t superBlock;
unsigned int iMapStartPosition;
unsigned int dMapStartPosition;
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
   bitmap[index] |= 0x1 << offset;
}




int lookup(int pinum, char *name){
    Msg reply;
    reply.inum = -1;
    reply.requestType = 9; // type is reply
    // pinode in the bitmap(in block number)
    int pinumBlock = superBlock.inode_region_addr + (pinum * sizeof(inode_t)) / UFS_BLOCK_SIZE; 
    // pinode array position(in address)
    int pinumPosition = inodeStartPosition + pinum * sizeof(inode_t);

    //check inode bitmap
    unsigned int iMapStartPos = (unsigned int)iMapStartPosition;
    if(get_bit(&iMapStartPos, pinumBlock) == 0){
        reply.inum = -1;
    }
    else{
        //check inode type
        inode_t pinode;
        //move read pointer to the position, SEEK_SET  means starting from the beginning
        lseek(fd, pinumPosition, SEEK_SET);
        read(fd, &pinode, sizeof(inode_t));
        if(pinode.type == UFS_REGULAR_FILE){
            reply.inum = -1;
        }
        else{
            //go through all 30 dir entries
            for(int i = 0; i< DIRECT_PTRS; i++){
                //not sure if it is block number or address
                unsigned int dirEntryBlock = pinode.direct[i];
                // read data block, get block number, the index should start from superblocks
                int dirEntryBlockAddr =  dirEntryBlock * UFS_BLOCK_SIZE;
                // int dirEntryBlockAddr = dataStartPosition + dirEntryBlock * UFS_BLOCK_SIZE;
                lseek(fd, dirEntryBlockAddr, SEEK_SET);
                char* entryBlock = malloc(UFS_BLOCK_SIZE);
                read(fd, entryBlock, UFS_BLOCK_SIZE);
                dir_ent_t* dirEntry;
                for(int j = 0; j < UFS_BLOCK_SIZE / sizeof(dir_ent_t); j++){
                    dirEntry = (dir_ent_t*)(entryBlock + j * sizeof(dir_ent_t));
                    if(dirEntry->inum != -1){
                        if(strcmp(dirEntry->name, name) == 0){
                            reply.inum =  dirEntry->inum;
                            return dirEntry->inum;
                        }
                    }
                }
            }
        }
    }

    UDP_Write(sd, addr, (char*)&reply, sizeof(Msg));
    return -1;
    
}

int FS_Stat(int inum, MFS_Stat_t *m){
    Msg reply;
    reply.inum = -1;
    reply.requestType = 9; // type is reply
    // pinode in the bitmap(in block number)
    int inumBlock = superBlock.inode_region_addr + (inum * sizeof(inode_t)) / UFS_BLOCK_SIZE; 
    // pinode array position(in address)
    int inumPosition = inodeStartPosition + inum * sizeof(inode_t);

    //check inode bitmap
    if(get_bit(&iMapStartPosition, inumBlock) == 0){
        reply.inum = -1;
    } 
    else{
        //check inode type
        inode_t inode;
        //move read pointer to the position, SEEK_SET  means starting from the beginning
        lseek(fd, inumPosition, SEEK_SET);
        read(fd, &inode, sizeof(inode_t));
        reply.inum = 0;
        reply.stat->size = inode.size;
        reply.stat->type = inode.type;
    }
    int rc = UDP_Write(sd, addr, (char*)&reply, sizeof(Msg));
    return rc;
}

int FS_Write(int inum, char *buffer, int offset, int nbytes){
return -1;
}

//TODO: Do UDP write after writing to buffer and send reply

int FS_Read(int inum, char *buffer, int offset, int nbytes)
{

Msg reply;
int inumBlock = superBlock.inode_bitmap_addr + (inum * sizeof(inode_t))/UFS_BLOCK_SIZE;
int inumPos = (sizeof(inode_t) * inum) +inodeStartPosition;
if(get_bit(&iMapStartPosition,inumBlock) != 1) {
reply.requestType = -1;
UDP_Write(sd, addr, (char*)&reply, sizeof(Msg));
return -1;
}
inode_t inode;
// Msg reply;
lseek(fd,inumPos,SEEK_SET);
read(fd,&inode, sizeof(inode_t));
// if(inode.type == UFS_DIRECTORY){
    //READ DIR CONTENTS
    
    if( (inode.type = MFS_DIRECTORY) && (nbytes % sizeof(dir_ent_t) != 0 || offset % sizeof(dir_ent_t) != 0)){
        reply.requestType = -1;
        UDP_Write(sd, addr, (char*)&reply, sizeof(Msg));
        return -1;
    }

    int dataBlock = offset/UFS_BLOCK_SIZE;
    //int offsetRemaining = 0;
    int nbytes1 =0;
    if(offset%UFS_BLOCK_SIZE >= nbytes){
    int nbytes1 = (inode.direct[dataBlock+1] - offset); //no of bytes to read in first block
    nbytes = nbytes - nbytes1;
    }
    if(inode.direct[dataBlock] == -1 || offset>inode.size){
        reply.requestType = -1;
        UDP_Write(sd, addr, (char*)&reply, sizeof(Msg));
        return -1;
    }
    if(nbytes1 ){
    //figure out how to read into buffer at buffer+remaining
    lseek(fd,dataStartPosition+inode.direct[dataBlock],SEEK_SET);
    read(fd,&buffer,nbytes1);
    lseek(fd,dataStartPosition+inode.direct[dataBlock+1],SEEK_SET);
    read(fd,&buffer+nbytes1,nbytes);
    strncpy(reply.buffer,buffer,nbytes+nbytes+1);
    return 1;
    }
    else{
        lseek(fd,dataStartPosition+inode.direct[dataBlock],SEEK_SET);
        read(fd,&buffer,nbytes);
        strncpy(reply.buffer,buffer,nbytes+nbytes+1);
        return 1;
    }

    
//}

// else{
//     //READ FILE CONTENTS INTO buffer
//     int dataBlock = offset/UFS_BLOCK_SIZE;
//     //int offsetRemaining = 0;
//     int nbytes1 =0;
//     if(offset%UFS_BLOCK_SIZE != 0){
//     int nbytes1 = (inode.direct[dataBlock+1] - offset); //no of bytes to read in first block
//     nbytes = nbytes - nbytes1;
//     }
//     if(inode.direct[dataBlock] == -1 || offset>inode.size){
//         reply.requestType = -1;
//         int rc = UDP_Write(sd, addr, (char*)&reply, sizeof(Msg));
//         return -1;
//     }
//     if(nbytes1 ){
//     //figure out how to read into buffer at buffer+remaining
//     lseek(fd,dataStartPosition+inode.direct[dataBlock],SEEK_SET);
//     read(fd,&buffer,nbytes1);
//     lseek(fd,dataStartPosition+inode.direct[dataBlock+1],SEEK_SET);
//     buffer = buffer+nbytes1;
//     read(fd,&buffer+nbytes1,nbytes);
//     strncpy(reply.buffer,buffer,nbytes+nbytes+1);
//     return 1;
//     }
//     else{
//         lseek(fd,dataStartPosition+inode.direct[dataBlock],SEEK_SET);
//         read(fd,&buffer,nbytes);
//         strncpy(reply.buffer,buffer,nbytes+nbytes+1);
//         return 1;
//     }

// }

//int rc = UDP_Write(sd, addr, (char*)&reply, sizeof(Msg));

}

int FS_Creat(int pinum, int type, char *name){
return -1;
}

int FS_Unlink(int pinum, char *name)
{
    Msg reply;
    reply.requestType = -1;
    unsigned int bitmapAddr = (unsigned int)superBlock.inode_bitmap_addr;
if(get_bit(&bitmapAddr,pinum) != 1){
    int rc = UDP_Write(sd, addr, (char*)&reply, sizeof(Msg));
    fsync(fd);
    return rc;
}
int inodePos = inodeStartPosition+ (pinum*sizeof(inode_t));
inode_t inode;
lseek(fd,inodePos,SEEK_SET);
read(fd,&inode,sizeof(inode_t));
if(inode.type == MFS_REGULAR_FILE){
    reply.requestType = 1;
    unsigned int inodeBitmapAddr = (unsigned int)superBlock.inode_bitmap_addr;
    set_bit(&inodeBitmapAddr,pinum);
    fsync(fd);
    UDP_Write(sd, addr, (char*)&reply, sizeof(Msg));
    return 1;
    
}
else{
    int inum = lookup(pinum,name);
    if(inum == -1){
    UDP_Write(sd, addr, (char*)&reply, sizeof(Msg));
    fsync(fd);
    return -1;
    }
    int dirPos = inodeStartPosition+ (inum*sizeof(inode_t));
    inode_t dirInode;
    lseek(fd,dirPos,SEEK_SET);
    read(fd,&dirInode,sizeof(inode_t));
    if(dirInode.direct[0] != -1){
    UDP_Write(sd, addr, (char*)&reply, sizeof(Msg));
    fsync(fd);
    return -1;
    }
    reply.requestType = 1;
    unsigned int inodeBitmapAddr = (unsigned int)superBlock.inode_bitmap_addr;
    set_bit(&inodeBitmapAddr,inum);
    UDP_Write(sd, addr, (char*)&reply, sizeof(Msg));
    fsync(fd);
    return 1;

}
}
    

int FS_Shutdown(){
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
    //char *src, *dst;
    //struct stat statbuf;

    if(fsInit(img) < 0) return -1;

    while (1) {
        Msg* request = malloc(sizeof(Msg));
        printf("server:: waiting...\n");
        int rc = UDP_Read(sd, addr, (char*)request, sizeof(Msg));
        //int responseRet;
        printf("server:: read message [size:%d contents:(%s)]\n", rc, (char*)request);
        if (rc > 0) {
            switch (request->requestType)
            {
            case 1:
                break;
            case 2:
                lookup(request->inum, request->name);
                break;
            case 3:
                FS_Stat(request->inum,request->stat);
                break;
            case 4:
                FS_Write(request->inum,request->buffer,request->offset,request -> nbytes);
                break;
            case 5:
             FS_Read(request->inum,request->buffer,request->offset,request -> nbytes);
                break;
            case 6:
                FS_Creat(request ->inum,request->type,request->name);
                break;
            case 7:
                FS_Unlink(request -> inum, request -> name);
                break;
            case 8:
                FS_Shutdown();
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