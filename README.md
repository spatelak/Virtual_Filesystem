# Virtual_Filesystem

### What is a virtual filesystem?
It is an application that can be used as a filesystem in userspace using FUSE. FUSE is a userspace filesystem framework used to build filesystems in userspace without editting kernel code. The FUSE module provides only a "bridge" to the actual kernel interfaces.

### What it does?
This filesystem mounts a regular directory onto a mount point to appear as regular filesystem where one can read/write/create files, directories, symbolic links and hard links using regular linux commands like ls, cd, cat, mkdir, rm -rf, rm, echo, ln -s, rmdir

##### Note: This program implments a recursive version of rmdir command.

### Challneges I ran into
It was a difficult task to write and modularize more than 1000 lines of code.

### How to use Virtual_Filesystem?

#### Install FUSE on your system and follow the following steps.

1. Go to FilesystemDriver inside fuse_fs directory and run the following commands to build an image for virtual filesystem under fuse_fs directory:<br>
  (i)  make<br>
  (ii) ./simpleFS
  #### Note: This builds an image for filesystem that is 20 blocks large with each block of size 512 bytes.

2. Now under fuse_fs run make command to build a daemo.

3. Run: ./fusefs -s -d [mount_point]
 -> Here the mount_point refers to an empty scratch directory created by you.
 
 Use the mount_point in another terminal to run the linux command under the image.
