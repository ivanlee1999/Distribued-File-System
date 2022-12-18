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

#define BUFFER_SIZE (4096)

int sd;
int fd;
struct sockaddr_in addr;
int portNumber;
char* IMGFILENAME;


super_t superBlock;
super_t* sb;
int blocksAvail;
unsigned int iMapStartPosition;
unsigned int dMapStartPosition;
int inodeStartPosition;
int dataStartPosition;
int imgSize;
char* startAddress;
char* imapAddress;
char* dmapAddress;
char* inodeBlockAddress;
char* dataBlockAddress;

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

int throwErr(Msg* reply){
    printf("THROW ERROR \n\n");
    reply->requestType = -1;
    reply->inum = -1;
    fsync(fd);
    UDP_Write(sd, &addr, (char*)(void*)reply, sizeof(Msg));
    return -1;
}

int findFreeDataBlock() {
    for(int i = 0; i<sb->num_data; i++) {
        int bit = get_bit((unsigned int *)dmapAddress, i);
        if(bit == 0) {
            return i;
        }
    }
    return -1;
}


inode_t* getpinode(int pinum){

    // int pinumPosition = inodeStartPosition + pinum * sizeof(inode_t);

    //check inode bitmap
    //check inode type
    inode_t* pinode = malloc(sizeof(inode_t));
    //move read pointer to the position, SEEK_SET  means starting from the beginning
    // lseek(fd, pinumPosition, SEEK_SET);
    // read(fd, pinode, sizeof(inode_t));
    pinode = (inode_t*)(inodeBlockAddress +   pinum * sizeof(inode_t));
    return pinode;
}

// int checkimap(int pinum){
//     int pinumBlock = superBlock.inode_region_addr + (pinum * sizeof(inode_t)) / UFS_BLOCK_SIZE; 
//     // pinode array position(in address)
//     int pinumPosition = inodeStartPosition + pinum * sizeof(inode_t);

//     //check inode bitmap
//     unsigned int iMapStartPos = (unsigned int)iMapStartPosition;
//     return get_bit(&iMapStartPos, pinumBlock);
// }



int lookup(int pinum, char *name){
    Msg reply;
    reply.inum = -1;
    reply.requestType = 9; // type is reply

    inode_t* pinode = malloc(sizeof(inode_t));
    pinode = getpinode(pinum);
    if(pinode->type != MFS_DIRECTORY){
        printf("lookup: not directory\n");
        fsync(fd);
        UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
        return -1;
    }

    for(int i = 0; i < DIRECT_PTRS; i++){
        if(pinode->direct[i] != -1){
            // printf("test %d\n", i);
            for(int j = 0; j < 128; j++){
                dir_ent_t* entryAddr =  (dir_ent_t*) (startAddress +  (pinode->direct[i] * UFS_BLOCK_SIZE) + j * sizeof(dir_ent_t));
                //printf(" direntry : %p, entry inum : %d, name : %s\n", entryAddr, entryAddr->inum, entryAddr->name);
                // char* entryName = (entryAddr+j)->name;
                // printf("name : %s, name should find %s\n", entryName, name);
                if(strcmp((entryAddr)->name, name) == 0){
                    reply.inum = (entryAddr)->inum;
                    fsync(fd);
                    UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
                    return reply.inum;
                }
            }
        }
    }
    printf("lookup: didn't find name in pinode\n");
    fsync(fd);
    UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
        return -1;



    // // pinode in the bitmap(in block number)
    // int pinumBlock = superBlock.inode_region_addr + (pinum * sizeof(inode_t)) / UFS_BLOCK_SIZE; 
    // // pinode array position(in address)
    // int pinumPosition = inodeStartPosition + pinum * sizeof(inode_t);

    // //check inode bitmap
    // unsigned int iMapStartPos = (unsigned int)iMapStartPosition;
    // if(get_bit(&iMapStartPos, pinumBlock) == 0){
    //     reply.inum = -1;
    // }
    // else{
    //     //check inode type
    //     inode_t pinode;
    //     //move read pointer to the position, SEEK_SET  means starting from the beginning
    //     lseek(fd, pinumPosition, SEEK_SET);
    //     read(fd, &pinode, sizeof(inode_t));
    //     if(pinode.type == UFS_REGULAR_FILE){
    //         reply.inum = -1;
    //     }
    //     else{
    //         //go through all 30 dir entries
    //         for(int i = 0; i< DIRECT_PTRS; i++){
    //             //not sure if it is block number or address
    //             unsigned int dirEntryBlock = pinode.direct[i];
    //             // read data block, get block number, the index should start from superblocks
    //             int dirEntryBlockAddr =  dirEntryBlock * UFS_BLOCK_SIZE;
    //             // int dirEntryBlockAddr = dataStartPosition + dirEntryBlock * UFS_BLOCK_SIZE;
    //             lseek(fd, dirEntryBlockAddr, SEEK_SET);
    //             char* entryBlock = malloc(UFS_BLOCK_SIZE);
    //             read(fd, entryBlock, UFS_BLOCK_SIZE);
    //             dir_ent_t* dirEntry;
    //             for(int j = 0; j < UFS_BLOCK_SIZE / sizeof(dir_ent_t); j++){
    //                 dirEntry = (dir_ent_t*)(entryBlock + j * sizeof(dir_ent_t));
    //                 if(dirEntry->inum != -1){
    //                     if(strcmp(dirEntry->name, name) == 0){
    //                         reply.inum =  dirEntry->inum;
    //                         return dirEntry->inum;
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }
    
}

int FS_Stat(int inum, MFS_Stat_t *m){
    Msg reply;
    reply.inum = -1;
    reply.requestType = 9; // type is reply
    // pinode in the bitmap(in block number)
    int inumBlock = superBlock.inode_region_addr + (inum * sizeof(inode_t)) / UFS_BLOCK_SIZE; 
    printf("inumBlock %d\n", inumBlock);
    // pinode array position(in address)
    // int inumPosition = inodeStartPosition + inum * sizeof(inode_t);

    //check inode bitmap
    if(get_bit((unsigned int *)imapAddress, inum) == 0){
        reply.inum = -1;
        reply.stat.size = -1;
        reply.stat.type = -1;
        fsync(fd);
        UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
        printf("stat: inode bimap invalid\n");
        return -1;
    } 
    else{
        //check inode type
        inode_t* inode;
        inode = getpinode(inum);
        //move read pointer to the position, SEEK_SET  means starting from the beginning
        // lseek(fd, inumPosition, SEEK_SET);
        // read(fd, &inode, sizeof(inode_t));
        reply.inum = 0;
        // reply.stat = malloc(sizeof(MFS_Stat_t));
        reply.stat.size = inode->size;
        reply.stat.type = inode->type;
    }
    int rc = UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
    printf("finished sending stat\n");
    return rc;
}

int FS_Write(int inum, char *buffer, int offset, int nbytes){
        Msg response;
    response.requestType = 4;
    if(nbytes > 4096 || get_bit( (unsigned int*) imapAddress, inum) != 1){
    fprintf(stderr, "DEBUG server write err1\n");
        throwErr(&response);
        return -1;
    }
    inode_t* inode = getpinode(inum);
    int type = inode->type;
    int size = inode->size;
    
    
    if(type == MFS_DIRECTORY || size < offset){
        fprintf(stderr, "DEBUG server write err2\n");
        throwErr(&response);
        return -1;
    }
    // write addresses
    int remainingBytes = nbytes;
    int bufferOffset = 0;
    int startBlock = offset / UFS_BLOCK_SIZE;
    int startOffset = offset % UFS_BLOCK_SIZE;
    int writeOffset = startOffset;
    int curBlock = inode->direct[startBlock]; // current writing block (in blocks)
    

   //printf("%i num data blocks \n",sb->num_data);
    if(curBlock < 0){
        
        if(blocksAvail == 0){
            fprintf(stderr, "DEBUG server write err3\n");
            throwErr(&response);
            return -1;
        }
        fprintf(stderr,"MONICA \n\n");
        //find a new block
        unsigned int newBlock = 0;
        // for(int i = 0; i < sb->num_data; i++){
        //     if(get_bit((unsigned int*) dmapAddress, i) == 0){
        //         newBlock = i + sb->data_region_addr;
        //         set_bit((unsigned int*)dmapAddress, i);
        //         curBlock = newBlock;
        //         break;
        //     }
        // }
        int newBlockNum = findFreeDataBlock();
        if(newBlockNum == -1){
            fprintf(stderr, "DEBUG server write err3\n");
            throwErr(&response);
            return -1;
        }
        newBlock = newBlockNum + sb->data_region_addr;
        curBlock = newBlock;
        set_bit((unsigned int*)dmapAddress, newBlockNum);
        blocksAvail--;
        
    }
    inode->direct[startBlock] = curBlock;


    if(!get_bit((unsigned int*)dmapAddress, curBlock - sb->data_region_addr)){
    
        throwErr(&response);
        return -1;
    }
    // relative addresses
    unsigned long currBlock_Addr = curBlock * UFS_BLOCK_SIZE; 
    unsigned long curWrite_Addr = currBlock_Addr + startOffset;

    // write until the current block is full. If not find an unused block
    //unused blocks can be either next block, or any other block
    int i = 0;
    unsigned long writeAddresses[nbytes]; 
    unsigned long newBlockUsed = -1;
   //byte by byte writing (or finding the addr atleast)
    while(remainingBytes > 0){
        if(writeOffset < 4096){
            writeAddresses[bufferOffset] = curWrite_Addr;
            curWrite_Addr += 1;
            bufferOffset += 1;
            writeOffset += 1;
            remainingBytes -= 1;

        }else{
            // next block
            int flag = 0;
            i += 1;
            if(startBlock + i > DIRECT_PTRS){
                fprintf(stderr, "DEBUG server write err3\n");
                throwErr(&response);
                return -1;
            }
            if(inode->direct[startBlock + i] != -1){
                // found
                curBlock = inode->direct[startBlock + i];
                currBlock_Addr = curBlock * UFS_BLOCK_SIZE;
                curWrite_Addr = currBlock_Addr;
                //flag = 1;
                writeOffset = 0;
                set_bit((unsigned int*)dmapAddress,curBlock);
                blocksAvail--;
                continue;
            }            
            //new block
            int freeFlag = 0;
            if(!flag){
                    
                // for(j = 0; j < sb->num_data; j++){
                //     if(get_bit((unsigned int*)dmapAddress, j)) continue; // jth data block is used, continue find
                    int j = findFreeDataBlock();
                    fprintf(stderr,"%i : JJJJJJ ", j);
                    if(j == -1){
                    throwErr(&response);
                    return -1;
                }
                    // use jth data block
                    set_bit((unsigned int*)dmapAddress,curBlock);
                    blocksAvail--;
                    curBlock = j + sb->data_region_addr;
                    currBlock_Addr = curBlock * UFS_BLOCK_SIZE;
                    curWrite_Addr = currBlock_Addr;
                    writeOffset = 0;
                    freeFlag = 1;
                    // jth data block will be used
                    newBlockUsed = curBlock;
                    break;                   
                
                // no place remaining!
                if(!freeFlag){
                    throwErr(&response);
                    return -1;
                }
            }
        }
    }
    //actually writing
    for(int i = 0; i < nbytes; i++){
        *(char*)(void*)(unsigned long)(writeAddresses[i] + (unsigned long)startAddress) = *(buffer + i); // real address
       
    }
    inode->size += nbytes;
    for(int i = 0; i < DIRECT_PTRS; i++){
        if(inode->direct[i] == -1){
            if(newBlockUsed != -1){
                inode->direct[i] = newBlockUsed + sb->data_region_addr;
                break;
            }
            
        }
    }
    // synchronize to disk
    // if(msync(startAddress, imgSize, MS_SYNC) == -1){
    //     fprintf(stderr, "DEBUG server write msync err\n");
    //     throwErr(&reply);
    //     return -1;
    // }
    fsync(fd);
    // reply
    response.inum = inum;
    
    fsync(fd);
    UDP_Write(sd, &addr, (char*)&response, sizeof(Msg));
    return 0;
}



//TODO: Do UDP write after writing to buffer and send reply

int FS_Read(int inum, char *buffer, int offset, int nbytes)
{

    Msg response;
    response.requestType = 5;
    unsigned long iBit = get_bit((unsigned int*)imapAddress, inum);
    //fprintf(stderr, "DEBUG server read:: nbytes: %d\n", nbytes);
    if(nbytes > 4096 || iBit != 1  || sizeof(buffer) > MFS_BLOCK_SIZE){
    throwErr(&response);
    return -1;
    }
	inode_t* inode = getpinode(inum);
    int size = inode->size;
    int type = inode->type;
    if(offset >= size){
        throwErr(&response);
        return -1;
    }
    if(type != MFS_REGULAR_FILE && type != MFS_DIRECTORY){
        throwErr(&response);
        return -1;
    }
    // find the start location of offset to read
    int startBlock = offset / UFS_BLOCK_SIZE;
    int startOffset = offset % UFS_BLOCK_SIZE;
    int remainingBytes = nbytes;
    int readOffset = startOffset;
    int bufferOffset = 0;
    unsigned long curBlock = inode->direct[startBlock]; // in blocks current writing block
    unsigned long curBlockAddress = curBlock * UFS_BLOCK_SIZE;
    unsigned long currentRead_Address = curBlockAddress + startOffset;

    // Directory case: cannot read if it doesn't read from start of each entry
    if(type == MFS_DIRECTORY && startOffset % 32 != 0){
        fprintf(stderr, "DEBUG server read err4\n");
        throwErr(&response);
        return -1;
    }

    // read until this block is full
    // to another block(if there's following block, use that, otherwise use a unused)
    int i = 0;
    while(remainingBytes > 0){
        if(readOffset < 4096){
            *(buffer + bufferOffset) = *(char*)((unsigned long)currentRead_Address + (unsigned long)startAddress); //TODO: check this
            currentRead_Address += 1;
            remainingBytes -= 1;
            bufferOffset += 1;
            readOffset += 1;
        }else{
            // find the next block
            int flag = 0;
            while( inode->direct[startBlock + i] != -1 && startBlock + i < 30){
                // found next block
                if(inode->direct[startBlock + i] != -1){
                    curBlock = inode->direct[startBlock + i];
                    curBlockAddress = curBlock * UFS_BLOCK_SIZE;
                    currentRead_Address = curBlockAddress;
                    flag = 1;
                    readOffset = 0;
                    break;
                }
                i++;
            }
            // did not find the next block, error
            if(!flag){
                throwErr(&response);
                return -1;
            }
        }
    }
    response.inum = inum;
    //actually writing to buffer
    for(int i = 0; i < nbytes; i++){
        
        response.buffer[i] = *(buffer+i);
    }
    fsync(fd);
    UDP_Write(sd, &addr, (char*)&response, sizeof(Msg));
    return 0;
}

// Msg reply;
// //int inumBlock = superBlock.inode_bitmap_addr + (inum * sizeof(inode_t))/UFS_BLOCK_SIZE;
// //int inumPos = (sizeof(inode_t) * inum) +inodeStartPosition;
// if(get_bit((unsigned int*)imapAddress,inum) != 1) {
// reply.requestType = -1;
// UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
// return -1;
// }
// // inode_t inode;
// // // Msg reply;
// // lseek(fd,inumPos,SEEK_SET);
// // read(fd,&inode, sizeof(inode_t));
// printf("bulle\n");
//     inode_t* inode = malloc(sizeof(inode_t));
//     inode = getpinode(inum);
//     printf("%i : name \n ",inode->type);

// // if(inode.type == UFS_DIRECTORY){
//     //READ DIR CONTENTS
    
//     if( (inode -> type = MFS_DIRECTORY) && (nbytes % sizeof(dir_ent_t) != 0 || offset % sizeof(dir_ent_t) != 0)){
//         reply.requestType = -1;
//         UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
//         return -1;
//     }

//     int dataBlock = 0;//offset/UFS_BLOCK_SIZE;
//     printf("data block: %i\n", dataBlock);
//     //int offsetRemaining = 0;
//     int nbytes1 =0;
//     if(offset%UFS_BLOCK_SIZE >= nbytes){
//     int nbytes1 = (inode->direct[dataBlock+1] - offset); //no of bytes to read in first block
//     nbytes = nbytes - nbytes1;
//     }
//     if(inode->direct[dataBlock] == -1 || offset>inode->size){
//         reply.requestType = -1;
//         UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
//         return -1;
//     }
//     char* test;
//     printf("block number : %i \n",inode->direct[dataBlock]);
//     lseek(fd,dataStartPosition+inode->direct[dataBlock],SEEK_SET);
//     read(fd,&test,nbytes1);

//     if(nbytes1 ){
//     printf("remaining case \n");
//     //figure out how to read into buffer at buffer+remaining
//     lseek(fd,dataStartPosition+inode->direct[dataBlock],SEEK_SET);
//     read(fd,&buffer,nbytes1);
//     lseek(fd,dataStartPosition+inode->direct[dataBlock+1],SEEK_SET);
//     read(fd,&buffer+nbytes1,nbytes);
//     strncpy(reply.buffer,buffer,nbytes+nbytes+1);
//      return (UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg)));
//     }
//     else{
//         printf("perfect case");
//         fseek(fd,(int*)dataBlockAddress+inode->direct[dataBlock],SEEK_SET);
//         fread(buffer,1,1,(FILE*)startAddress);
//         strncpy(reply.buffer,buffer,nbytes+nbytes+1);
//          return (UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg)));
//     }

    
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

   




int findFreeInode() {
    for(int i = 0; i<superBlock.num_inodes; i++) {
        int bit = get_bit((unsigned int *)imapAddress, i);
        if(bit==0) {
            return i;
        }
    }
    return -1;
}




int FS_Creat(int pinum, int type, char *name){
    Msg reply;
    reply.type = 9;
    reply.inum = -1;

    //check name length
    if(strlen(name) > 27) {
        throwErr(&reply);
        return -1;
    }
    //get pinode
    inode_t* pinode = getpinode(pinum);
    if(pinode->type != MFS_DIRECTORY){
        reply.inum = -1;
        fsync(fd);
        UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
        return 0;
    }

    //get newest freeinode
    int freeInodeNum = findFreeInode();
   // printf("freeInodeNum : %d\n", freeInodeNum);
    inode_t* freeInode;
    // freeInode = (inode_t*)(startAddress + sb->inode_region_addr * freeInodeNum);
    freeInode = (inode_t*) (inodeBlockAddress + freeInodeNum * sizeof(inode_t));
    

    //get the datablock of directory
   // printf("pinode->direct[0] : %d\n", pinode->direct[0]);
   // printf("start address : %p\n", startAddress);
    dir_ent_t*  pinodeDataBlockAddress = (dir_ent_t*) (startAddress + (pinode->direct[0]) * UFS_BLOCK_SIZE);

    for(int i = 0; i< UFS_BLOCK_SIZE/sizeof(dir_ent_t); i++) {
        // int entryAddress = pinodeDataBlockAddress + sizeof(dir_ent_t) * i;
        // lseek(fd, entryAddress, SEEK_SET);
        dir_ent_t* dirEntry;
        dirEntry = pinodeDataBlockAddress + i;
        // read(fd, dirEntry, sizeof(dir_ent_t));
        

        //printf(" direntry : %p, entry inum : %d\n", dirEntry, dirEntry->inum);
        if(dirEntry->inum == -1){
            dirEntry->inum = freeInodeNum;
            // strcpy(dirEntry->name, name);
            for(int z = 0; z != 28; z++){
                dirEntry->name[z] = *(name+z);
            }
            //printf("create file name : %s\n", dirEntry->name);
            set_bit((unsigned int *)imapAddress, freeInodeNum);
           // printf("imap value %d\n", get_bit((unsigned int *)imapAddress, 1));
            pinode->size += sizeof(dir_ent_t);
            freeInode->type = type;
            reply.inum = 0;
            break;
        }
    }

    if(type == MFS_REGULAR_FILE) {
        for(int i = 0; i < DIRECT_PTRS; i++){
            int newDataBlockNum = findFreeDataBlock();
            freeInode->direct[i] = newDataBlockNum + sb->data_region_addr;
            
            set_bit((unsigned int *)dmapAddress, newDataBlockNum);
        }
        //printf("finish creating regular file\n");
    }
    else if(type == MFS_DIRECTORY){
        int newDataBlockNum = findFreeDataBlock();
        freeInode->direct[0] = newDataBlockNum;
        //printf("new free data block number %d\n", newDataBlockNum);
        dir_ent_t* allDirEntryInsideBlock;
        allDirEntryInsideBlock =(dir_ent_t*) (startAddress + newDataBlockNum * UFS_BLOCK_SIZE);
        //printf("create direntry address %p\n", allDirEntryInsideBlock);
        //set '.' and '..'
        strcpy(allDirEntryInsideBlock->name, ".");
        strcpy((allDirEntryInsideBlock+1)->name, "..");
        allDirEntryInsideBlock->inum = freeInodeNum;
        (allDirEntryInsideBlock+1)->inum = pinum;

        for(int i = 2; i<UFS_BLOCK_SIZE/sizeof(dir_ent_t); i++) {
            (allDirEntryInsideBlock+i)->inum = -1;
        }
        set_bit((unsigned int *)dmapAddress, newDataBlockNum);
        for(int i = 1; i < 30; i++){
            freeInode->direct[i] = -1;
        }
        //printf("finish creating directory\n");
    }
    fsync(fd);


    UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
    return 0;

}

int FS_Unlink(int pinum, char *name)
{

    Msg reply;
    reply.requestType = 7;
    int inum = lookup(pinum, name);
    int reply_inum = lookup(pinum, name);

    inode_t* inode = getpinode(inum);
    if(inode->type == UFS_DIRECTORY ){
    printf("bulle\n\n");
    for(int i=0;i<31;i++){
        if(inode->direct[i] != -1){
            throwErr(&reply);
            return -1;
        }
        
    }
    }
    // unlink
    // set this inode bitmap to zero
    
    set_bit((unsigned int*)imapAddress, inum);
    // set data bitmaps to zero
    int i = 0;
    while(inode->direct[i] != -1){
        int data_blk = inode->direct[i];
        
        set_bit((unsigned int*)dmapAddress, (data_blk));
        i++;
    }
    
 
    fsync(fd);
    reply.inum = reply_inum;
    UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
    return 0;



//     Msg reply;
//     reply.requestType = -1;
//     unsigned int bitmapAddr = (unsigned int)superBlock.inode_bitmap_addr;
// if(get_bit(&bitmapAddr,pinum) != 1){
//     int rc = UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
//     fsync(fd);
//     return rc;
// }
// int inodePos = inodeStartPosition+ (pinum*sizeof(inode_t));
// inode_t inode;
// lseek(fd,inodePos,SEEK_SET);
// read(fd,&inode,sizeof(inode_t));
// if(inode.type == MFS_REGULAR_FILE){
//     reply.requestType = 1;
//     unsigned int inodeBitmapAddr = (unsigned int)superBlock.inode_bitmap_addr;
//     set_bit(&inodeBitmapAddr,pinum);
//     fsync(fd);
//     UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
//     return 1;
    
// }
// else{
//     int inum = lookup(pinum,name);
//     if(inum == -1){
//     UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
//     fsync(fd);
//     return -1;
//     }
//     int dirPos = inodeStartPosition+ (inum*sizeof(inode_t));
//     inode_t dirInode;
//     lseek(fd,dirPos,SEEK_SET);
//     read(fd,&dirInode,sizeof(inode_t));
//     if(dirInode.direct[0] != -1){
//     UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
//     fsync(fd);
//     return -1;
//     }
//     reply.requestType = 1;
//     unsigned int inodeBitmapAddr = (unsigned int)superBlock.inode_bitmap_addr;
//     set_bit(&inodeBitmapAddr,inum);
//     UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
//     fsync(fd);
//     return 1;

// }
}
    

int FS_Shutdown(){
    Msg reply;
    reply.type = 9;
    fsync(fd);
	close(fd);
	// m->c_received_rc = 0;
	//isShutdown = true;
    UDP_Write(sd, &addr, (char*)&reply, sizeof(Msg));
	UDP_Close(portNumber);
    printf("shutdown!\n");
	exit(0);
	return 0;
}




int fsInitMmap(int fd){
    struct stat fileStat;
    if(fstat(fd,&fileStat) < 0) 
    {
        exit(1);
    }
    imgSize = fileStat.st_size;
    assert(sd > -1);
    void* fileAddr = mmap(NULL, fileStat.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    sb = (super_t *) fileAddr;
    startAddress = fileAddr;
    printf("start address %p\n", startAddress);
    imapAddress = (char *)sb + sb->inode_bitmap_addr * UFS_BLOCK_SIZE;
    printf("imap address %p\n", imapAddress);
    dmapAddress = (char *)sb + sb->data_bitmap_addr * UFS_BLOCK_SIZE;
    inodeBlockAddress = (char *)sb + sb->inode_region_addr * UFS_BLOCK_SIZE;
    dataBlockAddress = (char *)sb + sb->data_region_addr * UFS_BLOCK_SIZE;
    blocksAvail = sb ->num_data;
    return 0;
}

int fsInit(char* img){
    if ((fd = open (img, O_RDWR|O_CREAT, S_IRWXU)) < 0)
    {  
        fprintf(stderr,"An error has occurred\n");
        return -1;
    }

    read(fd, &superBlock, sizeof(super_t));
    fsInitMmap(fd);
    printf("inode_bitmap_len : %d\n", superBlock.data_bitmap_len);
    printf("data_bitmap_len : %d\n", superBlock.data_bitmap_len);
    printf("inode_region_len : %d\n", superBlock.inode_region_len);
    printf("data_region_bitmap_len : %d\n", superBlock.data_region_len);
    printf("num_inodes : %d\n", superBlock.num_inodes);
    printf("num_data : %d\n", superBlock.num_data);
    iMapStartPosition = superBlock.inode_bitmap_addr * UFS_BLOCK_SIZE;     //should be equal to 4096
    dMapStartPosition = superBlock.data_bitmap_addr * UFS_BLOCK_SIZE;
    inodeStartPosition = superBlock.inode_region_addr * UFS_BLOCK_SIZE;
    dataStartPosition = superBlock.data_region_addr * UFS_BLOCK_SIZE;
    return 0;
}


int startServer(int port, char* img){
    portNumber = port;
    printf("server port %d \n", port);
    sd = UDP_Open(port);
    assert(sd > -1);
    //char *src, *dst;
    //struct stat statbuf;

    if(fsInit(img) < 0) return -1;

    while (1) {
        Msg* request = malloc(sizeof(Msg));
        printf("server:: waiting...\n");
        int rc = UDP_Read(sd, &addr, (char*)request, sizeof(Msg));

        // rc = UDP_Write(sd, &addr,"test connection", BUFFER_SIZE);

        //int responseRet;
        printf("server:: read message [size:%d contents:(%s)]\n", rc, (char*)request);
        if (rc > 0) {
            switch (request->requestType)
            {
            case 1:
                break;
            case 2:
                printf("request2\n");
                lookup(request->inum, request->name);
                break;
            case 3:
                printf("request3\n");
                FS_Stat(request->inum,&request->stat);
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
                exit(0);
                break;
            default:
                break;
            }
        } 
    }
    return 0; 
}

int main(int argc, char *argv[]) {
    printf("server beginning\n");
    // signal(SIGINT, intHandler);
    if (argc != 3)
		perror("Error : Incorrect arguments\n");
    IMGFILENAME = (char*) malloc(100 * sizeof(char));
	strcpy(IMGFILENAME, argv[2]);
    printf("before start server\n");
	startServer(atoi(argv[1]), IMGFILENAME);
}