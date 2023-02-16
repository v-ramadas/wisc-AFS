#pragma once
#include <fuse.h>

#ifdef __cplusplus
extern "C" {
#endif
int wiscAFS_getattr(const char *, struct stat *);
int wiscAFS_mkdir(const char *, mode_t);
int wiscAFS_unlink(const char *);
int wiscAFS_rmdir(const char *);
int wiscAFS_open(const char *, struct fuse_file_info *);
int wiscAFS_read(const char *, char *, size_t, off_t, struct fuse_file_info *);
int wiscAFS_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);
int wiscAFS_statfs(const char *, struct statvfs *);
int wiscAFS_opendir(const char *, struct fuse_file_info *);
int wiscAFS_readdir(const char *, void *buf, fuse_fill_dir_t, off_t, struct fuse_file_info*);
int wiscAFS_create(const char*, mode_t, struct fuse_file_info *);

int init_wiscAFS(const char*);

int temp_fn();
#ifdef __cplusplus
}
#endif
