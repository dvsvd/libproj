#define _GNU_SOURCE
#include "shared_buf.h"
#include "utility.h"
#include <time.h>
#include <errno.h>
#include <threads.h>
#include <stdio.h>
#include <string.h>

/* Library internal shared buf */
shared_buf_t lib_shared_buf;

__attribute__((constructor)) static void setup(void)
{
    /* Initialize library shared buf */
    shared_buf_init(&lib_shared_buf, PTHREAD_PROCESS_SHARED);
}

int write_with_timestamp(shared_buf_t* buf, const void* src, size_t n)
{
    int ret;
    int msecs;
    size_t tmsize; // size of the timestamp in the buffer
    struct timespec ts = {0};
    struct tm gt = {0};
    char databuf[BUF_SIZE] = {0};
    ret = timespec_get(&ts, TIME_UTC);
    if(ret == 0)
    {
        errno = ENOMSG;
        return -1;
    }
    localtime_r(&ts.tv_sec, &gt);
    msecs = ts.tv_nsec / 1000000L;
    tmsize = strftime(databuf, BUF_SIZE, "[%F %T.", &gt);
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
    return
}

__attribute__((destructor)) static void deinit(void)
{
    /* Deinitialize library shared buf */
    shared_buf_destroy(&lib_shared_buf);
}