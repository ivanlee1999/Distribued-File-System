int mfs_create(int pinum, int type, char *name) {
    if(strlen(name) > 28) {
        return -1;
    }
    if(inode_region[pinum].type != MFS_DIRECTORY) {
        return -1;
    }
    int data_ptr = inode_region[pinum].direct[0] - metaAddr->data_region_addr;
    int freeinum = get_available_inum();
    printf("Data PTR: %d\n", data_ptr);
    // printf("Inum: %d; Name: %s\n", data_region[data_ptr].entries[0].inum, data_region[data_ptr].entries[0].name);
    for(int i = 0; i< BLOCK_SIZE/sizeof(dir_ent_t); i++) {
        printf("Inum: %d\n", data_region[data_ptr].entries[i].inum);
        if(data_region[data_ptr].entries[i].inum == -1) {
            data_region[data_ptr].entries[i].inum = freeinum;
            printf("Index: %d, value: %d\n", i, freeinum);
            strcpy(data_region[data_ptr].entries[i].name, name);
            set_bit(get_addr(metaAddr->inode_bitmap_addr), freeinum);
            inode_region[pinum].size += sizeof(dir_ent_t);
            break;
        }
    }

    inode_region[freeinum].type = type;

    if(type == MFS_DIRECTORY) {
        dir_ent_t new_dir_entries[128];
        inode_region[freeinum].size = 2 * sizeof(dir_ent_t);
        strcpy(new_dir_entries[0].name, ".");
        new_dir_entries[0].inum = freeinum;
        strcpy(new_dir_entries[1].name, "..");
        new_dir_entries[1].inum = pinum;
        for(int i = 1; i<DIRECT_PTRS; i++) {
            inode_region[freeinum].direct[i] = -1;
        }
        for(int i = 2; i<BLOCK_SIZE/sizeof(dir_ent_t); i++) {
            new_dir_entries[i].inum = -1;
        }

        int new_data_ptr = get_available_datablock_addr();
        inode_region[freeinum].direct[0] = new_data_ptr + metaAddr->data_region_addr;
        set_bit(get_addr(metaAddr->data_bitmap_addr), new_data_ptr);

        memcpy(&data_region[new_data_ptr].entries, new_dir_entries, BLOCK_SIZE);
    }
    else {
        for(int i = 0; i<DIRECT_PTRS; i++) {
            int new_data_ptr = get_available_datablock_addr();
            inode_region[freeinum].direct[i] = new_data_ptr + metaAddr->data_region_addr;
            set_bit(get_addr(metaAddr->data_bitmap_addr), new_data_ptr);
        }
    }
    return 0;
}