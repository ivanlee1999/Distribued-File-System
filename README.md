# a distribued file system 
- Commucation using UDP 
- Client can send request to server by calling functions in mfs.c
- mkfs.c create initial file system image

## Client Library
- int MFS_Init(char *hostname, int port): MFS_Init() takes a host name and port number and uses those to find the server exporting the file system.
- int MFS_Lookup(int pinum, char *name): MFS_Lookup() takes the parent inode number (which should be the inode number of a directory) and looks up the entry name in it. The inode number of name is returned. Success: return inode number of name; failure: return -1. Failure modes: invalid pinum, name does not exist in pinum.
- int MFS_Stat(int inum, MFS_Stat_t *m): MFS_Stat() returns some information about the file specified by inum. Upon success, return 0, otherwise -1. The exact info returned is defined by MFS_Stat_t. Failure modes: inum does not exist. File and directory sizes are described below.
- int MFS_Write(int inum, char *buffer, int offset, int nbytes): MFS_Write() writes a buffer of size nbytes (max size: 4096 bytes) at the byte offset specified by offset. Returns 0 on success, -1 on failure. Failure modes: invalid inum, invalid nbytes, invalid offset, not a regular file (because you can't write to directories).
- int MFS_Read(int inum, char *buffer, int offset, int nbytes): MFS_Read() reads nbytes of data (max size 4096 bytes) specified by the byte offset offset into the buffer from file specified by inum. The routine should work for either a file or directory; directories should return data in the format specified by MFS_DirEnt_t. Success: 0, failure: -1. Failure modes: invalid inum, invalid offset, invalid nbytes.
- int MFS_Creat(int pinum, int type, char *name): MFS_Creat() makes a file (type == MFS_REGULAR_FILE) or directory (type == MFS_DIRECTORY) in the parent directory specified by pinum of name name. Returns 0 on success, -1 on failure. Failure modes: pinum does not exist, or name is too long. If name already exists, return success.
- int MFS_Unlink(int pinum, char *name): MFS_Unlink() removes the file or directory name from the directory specified by pinum. 0 on success, -1 on failure. Failure modes: pinum does not exist, directory is NOT empty. Note that the name not existing is NOT a failure by our definition (think about why this might be).
- int MFS_Shutdown(): MFS_Shutdown() just tells the server to force all of its data structures to disk and shutdown by calling exit(0). This interface will mostly be used for testing purposes.
