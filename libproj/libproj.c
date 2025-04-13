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
    //static logger_t real_logger; // placeholder for shm logger
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    //msg = dlerror();
    if(msg)
    {
        //fprintf(stderr, msg);
    }
    real_realloc = dlsym(RTLD_NEXT, "realloc");
    //msg = dlerror();
    if(msg)
    {
        //fprintf(stderr, msg);
    }
    real_free = dlsym(RTLD_NEXT, "free");
    //msg = dlerror();
    if(msg)
    {
        //fprintf(stderr, msg);
    }
    real_open = dlsym(RTLD_NEXT, "open");
    //msg = dlerror();
    if(msg)
    {
        //fprintf(stderr, msg);
    }
    real_close = dlsym(RTLD_NEXT, "close");
    //msg = dlerror();
    if(msg)
    {
        //fprintf(stderr, msg);
    }
    real_lseek = dlsym(RTLD_NEXT, "lseek");
    //msg = dlerror();
    if(msg)
    {
        //fprintf(stderr, msg);
    }
    real_read = dlsym(RTLD_NEXT, "read");
    //msg = dlerror();
    if(msg)
    {
        //fprintf(stderr, msg);
    }
    real_write = dlsym(RTLD_NEXT, "write");
    //msg = dlerror();
    if(msg)
    {
        //fprintf(stderr, msg);
    }
    //logger = (logger_t*)real_malloc(sizeof(logger_t));
    //logger = &real_logger;
    if(ret == -1)
    {
        //perror("logger_init() failed: ");
    }
    //logger = &real_logger;
}

__attribute__((destructor)) static void deinit(void)
{
    logger_destroy(get_logger());
}