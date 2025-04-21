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
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, "%s\n", msg);
    }
    real_free = dlsym(RTLD_NEXT, "free");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, "%s\n", msg);
    }
    real_realloc = dlsym(RTLD_NEXT, "realloc");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, "%s\n", msg);
    }
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

mqd_t get_mem_mq(void)
{
    static mqd_t fd;
    static _Bool is_init = !!0;
    struct mq_attr a = {0};
    a.mq_maxmsg = 10;
    a.mq_msgsize = sizeof(msg_t);
    if(!is_init)
    {
        fd = mq_open(MEM_MQ_NAME, O_WRONLY | O_CREAT, 0600, &a);
        is_init = !!1;
    }
    return fd;
}

mqd_t get_io_mq(void)
{
    static mqd_t fd;
    static _Bool is_init = !!0;
    struct mq_attr a = {0};
    a.mq_maxmsg = 10;
    a.mq_msgsize = sizeof(msg_t);
    if(!is_init)
    {
        fd = mq_open(IO_MQ_NAME, O_WRONLY | O_CREAT, 0600, &a);
        is_init = !!1;
    }
    return fd;
}

__attribute__((destructor)) static void deinit(void)
{
    if(mq_unlink(MEM_MQ_NAME) == -1)
    {
        perror("shm_unlink() failed in "__FILE__" at line "LINESTR);
    }
    if(mq_unlink(IO_MQ_NAME) == -1)
    {
        perror("shm_unlink() failed in "__FILE__" at line "LINESTR);
    }
}