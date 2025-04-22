#include "iofcn.h"
#include "msg_buf.h"
#include "utility.h"
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <mqueue.h>
#include <string.h>

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
    mqd_t qfd;
    msg_t m;
    size_t pathlen;

    /* Get call time */
    timespec_get(&m.ts, TIME_UTC);

    /* Parse varargs to pass them to real_open */
    if(flags & O_CREAT || flags & O_TMPFILE)
    {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }

    /* Get mq */
    qfd = get_io_mq();

    /* Execute real function and save its errno */
    tmp = real_open(pathname, flags, mode);
    err = errno;

    /* Fill in the message */
    m.fn_id = OPEN;
    m.pid = getpid();
    m.open_msg.fd = tmp;
    m.open_msg.flags = flags;
    pathlen = strlen(pathname);
    memcpy(m.open_msg.pathname, pathname, pathlen);
    memset(m.open_msg.pathname + pathlen, 0, sizeof m.open_msg.pathname - pathlen);

    /* Send the message */
    if(mq_send(qfd, (const char*)&m, sizeof(msg_t), 0) == -1)
    {
        switch (errno)
        {
        case EBADF:
            break;
        default:
            perror("mq_send() failed in "__FILE__" at line "LINESTR);
        }
    }

    /* Restore real errno */
    errno = err;
    return tmp;
}

int close(int fd)
{
    mqd_t qfd;
    int tmp;
    int err; /* real errno */
    int ret;
    msg_t m;

    /* Get call time */
    timespec_get(&m.ts, TIME_UTC);

    /* Get mq */
    qfd = get_io_mq();

    /* Get fd before closing it */
    m.close_msg.fd = fd;
    tmp = real_close(fd);
    err = errno;

    /* Fill in the message */
    m.fn_id = CLOSE;
    m.pid = getpid();
    m.close_msg.ret = tmp;

    /* Send the message */
    if(mq_send(qfd, (const char*)&m, sizeof(msg_t), 0) == -1)
    {
        switch (errno)
        {
        case EBADF:
            break;
        default:
            perror("mq_send() failed in "__FILE__" at line "LINESTR);
        }
    }

    /* Restore real errno */
    errno = err;
    return tmp;
}

off_t lseek(int fd, off_t offset, int whence)
{
    mqd_t qfd;
    off_t tmp;
    int err; /* real errno */
    int ret;
    msg_t m;

    /* Get call time */
    timespec_get(&m.ts, TIME_UTC);

    /* Get mq */
    qfd = get_io_mq();

    /* Execute real function and save its errno */
    tmp = real_lseek(fd, offset, whence);
    err = errno;

    /* Fill in the message */
    m.fn_id = LSEEK;
    m.pid = getpid();
    m.lseek_msg.fd = fd;
    m.lseek_msg.offset = offset;
    m.lseek_msg.whence = whence;
    m.lseek_msg.new_pos = tmp;

    /* Send the message */
    if(mq_send(qfd, (const char*)&m, sizeof(msg_t), 0) == -1)
    {
        switch (errno)
        {
        case EBADF:
            break;
        default:
            perror("mq_send() failed in "__FILE__" at line "LINESTR);
        }
    }

    /* Restore real errno */
    errno = err;
    return tmp;
}

ssize_t read(int fd, void* buf, size_t count)
{
    mqd_t qfd;
    ssize_t tmp;
    int err; /* real errno */
    int ret;
    msg_t m;

    /* Get call time */
    timespec_get(&m.ts, TIME_UTC);

    /* Get mq */
    qfd = get_io_mq();

    /* Execute real function and save its errno */
    tmp = real_read(fd, buf, count);
    err = errno;

    /* Fill in the message */
    m.fn_id = READ;
    m.pid = getpid();
    m.rw_msg.fd = fd;
    m.rw_msg.buf_ptr = buf;
    m.rw_msg.bytes_transmitted = tmp;
    m.rw_msg.count = count;

    /* Send the message */
    if(mq_send(qfd, (const char*)&m, sizeof(msg_t), 0) == -1)
    {
        switch (errno)
        {
        case EBADF:
            break;
        default:
            perror("mq_send() failed in "__FILE__" at line "LINESTR);
        }
    }

    /* Restore real errno */
    errno = err;
    return tmp;
}

ssize_t write(int fd, const void* buf, size_t count)
{
    mqd_t qfd;
    ssize_t tmp;
    int err; /* real errno */
    int ret;
    msg_t m;

    /* Get call time */
    timespec_get(&m.ts, TIME_UTC);

    /* Get mq */
    qfd = get_io_mq();

    /* Execute real function and save its errno */
    tmp = real_write(fd, buf, count);
    err = errno;

    /* Fill in the message */
    m.fn_id = WRITE;
    m.pid = getpid();
    m.rw_msg.fd = fd;
    m.rw_msg.buf_ptr = (void*)buf;
    m.rw_msg.bytes_transmitted = tmp;
    m.rw_msg.count = count;
    
    /* Send the message */
    if(mq_send(qfd, (const char*)&m, sizeof(msg_t), 0) == -1)
    {
        switch (errno)
        {
        case EBADF:
            break;
        default:
            perror("mq_send() failed in "__FILE__" at line "LINESTR);
        }
    }
    
    /* Restore real errno */
    errno = err;
    return tmp;
}
