#ifndef AFSWISC_OPS_HH
#define AFSWISC_OPS_HH

#include <fuse.h>
#include "wiscAFS.grpc.pb.h"

int wiscAFS_getattr(const char *, struct stat *);
int wiscAFS_mkdir(const char *, mode_t);
int wiscAFS_unlink(const char *);
int wiscAFS_rmdir(const char *);
int wiscAFS_open(const char *, struct fuse_file_info *);
int wiscAFS_read(const char *, char *, size_t, off_t, struct fuse_file_info *);
int wiscAFS_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);
int wiscAFS_statfs(const char *, struct statvfs *);
int wiscAFS_openddir(const char *, struct fuse_file_info *);
int wiscAFS_readdir(const char *, void *buf, fuse_fill_dir_t);
int wiscAFS_create(const char*, mode_t, struct fuse_file_info *);

#endif /* AFSWISC_OPS_HH */
