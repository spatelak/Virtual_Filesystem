#include "simpleFS.h"
#include "helper.h"

/**
 *
 */
// Initialize a filesystem of the size specified in number of data blocks
void init_filesystem(unsigned int size, char *real_path, unsigned int n)
{
  /*
   * Initalize a superblock. Write that to disk.
   * Create the bitmaps for block_bm and inode_bm.
   * Write both to disk
   * Make a 5 block large inode table.
   * Then initialize the "size" datablocks.
   *
   * Then have a file system ready.
   * with a parent directory of '/' at inode 2.
   */

  // error check
  if (size > N_INODES - 2) {
    printf("Filesystem is too large to initialize\n");
    exit(1);
    //return;
  }
  
  // create path and open file
  char *npath = create_path(real_path, n);
  if (npath == NULL) exit(1); //return;
  fp = fopen(npath, "w+");

  // initialize super block and write that to disk
  sb.s_inodes_count      = N_INODES;
  sb.s_blocks_count      = size;
  sb.s_free_inodes_count = N_INODES - 3;
  sb.s_free_blocks_count = size - 1;
  sb.s_first_data_block  = START_DATA; 
  sb.s_first_ino         = START_INODE;
  sb.s_magic             = MAGIC_SIGN;

  char padding[BLOCK_SIZE] = "";
  memcpy(padding, &sb, sizeof(struct superblock));
  fwrite(padding, BLOCK_SIZE, 1, fp);
    
  // create bitmaps and write them to the disk image
  int i;
  for (i = 0; i < BLOCK_SIZE; i++) block_bm[i] = 0;
  for (i = 0; i < BLOCK_SIZE; i++) inode_bm[i] = 0;
  inode_bm[0] = 7;
  
  // create 5 block inode table (40 inodes), initialize inode 2 for root directory and write that to disk image 
  struct inode inodes[N_INODES];
  init_inode(&inodes[2], 2, sizeof(struct directory_entry) * 2, 2);
  update_bitmaps();
  
  char padding1[BLOCK_SIZE * 5] = "";
  memcpy(padding1, inodes, sizeof(struct inode) * N_INODES);
  fwrite(padding1, BLOCK_SIZE * 5, 1, fp);

  
  // initialize and write "size" data blocks to disk image
  char padding2[BLOCK_SIZE] = "";
  struct directory_entry root_dir[MAX_DIRENT];
  init_direntry(root_dir, 2, 2);
  memcpy(padding2, root_dir, sizeof(struct directory_entry) * 2);
  fwrite(padding2, BLOCK_SIZE, 1, fp);

  for (i = 0; i < BLOCK_SIZE * (size - 1); i++) {
    char *c = "\0";
    fwrite(c, 1, 1, fp);
  }
}

/**
 *
 */
void open_filesystem(char *real_path, unsigned int n)
{
  /*
   * Open an existing file system image from disk.
   * Read the file system information from the disk and initialize the
   * in-memory variables to match what was on disk.
   * use this file system for everything from now on.
   * Fail if the magic signature doesn't match
   */

  // create path
  char *npath = create_path(real_path, n);
  if (npath == NULL) exit(1);

  // open the file and read the information from the disk and initialize the in-memory variables
  fp = fopen(npath, "r+");
  if (fp == NULL) {
    printf("Path %s does not exist\n", npath);
    exit(1);
  }

  // read super block and fail if magic signature does not match
  char block[BLOCK_SIZE] = "";
  fread(block, BLOCK_SIZE, 1, fp);
  memcpy(&sb, block, sizeof(struct superblock));
  
  if (sb.s_magic != MAGIC_SIGN) {
    printf("Wrong filesystem - Magic signature does not match\n");
    fclose(fp);
    exit(1);
  }

  // read the bitmaps
  fread(block_bm, 1, BLOCK_SIZE, fp);
  fread(inode_bm, 1, BLOCK_SIZE, fp);
}

/**
 *
 */
int my_create(char *path, unsigned int n, int size, char *data, int type)
{
  // 1. create and validate path
  if (*(path + n - 1) == '/' && type == 1) {
    printf("Invalid path - file path cannot end with /\n");
    return -ENOENT;
    //exit(1);
  }
  
  char *npath = create_path(path, n);
  if (npath == NULL || strlen(npath) == 0) return -ENOMEM;//exit(1);
   
  char *name = npath + 1;
  char *prev = npath;
  while (*name != '\0') {
    if (*name == '/') prev = name;
    name++;
  }
  
  char *temp       = strndup(npath, strlen(npath) - strlen(prev));
  int parent_index = validate_path(temp, 2);
  free(temp);
  
  if (parent_index < 0) return parent_index;//exit(1);
   
  // 2.0 get the parent inode
  struct inode parent;
  read_inode(&parent, parent_index); 
 
  // check permissions
  if (parent.i_uid == getuid()) {
    if (check_permissions(parent.i_mode, S_IRUSR) == 0 || check_permissions(parent.i_mode, S_IWUSR) == 0) {
      printf("User does not have read/write permissions\n");
      return -EACCES;
      //exit(1);
    }
  }
  else if (parent.i_gid == getgid()) {
    if (check_permissions(parent.i_mode, S_IRGRP) == 0 || check_permissions(parent.i_mode, S_IWGRP) == 0) {
      printf("Group does not have read/write permissions\n");
      return -EACCES;
      //exit(1);   
    }
  }
  else {
    if (check_permissions(parent.i_mode, S_IROTH) == 0 || check_permissions(parent.i_mode, S_IWOTH) == 0) {
      printf("Other does not have read/write permissions\n");
      return -EACCES;
      //exit(1);   
    }
  }
  
  // if max directories already present, then don't create new
  if (parent.i_size >= sizeof(struct directory_entry) * MAX_DIRENT) {
    printf("Cannot add more directories to this path\n");
    return -ENOSPC;
    //exit(1);   
  }

  // 2.1 get parent directory entries and check if target already exists
  int entries = parent.i_size / sizeof(struct directory_entry), j = 0;
  struct directory_entry dir[MAX_DIRENT];
  read_direntry(dir, parent.i_block[0], MAX_DIRENT);
  for (j = 0; j < entries; j++) {
    if (strcmp(dir[j].d_name, prev + 1) == 0) {
      printf("Target already exists\n");
      return -EEXIST;
      //exit(1);   
    }
  }
  
  // 3. get index of new inode for new directory
  int index = get_inode();
  if (index == -1) {
    printf("Disk is full\n");
    return -EDQUOT;
    //exit(1);   
  }

  // 4. make new inode and write that to inode table
  struct inode *child_inode = (struct inode *) malloc(sizeof(struct inode));
  if (type == 2) init_inode(child_inode, 2, sizeof(struct directory_entry) * 2, index);
  else           init_inode(child_inode, 1, size, index);
  
  write_inode(child_inode, index);
    
  // 5. write the data block for new directory/file to disk
  if (type == 2) {
    struct directory_entry new_dir[MAX_DIRENT];
    init_direntry(new_dir, index, parent_index);
    write_direntry(new_dir, child_inode->i_block[0], 2);
  }
  else if (data != NULL) {
    char *temp = create_path(data, size);
    int i      = 0;
    for (i = 0; i < child_inode->i_blocks - 1; i++) {
      write_data(temp, child_inode->i_block[i], BLOCK_SIZE);
      temp += BLOCK_SIZE;
    }
    if (size % BLOCK_SIZE != 0) write_data(temp, child_inode->i_block[i], size % BLOCK_SIZE);
    else                        write_data(temp, child_inode->i_block[i], size);  
  }
  
  // 6. add new directory entry to parent direcrtory's data block and write it back to disk
  dir[entries].d_inode     = index;
  dir[entries].d_file_type = type;
  dir[entries].d_name_len  = strlen(prev + 1);

  char pad[57] = "";
  strcpy(dir[entries].d_name, pad);
  strncpy(dir[entries].d_name, prev  + 1, strlen(prev + 1));
 
  write_direntry(dir, parent.i_block[0], entries + 1); 
  
  // 7. change the size and times of parent inode and write it back to disk
  time_t t = time(NULL);
  
  parent.i_size   += sizeof(struct directory_entry);
  parent.i_time    = t;
  parent.i_mtime   = t;
  write_inode(&parent, parent_index);
  
  // 8. change number of free inodes and block in SB and write it back to image
  update_superblock(0, child_inode->i_blocks);

  // 9. WRITE BACK THE UPDATED BITMAPS
  update_bitmaps();
  
  free(child_inode);

  return 0;
}

/**
 *
 */
unsigned int my_read(char *path, unsigned int n, char *data, int type)
{
  // 1. create and validate path
  if (*(path + n - 1) == '/' && type == 1) {
    printf("Invalid path\n");
    return -ENOENT;
    //exit(1);   
  }
  char *npath = create_path(path, n);
  if (npath == NULL) return -ENOMEM;//exit(1);

  char *temp = strdup(npath);
  int parent_index = validate_path(temp, type);
  free(temp);
  if (parent_index < 0) return parent_index;//exit(1);

  // 2. get the parent inode
  struct inode parent;
  read_inode(&parent, parent_index);

  if (type == 1 && (S_ISREG(parent.i_mode) == 0 && S_ISLNK(parent.i_mode) == 0)) return -EISDIR;
  
  // check permissions
  if (parent.i_uid == getuid()) {
    if (check_permissions(parent.i_mode, S_IRUSR) == 0) {
      printf("User does not have read permission\n");
      return -EACCES;;
      //exit(1);   
    }
  }
  else if (parent.i_gid == getgid()) {
    if (check_permissions(parent.i_mode, S_IRGRP) == 0) {
      printf("Group does not have read permission\n");
      return -EACCES;;
      //exit(1);   
    }
  }
  else {
    if (check_permissions(parent.i_mode, S_IROTH) == 0) {
      printf("Other does not have read permission\n");
      return -EACCES;
      //exit(1);   
    }
  }

  int i = 0, bytes_read = 0;
  char *temp1 = data;

  if (parent.i_size > 0 ) {
    fseek(fp, START_DATA_ADDR + parent.i_block[0] * BLOCK_SIZE, SEEK_SET);
    
    for (i = 0; i < parent.i_blocks - 1; i++) {
      bytes_read += read_data(temp1, parent.i_block[i], BLOCK_SIZE);
      temp1       += BLOCK_SIZE;
    }
    bytes_read = (parent.i_size % BLOCK_SIZE == 0) ?  bytes_read + read_data(temp1, parent.i_block[i], BLOCK_SIZE) : bytes_read + read_data(temp1, parent.i_block[i], parent.i_size % BLOCK_SIZE);
  }  
  // update access time of parent inode
  parent.i_time = time(NULL);
  write_inode(&parent, parent_index);
 
  return bytes_read;
}

/**
 *
 */
int my_remove(char *path, unsigned int n, int type)
{
  int blocks = 0;
  
  // 1. create and validate path
  if (*(path + n - 1) == '/' && type == 1) {
    printf("Invalid path\n");
    return -ENOENT;
  }
  char *npath = create_path(path, n);
  if (npath == NULL) return -ENOMEM;

  if (strlen(npath) == 0) {
    printf("Cannot remove root directory\n");
    return -EBUSY;
  }
  
  char *name = npath + 1;
  char *prev = npath;
  while (*name != '\0') {
    if (*name == '/') prev = name;
    name++;
  }
  
  char *temp       = strndup(npath, strlen(npath) - strlen(prev));
  int parent_index = validate_path(temp, 2);
  free(temp);
 
  if (parent_index < 0) return parent_index;
 
  // 2. get the parent inode
  struct inode parent;
  read_inode(&parent, parent_index);
 
  // check permissions
  if (parent.i_uid == getuid()) {
    if (check_permissions(parent.i_mode, S_IRUSR) == 0 || check_permissions(parent.i_mode, S_IWUSR) == 0) {
      printf("User does not have read/write permissions\n");
      return -EACCES;
    }
  }
  else if (parent.i_gid == getgid()) {
    if (check_permissions(parent.i_mode, S_IRGRP) == 0 || check_permissions(parent.i_mode, S_IWGRP) == 0) {
      printf("Group does not have read/write permissions\n");
      return -EACCES;
    }
  }
  else {
    if (check_permissions(parent.i_mode, S_IROTH) == 0 || check_permissions(parent.i_mode, S_IWOTH) == 0) {
      printf("Other does not have read/write permissions\n");
      return -EACCES;
    }
  }
  
  // 3. get the parent data block
  int i, entries = parent.i_size / sizeof(struct directory_entry);
  struct directory_entry dir[MAX_DIRENT];
  read_direntry(dir, parent.i_block[0], MAX_DIRENT);

  for (i = 0; i < entries; i++) {
    if (strcmp(dir[i].d_name, prev + 1) == 0) {
      if (dir[i].d_file_type != type) {
	if (type == 2) {
	  printf("%s is not a directory\n", npath);
	  return -ENOTDIR;
	}
	else {
	  printf("%s is not a file\n", npath);
	  return -EISDIR;
	}
      }      
      else {
	struct inode child;
	read_inode(&child, dir[i].d_inode);
	
	// check permissions
	if (child.i_uid == getuid()) {
	  if (check_permissions(child.i_mode, S_IRUSR) == 0 || check_permissions(child.i_mode, S_IWUSR) == 0) {
	    printf("User does not have read/write permissions\n");
	    return -EACCES;
	  }
	}
	else if (child.i_gid == getgid()) {
	  if (check_permissions(child.i_mode, S_IRGRP) == 0 || check_permissions(child.i_mode, S_IWGRP) == 0) {
	    printf("Group does not have read/write permissions\n");
	    return -EACCES;
	  }
	}
	else {
	  if (check_permissions(child.i_mode, S_IROTH) == 0 || check_permissions(child.i_mode, S_IWOTH) == 0) {
	    printf("Other does not have read/write permissions\n");
	    return -EACCES;
	  }
	}
	
	if (type == 2 && child.i_size > sizeof (struct directory_entry) * 2) {
	  printf("directory is not empty\n");
	  return -ENOTEMPTY;
	}
	
	// UPDATE Child Inodes delete time AND WRITE BACK TO DISK
	if (child.i_links_count == 1) {
	  int j;
	  for (j = 0; j < child.i_blocks; j++) {
	    block_bm[child.i_block[j] / 8] &= ~(1 << (child.i_block[j] % 8));
	  }

	  inode_bm[dir[i].d_inode / 8] &= ~(1 << (dir[i].d_inode % 8));
	  blocks                        = child.i_blocks;
	  child.i_dtime                 = time(NULL);
	}
	else if (child.i_links_count > 1) {
	  time_t t = time(NULL);
	  child.i_time         = t;
	  child.i_mtime        = t;
	  child.i_links_count -= 1;
	}
	
	write_inode(&child, dir[i].d_inode);
	break;
      }
    }
  }
  
  if (i == entries) return -ENOENT;

  // 4. move over the entries and write the parent inode and data block back to disk
  while (i < entries - 1) {
    dir[i] = dir[i + 1];
    i++;
  }
  time_t t       = time(NULL);
  parent.i_time  = t;
  parent.i_mtime = t;
  parent.i_dtime = t;
  parent.i_size -= sizeof(struct directory_entry);

  write_inode(&parent, parent_index);
  write_direntry(dir, parent.i_block[0], entries - 1);

  // 5. update superblock
  update_superblock(1, blocks);

  // 6. WRITE BACK THE UPDATED BITMAPS
  update_bitmaps();

  return 0;
}

/*                                                                                                                                                                              
 * For the file system image that is currently opened.                                                                                                                           
 * Make a new directory at the path provided.                                                                                                                                    
 * Make sure that the path is valid.                                                                                                                                            
 */
/**
 *
 */
int make_directory(char *path, unsigned int n)
{
  return my_create(path, n, 0, NULL, 2);
}

/*                                                                                                                                                                               
 * For the file system image that is currently opened.                                                                                                                           
 * Read the contents of the directory in the path provided.                                                                                                                      
 * Make sure that the path is valid.                                                                                                                                             
 * Place the directory entries in data.                                                                                                                                          
 * and return the number of bytes that have been placed in data.                                                                                                                 
 */
/**
 *
 */
unsigned int read_directory(char *path, unsigned int n, char *data)
{
  return my_read(path, n, data, 2);
}

/*                                                                                                                                                                               
 * For the file system image that is currently opened.                                                                                                                           
 * Delete the directory in the path provided.                                                                                                                                    
 * Make sure that the directory doesn't have any files remaining                                                                                                              
 */
/**
 *
 */
int rm_directory(char *path, unsigned int n)
{
  return my_remove(path, n, 2);
}

/*                                                                                                                                                                            
 * For the file system image that is currently opened.                                                                                                                        
 * Create a new file at path.                                                                                                                                                 
 * Make sure that the path is valid.                                                                                                                                          
 * Write to the contents of the file in the data provided.                                                                                                                    
 * size is the number of bytes in the data.                                                                                                                                   
 */
/**
 *
 */
int create_file(char *path, unsigned int n, unsigned int size, char *data)
{
  return my_create(path, n, size, data, 1);  
}

/*                                                                                                                                                                            
 * For the file system image that is currently opened.                                                                                                                        
 * Delete the file in the path provided.                                                                                                                                      
 * Make sure that the data blocks are freed after deleting the file.                                                                                                          
 */
/**
 *
 */
int rm_file(char *path, unsigned int n)
{
  return my_remove(path, n, 1);
}


/*                                                                                                                                                                           
 * For the file system image that is currently opened.                                                                                                                        
 * Read the contents of the file in the path provided.                                                                                                                        
 * Make sure that the path is valid.                                                                                                                                          
 * Place the file contents in data                                                                                                                                            
 * and return the number of bytes in the file.                                                                                                                                
 */
/**
 *
 */
unsigned int read_file(char *path, unsigned int n, char *data)
{
  return my_read(path, n, data, 1);
}

/*                                                                                                                                                                            
 * make a new hard link in the path to target                                                                                                                                 
 * make sure that the path and target are both valid.                                                                                                                         
 */
/**
 *
 */
int make_link(char *path, unsigned int n, char *target)
{ 
  // 1. create and validate path
  if (*(path + n - 1) == '/') {
    printf("Invalid path\n");
    return -1;
    //exit(1);   
  }
  char *npath = create_path(path, n);
  if (npath == NULL) return -1;//exit(1);

  char *name = npath + 1;
  char *prev = npath;
  while (*name != '\0') {
    if (*name == '/') prev = name;
    name++;
  }

  char *temp       = strndup(npath, strlen(npath) - strlen(prev));
  int link_parent_index = validate_path(temp, 2);
  free(temp);

  if (link_parent_index < 0) return -1;//exit(1);
  
  // 2. create and validate path of target
  if (*(target + strlen(target) - 1) == '/') {
    printf("Invalid path\n");
    return -1;
    //exit(1);   
  }

  char *temp1 = strdup(target);
  int target_parent_index = validate_path(temp1, 1);
  free(temp1);
  if (target_parent_index < 0) return -1;//exit(1);
  
  // 3. get path's parent inode
  struct inode path_inode;
  read_inode(&path_inode, link_parent_index);

  // check permissions for path's inode
  if (path_inode.i_uid == getuid()) {
    if (check_permissions(path_inode.i_mode, S_IRUSR) == 0 || check_permissions(path_inode.i_mode, S_IWUSR) == 0) {
      printf("User does not have read/write permissions\n");
      return -1;
      //exit(1);   
    }
  }
  else if (path_inode.i_gid == getgid()) {
    if (check_permissions(path_inode.i_mode, S_IRGRP) == 0 || check_permissions(path_inode.i_mode, S_IWGRP) == 0) {
      printf("Group does not have read/write permissions\n");
      return -1;
      //exit(1);   
    }
  }
  else {
    if (check_permissions(path_inode.i_mode, S_IROTH) == 0 || check_permissions(path_inode.i_mode, S_IWOTH) == 0) {
      printf("Other does not have read/write permissions\n");
      return -1;
      //exit(1);   
    }
  }
  
  // If max directories already present, then don't create new
  if (path_inode.i_size >= sizeof(struct directory_entry) * MAX_DIRENT) {
    printf("Cannot add more directories to this path\n");
    return -1;
    //exit(1);   
  }

  // 2.1 get parent directory entries and check if target already exists
  int entries = path_inode.i_size / sizeof(struct directory_entry), j = 0;
  struct directory_entry dir[MAX_DIRENT];
  read_direntry(dir, path_inode.i_block[0], MAX_DIRENT);
  for (j = 0; j < entries; j++) {
    if (strcmp(dir[j].d_name, prev + 1) == 0) {
      printf("Target already exists\n");
      return -1;
      //exit(1);   
    }
  }

  // 3. get target's parent inode
  struct inode target_inode;
  read_inode(&target_inode, target_parent_index);

  // check permissions for path's inode
  if (target_inode.i_uid == getuid()) {
    if (check_permissions(target_inode.i_mode, S_IRUSR) == 0 || check_permissions(target_inode.i_mode, S_IWUSR) == 0) {
      printf("User does not have read/write permissions\n");
      return -1;
      //exit(1);   
    }
  }
  else if (target_inode.i_gid == getgid()) {
    if (check_permissions(target_inode.i_mode, S_IRGRP) == 0 || check_permissions(target_inode.i_mode, S_IWGRP) == 0) {
      printf("Group does not have read/write permissions\n");
      return -1;
      //exit(1);   
    }
  }
  else {
    if (check_permissions(target_inode.i_mode, S_IROTH) == 0 || check_permissions(target_inode.i_mode, S_IWOTH) == 0) {
      printf("Other does not have read/write permissions\n");
      return -1;
      //exit(1);   
    }
  }
  
  // 6. add new directory entry to path's parent direcrtory's data block and write it back to disk
  dir[entries].d_inode     = target_parent_index;
  dir[entries].d_file_type = 1;
  dir[entries].d_name_len  = strlen(prev + 1);
  strcpy(dir[entries].d_name, prev  + 1);

  write_direntry(dir, path_inode.i_block[0], entries + 1);

  // 7. change the size and times of path's parent inode and write it back to disk
  time_t t = time(NULL);

  path_inode.i_size   += sizeof(struct directory_entry);
  path_inode.i_time    = t;
  path_inode.i_mtime   = t;
  
  write_inode(&path_inode, link_parent_index);
  

  // 7. change the links and times of target inode and write it back to disk
  time_t t1 = time(NULL);

  target_inode.i_links_count += 1;
  target_inode.i_time         = t1;
  target_inode.i_mtime        = t1;
  
  write_inode(&target_inode, target_parent_index);

  return 0;
}
