#define _GNU_SOURCE
#include "utility.h"
#include "msg_buf.h"
#include "memfcn.h"
#include "iofcn.h"
#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h> 
#include <sys/stat.h>
#include <mqueue.h>

__attribute__((constructor)) static void setup(void)
{
    char* msg;
    mqd_t mqfd, ioqfd;
    int ret;
    real_perror = dlsym(RTLD_NEXT, "perror");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, "%s\n", msg);
    }
    real_open = dlsym(RTLD_NEXT, "open");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, "%s\n", msg);
        return;
    }
    real_close = dlsym(RTLD_NEXT, "close");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, "%s\n", msg);
        return;
    }
    real_lseek = dlsym(RTLD_NEXT, "lseek");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, "%s\n", msg);
        return;
    }
    real_read = dlsym(RTLD_NEXT, "read");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, "%s\n", msg);
        return;
    }
    real_write = dlsym(RTLD_NEXT, "write");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, "%s\n", msg);
        return;
    }
    is_default = !!0;
}

/* mem_mq singleton */
mqd_t get_mem_mq(void)
{
    static mqd_t fd;
    static _Bool is_init = !!0;
    if(!is_init)
    {
        fd = mq_open(MEM_MQ_NAME, O_WRONLY);
        if(fd != -1) is_init = !!1;
    }
    return fd;
}

/* io_mq singleton */
mqd_t get_io_mq(void)
{
    static mqd_t fd;
    static _Bool is_init = !!0;
    if(!is_init)
    {
        fd = mq_open(IO_MQ_NAME, O_WRONLY);
        if(fd != -1) is_init = !!1;
    }
    return fd;
}

__attribute__((destructor)) static void deinit(void)
{
    mqd_t iomq = get_io_mq();
    mqd_t memmq = get_mem_mq();
    mq_close(iomq);
    mq_close(memmq);
    mq_unlink(MEM_MQ_NAME);
    mq_unlink(IO_MQ_NAME);
}