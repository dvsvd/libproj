#include "shared_buf.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int shared_buf_init(shared_buf_t* buf, int pshared)
{
    int ret;
    pthread_rwlockattr_t attr;
    if(buf == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    ret = pthread_rwlockattr_init(&attr);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    ret = pthread_rwlockattr_setpshared(&attr, pshared);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    ret = pthread_rwlock_init(&buf->lock, &attr);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    ret = pthread_rwlockattr_destroy(&attr);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    memset(buf->buf, 0, BUF_SIZE);
    buf->count = 0;
    return 0;
}

int shared_buf_destroy(shared_buf_t* buf)
{
    int ret;
    if(buf == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    ret = pthread_rwlock_rdlock(&buf->lock);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    ret = pthread_rwlock_destroy(&buf->lock);
    if(ret)
    {
        errno = ret;
        return -1;
    }
}

int shared_buf_write(shared_buf_t* buf, const void* src, size_t n)
{
    int ret;
    ssize_t diff;
    if(buf == NULL || src == NULL || n > BUF_SIZE)
    {
        errno = EINVAL;
        return -1;
    }
    if(buf->buf == src)
    {
        return 0;
    }
    ret = pthread_rwlock_wrlock(&buf->lock);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    memmove(buf->buf, src, n);

    diff = buf->count - n;
    if(diff > 0)
    {
        /* Zero leftovers */
        memset(buf->buf + n, 0, diff);
    }
    buf->count = n;
    ret = pthread_rwlock_unlock(&buf->lock);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    return 0;
}

int shared_buf_read(const shared_buf_t* buf, void* dst, size_t n)
{
    int ret;
    if(buf == NULL || dst == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    if(buf->buf == dst)
    {
        return 0;
    }
    ret = pthread_rwlock_rdlock(&buf->lock);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    memmove(dst, buf->buf, n);
    ret = pthread_rwlock_unlock(&buf->lock);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    return n;
}
