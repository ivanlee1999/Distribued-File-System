#include "mfs.h"
#include "ufs.h"

typedef struct Msg{
    int requestType;
    int inum;
    int type;       //regular or directory
    char name[28];
    int offset;
    int nbytes;
    MFS_Stat_t stat;
    char buffer[MFS_BLOCK_SIZE];
}Msg;
