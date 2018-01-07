#define FUSE_USE_VERSION 26

#ifdef linux
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
//#include <sys/stat.h>
#include <sys/statfs.h>
#include <stdbool.h>
#include <stdlib.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif


#define BLOCK_SIZE 512
struct superblock {
    uint32_t s_inodes_count; /* total number of inodes (used and free) */
    uint32_t s_blocks_count; /* total number of blocks (used and free) */ 
    uint32_t s_free_inodes_count; /* total number of free inodes */
    uint32_t s_free_blocks_count; /* total number of free data blocks */
    uint32_t s_first_data_block; /* which block is the first data block */
    uint32_t s_first_ino; /* index to first inode thats non-reserved */
    uint32_t s_magic;   /* Magic Signature is 0x554e4958 */
    /* remaining bytes are unused*/
};

struct inode {
    uint16_t i_mode;          /* File type (S_ISREG or S_ISDIR) and Permissions */
    uint16_t i_uid;           /* File owner */
    uint16_t i_gid;           /* File group */
    uint16_t i_links_count;   /* How many hard links are there to this file */
    uint32_t i_size;          /* Number of bytes in the file */
    uint32_t i_time;          /* Last access time */
    uint32_t i_ctime;         /* Creation time */
    uint32_t i_mtime;         /* Last modified time */
    uint32_t i_dtime;         /* When was this inode deleted */
    uint32_t i_blocks;        /* How many blocks are allocated to this file */

    uint32_t i_block[8];
    /* pointers to the blocks of data. (which datablock from first)
     * All point to direct blocks
     * */
};

/*
 * A directory should have 2 default entries when starting
 * first: a '.' dir pointing to itself
 * second: a '..' dir pointing to the parent directory
 * Also, a directory shouldn't fill in more than one block
 */
struct directory_entry {
    uint32_t        d_inode;        /* inode number */ 
    uint16_t        d_file_type;    /* 1 for regular file, 2 for directory */
    uint8_t         d_name_len;     /* length of file name */
    char            d_name[57]   ;    /* file name 0-57 bytes*/
};


/* 
 * Prototypes
 */
extern void init_filesystem(unsigned int size, char *real_path, unsigned int n);
extern void open_filesystem(char *real_path, unsigned int n);
extern int make_directory(char *path, unsigned int n);
extern unsigned int read_directory(char *path, unsigned int n, char *data);
extern int rm_directory(char *path, unsigned int n);
extern int create_file(char *path, unsigned int n, unsigned int size, char *data);
extern int rm_file (char *path, unsigned int n);
extern unsigned int read_file(char *path, unsigned int n, char *data);
extern int make_link(char *path, unsigned int n, char *target);

/*-------------------------------------------------------------------------*/
int          my_create(char *path, unsigned int n, int size, char *data, int type);
unsigned int my_read(char *path, unsigned int n, char *data, int type);
int          my_remove(char *path, unsigned int n, int type);

void init_inode(struct inode *node, int type, int size, int index);
void init_direntry(struct directory_entry *dirent, uint32_t current_inode, uint32_t parent_index);

char *create_path(char *path, unsigned int n);
int   validate_path(char *npath, int type);
int   check_permissions(uint16_t mode, uint16_t mask);

void update_superblock(int add, int num_data_blocks);
void update_bitmaps();

void         read_inode(struct inode *node, uint32_t index);
void         read_direntry(struct directory_entry *entries, uint32_t index, int n);
unsigned int read_data(char *data, uint32_t index, int n);

void write_inode(struct inode *node, uint32_t index);
void write_direntry(struct directory_entry *entries, uint32_t index, int n);
void write_data(char *data, int index, int n);

int  get_inode();
int  get_datablock(int index);

int errno; 
FILE *fp;
struct superblock sb;
unsigned char block_bm[BLOCK_SIZE];
unsigned char inode_bm[BLOCK_SIZE];
extern unsigned int INO_SIZE;
extern unsigned int DIR_ENTRY_SIZE;

