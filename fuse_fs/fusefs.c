/*
  FUSE: Filesystem in Userspace
  This skeleton code is made from the function prototypes found in
  /usr/include/fuse/fuse.h Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPL v2.
*/

#include "fusefs.h"

static int sfs_delete(const char *path);

static void *sfs_mount(struct fuse_conn_info *conn) {
  
  open_filesystem("./filesystemImage", strlen("./filesystemImage"));

  if (fp == NULL) exit(1);
  
  return NULL;
}

static void sfs_unmount (void *private_data) {
  fclose(fp);
}


 static int sfs_getattr(const char *path, struct stat *stbuf)
{
  int parent_inode_num = validate_path((char *) path, 3);

  if (parent_inode_num < 0) {
    errno = -parent_inode_num;
    return -errno;
  }

  struct inode node;
  read_inode(&node, parent_inode_num);
  stbuf->st_mode   = node.i_mode;
  stbuf->st_nlink  = node.i_links_count;
  stbuf->st_uid    = node.i_uid;
  stbuf->st_gid    = node.i_gid;
  stbuf->st_size   = node.i_size;
  stbuf->st_blocks = node.i_blocks;
  stbuf->st_atime  = node.i_time;
  stbuf->st_mtime  = node.i_mtime;
  stbuf->st_ctime  = node.i_ctime;
  
  return 0;
}

static int sfs_mkdir(const char *path, mode_t mode)
{
  int result = make_directory((char *) path, strlen(path));

  if (result < 0) {
    errno = -result;
    return -errno;
  }
  
  return 0;
}


static int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi)
{
  char *data = (char *) malloc(BLOCK_SIZE);
  int bytes_read = read_directory((char *) path, strlen(path), data);

  if (bytes_read < 0) {
    errno = -bytes_read;
    return -errno;
  }
  
  int n = bytes_read / sizeof(struct directory_entry), i = 0;
  struct directory_entry dirents[n];
  memcpy(dirents, data, bytes_read);
  
  while (i < n) {
    struct stat st;
    struct inode node;
    read_inode(&node, dirents[i].d_inode);
    memset(&st, 0, sizeof(st));
    st.st_ino  = dirents[i].d_inode;
    st.st_mode = node.i_mode;
    if (filler(buf, dirents[i++].d_name, &st, 0)) break;
  }

  free(data);
  return 0;
}


static int sfs_create(const char *path, mode_t mode, dev_t rdev)
{
  int result = create_file((char *) path, strlen(path), 0, NULL);

  if (result < 0) {
    errno = -result;
    return -errno;
  }

  return 0;
}


static int sfs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{
  int bytes_read = read_file((char *) path, strlen(path), buf);

  if (bytes_read < 0) {
    errno = -bytes_read;
    return -errno;
  }
  
  return bytes_read;
}


static int sfs_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
  int parent_inode_num = validate_path((char *) path, 3);

  if (parent_inode_num == -1) {
    errno = ENOTDIR;
    return -errno;
  }

  if (parent_inode_num == -2) {
    errno = ENOENT;
    return -errno;
  }

  struct inode node;
  read_inode(&node, parent_inode_num);
  node.i_size += size;
  write_inode(&node, parent_inode_num);
  
  char *temp = create_path((char *)buf, size);
  int i = 0;
  for (i = 0; i < node.i_blocks - 1; i++) {
    write_data(temp, node.i_block[i], BLOCK_SIZE);
    temp += BLOCK_SIZE;
  }
  if (size % BLOCK_SIZE != 0) write_data(temp, node.i_block[i], size % BLOCK_SIZE);
  else                        write_data(temp, node.i_block[i], size);
  
  return size;
}

static int sfs_remove_dir(const char *path) 
{
  char *copy = strdup(path);
  int parent_inode_num = validate_path(copy, 3);

  if (parent_inode_num == -1) {
    errno = ENOTDIR;
    return -errno;
  }

  if (parent_inode_num == -2) {
    errno = ENOENT;
    return -errno;
  }

  struct inode node;
  read_inode(&node, parent_inode_num);

  int n = node.i_size / sizeof(struct directory_entry), i = 0, res;
  if (n > 2) {
    struct directory_entry entries[8];
    read_direntry(entries, node.i_block[0], n);

    for (i = 2; i < n; i++) {
      char new_path[strlen(path) + strlen(entries[i].d_name) + 2];
      strcpy(new_path, path);
      if (*(path + strlen(path) - 1) != '/') strcat(new_path, "/");
      strcat(new_path, entries[i].d_name);

      if (entries[i].d_file_type == 2) res = sfs_remove_dir(new_path);
      else                             res = sfs_delete(new_path);

      if (res != 0) {
	errno = -res;
	return -errno;
      }
    }
  }

  int result = rm_directory((char *) path, strlen(path));

  if (result < 0) {
    errno = -result;
    return -errno;
  }
  
  return 0;
}

static int sfs_delete(const char *path) 
{
  int result = rm_file((char *) path, strlen(path));

  if (result < 0) {
    errno = -result;
    return -errno;
  }
  
  return 0;
}

static int sfs_symlink(const char *from, const char *to) 
{
  int result = create_file((char *) to, strlen(to), strlen(from), (char *) from);

  if (result < 0) {
    errno = -result;
    return -errno;
  }
  
  int inum = validate_path(strdup(to), 3);

  struct inode node;
  read_inode(&node, inum);
  node.i_mode = S_IFLNK | S_IRWXU | S_IRWXG | S_IRWXO;

  write_inode(&node, inum);
  
  return 0;
}

static int sfs_readlink(const char *path, char *buf, size_t size)
{
  int bytes_read  = read_file((char *) path, strlen(path), buf);

  if (bytes_read < 0) {
    errno = -bytes_read;
    return -errno;
  }
  buf[bytes_read] = '\0';
  
  return 0;
}



static struct fuse_operations sfs_oper = {
    .init    = sfs_mount,
    .destroy = sfs_unmount,
    .getattr = sfs_getattr,
    .mkdir	 = sfs_mkdir,
    .readdir = sfs_readdir,
    .symlink = sfs_symlink,
    .readlink = sfs_readlink,
    .mknod	 = sfs_create,
    .read	 = sfs_read,
    .write	 = sfs_write,
    .unlink	 = sfs_delete,
    .rmdir       = sfs_remove_dir,
};

int main(int argc, char *argv[])
{
    umask(0); 
    return fuse_main(argc, argv, &sfs_oper, NULL);
}

