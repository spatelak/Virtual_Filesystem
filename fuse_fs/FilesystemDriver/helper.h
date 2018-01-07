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