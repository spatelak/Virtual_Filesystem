#include <sys/stat.h> // For the macros used for i_mode
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <errno.h>

/* Filesystem layout (based on OSTEP and EXT2)
 * superblock   first block
 * block bitmap 1 block
 * inode bitmap 1 block
 * inode table  5 blocks (total of 8 blocks now)
 * data blocks  1 block each untill end of disk image
 *
 * Notes:
 * The first usable inode is 2.
 */

#define BLOCK_SIZE       512  /* Old school hardware has 512 bytes per block */
#define N_INODES         40
#define START_DATA       8
#define START_DATA_ADDR  BLOCK_SIZE * 8
#define START_INODE      2
#define START_INODE_ADDR BLOCK_SIZE * 3
#define INODE_SIZE       64
#define MAX_DIRENT       8
#define MAGIC_SIGN       0x554e4958

struct superblock
{
    uint32_t s_inodes_count; /* total number of inodes (used and free) */
    uint32_t s_blocks_count; /* total number of blocks (used and free) */
    uint32_t s_free_inodes_count; /* total number of free inodes */
    uint32_t s_free_blocks_count; /* total number of free data blocks */
    uint32_t s_first_data_block; /* which block is the first data block */
    uint32_t s_first_ino; /* index to first inode thats non-reserved */
    uint32_t s_magic;   /* Magic Signature is 0x554e4958 */
    /* remaining bytes are unused */
};

struct inode
{
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
    /* indices to the blocks of data. (which datablock from first)
     * All point to direct blocks
     * */
};

/*
 * A directory should have 2 default entries when starting
 * first: a '.' dir pointing to itself
 * second: a '..' dir pointing to the parent directory
 * Also, a directory shouldn't fill in more than one block
 */
struct directory_entry
{
    uint32_t        d_inode;        /* inode number */
    uint16_t        d_file_type;    /* 1 for regular file, 2 for directory */
    uint8_t         d_name_len;     /* length of file name */
    char            d_name[57];     /* file name 0-57 bytes*/
};

/*********** HIGH LEVEL FS OPERATIONS ***********/
// Initialize a filesystem with size specifying number of data blocks at path.
// real_path is the location of the virtual drive
// n is the length of the string real_path
extern void init_filesystem(unsigned int size, char *real_path, unsigned int n);

// Open a file system given at path.
// n is the length of the string real_path
extern void open_filesystem(char *real_path, unsigned int n);

// Make a new directory in with the path *path.
// n is the length of the string path
extern int make_directory(char *path, unsigned int n);

// Places the directory contents in *data.
// Returns the number of bytes that are in data.
// n is the length of the string path
extern unsigned int read_directory(char *path, unsigned int n, char *data);

// Deletes a file in the path
// n is the length of the string path
extern int rm_directory(char *path, unsigned int n);

// Make a new file of the specified size in the path provided.
// Initial data has to be passed in
// file contents are in data
// n is the length of the string path
extern int create_file(char *path, unsigned int n, unsigned int size, char *data);

// Delete a file of the path.
// Make sure to also free up any data blocks in the file
// n is the length of the string path
extern int rm_file (char *path, unsigned int n);

// Read a files contents and store the contents in data.
// Returns the number of bytes in the file.
// n is the length of the string path
extern unsigned int read_file(char *path, unsigned int n, char *data);

// Make a hard link to the "*target" file at the "*path"
// n is the length of the string path
extern int make_link(char *path, unsigned int n, char *target);

// Global vars to keep in memory for performance reasons
FILE *fp; // The file system image currently in use
struct superblock sb;
unsigned char block_bm[BLOCK_SIZE];
unsigned char inode_bm[BLOCK_SIZE];
