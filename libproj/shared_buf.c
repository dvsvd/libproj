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
    if( (ret = pthread_rwlockattr_init(&attr)) ||
        (ret = pthread_rwlockattr_setpshared(&attr, pshared)) ||
        (ret = pthread_rwlock_init(&buf->lock, &attr)) ||
        (ret = pthread_rwlockattr_destroy(&attr))
    )
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
    if((ret = pthread_rwlock_rdlock(&buf->lock)) ||
       (ret = pthread_rwlock_destroy(&buf->lock))
    )
    {
        errno = ret;
        return -1;
    }
    return 0;
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
    ssize_t ret;
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
    ret = pthread_rwlock_wrlock(&buf->lock);
    if(ret)
    {
        errno = ret;
        return -1;
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
    ret = pthread_rwlock_unlock(&buf->lock);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    return n;
}

ssize_t shared_buf_append(shared_buf_t* buf, const void* src, size_t n)
{
    ssize_t ret;
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
    ret = pthread_rwlock_wrlock(&buf->lock);
    if(ret)
    {
        errno = ret;
        return -1;
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
    ret = pthread_rwlock_unlock(&buf->lock);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    return n;
}

ssize_t shared_buf_read(const shared_buf_t* buf, void* dst, size_t n)
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
    n = MIN(buf->count, n);
    memmove(dst, buf->buf, n);
    ret = pthread_rwlock_unlock(&buf->lock);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    return n;
}
