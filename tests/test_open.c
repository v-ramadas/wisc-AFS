#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/stat.h>
#include <sys/statvfs.h>
int main()
{
    //int fd = open("/tmp/fs/b.txt", O_CREAT|O_RDWR|O_TRUNC);
    int fd = open("/tmp/fs/b.txt", O_RDWR|O_TRUNC);
    char *c = (char*)calloc (100, sizeof(char));
    if (fd == -1) {
        printf("ERROR Could not open file\n");
    }
    else {
        printf("Received fd = %d\n", fd);
        int sz = pread (fd, c, 5, 0);
        printf("read sz = %d\n", sz);
        c[sz] = '\0';
        printf("Read - %s\n", c);
        sz = pwrite(fd, "22222", 5, 0);
        printf("write sz = %d\n", sz);
        sz = pwrite(fd, "33333", 5, 5);
        printf("write sz = %d\n", sz);

        sz = pread (fd, c, 5, 0);
        printf("read sz = %d\n", sz);
        c[sz] = '\0';
        printf("Read - %s\n", c);

        struct stat buffer;
        int status = lstat("/tmp/fs/b.txt", &buffer);
        printf("Returned lstat, access time = %ld, modification time = %ld\n ", buffer.st_atime, buffer.st_mtime);

        struct statvfs vfs;
        status = statvfs("/tmp/fs", &vfs);
        printf("Returned statvfs, block size = %ld, filesystem block size = %ld, number of free blocks available = %ld\n ", vfs.f_bsize, vfs.f_frsize, vfs.f_bavail);

        sz = pwrite(fd, "44444", 5, 10);
        printf("write sz = %d\n", sz);

        status = lstat("/tmp/fs/b.txt", &buffer);
        printf("Returned lstat after new write, access time = %ld, modification time = %ld\n ", buffer.st_atime, buffer.st_mtime);

        close(fd);
        printf("Closed file\n");

        //status = unlink("/tmp/fs/b.txt");
        //printf("Unlink - status after unlink call = %d\n", status);

    }



}
