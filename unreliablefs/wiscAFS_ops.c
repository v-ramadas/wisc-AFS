#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/file.h>

#include "wiscAFS_ops.h"
#include "unreliable_ops.h"

int wiscAFS_getattr(const char *path, struct stat *buf)
{
    memset(buf, 0, sizeof(struct stat));
    if (afsClient.GetAttr(path, bug) == 1) {
	return -errno;
    }
    return 0;
}

int wiscAFS_mkdir(const char *path, mode_t mode)
{
    int ret = afsClient.CreateDir(path, mode);
    if (ret == -1) {
        return -errno;
    }

    return 0;
}

int wiscAFS_unlink(const char* path)
{
    int ret = afsClient.DeleteFile(path);
    if (ret == -1) {
	return -errno;
    }
    return 0;
}

int wiscAFS_rmdir(const char* path)
{
    int ret = afsClient.RemoveDir(path);
    if (ret == -1) {
	return -errno;
    }
    
    return 0;
}

int wiscAFS_open(const char * path, struct fuse_file_info *fi)
{
    int ret = afsClient.OpenFile(path, fi->flags);
    if (ret == -1) {
        return -errno;
    }

    fi->fh = ret;
    
    return 0;
}

int wiscAFS_read(const char * path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int fd;

    if (fi == NULL) {
	fd = afsClient.OpenFile(path, O_RDONLY);
    } else {
	fd = fi->fh;
    }

    if (fd == -1) {
	return -errno;
    }

    ret = afsClient.ReadFile(fd, buf, size, offset);
    if (ret == -1) {
        ret = -errno;
    }

    if (fi == NULL) {
	close(fd);
    }

    return ret;
}

int wiscAFS_write (const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
    int fd;
    (void) fi;
    if(fi == NULL) {
	fd = afsClient.OpenFile(path, O_WRONLY);
    } else {
	fd = fi->fh;
    }

    if (fd == -1) {
	return -errno;
    }

    ret = afsClient.WriteFile(fd, buf, size, offset);
    if (ret == -1) {
        ret = -errno;
    }

    if(fi == NULL) {
        close(fd);
    }

    return ret;
}

int wiscAFS_statfs(const char *path, struct statvfs *buf)
{
    //TODO Implement

    return 0;
}

int wiscAFS_opendir(const char *path, struct fuse_file_info *fi)
{
    DIR *dir = afsClient.OpenDir(path);

    if (!dir) {
        return -errno;
    }
    fi->fh = (int64_t) dir;

    return 0;
}

int wiscAFS_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi)
{
//    TODO : Implement later if needed
//    DIR *dp = afsClient.OpenDir(path);
//    if (dp == NULL) {
//	return -errno;
//    }
//    struct dirent *de;
//
//    (void) offset;
//    (void) fi;
//
//    while ((de = afsClient.ReadDir(dp)) != NULL) {
//        struct stat st;
//        memset(&st, 0, sizeof(st));
//        st.st_ino = de->d_ino;
//        st.st_mode = de->d_type << 12;
//        if (filler(buf, de->d_name, &st, 0))
//            break;
//    }
//    closedir(dp);

    return 0;
}

int wiscAFS_create(const char *path, mode_t mode,
                      struct fuse_file_info *fi)
{
    int ret = wiscAFS.CreateDir(path, fi->flags, mode);
    if (ret == -1) {
        return -errno;
    }
    fi->fh = ret;

    return 0;    
}
