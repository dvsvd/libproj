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
    mqd_t fd;
    msg_t m;
    open_msg_t msg;
    timespec_get(&m.ts, TIME_UTC);
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
    m.fn_id = OPEN;
    msg.fd = tmp;
    msg.flags = flags;
    msg.pathlen = strlen(pathname);
    memcpy(msg.pathname, pathname, msg.pathlen);
    memset(msg.pathname + msg.pathlen, 0, sizeof msg.pathname - msg.pathlen);
    memcpy(m.payload, &msg, sizeof(open_msg_t));
    memset(m.payload + sizeof(open_msg_t), 0, sizeof m.payload - sizeof(open_msg_t));
    if(mq_send(fd, &m, sizeof(msg_t), 0) == -1)
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
    msg_t m;
    close_msg_t msg;
    timespec_get(&m.ts, TIME_UTC);
    if((qfd = get_io_mq()) == -1)
    {
        perror("get_io_mq() failed: ");
        exit(1337);
    }
    msg.fd = fd;
    tmp = real_close(fd);
    err = errno;
    m.fn_id = CLOSE;
    msg.ret = tmp;
    memcpy(m.payload, &msg, sizeof(close_msg_t));
    memset(m.payload + sizeof(close_msg_t), 0, sizeof m.payload - sizeof(close_msg_t));
    if(mq_send(fd, &m, sizeof(msg_t), 0) == -1)
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
    msg_t m;
    lseek_msg_t msg;
    timespec_get(&m.ts, TIME_UTC);
    if((qfd = get_io_mq()) == -1)
    {
        perror("get_io_mq() failed: ");
        exit(1337);
    }
    tmp = real_lseek(fd, offset, whence);
    err = errno;
    m.fn_id = LSEEK;
    msg.fd = fd;
    msg.offset = offset;
    msg.whence = whence;
    msg.new_pos = tmp;
    memcpy(m.payload, &msg, sizeof(lseek_msg_t));
    memset(m.payload + sizeof(lseek_msg_t), 0, sizeof m.payload - sizeof(lseek_msg_t));
    if(mq_send(fd, &m, sizeof(msg_t), 0) == -1)
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
    msg_t m;
    rw_msg_t msg;
    timespec_get(&m.ts, TIME_UTC);
    if((qfd = get_io_mq()) == -1)
    {
        perror("get_io_mq() failed: ");
        exit(1337);
    }
    tmp = real_read(fd, buf, count);
    err = errno;
    m.fn_id = READ;
    msg.fd = fd;
    msg.buf_ptr = buf;
    msg.bytes_transmitted = tmp;
    msg.count = count;
    memcpy(m.payload, &msg, sizeof(rw_msg_t));
    memset(m.payload + sizeof(rw_msg_t), 0, sizeof m.payload - sizeof(rw_msg_t));
    if(mq_send(fd, &m, sizeof(msg_t), 0) == -1)
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
    msg_t m;
    rw_msg_t msg;
    timespec_get(&m.ts, TIME_UTC);
    if((qfd = get_io_mq()) == -1)
    {
        perror("get_io_mq() failed: ");
        exit(1337);
    }
    tmp = real_write(fd, buf, count);
    err = errno;
    m.fn_id = WRITE;
    msg.fd = fd;
    msg.buf_ptr = buf;
    msg.bytes_transmitted = tmp;
    msg.count = count;
    memcpy(m.payload, &msg, sizeof(rw_msg_t));
    memset(m.payload + sizeof(rw_msg_t), 0, sizeof m.payload - sizeof(rw_msg_t));
    if(mq_send(fd, &m, sizeof(msg_t), 0) == -1)
    {
        perror("mq_send() failed: ");
    }
    errno = err; /* Restore real errno */
    return tmp;
}
