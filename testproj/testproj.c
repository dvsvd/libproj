#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char** argv)
{
    int fd = open("testfile", O_CREAT | O_RDWR, 0755);
    ssize_t ret = write(fd, "This is a test string", 22);
    char* buf = (char*)malloc(23);
    memset(buf, 0, 23);
    ret = read(fd, buf, 22);
    off_t offset = lseek(fd, -22, SEEK_CUR);
    buf = (char*)realloc(buf, 23 * 2);
    ret = read(fd, buf, 22);
    free(buf);
    int ret2 = close(fd);
    return 0;
}