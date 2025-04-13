#include "logger.h"
#include "utility.h"
#include "memfcn.h"

/* Library internal logger */
logger_t logger;

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
    /* Blocking call to pop */
    while(buf = pop(&l->q))
    {
        fwrite(buf->buf, sizeof buf->buf[0], buf->count, l->file);
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
    ret = fclose(l->file);
}

void log_write(logger_t* l, const char* msg, size_t n, struct timespec* ts)
{
    int ret;
    shared_buf_t* buf;
    if(l == NULL || msg == NULL)
    {
        errno = EINVAL;
        return;
    }
    buf = (shared_buf_t*)real_malloc(sizeof(shared_buf_t));
    ret = shared_buf_init(buf, PTHREAD_PROCESS_SHARED);
    ret = write_with_timestamp(buf, msg, ret, ts);
    push(&l->q, buf);
}

logger_t* get_logger()
{
    logger_t* p = &logger;
    return p;
}
