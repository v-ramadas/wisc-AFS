#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/file.h>

#include "wiscAFS_ops.h"
#include "unreliablefs_ops.h"
#include "wiscAFS.h"

#ifdef __cplusplus
extern "C" {
#endif

int wiscAFS_getattr(const char *path, struct stat *buf)
{
    memset(buf, 0, sizeof(struct stat));
    std::string s_path = path;
    printf("wiscAFS_getattr: Sending afsClient request\n");
    RPCResponse ret = afsClient->GetAttr(s_path);
    buf->st_dev = ret.fileinfo().st_dev();
    buf->st_ino = ret.fileinfo().st_ino();
    buf->st_mode = ret.fileinfo().st_mode();
    buf->st_nlink = ret.fileinfo().st_nlink();
    buf->st_uid = ret.fileinfo().st_uid();
    buf->st_gid = ret.fileinfo().st_gid();
    buf->st_rdev = ret.fileinfo().st_rdev();
    buf->st_size = ret.fileinfo().st_size();
    buf->st_blksize = ret.fileinfo().st_blksize();
    buf->st_blocks = ret.fileinfo().st_blocks();
    buf->st_atime = ret.fileinfo().st_atim();
    buf->st_mtime = ret.fileinfo().st_mtim();
    buf->st_ctime = ret.fileinfo().st_ctim();
    printf("wiscAFS_getattr: Set buf->mtim = %ld, ret.st_mtim = %ld\n", buf->st_mtime, ret.fileinfo().st_mtim());
    if (ret.status() == -1) {
    	return -ret.error();
    }
    return 0;
}

int wiscAFS_getxattr(const char *path, const char *name, char *value, size_t size)
{
    std::string s_path = path;
    std::string s_name = name;
    if (value == nullptr) {
        value = new char[1024];
    }
    printf("wiscAFS_getattr: Sending afsClient request\n");
    RPCResponse ret = afsClient->GetXAttr(s_path, s_name, value, size);
    if (ret.xattr_size() < 0) {
        return -ret.error();
    }
    memcpy(value, ret.xattr_value().c_str() ,ret.xattr_size()*sizeof(char));

    return 0;
}

int wiscAFS_mkdir(const char *path, mode_t mode)
{
    std::string s_path = path;
    printf("wiscAFS_mkdir: Sending afsClient request\n");
    RPCResponse ret = afsClient->CreateDir(s_path, mode);
    if (ret.status() == -1) {
        return -ret.error();
    }

    return 0;
}

int wiscAFS_unlink(const char* path)
{
    std::string s_path = path;
    RPCResponse ret = afsClient->DeleteFile(s_path);
    if (ret.status() == -1) {
	return -errno;
    }
    return 0;
}

int wiscAFS_rmdir(const char* path)
{
    std::string s_path = path;
    RPCResponse ret = afsClient->RemoveDir(s_path);
    if (ret.status() == -1) {
	return -errno;
    }
    
    return 0;
}

int wiscAFS_open(const char * path, struct fuse_file_info *fi)
{
    std::string s_path = path;
    printf("wiscOPS:Open: Sending openFile to client\n");
    int ret = afsClient->OpenFile(s_path, fi->flags);
    if (ret == -1) {
        return -errno;
    }
    printf("wiscOPS:Open: In wisAFS_open Ret = %d\n", ret);

    /*ret2 = open(path, fi->flags);
    if (ret2 == -1) {
        return -errno;
    }*/
    fi->fh = ret;

    return 0;
}

int wiscAFS_flush(const char * path, struct fuse_file_info *fi) {
    fsync(fi->fh);
    close(fi->fh);
    std::string s_path(path);
    int ret = afsClient->CloseFile(s_path);
    if (ret == -1) {
        return -errno;
    }
    return 0;
}
int wiscAFS_read(const char * path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    /*int ret = error_inject(path, OP_READ);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }*/

    int fd, ret;
    printf("wiscAFS_read: Path = %s\n", path);

    if (fi == NULL) {
	fd = open(path, O_RDONLY);
    } else {
	fd = fi->fh;
    }

    if (fd == -1) {
	return -errno;
    }

    ret = pread(fd, buf, size, offset);
    if (ret == -1) {
        ret = -errno;
    }
    printf("wiscAFS_read: buf = %s\n", buf);

    if (fi == NULL) {
	close(fd);
    }

    return ret;
}

int wiscAFS_write (const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
    std::string s_path = path;
    /*int ret = error_inject(path, OP_WRITE);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }*/
    int ret;

    int fd;
    (void) fi;
    if(fi == NULL) {
        fd = open(path, O_WRONLY);
    } else {
        fd = fi->fh;
    }

    if (fd == -1) {
        return -errno;
    }

    ret = pwrite(fd, buf, size, offset);
    if (ret == -1) {
        ret = -errno;
    }

    printf("wiscAFS_write: Calling afsClient writeFile, Path = %s\n", path);
    int afsRet = afsClient->WriteFile(s_path);
    if (afsRet == -1) {
        //fprintf(stdout, "Sorry!");
        printf("wiscAFS_write: afsRet = -1\n");
        write(fd, "Sorry!\n", strlen("Sorry\n!"));
        return -errno;
    }

    if(fi == NULL) {
        close(fd);
    }

    return ret;
}

int wiscAFS_statfs(const char *path, struct statvfs *buf)
{
    RPCResponse ret = afsClient->Statfs(path);
    buf->f_bsize = ret.statfs().f_bsize();
    buf->f_frsize = ret.statfs().f_frsize();
    buf->f_blocks = ret.statfs().f_blocks();
    buf->f_ffree = ret.statfs().f_ffree();
    buf->f_bavail = ret.statfs().f_bavail();
    buf->f_files = ret.statfs().f_files();
    buf->f_ffree = ret.statfs().f_ffree();
    buf->f_favail = ret.statfs().f_favail();
    buf->f_fsid = ret.statfs().f_fsid();
    buf->f_flag = ret.statfs().f_flag();
    buf->f_namemax = ret.statfs().f_namemax();

    if (ret.status() == -1) {
        return -errno;
    }


    return 0;
}

int wiscAFS_opendir(const char *path, struct fuse_file_info *fi)
{
    std::string s_path = path;
    //TODO : Figure out what to do here. Need to return directory pointer so that it can be set to fi
    DIR *dir = NULL;
    afsClient->OpenDir(s_path, fi->flags);

    if (!dir) {
        return -errno;
    }
    fi->fh = (int64_t) dir;

    return 0;
}

int wiscAFS_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi)
{
    //TODO: For now just copied original, change later
    int res = afsClient->ReadDir(path, buf, filler);
    return res;
}

int wiscAFS_create(const char *path, mode_t mode,
                      struct fuse_file_info *fi)
{
    std::string s_path = path;
    printf("wiscOPS:Open: Sending openFile to client\n");
    int ret = afsClient->CreateFile(s_path, fi->flags, mode);
    if (ret == -1) {
        return -errno;
    }
    printf("wiscOPS:Open: In wisAFS_open Ret = %d\n", ret);

    /*ret2 = open(path, fi->flags);
    if (ret2 == -1) {
        return -errno;
    }*/
    fi->fh = ret;

    return 0;    
}

int wiscAFS_truncate(const char *path, off_t length)
{
    /*int ret = error_inject(path, OP_TRUNCATE);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }*/

    /*int ret = truncate(path, length); 
    if (ret == -1) {
        return -errno;
    }*/

    return 0;
}

int wiscAFS_access(const char *path, int mode)
{
    std::string s_path = path;
    RPCResponse ret = afsClient->AccessFile(s_path, mode);
    if (ret.status() == -1) {
        return -ret.error();
    }
    //ret = access(path, mode); 
    
    return 0;
}

int wiscAFS_rename(const char* oldname, const char* newname) {

    std::string s_oldpath = oldname;
    std::string s_newpath = newname;
    RPCResponse ret = afsClient->RenameFile(s_oldpath, s_newpath);
    if (ret.status() == -1) {
        return -ret.error();
    }

    return 0;

}

void *wiscAFS_init(struct fuse_conn_info *conn) {
    afsClient = new wiscAFSClient (
      grpc::CreateChannel(std::string("10.10.1.2:50051"), grpc::InsecureChannelCredentials()));
    return NULL;
}
#ifdef __cplusplus
}
#endif
