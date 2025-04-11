#include "iofcn.h"
#include "shared_buf.h"
#include "utility.h"
#include "logger.h"
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>

open_t real_open;
close_t real_close;
lseek_t real_lseek;
read_t real_read;
write_t real_write;

/* Library internal shared buf */
extern shared_buf_t lib_shared_buf;

const char* whence_text(int whence)
{
    switch (whence)
    {
    case SEEK_CUR:
        return "SEEK_CUR";
    case SEEK_SET:
        return "SEEK_SET";
    case SEEK_END:
        return "SEEK_END";
    default:
        return "UNKNOWN";
    }
}

int open(const char* pathname, int flags, ...)
{
    mode_t mode = 0;
    int tmp;
    int err; /* real errno */
    int ret;
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
    tmp = real_open(pathname, flags, mode);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "open() called: filename: %s, flags: %#0X \
fd: %d\n", pathname, flags, tmp);
    log_write(get_logger(), msg, ret, &ts);
    errno = err; /* Restore real errno */
    return tmp;
}

int close(int fd)
{
    int tmp;
    int err; /* real errno */
    int ret;
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    tmp = real_close(fd);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "close() called: fd: %d, return code: %d\n", fd, tmp);
    log_write(get_logger(), msg, ret, &ts);
    errno = err; /* Restore real errno */
    return tmp;
}

off_t lseek(int fd, off_t offset, int whence)
{
    off_t tmp;
    int err; /* real errno */
    int ret;
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    tmp = real_lseek(fd, offset, whence);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "lseek() called: filename: %d, requested offset: %d \
whence: %s, resulted offset: %d\n", fd, offset, whence_text(whence), tmp);
    log_write(get_logger(), msg, ret, &ts);
    errno = err; /* Restore real errno */
    return tmp;
}

ssize_t read(int fd, void* buf, size_t count)
{
    ssize_t tmp;
    int err; /* real errno */
    int ret;
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    tmp = real_read(fd, buf, count);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "lseek() called: fd: %d, buf pointer: %#0"PRIXPTR", \
count: %d, bytes read: %d\n", fd, (uintptr_t)buf, count, tmp);
    log_write(get_logger(), msg, ret, &ts);
    errno = err; /* Restore real errno */
    return tmp;
}

ssize_t write(int fd, const void* buf, size_t count)
{
    ssize_t tmp;
    int err; /* real errno */
    int ret;
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    tmp = real_write(fd, buf, count);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "lseek() called: fd: %d, buf pointer: %#0"PRIXPTR", \
count: %d, bytes written: %d\n", fd, (uintptr_t)buf, count, tmp);
    log_write(get_logger(), msg, ret, &ts);
    errno = err; /* Restore real errno */
    return tmp;
}
