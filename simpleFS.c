#include "simpleFS.h"
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAGIC   0x554e4958

unsigned char * mem;
long size_of_mem;

char file_system_opened = 0;

struct superblock * superblock;
char * inode_bm;
char * block_bm;
struct inode * inode_table;
unsigned char * data_blocks;

/*
 * TODO: make inode table and data blocks reusable with bitmap
 * TODO: prevent data from written beyond boundry
 * TODO: do more testing
 */

char get_bit_from_bitmap(char * bitmap, int index);

int main(int argc, char **argv)
{
    /*
     * prompt the user for which function they would like to perform.
     * prompt the user for parameters.
     * run the function.
     * repeat
     */
    
    int input;
    int size = 0;
    char path[100];
    char target_path[100];
    
    do {
        printf("----------------------------\n");
        printf("[0] initialie file system\n");
        printf("[1] open file system\n");
        printf("----------------------------\n");
        
        input = getchar();
        
        switch (input) {
            case '0':
                printf("enter file system image path: ");
                scanf("%s", path);
                getchar();
                
                fp = fopen(path, "w+");
                if (fp == NULL) {
                    file_system_opened = 0;
                    printf("[invalid path for image. image cannot be opened]\n");
                    break;
                }
                
                printf("\nenter size of the file system (number of data blocks): ");
                scanf("%d", &size);
                getchar();
                
                // check arguments
                if (size < 16 || size > 4104) {
                    file_system_opened = 0;
                    printf("[invalid size]\n");
                    break;
                }
                
                init_filesystem(size, path, (unsigned int) strlen(path));
                file_system_opened = 1;
                
                break;
            case '1':
                printf("enter file system image path: ");
                scanf("%s", path);
                getchar();
                
                open_filesystem(path, (unsigned int) strlen(path));
                
                break;
                
            default:
                printf("enter either 0 or 1\n");
                break;
        }
    } while (!file_system_opened);
    
    char read_data[1000];
    
    do {
        printf("----------------------------\n");
        printf("[0] make a new directory\n");
        printf("[1] read a direcotry\n");
        printf("[2] remove a direcotry\n");
        printf("[3] create a new file\n");
        printf("[4] read a file\n");
        printf("[5] remove a file\n");
        printf("[6] make a hard link\n");
        printf("[7] exit\n");
        printf("----------------------------\n");
        
        input = getchar();
        
        switch (input) {
                
            case '0': /* make a new directory */
                printf("enter directory path: ");
                scanf("%s", path);
                getchar();
                if (path[0] != '/') {
                    printf("[invalid path. must start with \"/\"]\n");
                    break;
                }
                
                make_directory(path, (unsigned int) strlen(path));
                
                break;
                
            case '1': /* read a directory */
                printf("enter directory path: ");
                scanf("%s", path);
                getchar();
                if (path[0] != '/') {
                    printf("[invalid path. must start with \"/\"]\n");
                    break;
                }
                
                read_directory(path, (unsigned int) strlen(path), read_data);
                printf("%s", read_data);
                read_data[0] = '\0';
                
                break;
                
            case '2': /* remove a directory */
                printf("enter directory path: ");
                scanf("%s", path);
                getchar();
                if (path[0] != '/') {
                    printf("[invalid path. must start with \"/\"]\n");
                    break;
                }
                
                rm_directory(path, (unsigned int) strlen(path));
                
                break;
                
            case '3': /* create a new file */
                printf("enter file path: ");
                scanf("%s", path);
                getchar();
                if (path[0] != '/') {
                    printf("[invalid path. must start with \"/\"]\n");
                    break;
                }
                
                printf("enter file content: ");
                
                char * content = NULL;
                unsigned long content_size = 0;
                
                getline(&content, &content_size, stdin);
                
                if (content_size > 1000) {
                    printf("[content size cannot exceed 1000 characters]\n");
                    break;
                }
                
                create_file(path, (unsigned int) strlen(path), (unsigned int) content_size, content);
                
                content[0] = '\0';
                free(content);
                
                break;
                
            case '4': /* read a file */
                printf("enter file path: ");
                scanf("%s", path);
                getchar();
                if (path[0] != '/') {
                    printf("[invalid path. must start with \"/\"]\n");
                    break;
                }
                
                int bytes = read_file(path, (unsigned int) strlen(path), read_data);
                printf("number of bytes read: %d\n", bytes);
                if (bytes > 0) {
                    printf("%s", read_data);
                    read_data[0] = '\0';
                }
                
                break;
                
            case '5': /* remove a file */
                printf("enter file path: ");
                scanf("%s", path);
                getchar();
                if (path[0] != '/') {
                    printf("[invalid path. must start with \"/\"]\n");
                    break;
                }
                
                rm_file(path, (unsigned int) strlen(path));
                
                break;
                
            case '6': /* make a hard link */
                printf("enter target file path: ");
                scanf("%s", target_path);
                getchar();
                if (target_path[0] != '/') {
                    printf("[invalid target path. must start with \"/\"]\n");
                    break;
                }
                
                printf("enter destination file path: ");
                scanf("%s", path);
                getchar();
                if (path[0] != '/') {
                    printf("[invalid destination file path. must start with \"/\"]\n");
                    break;
                }
                
                make_link(path, (unsigned int) strlen(path), target_path);
                
                break;
                
            case '7': /* exit */
                input = -1;
                printf("bye.\n");
                
                break;
                
            default:
                printf("[invalid input]\n");
                break;
        }
        
        
    } while (input != -1);

    free(mem);
    
    return 0;
}

void set_bit_to_bitmap(char * bitmap, int index, char bit) {
    int byte_index = index / 8;
    int offset = index % 8;
    
    if (bit == 0) {
        switch (offset) {
            case 0:
                bitmap[byte_index] = bitmap[byte_index] & ~bitmap_mask0;
                break;
            case 1:
                bitmap[byte_index] = bitmap[byte_index] & ~bitmap_mask1;
                break;
            case 2:
                bitmap[byte_index] = bitmap[byte_index] & ~bitmap_mask2;
                break;
            case 3:
                bitmap[byte_index] = bitmap[byte_index] & ~bitmap_mask3;
                break;
            case 4:
                bitmap[byte_index] = bitmap[byte_index] & ~bitmap_mask4;
                break;
            case 5:
                bitmap[byte_index] = bitmap[byte_index] & ~bitmap_mask5;
                break;
            case 6:
                bitmap[byte_index] = bitmap[byte_index] & ~bitmap_mask6;
                break;
            case 7:
                bitmap[byte_index] = bitmap[byte_index] & ~bitmap_mask7;
                break;
        }
    } else {
        switch (offset) {
            case 0:
                bitmap[byte_index] = bitmap[byte_index] | bitmap_mask0;
                break;
            case 1:
                bitmap[byte_index] = bitmap[byte_index] | bitmap_mask1;
                break;
            case 2:
                bitmap[byte_index] = bitmap[byte_index] | bitmap_mask2;
                break;
            case 3:
                bitmap[byte_index] = bitmap[byte_index] | bitmap_mask3;
                break;
            case 4:
                bitmap[byte_index] = bitmap[byte_index] | bitmap_mask4;
                break;
            case 5:
                bitmap[byte_index] = bitmap[byte_index] | bitmap_mask5;
                break;
            case 6:
                bitmap[byte_index] = bitmap[byte_index] | bitmap_mask6;
                break;
            case 7:
                bitmap[byte_index] = bitmap[byte_index] | bitmap_mask7;
                break;
        }
    }
}

char get_bit_from_bitmap(char * bitmap, int index) {
    int byte_index = index / 8;
    int offset = index % 8;
    
    char byte = 0;
    switch (offset) {
        case 0:
            byte = bitmap[byte_index] & bitmap_mask0;
            break;
        case 1:
            byte = bitmap[byte_index] & bitmap_mask1;
            break;
        case 2:
            byte = bitmap[byte_index] & bitmap_mask2;
            break;
        case 3:
            byte = bitmap[byte_index] & bitmap_mask3;
            break;
        case 4:
            byte = bitmap[byte_index] & bitmap_mask4;
            break;
        case 5:
            byte = bitmap[byte_index] & bitmap_mask5;
            break;
        case 6:
            byte = bitmap[byte_index] & bitmap_mask6;
            break;
        case 7:
            byte = bitmap[byte_index] & bitmap_mask7;
            break;
    }
    if (byte) {
        return 1;
    } else {
        return 0;
    }
}

uint32_t get_time_stamp() {
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    
    char * time_str = asctime (timeinfo);
    
    char month_str[10];
    int month;
    int date;
    int hour;
    int minute;
    int second;
    int year;
    
    sscanf(time_str, "%s %s %d %d:%d:%d %d", month_str, month_str, &date, &hour, &minute, &second, &year);
    
    if (!strcmp(month_str, "Jan")) {
        month = 1;
    } else if (!strcmp(month_str, "Feb")) {
        month = 2;
    } else if (!strcmp(month_str, "Mar")) {
        month = 3;
    } else if (!strcmp(month_str, "Apr")) {
        month = 4;
    } else if (!strcmp(month_str, "May")) {
        month = 5;
    } else if (!strcmp(month_str, "Jun")) {
        month = 6;
    } else if (!strcmp(month_str, "Jul")) {
        month = 7;
    } else if (!strcmp(month_str, "Aug")) {
        month = 8;
    } else if (!strcmp(month_str, "Sep")) {
        month = 9;
    } else if (!strcmp(month_str, "Oct")) {
        month = 10;
    } else if (!strcmp(month_str, "Nov")) {
        month = 11;
    } else if (!strcmp(month_str, "Dec")) {
        month = 12;
    } else {
        month = 11;
    }
    
    uint32_t time = 0;
    time = month*1000000 + date*10000 + hour*100 + minute;
    return time;
}

/* given a path string and return an array of each directory */
/* after parsing the size will be the size of the array */
char ** parsePath(char * path, int * size) {
    char * token = strtok(path, "/");
    
    char ** paths = (char **) malloc(10*sizeof(char *));
    int i = 0;
    
    while (token != NULL) {
        paths[i] = token;
        token = strtok(NULL, "/");
        i++;
    }
    *size = i;
    return paths;
}

/* if directory exists, return inode number of the directory */
/* if directory does not exist, return 0 */
uint32_t contains(struct inode * dir, char * dirName) {
    int i;
    int num_of_entries = dir->i_size/sizeof(struct directory_entry);
    struct directory_entry * entries = (struct directory_entry *) (dir->i_block[0]*BLOCK_SIZE + data_blocks);
    
    for (i = 0; i < num_of_entries; i++) {
        struct directory_entry entry = entries[i];
        if (!strcmp(entry.d_name, dirName)) {
            return entry.d_inode;
        }
    }
    return -1;
}

void printDataHelper(int m, int n) {
    int i, o;
    for (i = m*BLOCK_SIZE, o = 1; i < n*BLOCK_SIZE; i++, o++) {
        printf("%d ", mem[i]);
        if (o % 4 == 0) { printf("| "); }
        if (o % 40 == 0) { printf("\n"); }
    }
    printf("\n");
}

/* print the memory map */
void printData() {
    printf("superblock:\n");
    printDataHelper(0, 1);
    
    printf("inode bmap:\n");
    printDataHelper(1, 2);
    
    printf("data bmap:\n");
    printDataHelper(2, 3);
    
    printf("inode table block 1:\n");
    printDataHelper(3, 4);
    
    printf("data blocks:\n");
    printDataHelper(8, 9);
}

void writeDataToDisk() {
    if (fp == NULL) {
        return;
    }
    
    fseek(fp, SEEK_SET, 0);
    unsigned long bytes_written = fwrite(mem, 1, size_of_mem, fp);
    
    if (bytes_written == size_of_mem) {
//        printf("[successfully written data to disk]\n");
    } else {
        printf("[failed to write data to disk]\n");
    }
}

// Initialize a filesystem of the size specified in number of blocks
void init_filesystem(unsigned int size, char *real_path, unsigned int n)
{
    /* initialize memory */
    mem = (unsigned char *) malloc(size*BLOCK_SIZE);
    if (mem == NULL) {
        file_system_opened = 0;
        printf("[cannot allocate memory]\n");
        return;
    }

    size_of_mem = size*BLOCK_SIZE;
    superblock = (struct superblock *) mem;
    inode_bm = (char *)superblock + BLOCK_SIZE;
    block_bm = inode_bm + BLOCK_SIZE;
    inode_table = (struct inode *)(block_bm + BLOCK_SIZE);
    data_blocks = (unsigned char *)inode_table + 5 * BLOCK_SIZE;
    
     /* initalize a superblock. Write that to disk. */
    superblock->s_magic = MAGIC;
    superblock->s_inodes_count = 5*BLOCK_SIZE/sizeof(struct inode);
    superblock->s_blocks_count = size-8;
    superblock->s_free_inodes_count = superblock->s_inodes_count-1;
    superblock->s_free_blocks_count = superblock->s_blocks_count-1;
    superblock->s_first_data_block = 2;
    superblock->s_first_ino = 3;
    
     // * Create the bitmaps for block_bm and inode_bm. 
     // * write both to disk
    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {
        block_bm[i] = 0;
        inode_bm[i] = 0;
    }
    set_bit_to_bitmap(inode_bm, 2, 1);
    set_bit_to_bitmap(block_bm, 1, 1);

     // * Make a 5 block large inode table.
     // * Then initialize the "size" datablocks.


     // * Also, when this is all done. you should have a file system ready 
     // * with a parent directory of '/' at inode 2.
    struct inode *root_inode = &inode_table[2];
    root_inode->i_mode = S_DIR;
    root_inode->i_uid = getuid();
    root_inode->i_gid = getpgid(root_inode->i_uid);
    root_inode->i_links_count = 0;
    root_inode->i_size = 2 * sizeof(struct directory_entry);
    root_inode->i_time = get_time_stamp();
    root_inode->i_ctime = get_time_stamp();
    root_inode->i_mtime = get_time_stamp();
    root_inode->i_blocks = 1;
    root_inode->i_block[0] = 1;

    struct directory_entry *curr_dir = (struct directory_entry *)&data_blocks[BLOCK_SIZE];
    curr_dir->d_inode = 2;
    curr_dir->d_file_type = S_DIR;
    strcpy(curr_dir->d_name, ".");
    curr_dir->d_name_len = 1;

    struct directory_entry *prnt_dir = curr_dir + 1;
    prnt_dir->d_inode = 2;
    prnt_dir->d_file_type = S_DIR;
    strcpy(prnt_dir->d_name, "..");
    prnt_dir->d_name_len = 2;
    
    writeDataToDisk();
}

void open_filesystem(char *real_path, unsigned int n)
{
    /*
     * You will need to open an existing file system image from disk.
     * Read the file system information from the disk and initialize the
     * in-memory variables to match what was on disk.
     * use this file system for everything from now on.
     * Fail if the magic signature doesn't match
     */
    
    fp = fopen(real_path, "r");
    
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    
    if (mem != NULL) {
        free(mem);
    }
    
    fseek(fp, SEEK_SET, 0); // seek to beginning of the file
    
    mem = (unsigned char *) malloc(size*sizeof(char));
    long bytes_read = fread(mem, 1, size, fp);
    
    if (bytes_read == 0) {
        file_system_opened = 0;
        printf("[failed to read data]\n");
        fclose(fp);
        return;
    }
    
    fclose(fp);
    
    size_of_mem = size;
    superblock = (struct superblock *) mem;
    inode_bm = (char *)superblock + BLOCK_SIZE;
    block_bm = inode_bm + BLOCK_SIZE;
    inode_table = (struct inode *)(block_bm + BLOCK_SIZE);
    data_blocks = (unsigned char *)inode_table + 5 * BLOCK_SIZE;
    
    if (superblock->s_magic != MAGIC) {
        file_system_opened = 0;
        printf("[magic don't match]\n");
        return;
    }
    
    fp = fopen(real_path, "w");
    file_system_opened = 1;
    
    printf("[successfully opened file system image]\n");
}

void make_directory(char *path, unsigned int n)
{ 
    /*
     * For the file system image that is currently opened.
     * Make a new directory at the path provided.
     * Make sure that the path is valid.
     */
    
    /* parse path */
    int depth_of_path = 0;
    char ** paths = parsePath(path, &depth_of_path);
    
    /* go to the directory that the new directory is created */
    struct inode * curr = &inode_table[2];
    int curr_inode_num = 2;
    
    int i;
    for (i = 0; i < depth_of_path-1; i++) {
        uint32_t inode_num = contains(curr, paths[i]);
        if (inode_num == -1) { // dir does not exist
            printf("[parent directory does not exist]\n");
            return;
        } else {
            curr = &inode_table[inode_num];
            curr_inode_num = inode_num;
        }
    }
    
    /* check if directory already exits */
    uint32_t inode_num = contains(curr, paths[i]);
    if (inode_num != -1) {
        printf("[directory already exits]\n");
        return;
    }
    
    /* create new directory */
    int new_inode_num = superblock->s_first_ino;
    superblock->s_first_ino++; // TODO: resue inode table
    
    /* create new inode */
    struct inode * new_inode = &inode_table[new_inode_num];
    new_inode->i_mode = S_DIR;
    new_inode->i_uid = getuid();
    new_inode->i_gid = getpgid(new_inode->i_uid);
    new_inode->i_links_count = 0;
    new_inode->i_size = 2 * sizeof(struct directory_entry);
    new_inode->i_time = get_time_stamp();
    new_inode->i_ctime = get_time_stamp();
    new_inode->i_mtime = get_time_stamp();
    new_inode->i_blocks = 1;
    new_inode->i_block[0] = superblock->s_first_data_block;
    
    superblock->s_first_data_block++; // TODO: reuse data block
    set_bit_to_bitmap(block_bm, new_inode->i_block[0], 1);
    
    struct directory_entry * curr_dir = (struct directory_entry * )&data_blocks[new_inode->i_block[0] * BLOCK_SIZE];
    curr_dir->d_inode = new_inode_num;
    curr_dir->d_file_type = S_DIR;
    strcpy(curr_dir->d_name, ".");
    curr_dir->d_name_len = 1;
    
    set_bit_to_bitmap(inode_bm, new_inode_num, 1);
    
    struct directory_entry * prnt_dir = curr_dir + 1;
    prnt_dir->d_inode = curr_inode_num;
    prnt_dir->d_file_type = S_DIR;
    strcpy(prnt_dir->d_name, "..");
    prnt_dir->d_name_len = 2;
    
    /* add new directory_entry to current inode */
    struct directory_entry * entries = (struct directory_entry *) &data_blocks[curr->i_block[0]*BLOCK_SIZE];
    int num_of_entries = curr->i_size/sizeof(struct directory_entry);
    
    struct directory_entry * new_dir = &entries[num_of_entries];
    curr->i_size += sizeof(struct directory_entry);
    
    new_dir->d_inode = new_inode_num;
    strcpy(new_dir->d_name, paths[depth_of_path-1]);
    new_dir->d_name_len = strlen(new_dir->d_name);
    new_dir->d_file_type = S_DIR;
    
    /* update superblock */
    superblock->s_free_inodes_count -= 1;
    superblock->s_free_blocks_count -= 1;

    printf("[directory created]\n");
    
    writeDataToDisk();
}

unsigned int read_directory(char *path, unsigned int n, char *data)
{
    /*
     * For the file system image that is currently opened.
     * Read the contents of the directory in the path provided.
     * Make sure that the path is valid.
     * Place the directory entries in data 
     * and return the number of bytes that have been placed in data.
     */
    
    /* parse path */
    int depth_of_path = 0;
    char ** paths = parsePath(path, &depth_of_path);
    
    /* find directory */
    struct inode * curr = &inode_table[2];
    
    int i;
    for (i = 0; i < depth_of_path; i++) {
        uint32_t inode_num = contains(curr, paths[i]);
        if (inode_num == -1) { // dir does not exist
            printf("[directory %s does not exist]\n", paths[i]);
            return 0;
        } else {
            curr = &inode_table[inode_num];
        }
    }
    
    int num_of_dirs = curr->i_size / sizeof(struct directory_entry);
    struct directory_entry * entries = (struct directory_entry *) &data_blocks[curr->i_block[0]*BLOCK_SIZE];
    
    int bytes = 0;
    char * entry_types[3];
    entry_types[1] = strdup("REG");
    entry_types[2] = strdup("DIR");
    
    for (i = 0; i < num_of_dirs; i++) {
        struct directory_entry entry = entries[i];
        bytes = sprintf(data, "%s\t\t%d\t\t%s\n", entry.d_name, entry.d_inode, entry_types[entry.d_file_type]);
        data += bytes;
    }
    
    curr->i_time = get_time_stamp();
    
    return bytes;
}

void rm_directory(char *path, unsigned int n)
{ 
    /*
     * For the file system image that is currently opened.
     * Delete the directory in the path provided.
     * Make sure that the directory doesn't have any files remaining
     */
    
    /* parse path */
    int depth_of_path = 0;
    char ** paths = parsePath(path, &depth_of_path);
    
    /* find directory */
    struct inode * curr = &inode_table[2];
    int curr_inode_num = 2;
    struct inode * prev = curr;
    
    int i;
    for (i = 0; i < depth_of_path; i++) {
        uint32_t inode_num = contains(curr, paths[i]);
        if (inode_num == -1) { // dir does not exist
            printf("[directory %s does not exist]\n", paths[i]);
            return;
        } else {
            prev = curr;
            curr = &inode_table[inode_num];
            curr_inode_num = inode_num;
        }
    }
    
    // prev is the directory that contains the directory to be removed
    // curr is the direotory to be removed
    
    /* check if directory has remaining files */
    int num_of_dirs = curr->i_size / sizeof(struct directory_entry);
    if (num_of_dirs > 2) {
        printf("[diretory that has remaining files cannot be remove]\n");
        return;
    }
    
    /* remove directory */
    struct directory_entry * prev_entries = (struct directory_entry *) &data_blocks[prev->i_block[0]*BLOCK_SIZE];
    int prev_num_of_dirs = prev->i_size / sizeof(struct directory_entry);
    for (i = 0; i < prev_num_of_dirs; i++) {
        if (!strcmp(prev_entries[i].d_name, paths[depth_of_path-1])) {
            int j;
            for (j = i; j < prev_num_of_dirs; j++) {
                prev_entries[j] = prev_entries[j+1];
            }
            prev->i_size -= sizeof(struct directory_entry);
            break;
        }
    }
    
    curr->i_dtime = get_time_stamp();
    
    set_bit_to_bitmap(inode_bm, curr_inode_num, 0);
    set_bit_to_bitmap(block_bm, curr->i_block[0], 0);
    
    /* update superblock */
    superblock->s_free_inodes_count += 1;
    superblock->s_free_blocks_count += 1;

    printf("[directory removed]\n");
    
    writeDataToDisk();
}

void create_file(char *path, unsigned int n, unsigned int size, char *data)
{ 
    /*
     * For the file system image that is currently opened.
     * Create a new file at path.
     * Make sure that the path is valid.
     * Write to the contents of the file in the data provided.
     * size is the number of bytes in the data.
     */
    
    /* parse path */
    int depth_of_path = 0;
    char ** paths = parsePath(path, &depth_of_path);
    
    /* go to the directory that the new directory is created */
    struct inode * curr = &inode_table[2];
    
    int i;
    for (i = 0; i < depth_of_path-1; i++) {
        uint32_t inode_num = contains(curr, paths[i]);
        if (inode_num == -1) { // dir does not exist
            printf("[parent directory does not exist]\n");
            return;
        } else {
            curr = &inode_table[inode_num];
        }
    }
    
    /* check if file already exits */
    uint32_t inode_num = contains(curr, paths[i]);
    if (inode_num != -1) {
        printf("[file already exits]\n");
        return;
    }
    
    /* create new file */
    int new_inode_num = superblock->s_first_ino;
    superblock->s_first_ino++;
    set_bit_to_bitmap(inode_bm, new_inode_num, 1);

    struct inode * new_inode = &inode_table[new_inode_num];
    new_inode->i_mode = S_REG;
    new_inode->i_uid = getuid();
    new_inode->i_gid = getpgid(new_inode->i_uid);
    new_inode->i_links_count = 0;
    new_inode->i_size = size;
    new_inode->i_time = get_time_stamp();
    new_inode->i_ctime = get_time_stamp();
    new_inode->i_mtime = get_time_stamp();
    new_inode->i_blocks = size/ BLOCK_SIZE + 1;
    new_inode->i_block[0] = superblock->s_first_data_block;

    int data_block_num = superblock->s_first_data_block;
    superblock->s_first_data_block += new_inode->i_blocks;
    for (i = 0; i < new_inode->i_blocks; i++) {
        set_bit_to_bitmap(block_bm, data_block_num+i, 1);
    }
    char * data_addr = (char *) &data_blocks[data_block_num * BLOCK_SIZE];
    new_inode->i_block[0] = data_block_num;
    strncpy(data_addr, data, size);
    
    struct directory_entry * entries = (struct directory_entry *) &data_blocks[curr->i_block[0]*BLOCK_SIZE];
    int num_of_dirs = curr->i_size / sizeof(struct directory_entry);
    struct directory_entry * new_entry = &entries[num_of_dirs];
    new_entry->d_file_type = S_REG;
    new_entry->d_inode = new_inode_num; // TODO: reuse inode table entry
    strcpy(new_entry->d_name, paths[depth_of_path-1]);
    new_entry->d_name_len = strlen(new_entry->d_name);
    
    curr->i_size += sizeof(struct directory_entry);
    
    /* update superblock */
    superblock->s_free_inodes_count -= 1;
    superblock->s_free_blocks_count -= new_inode->i_blocks;
    
    printf("[file created]\n");
    
    writeDataToDisk();
}

unsigned int read_file(char *path, unsigned int n, char *data)
{
    /*
     * For the file system image that is currently opened.
     * Read the contents of the file in the path provided.
     * Make sure that the path is valid.
     * Place the file contents in data
     * and return the number of bytes in the file.
     */
    
    /* parse path */
    int depth_of_path = 0;
    char ** paths = parsePath(path, &depth_of_path);
    
    /* find file */
    struct inode * curr = &inode_table[2];
    
    int i;
    for (i = 0; i < depth_of_path; i++) {
        uint32_t inode_num = contains(curr, paths[i]);
        if (inode_num == -1) { // dir does not exist
            printf("[%s does not exist]\n", paths[i]);
            return 0;
        } else {
            curr = &inode_table[inode_num];
        }
    }
    
    char * data_addr = (char *) &data_blocks[curr->i_block[0]*BLOCK_SIZE];
    strncpy(data, data_addr, curr->i_size);
    
    curr->i_time = get_time_stamp();
    
    return curr->i_size;
}

void rm_file(char *path, unsigned int n)
{ 
    /*
     * For the file system image that is currently opened.
     * Delete the file in the path provided.
     * Make sure that the data blocks are freed after deleting the file.
     */
    
    /* parse path */
    int depth_of_path = 0;
    char ** paths = parsePath(path, &depth_of_path);
    
    /* find file */
    struct inode * curr = &inode_table[2];
    int curr_inode_num = 2;
    struct inode * prev = curr;
    
    int i;
    for (i = 0; i < depth_of_path; i++) {
        uint32_t inode_num = contains(curr, paths[i]);
        if (inode_num == -1) { // dir does not exist
            printf("[%s does not exist]\n", paths[i]);
            return;
        } else {
            prev = curr;
            curr = &inode_table[inode_num];
            curr_inode_num = inode_num;
        }
    }
    
    // prev is the directory that contains the file to be removed
    // curr is the file to be removed
    
    /* remove file */
    struct directory_entry * prev_entries = (struct directory_entry *) &data_blocks[prev->i_block[0]*BLOCK_SIZE];
    int prev_num_of_dirs = prev->i_size / sizeof(struct directory_entry);
    for (i = 0; i < prev_num_of_dirs; i++) {
        if (!strcmp(prev_entries[i].d_name, paths[depth_of_path-1])) {
            int j;
            for (j = i; j < prev_num_of_dirs; j++) {
                prev_entries[j] = prev_entries[j+1];
            }
            prev->i_size -= sizeof(struct directory_entry);
            break;
        }
    }
    
    /* if inode has hard links, dont't actually remove inode and linked data blocks */
    if (curr->i_links_count > 0) {
        curr->i_links_count--;
        printf("[file removed]\n");
        writeDataToDisk();
        return;
    }
    
    curr->i_dtime = get_time_stamp();
    
    set_bit_to_bitmap(inode_bm, curr_inode_num, 0);
    
    /* free data blocks */
    int data_block_count = curr->i_blocks;
    int start_data_block = curr->i_block[0];
    for (i = 0; i < data_block_count; i++) {
        set_bit_to_bitmap(block_bm, start_data_block+i, 0);
    }
    
    /* update superblock */
    superblock->s_free_inodes_count += 1;
    superblock->s_free_blocks_count += curr->i_blocks;
    
    printf("[file removed]\n");
    
    writeDataToDisk();
}

void make_link(char *path, unsigned int n, char *target)
{ 
    /*
     * make a new hard link in the path to target
     * make sure that the path and target are both valid.
     */
    
    /* parse target path */
    int depth_of_target_path = 0;
    char ** paths_target = parsePath(target, &depth_of_target_path);
    
    /* find target file */
    struct inode * curr_target = &inode_table[2];
    int curr_target_inode_num = 2;
    
    int i;
    for (i = 0; i < depth_of_target_path; i++) {
        uint32_t inode_num = contains(curr_target, paths_target[i]);
        if (inode_num == -1) { // dir does not exist
            printf("[%s does not exist]\n", paths_target[i]);
            return;
        } else {
            curr_target = &inode_table[inode_num];
            curr_target_inode_num = inode_num;
        }
    }
    
    /* check if the target file is a directory */
    if (curr_target->i_mode == S_DIR) {
        printf("[%s is a directory]\n", paths_target[depth_of_target_path-1]);
        return;
    }
    
    // at this point we should have the target file, which is curr_target
    
    /* parse path */
    int depth_of_path = 0;
    char ** paths = parsePath(path, &depth_of_path);
    
    /* go to the directory that the new directory is created */
    struct inode * curr = &inode_table[2];
    
    for (i = 0; i < depth_of_path-1; i++) {
        uint32_t inode_num = contains(curr, paths[i]);
        if (inode_num == -1) { // dir does not exist
            printf("[parent directory does not exist]\n");
            return;
        } else {
            curr = &inode_table[inode_num];
        }
    }
    
    /* check if file already exits */
    uint32_t inode_num = contains(curr, paths[i]);
    if (inode_num != -1) {
        printf("[file %s already exits]\n", paths[i]);
        return;
    }
    
    /* update inode */
    curr_target->i_links_count += 1;
    
    /* add existing inode to current directory */
    struct directory_entry * entries = (struct directory_entry *) &data_blocks[curr->i_block[0]*BLOCK_SIZE];
    int num_of_dirs = curr->i_size / sizeof(struct directory_entry);
    struct directory_entry * new_entry = &entries[num_of_dirs];
    new_entry->d_file_type = S_REG;
    new_entry->d_inode = curr_target_inode_num;
    strcpy(new_entry->d_name, paths[depth_of_path-1]);
    new_entry->d_name_len = strlen(new_entry->d_name);
    
    curr->i_size += sizeof(struct directory_entry);
    
    printf("[hard link created]\n");
    
    writeDataToDisk();
}
