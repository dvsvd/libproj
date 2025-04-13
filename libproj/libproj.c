#define _GNU_SOURCE
#include "buf_queue.h"
#include "utility.h"
#include "memfcn.h"
#include "iofcn.h"
#include "logger.h"
#include <errno.h>

__attribute__((constructor)) static void setup(void)
{
    char* msg;
    int fd, ret;
    real_open = dlsym(RTLD_NEXT, "open");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, msg);
    }
    real_close = dlsym(RTLD_NEXT, "close");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, msg);
    }
    real_lseek = dlsym(RTLD_NEXT, "lseek");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, msg);
    }
    real_read = dlsym(RTLD_NEXT, "read");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, msg);
    }
    real_write = dlsym(RTLD_NEXT, "write");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, msg);
    }
    ret = logger_init(get_logger(), "log.txt");
    if(ret == -1)
    {
        perror("logger_init() failed: ");
    }
    is_default = !!0;
}

__attribute__((destructor)) static void deinit(void)
{
    logger_destroy(get_logger());
}