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
int wiscAFS_flush(const char *, struct fuse_file_info *);
int wiscAFS_read(const char *, char *, size_t, off_t, struct fuse_file_info *);
int wiscAFS_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);
int wiscAFS_statfs(const char *, struct statvfs *);
int wiscAFS_opendir(const char *, struct fuse_file_info *);
int wiscAFS_readdir(const char *, void *buf, fuse_fill_dir_t, off_t, struct fuse_file_info*);
int wiscAFS_create(const char*, mode_t, struct fuse_file_info *);

void* wiscAFS_init(struct fuse_conn_info*);
int wiscAFS_getxattr(const char *path, const char *name, char *value, size_t size);
int wiscAFS_truncate(const char *path, off_t length);
int wiscAFS_access(const char *path, int mode);
int wiscAFS_rename(const char* , const char*);
int temp_fn();
#ifdef __cplusplus
}
#endif
