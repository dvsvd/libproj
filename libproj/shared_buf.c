#include "shared_buf.h"
#include "utility.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

int shared_buf_init(shared_buf_t* buf, int pshared)
{
    if(buf == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    memset(buf->buf, 0, BUF_SIZE);
    buf->count = 0;
    return 0;
}

int shared_buf_destroy(shared_buf_t* buf)
{
    if(buf == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

ssize_t shared_buf_write(shared_buf_t* buf, const void* src, size_t n)
{
    size_t avail, diff;
    if(buf == NULL || src == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    if(buf->buf == src)
    {
        return 0;
    }
    /* The function will not fail if there's not enough space in buffer;
     * it'll copy as many bytes as it can */
    n = MIN(BUF_SIZE, n);
    memcpy(buf->buf, src, n);
    buf->count = n;
    diff = BUF_SIZE - buf->count;
    if(diff > 0)
    {
        /* Zero leftovers */
        memset(buf->buf + buf->count, 0, diff);
    }
    return n;
}

ssize_t shared_buf_append(shared_buf_t* buf, const void* src, size_t n)
{
    size_t avail, diff;
    if(buf == NULL || src == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    if(buf->buf == src)
    {
        return 0;
    }
    /* The function will not fail if there's not enough space in buffer;
     * it'll copy as many bytes as it can */
    avail = BUF_SIZE - buf->count;
    if(avail == 0)
    {
        errno = ENOMEM;
        return -1;
    }
    n = MIN(avail, n);
    memcpy(buf->buf + buf->count, src, n);
    buf->count += n;
    return n;
}

ssize_t shared_buf_read(const shared_buf_t* buf, void* dst, size_t n)
{
    if(buf == NULL || dst == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    if(buf->buf == dst)
    {
        return 0;
    }
    n = MIN(buf->count, n);
    memcpy(dst, buf->buf, n);
    return n;
}

int write_with_timestamp(shared_buf_t* buf, const void* src, size_t n, struct timespec* ts)
{
    int ret;
    int msecs;
    size_t tmsize; // size of the timestamp in the buffer
    //struct timespec ts = {0};
    struct tm gt = {0};
    char databuf[100] = {0};
    // ret = timespec_get(ts, TIME_UTC);
    // if(ret == 0)
    // {
    //     errno = ENOMSG;
    //     return -1;
    // }
    localtime_r(&ts->tv_sec, &gt);
    msecs = ts->tv_nsec / 1000000L;
    tmsize = strftime(databuf, 100, "[%F %T.", &gt);
    if(tmsize == 0)
    {
        errno = ENOMEM;
        return -1;
    }
    ret = sprintf(databuf + tmsize, "%03d] ", msecs);
    if(ret < 0)
    {
        errno = ENOMSG;
        return -1;
    }
    tmsize += ret;
    ret = shared_buf_write(buf, databuf, tmsize);
    ret = shared_buf_append(buf, src, n);
    return 0;
}