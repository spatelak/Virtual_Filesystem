#include "simpleFS.h"
#include "helper.h"

/* ------------------------------------------------------ */
/*                 LOW LEVEL FUNCTIONS                    */
/* ------------------------------------------------------ */

/**
 * Initialize an inode entry
 */
void init_inode(struct inode *node, int type, int size, int index)
{
  time_t t = time(NULL);

  node->i_mode        = type == 2 ? S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO : S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO;
  node->i_uid         = getuid();
  node->i_gid         = getgid();
  node->i_links_count = 1;
  node->i_size        = size;
  node->i_time        = t;
  node->i_ctime       = t;
  node->i_mtime       = t;
  node->i_dtime       = 0;
  node->i_blocks      = (size == 0) ? 1 : size / BLOCK_SIZE + ((size % BLOCK_SIZE != 0));

  // if all data blocks are occupied then don't initialize inode
  if (sb.s_free_blocks_count < node->i_blocks) {
    printf("Disk is full - blocks = %d\n", sb.s_free_blocks_count);
    inode_bm[index / 8] &= ~(1 << (index % 8));
    node = NULL;
    exit(1);
  }

  int i;
  if (node->i_size != 0) for (i = 0; i < node->i_blocks; i++)  node->i_block[i] = get_datablock(index);
  else node->i_block[0] = get_datablock(index);
}

/**
 * Initialize directory entries
 */
void init_direntry(struct directory_entry *dirent, uint32_t current_inode, uint32_t parent_inode)
{
  char pad[57] = "";
  
  dirent[0].d_inode     = current_inode;
  dirent[0].d_file_type = 2;
  dirent[0].d_name_len  = 1;
  memcpy(dirent[0].d_name, pad, 57);
  strcpy(dirent[0].d_name, ".");

  dirent[1].d_inode     = parent_inode;
  dirent[1].d_file_type = 2;
  dirent[1].d_name_len  = 2;
  memcpy(dirent[1].d_name, pad, 57);
  strcpy(dirent[1].d_name, "..");
}

/**
 * Check read/write/execute permission for users/groups/others
 */
int check_permissions(uint16_t mode, uint16_t mask)
{
  if ((mode & mask) == mask) return 1;

  return 0;
}

/**
 * Create a path string as a null terminated string
 */
char *create_path(char *path, unsigned int n)
{
  char *npath = NULL;
  if (*(path + n - 1) == '/') npath = malloc(n);
  else                        npath = malloc(n + 1);

  if (npath == NULL) {
    printf("Malloc failed\n");
    return NULL;
  }

  if (*(path + n - 1) == '/') {
    strncpy(npath, path, n - 1);
    npath[n - 1] = '\0';
  }
  else {
    strncpy(npath, path, n);
    npath[n]     = '\0';
  }
   
  return npath;
}

/**
 *  Validate the given path and return the inode index of the target file/dir if the path is valid
 */
int validate_path(char *npath, int target_type)
{
  char *current_child   = strtok(npath, "/");
  uint32_t parent_inode = START_INODE;
  
  // read the root inode from disk
  struct inode current_inode;
  read_inode(&current_inode, parent_inode);

  // read the directory entries of root directory
  int n = current_inode.i_size / sizeof(struct directory_entry);
  struct directory_entry entries[MAX_DIRENT];
  read_direntry(entries, current_inode.i_block[0], MAX_DIRENT);
 
  // check if current_child is in parent directory
  while (current_child != NULL) {
    int i = 0;
    while (i < n && strcmp(current_child, entries[i].d_name) != 0) i++;
  
    // update parent and child
    if (i < n) parent_inode  = entries[i].d_inode;
    current_child = strtok(NULL, "/");
    
    if (current_child == NULL) {//&& entries[i].d_file_type != target_type) {
      if (i == n) return -ENOENT;
      if (target_type != 3 && entries[i].d_file_type != target_type) {
	printf("Invalid path\n");
	return -ENOENT;
      }
    }
  
  
    // return error if invlid path
    if (current_child != NULL && (i == n || entries[i].d_file_type == 1)) {
      printf("Invalid path\n");
      return -ENOTDIR;
    }

    // read new parent inode and its directories
    read_inode(&current_inode, parent_inode);
    
    n = current_inode.i_size / sizeof(struct directory_entry);
    read_direntry(entries, current_inode.i_block[0], MAX_DIRENT);
  }

  return parent_inode;
}

/**
 * Update and write superblock to the disk
 */
void update_superblock(int add, int num_data_blocks)
{

  // update
  if (add == 1) {
    sb.s_free_inodes_count += 1;
    sb.s_free_blocks_count += num_data_blocks;
  }
  else {
    sb.s_free_inodes_count -= 1;
    sb.s_free_blocks_count -= num_data_blocks;
  }
  // write
  fseek(fp, 0, SEEK_SET);
  fwrite(&sb, sizeof(struct superblock), 1, fp);
}

/**
 * Read inode from the disk
 */
void read_inode(struct inode *node, uint32_t index)
{
  fseek(fp, START_INODE_ADDR + sizeof(struct inode) * index, SEEK_SET);
  fread(node, sizeof(struct inode), 1, fp);
}

/**
 * Read directory entries from the disk
 */
void read_direntry(struct directory_entry *entries, uint32_t index, int n)
{
  fseek(fp, START_DATA_ADDR + BLOCK_SIZE * index, SEEK_SET);
  fread(entries, BLOCK_SIZE, 1, fp);
}

/**
 * Read data from the disk
 */
unsigned int read_data(char *data, uint32_t index, int n)
{
  fseek(fp, START_DATA_ADDR + BLOCK_SIZE * index, SEEK_SET);
  return fread(data, 1, n, fp);
}

/**
 * Write inode to the disk
 */
void write_inode(struct inode *node, uint32_t index)
{
  fseek(fp, START_INODE_ADDR + sizeof(struct inode) * index, SEEK_SET);
  fwrite(node, sizeof(struct inode), 1, fp);
}

/**
 * Write data for a directory to the disk
 */
void write_direntry(struct directory_entry *entries, uint32_t index, int n)
{
  char padding[BLOCK_SIZE] = "";
  memcpy(padding, entries, sizeof(struct directory_entry) * n);
  fseek(fp, START_DATA_ADDR + BLOCK_SIZE * index, SEEK_SET);
  fwrite(padding, BLOCK_SIZE, 1, fp);
}

/**
 *  Write data to the disk
 */
void write_data(char *data, int index, int n)
{
  char padding[BLOCK_SIZE] = "";
  memcpy(padding, data, n);

  fseek(fp, START_DATA_ADDR + BLOCK_SIZE * index, SEEK_SET);
  fwrite(padding, BLOCK_SIZE, 1, fp);
}

/**
 * Get the index of next free inode. Each bit determines if inode is free (bit = 0) or occupied (bit = 1)
 */
int get_inode()
{
  int temp, count, i = 0;
  for (; i < 5; i++) {
    if (inode_bm[i] < 255) {
      count = 0;
      temp = inode_bm[i];
      while ((temp & 1) == 1) {
                         	count++;
				temp >>= 1;
      }
      
      inode_bm[i] = (inode_bm[i]) | (1 << count);
      return i * 8 + count;
    }
  } 
  
  return -1;
}

/**
 * Get the index of next free datablock. Each bit determines if datablock is free (bit = 0) or occupied (bit = 1)
 */
int get_datablock(int index)
{
  int temp, count, i = 0;
  for (; i < sb.s_blocks_count / 8; i++) {
    if (block_bm[i] < 255) {
      count = 0;
      temp = block_bm[i];
      while ((temp & 1) == 1) {
	       count++;
	       temp >>= 1;
      }
      
      block_bm[i] = (block_bm[i]) | (1 << count);
      return i * 8 + count;
    }
  }

  if (sb.s_blocks_count % 8 != 0) {
    int bits = sb.s_blocks_count % 8;
    count    = 0;
    temp     = block_bm[i];
    while ((temp & 1) == 1) {
      count++;
      temp >>= 1;
    }
    
    if (count < bits) {
      block_bm[i] = (block_bm[i]) | (1 << count);
      return i * 8 + count;	
    }
  }
  
  inode_bm[index / 8] &= ~(1 << (index % 8));
  return -1;
}

/**
 * Write the updated bitmaps to the disk
 */
void update_bitmaps()
{
  fseek(fp, BLOCK_SIZE, SEEK_SET);
  fwrite(block_bm, 1, BLOCK_SIZE, fp);
  fwrite(inode_bm, 1, BLOCK_SIZE, fp);
}
