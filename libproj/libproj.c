#define _GNU_SOURCE
#include "buf_queue.h"
#include "utility.h"
#include "memfcn.h"
#include "iofcn.h"
#include "logger.h"
#include <time.h>
#include <errno.h>
#include <threads.h>
#include <stdio.h>
#include <string.h>

/* Library internal logger */
logger_t logger;

__attribute__((constructor)) static void setup(void)
{
    char* msg;
    int fd, ret;
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, msg);
    }
    real_realloc = dlsym(RTLD_NEXT, "realloc");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, msg);
    }
    real_free = dlsym(RTLD_NEXT, "free");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, msg);
    }
    ret = logger_init(&logger, "log.txt");
    if(ret == -1)
    {
        perror("logger_init() failed: ");
    }
}

int write_with_timestamp(shared_buf_t* buf, const void* src, size_t n)
{
    int ret;
    int msecs;
    size_t tmsize; // size of the timestamp in the buffer
    struct timespec ts = {0};
    struct tm gt = {0};
    char databuf[100] = {0};
    ret = timespec_get(&ts, TIME_UTC);
    if(ret == 0)
    {
        errno = ENOMSG;
        return -1;
    }
    localtime_r(&ts.tv_sec, &gt);
    msecs = ts.tv_nsec / 1000000L;
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

__attribute__((destructor)) static void deinit(void)
{
    
}