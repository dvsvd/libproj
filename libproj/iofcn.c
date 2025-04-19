#include "iofcn.h"
#include "shared_buf.h"
#include "utility.h"
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <mqueue.h>

open_t real_open;
close_t real_close;
lseek_t real_lseek;
read_t real_read;
write_t real_write;

int open(const char* pathname, int flags, ...)
{
    mode_t mode = 0;
    int tmp;
    int err; /* real errno */
    int ret;
    mqd_t fd;
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    if(flags & O_CREAT || flags & O_TMPFILE)
    {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }
    if((fd = get_io_mq()) == -1)
    {
        perror("get_io_mq() failed: ");
        exit(1337);
    }
    tmp = real_open(pathname, flags, mode);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "open() called: filename: %s, flags: %#0X, "
                    "fd: %d\n", pathname, flags, tmp);
    if(mq_send(fd, msg, ret, 0) == -1)
    {
        perror("mq_send() failed: ");
    }
    errno = err; /* Restore real errno */
    return tmp;
}

int close(int fd)
{
    mqd_t qfd;
    int tmp;
    int err; /* real errno */
    int ret;
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    if((qfd = get_io_mq()) == -1)
    {
        perror("get_io_mq() failed: ");
        exit(1337);
    }
    tmp = real_close(fd);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "close() called: fd: %d, return code: %d\n", fd, tmp);
    if(mq_send(qfd, msg, ret, 0) == -1)
    {
        perror("mq_send() failed: ");
    }
    errno = err; /* Restore real errno */
    return tmp;
}

off_t lseek(int fd, off_t offset, int whence)
{
    mqd_t qfd;
    off_t tmp;
    int err; /* real errno */
    int ret;
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    if((qfd = get_io_mq()) == -1)
    {
        perror("get_io_mq() failed: ");
        exit(1337);
    }
    tmp = real_lseek(fd, offset, whence);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "lseek() called: filename: %d, requested offset: %d, "
                    "whence: %s, resulted offset: %d\n", fd, offset, whence_text(whence), tmp);
    if(mq_send(qfd, msg, ret, 0) == -1)
    {
        perror("mq_send() failed: ");
    }
    errno = err; /* Restore real errno */
    return tmp;
}

ssize_t read(int fd, void* buf, size_t count)
{
    mqd_t qfd;
    ssize_t tmp;
    int err; /* real errno */
    int ret;
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    if((qfd = get_io_mq()) == -1)
    {
        perror("get_io_mq() failed: ");
        exit(1337);
    }
    tmp = real_read(fd, buf, count);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "lseek() called: fd: %d, buf pointer: %#0"PRIXPTR
                    ", count: %d, bytes read: %d\n", fd, (uintptr_t)buf, count, tmp);
    if(mq_send(qfd, msg, ret, 0) == -1)
    {
        perror("mq_send() failed: ");
    }
    errno = err; /* Restore real errno */
    return tmp;
}

ssize_t write(int fd, const void* buf, size_t count)
{
    mqd_t qfd;
    ssize_t tmp;
    int err; /* real errno */
    int ret;
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    if((qfd = get_io_mq()) == -1)
    {
        perror("get_io_mq() failed: ");
        exit(1337);
    }
    tmp = real_write(fd, buf, count);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "lseek() called: fd: %d, buf pointer: %#0"PRIXPTR
                    ", count: %d, bytes written: %d\n", fd, (uintptr_t)buf, count, tmp);
    if(mq_send(qfd, msg, ret, 0) == -1)
    {
        perror("mq_send() failed: ");
    }
    errno = err; /* Restore real errno */
    return tmp;
}
