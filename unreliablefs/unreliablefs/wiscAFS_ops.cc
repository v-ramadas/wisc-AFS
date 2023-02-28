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

extern int error_inject(const char* path, fuse_op operation);

void pushQueue(struct queueNode **q,  int op, const char* buf, size_t size, off_t offset, int fd){
    struct queueNode *newNode = (struct queueNode*)malloc(sizeof(struct queueNode));
    newNode->op = op;
    newNode->buf = (char*)malloc(sizeof(char)*(size+1));
    memcpy(newNode->buf, buf, size);
    newNode->size = size;
    newNode->offset = offset;
    newNode->fd = fd;
    newNode->next = NULL;
    struct queueNode *ptr = *q; 
    if(*q == NULL)	*q = newNode;
    else
    {
        while(ptr->next != NULL) ptr = ptr->next;
        ptr->next = newNode;
    }
}

int findInQueue(struct queueNode **q,  int op){
	struct queueNode *ptr = *q;
	int index = 0;
	if(*q == NULL) {
        return -1;
    }
	while(ptr != NULL)
	{
		if(ptr->op == op) 
			return index;
		ptr = ptr->next;
		index++;
	}
	return -1;
}

void delFromQueue(struct queueNode **q, int index)
{
	struct queueNode *ptr = *q;
	struct queueNode *prev = NULL;
	int pos = 0;
	if(index == 0)
	{
		*q = (*q)->next;
		//free(ptr);
		return;
	}
	while(ptr!= NULL)
	{
		if(pos == index)
		{
			prev->next = ptr->next;
			//free(ptr);
			return;
		}
		prev = ptr;
		ptr = ptr->next;
		pos++;
	}
	return;
}

int sizeQueue(struct queueNode **q)
{
    struct queueNode *ptr = *q;
    int count = 0;
    if(ptr == NULL)
        return 0;
    while(ptr != NULL)
    {
        ptr = ptr->next;
        count++;
    }
    return count;
}

void clearQueue(struct queueNode **q)
{
    struct queueNode *ptr = *q;
    if(ptr == NULL)
        return;
    while (*q != NULL)
    {
        *q = (*q)->next;
        //free(ptr);
        ptr = *q;
    }

}
void printQueue(struct queueNode *q)
{
    struct queueNode *ptr = q;
    while(ptr!=NULL) {
        printf("%d,", ptr->op);
        ptr = ptr->next;
    }
    printf("\n");
}
struct queueNode* getValueQueue(struct queueNode **q, int index)
{
    struct queueNode *ptr = *q;
    if (ptr == NULL)
        return NULL;
    for(int i = 0; i < index; i++)
    {
        ptr = ptr->next;
    }
    return (ptr);
}

int wiscAFS_getattr(const char *path, struct stat *buf)
{
    sem_wait(&wiscOPSem);
    memset(buf, 0, sizeof(struct stat));
    std::string s_path = path;
    printf("wiscAFS_getattr: Start path = %s, ino = %ld\n", path, buf->st_ino);
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
    printf("wiscAFS_getattr: End path = %s, ino = %ld\n", path, buf->st_ino);
    if (ret.status() == -1) {
        printf("wiscAFS_getattr: Ret status -1, path = %s, ino = %ld\n", path, buf->st_ino);
        sem_post(&wiscOPSem);
    	return -ret.error();
    }
    sem_post(&wiscOPSem);
    return 0;
}

int wiscAFS_fgetattr(const char* path, struct stat* buf, struct fuse_file_info* fi)
{
    if (fi == nullptr){
        printf("wiscAFS_fgetattr fd null, path = %s\n", path);
        return wiscAFS_getattr(path, buf);
    }
    else {
        printf("wiscAFS_fgetattr fd = %ld, path = %s\n", fi->fh, path);
        int ret = fstat((int) fi->fh, buf);
        if (ret == -1) {
            printf("MYERROR: wiscAFS_fgetattr failed for fd = %ld, path = %s\n", fi->fh, path);
            return -errno;
        }
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
    printf("wiscAFS_mkdir: path = %s, mode = %d\n", path, mode);
    RPCResponse ret = afsClient->CreateDir(s_path, mode);
    if (ret.status() == -1) {
        printf("MYERROR: wiscAFS_mkdir: failed path = %s, mode = %d\n", path, mode);
        return -ret.error();
    }

    return 0;
}

int wiscAFS_unlink(const char* path)
{
    sem_wait(&wiscOPSem);
    printf("wiscAFS_unlink: path = %s\n", path);
    std::string s_path = path;
    RPCResponse ret = afsClient->DeleteFile(s_path);
    if (ret.status() == -1) {
        printf("MYERROR: wiscAFS_unlink: failed path = %s\n", path);
        sem_post(&wiscOPSem);
        return -ret.error();
    }
    sem_post(&wiscOPSem);
    return 0;
}

int wiscAFS_rmdir(const char* path)
{
    printf("wiscAFS_rmdir: path = %s\n", path);
    std::string s_path = path;
    RPCResponse ret = afsClient->RemoveDir(s_path);
    if (ret.status() == -1) {
        printf("MYERROR: wiscAFS_rmdir: failed path = %s\n", path);
        return -ret.error();
    }
    
    return 0;
}

int wiscAFS_open(const char * path, struct fuse_file_info *fi)
{
    sem_wait(&wiscOPSem);
    std::string s_path = path;
    printf("wiscAFS_Open: Start path = %s\n", path);
    RPCResponse ret = afsClient->OpenFile(s_path, fi->flags);
    if (ret.status() == -1) {
        sem_post(&wiscOPSem);
        printf("MYERROR: wiscAFS_Open: Failed path = %s", path);
        return -ret.error();
    }
    fi->fh = ret.file_descriptor();
    printf("wiscAFS_Open: End path = %s, fd = %ld\n", path, fi->fh);
    sem_post(&wiscOPSem);

    return 0;
}

int wiscAFS_flush(const char * path, struct fuse_file_info *fi) {
    sem_wait(&wiscOPSem);
    //fsync(fi->fh);
    printf("wiscAFS_flush: Start fd = %ld, path = %s\n", fi->fh, path);
    close(dup(fi->fh));
    std::string s_path(path);
    RPCResponse ret = afsClient->CloseFile(s_path, false);
    if (ret.status() == -1) {
        printf("MYERROR: wiscAFS_flush: Failed path= %s, errno = %d\n", path, ret.error());
        sem_post(&wiscOPSem);
        return -ret.error();
    }
    printf("wiscAFS_flush: End fd = %ld, path = %s\n", fi->fh, path);
    sem_post(&wiscOPSem);
    return 0;
}

int wiscAFS_release(const char * path, struct fuse_file_info *fi) {
    //sem_wait(&wiscOPSem);
    //fsync(fi->fh);
    printf("wiscAFS_release: Start fd = %ld, path = %s\n", fi->fh, path);
    close(fi->fh);
    /*std::string s_path(path);
    RPCResponse ret = afsClient->CloseFile(s_path, true);
    if (ret.status() == -1) {
        printf("MYERROR: wiscAFS_release: Failed path= %s, errno = %d\n", path, ret.error());
        sem_post(&wiscOPSem);
        return -ret.error();
    }
    printf("wiscAFS_release: End fd = %ld, path = %s\n", fi->fh, path);
    sem_post(&wiscOPSem);*/
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

    int val;
    sem_getvalue(&wiscOPSem, &val);
    printf("wiscAFS_read: Semaphore value = %d\n", val);
    RPCResponse reply;
    int fd, ret;
    printf("wiscAFS_read: Start fd = %ld, Path = %s\n", fi->fh, path);

    if (fi == NULL) {
        printf("MYERROR: wiscAFS_read: fi is NULL fd = %ld, Path = %s\n", fi->fh, path);
        //fd = open(path, O_RDONLY);
        reply = afsClient->OpenFile(std::string(path), O_RDONLY);
        fd = reply.file_descriptor();
    } 
    else {
        fd = fi->fh;
    }

    if (fd == -1) {
        sem_post(&wiscOPSem);
        printf("MYERROR: wiscAFS_read: failed descriptor invalid fd = %d, Path = %s\n", fd, path);
        return -errno;
    }

    int last = -1; 
    for (int i = 0; i < sizeQueue(&opQueue); i++){
        struct queueNode *ptr = getValueQueue(&opQueue, i);
        if (ptr->fd == fd){
            last = i;
        }
    }
    printf("READ:: last = %d, offset = %ld, size = %ld\n", last, offset,size);
    if (last == -1){
        ret = pread(fd, buf, size, offset);
    }
    else{
        struct queueNode *ptr = getValueQueue(&opQueue, last);
        printf("READ:: last = %d, offset = %ld, size = %ld, ptr->buf = %s, ptr->offset = %ld, ptr->size = %ld\n", last, offset,size, ptr->buf, ptr->offset, ptr->size);
        if (offset == ptr->offset){
            printf("READ:: Using queue\n");
            memcpy(buf, ptr->buf, size);
            ret = ptr->size;
        }
        else{
            ret = pread(ptr->fd, buf, size, offset);
        }
    }

    if (ret == -1) {
        printf("MYERROR: wiscAFS_read: pread failed for fd = %d, Path = %s\n", fd, path);
        ret = -errno;
    }
    //printf("wiscAFS_read: buf = %s\n", buf);

    if (fi == NULL) {
        close(fd);
    }
    printf("wiscAFS_read: End fd = %ld, Path = %s\n", fi->fh, path);

    return ret;
}

int wiscAFS_write (const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
    printf("Start of write\n");
    std::string s_path = path;
    char *q_buf = (char*)malloc(sizeof(char)*(size+1));
    memcpy(q_buf, buf, size);
    pushQueue(&opQueue, OP_WRITE, q_buf, size, offset, fi->fh); 
    printf("Pushed to queue\n");
    int afsRet = afsClient->WriteFile(s_path);
    if (afsRet == -1) {
        printf("MYERROR: wiscAFS_write: Client returned -1 fd = %ld, Path = %s, size = %ld\n", fi->fh, path, size);
        return -errno;
    }

    return size;

    int val;
    sem_getvalue(&wiscOPSem, &val);
    printf("wiscAFS_write: Semaphore value = %d\n", val);
    RPCResponse reply; 

    printf("wiscAFS_write: Start fd = %ld, Path = %s, size = %ld\n", fi->fh, path, size);
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
        printf("MYERROR: wiscAFS_write: fi is NULL fd = %ld, Path = %s, size = %ld\n", fi->fh, path, size);
        //fd = open(path, O_WRONLY);
        reply = afsClient->OpenFile(s_path, O_WRONLY);
        fd = reply.file_descriptor();
    } else {
        fd = fi->fh;
    }

    if (fd == -1) {
        printf("MYERROR: wiscAFS_write: fd is -1 fd = %d, Path = %s, size = %ld\n", fd, path, size);
        return -errno;
    }

    ret = pwrite(fd, buf, size, offset);
    if (ret == -1) {
        printf("MYERROR: wiscAFS_write: pwrite failed fd = %d, Path = %s, size = %ld, errno = %d\n", fd, path, size, errno);
        ret = -errno;
    }

    afsRet = afsClient->WriteFile(s_path);
    if (afsRet == -1) {
        printf("MYERROR: wiscAFS_write: Client returned -1 fd = %d, Path = %s, size = %ld\n", fd, path, size);
        return -errno;
    }

    if(fi == NULL) {
        close(fd);
    }
    printf("wiscAFS_write: End fd = %ld, Path = %s, size = %ld, offset = %ld\nPrinting buf = ", fi->fh, path, size, offset);
    for (unsigned int k = 0; k < size; k++)
        printf("%c", buf[k]);
    printf("\n");

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

int wiscAFS_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    printf("FSYNC: Loop  starting , sizeQueue = %d", sizeQueue(&opQueue));
    struct queueNode *ptr = opQueue;
    while(sizeQueue(&opQueue) != 0){
       int ret = error_inject(path, OP_FSYNC);
       printf("fsync ret = %d\n", ret);
       int index = 0; 
       if (ret == -1){
           //Simple delay
           ptr = getValueQueue(&opQueue, index);
           printf("About to write delay %s\n", ptr->buf);
           int wret = pwrite(ptr->fd, ptr->buf, ptr->size, ptr->offset);
           if (wret == -1){
              printf("ERROR WHILE WRITING\n");
           }
       }
       else {
           //Reorder 
           index = ret;
           ptr = getValueQueue(&opQueue, index);
           printf("About to write reorder %s\n", ptr->buf);
           int wret = pwrite(ptr->fd, ptr->buf, ptr->size, ptr->offset);
           if (wret == -1){
              printf("ERROR WHILE WRITING\n");
           }
       }
       free(ptr->buf);
       delFromQueue(&opQueue, index);
    }

    printf("FSYNC: Loop  completed, sizeQueue = %d", sizeQueue(&opQueue));

    /*if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }*/
    int ret;
    if (datasync) {
        ret = fdatasync(fi->fh);
        if (ret == -1) {
            return -errno;
        }
    } else {
        ret = fsync(fi->fh);
        if (ret == -1) {
            return -errno;
        }
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
    RPCDirReply res = afsClient->ReadDir(path, buf, filler);
    if (res.status() == -1){
        return -res.error();
    }
    return 0;
}

int wiscAFS_create(const char *path, mode_t mode,
                      struct fuse_file_info *fi)
{
    sem_wait(&wiscOPSem);
    printf("wiscAFS_create: Start  Path = %s, mode = %d\n", path, mode);
    std::string s_path = path;
    RPCResponse ret = afsClient->CreateFile(s_path, fi->flags, mode);
    if (ret.status() == -1) {
        printf("MYERROR: wiscAFS_create: Failed to createFile, Path = %s, mode = %d\n", path, mode);
        sem_post(&wiscOPSem);
        return -ret.error();
    }
    fi->fh = ret.file_descriptor();
    printf("wiscAFS_create: End Path = %s, mode = %d\n", path, mode);
    sem_post(&wiscOPSem);

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
    sem_wait(&wiscOPSem);
    std::string s_path = path;
    RPCResponse ret = afsClient->TruncateFile(s_path, length);
    if (ret.status() == -1) {
        sem_post(&wiscOPSem);
        return -ret.error();
    }
    //ret = access(path, mode); 
    
    sem_post(&wiscOPSem);
    return 0;
}

int wiscAFS_access(const char *path, int mode)
{
    sem_wait(&wiscOPSem);
    std::string s_path = path;
    RPCResponse ret = afsClient->AccessFile(s_path, mode);
    if (ret.status() == -1) {
        sem_post(&wiscOPSem);
        return -ret.error();
    }
    //ret = access(path, mode); 
    
    sem_post(&wiscOPSem);
    return 0;
}

int wiscAFS_rename(const char* oldname, const char* newname) {

    sem_wait(&wiscOPSem);
    printf("wiscAFS_rename: Start  Oldname = %s, newname = %s\n", oldname, newname);
    std::string s_oldpath = oldname;
    std::string s_newpath = newname;
    RPCResponse ret = afsClient->RenameFile(s_oldpath, s_newpath);
    if (ret.status() == -1) {
        printf("MYERROR: wiscAFS_rename: End Oldname = %s, newname = %s\n", oldname, newname);
        sem_post(&wiscOPSem);
        return -ret.error();
    }
    printf("wiscAFS_rename: End Oldname = %s, newname = %s\n", oldname, newname);

    sem_post(&wiscOPSem);
    return 0;

}

int wiscAFS_chmod(const char* path, mode_t mode) {
    sem_wait(&wiscOPSem);
    std::string s_path = path;
    RPCResponse ret = afsClient->Chmod(s_path, mode);
    if(ret.status() == -1) {
        sem_post(&wiscOPSem);
        return -ret.error();
    }
    sem_post(&wiscOPSem);
    return 0;
}

int wiscAFS_chown(const char *path, uid_t uid, gid_t gid) {
    sem_wait(&wiscOPSem);
    std::string s_path = path;
    RPCResponse ret = afsClient->Chown(s_path, uid, gid);
    if(ret.status() == -1) {
        sem_post(&wiscOPSem);
        return -ret.error();
    }
    sem_post(&wiscOPSem);
    return 0;
}


void *wiscAFS_init(struct fuse_conn_info *conn) {
    afsClient = new wiscAFSClient (
      grpc::CreateChannel(std::string("10.10.1.1:50052"), grpc::InsecureChannelCredentials()));
    sem_init(&wiscOPSem, 0,1);
    return NULL;
}
#ifdef __cplusplus
}
#endif
