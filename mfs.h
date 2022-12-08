#ifndef __MFS_h__
#define __MFS_h__

#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)

#define MFS_BLOCK_SIZE   (4096)

typedef struct __MFS_Stat_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    // note: no permissions, access times, etc.
} MFS_Stat_t;

typedef struct __MFS_DirEnt_t {
    char name[28];  // up to 28 bytes of name in directory (including \0)
    int  inum;      // inode number of entry (-1 means entry not used)
} MFS_DirEnt_t;


int MFS_Init(char *hostname, int port);                               //1
int MFS_Lookup(int pinum, char *name);                                //2
int MFS_Stat(int inum, MFS_Stat_t *m);                                //3
int MFS_Write(int inum, char *buffer, int offset, int nbytes);        //4
int MFS_Read(int inum, char *buffer, int offset, int nbytes);         //5
int MFS_Creat(int pinum, int type, char *name);                       //6
int MFS_Unlink(int pinum, char *name);                                //7
int MFS_Shutdown();                                                   //8

#endif // __MFS_h__
