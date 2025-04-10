#include "shared_buf.h"
#include "utility.h"
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

ssize_t shared_buf_write_pos(shared_buf_t* buf, size_t pos, const void* src, size_t n)
{
    int ret;
    ssize_t avail = BUF_SIZE - pos; /* Bytes available */
    ssize_t diff;
    if(buf == NULL || src == NULL || pos >= BUF_SIZE || n > avail)
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
    /* Prevent 0-valued gaps */
    if(pos - buf->count > 1)
    {
        errno = EINVAL;
        return -1;
    }
    /* The function will not fail if there's not enough space in buffer;
     * it'll copy as many bytes as it can */
    n = MIN(avail, n);
    /* Update count if neccesary */
    if(pos + n > buf->count)
    {
        buf->count = pos + n;
    }
    memcpy(buf->buf + pos, src, n);
    diff = BUF_SIZE - n;
    if(diff > 0)
    {
        /* Zero leftovers */
        memset(buf->buf + pos + n, 0, diff);
    }
    ret = pthread_rwlock_unlock(&buf->lock);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    return n;
}

ssize_t shared_buf_write(shared_buf_t* buf, const void* src, size_t n)
{
    ssize_t ret = shared_buf_write_pos(buf, 0, src, n);
    if(ret == -1)
    {
        return ret;
    }
    size_t diff = BUF_SIZE - ret;
    if(diff > 0)
    {
        /* Zero leftovers */
        memset(buf->buf + ret, 0, diff);
    }
    return ret;
}

ssize_t shared_buf_append(shared_buf_t* buf, const void* src, size_t n)
{
    ssize_t ret, diff;
    _Atomic size_t* cnt = &buf->count;
    ret = shared_buf_write_pos(buf, 0, src, n);
    if(ret == -1)
    {
        return ret;
    }
    size_t diff = BUF_SIZE - ret;
    if(diff > 0)
    {
        /* Zero leftovers */
        memset(buf->buf + ret, 0, diff);
    }
    return ret;
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
    n = 
    memmove(dst, buf->buf, n);
    ret = pthread_rwlock_unlock(&buf->lock);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    return n;
}
