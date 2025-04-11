#include "logger.h"

int logger_init(logger_t* l, const char* pathname)
{
    int fd, ret;
    if(l == NULL || pathname == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    if((ret = buf_queue_init(&l->q, PTHREAD_PROCESS_SHARED)) == -1 ||
    (fd = real_open(pathname, O_APPEND | O_WRONLY | O_CREAT, 0766)) == -1 ||
    (l->file = fdopen(fd, "a")) == NULL
    )
    {
        return -1;
    }
    return 0;
}

int log_from_queue(void* data)
{
    logger_t* l = (logger_t*)data;
    shared_buf_t* buf;
    while(1)
    {
        /* Blocking call to pop */
        buf = pop(&l->q);
        if(buf)
        {
            fwrite(buf->buf, sizeof buf->buf[0], buf->count, l->file);
            fputc('\n', l->file);
        }
    }
    return 0;
}

int listen_for_messages(logger_t* l)
{
    int ret;
    if(l == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    ret = thrd_create(&l->t, log_from_queue, l);
    if(ret != thrd_success)
    {
        return ret;
    }
    return 0;
}

int logger_destroy(logger_t* l)
{
    int ret;
    if(l == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    ret = buf_queue_destroy(&l->q);
}
