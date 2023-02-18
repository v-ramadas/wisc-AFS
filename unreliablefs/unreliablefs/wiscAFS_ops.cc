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
    RPCResponse ret = afsClient->GetAttr(s_path);
    if (ret.status() == 1) {
    	return -errno;
    }
    return 0;
}

int wiscAFS_mkdir(const char *path, mode_t mode)
{
    std::string s_path = path;
    RPCResponse ret = afsClient->CreateDir(s_path, mode);
    if (ret.status() == -1) {
        return -errno;
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
    char slog[1000];
    int fd = open("/users/vramadas/test.log", O_CREAT|O_RDWR|O_APPEND, 0777);
    //fprintf(stdout, "Here\n");
    write(fd, "New File!\n", strlen("New File\n!"));
    int ret = afsClient->OpenFile(s_path, fi->flags);
    if (ret == -1) {
        //fprintf(stdout, "Sorry!");
        write(fd, "Sorry!\n", strlen("Sorry\n!"));
        return -errno;
    }
    printf("wiscOPS: In wisAFS_open Ret = %d\n", ret);
    sprintf(slog, "In wisAFS_open Ret = %d\n", ret);
    write(fd, slog, strlen(slog));
//    write(fd, ret.data().c_str(), strlen(ret.data().c_str()));

    /*ret2 = open(path, fi->flags);
    if (ret2 == -1) {
        return -errno;
    }*/
    fi->fh = ret;

    return 0;
}

int wiscAFS_release(const char * path, struct fuse_file_info *fi) {
    close(fi->fh);
    std::string s_path(path);
    int fd = open("/users/vramadas/test.log", O_CREAT|O_RDWR|O_APPEND, 0777);
    int ret = afsClient->CloseFile(s_path);
    if (ret == -1) {
        write(fd, "Release Failed\n", strlen("Release Failed\n"));
        return -errno;
    }
    write(fd, "Release Passed\n", strlen("Release Passed\n"));
    close(fd);
    return 0;
}
int wiscAFS_read(const char * path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    return 0;
}

int wiscAFS_write (const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
    return 0;
}

int wiscAFS_statfs(const char *path, struct statvfs *buf)
{
    //TODO Implement
    int ret = statvfs(path, buf);
    if (ret == -1) {
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

    DIR *dp = opendir(path);
    if (dp == NULL) {
	return -errno;
    }
    struct dirent *de;

    (void) offset;
    (void) fi;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }
    closedir(dp);

    return 0;
}

int wiscAFS_create(const char *path, mode_t mode,
                      struct fuse_file_info *fi)
{
    char slog[1000];
    int fd = open("/users/vramadas/test.log", O_CREAT|O_RDWR|O_APPEND, 0777);
    write(fd, "Inside Create (Client)!\n", strlen("Inside Create (Client)!\n!"));
    std::string s_path = path;
    int ret = afsClient->OpenFile(s_path, mode);
    if (ret == -1) {
        write(fd, "Create Failed!\n", strlen("Create Failed\n!"));
        return -errno;
    }
    sprintf(slog, "In wisAFS_create Ret = %d\n\0", ret);
    write(fd, slog, strlen(slog));

    fi->fh = ret;

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
