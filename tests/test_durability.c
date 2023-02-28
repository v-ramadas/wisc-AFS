#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

int main()
{
    int fd = open("/tmp/fs/a.txt", O_RDWR|O_CREAT|O_TRUNC, 0777);

    int ret_write = pwrite(fd, "11111111", strlen("11111111"), 0);
    if (ret_write == -1){
        printf("Write faield\n");
    }
    ret_write = pwrite(fd, "22222222", strlen("22222222"), 0);
    if (ret_write == -1){
        printf("Write faield 2 \n");
    }
    ret_write = pwrite(fd, "33333333", strlen("33333333"), 0);
    if (ret_write == -1){
        printf("Write faield 3\n");
    }
    ret_write = pwrite(fd, "44444444", strlen("44444444"), 0);
    if (ret_write == -1){
        printf("Write faield 4\n");
    }
    ret_write = pwrite(fd, "55555555", strlen("55555555"), 0);
    if (ret_write == -1){
        printf("Write faield 5\n");
    }
    ret_write = pwrite(fd, "66666666", strlen("66666666"), 0);
    if (ret_write == -1){
        printf("Write faield 6\n");
    }

    char* buf = (char*)malloc(sizeof(char)*1000); 
    int ret = pread(fd, buf, 8, 0);
    buf[ret] = '\0';
    printf("before fsync ret = %d\n", ret);
    printf("before fsync buf = %s\n", buf);

    fsync(fd);

    close(fd);
    fd = open("/tmp/fs/a.txt", O_RDWR);
    char* buf2 = (char*)malloc(sizeof(char)*1000); 
    ret = pread(fd, buf2, 8, 0);
    buf2[ret] = '\0';
    printf("after fsync ret = %d\n", ret);
    printf("after fsync buf = %s\n", buf2);
    /*int fw = open("b.txt", O_CREAT|O_RDWR|O_TRUNC, 0777);
    printf("fw = %d\n", fw);
    int ret2 = write(fw, buf, 100);
    printf("ret = %d\n", ret2);*/
    close(fd);

    return 0;
}

