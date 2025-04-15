#include "memfcn.h"
#include "logger.h"
#include "utility.h"
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>

malloc_t real_malloc;
free_t real_free;
realloc_t real_realloc;
//atomic_bool is_default = ATOMIC_VAR_INIT(!!1); /* Default memory functions behaviour flag */

/* Library internal get_logger() */
//logger_t* get_logger();

void* malloc(size_t size)
{
    char* e_msg;
    int ret;
    void* tmp;
    int err; /* real errno */
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    if(!real_malloc)
    {
        real_malloc = dlsym(RTLD_NEXT, "malloc");
        //e_msg = dlerror();
        if(e_msg)
        {
            //fprintf(stderr, e_msg);
        }
    }
    if(is_default)
    {
        return real_malloc(size);
    }
    tmp = real_malloc(size);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "malloc() called: bytes requested: %d, allocated address: %#0\n" PRIXPTR, size, (uintptr_t)tmp);
    log_write(get_logger(), msg, ret, &ts);
    errno = err; /* Restore real errno */
    return tmp;
}

void free(void* ptr)
{
    char* e_msg;
    int ret;
    struct timespec ts = {0};
    shared_buf_t* buf;
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    if(!real_free)
    {
        real_free = dlsym(RTLD_NEXT, "free");
        //e_msg = dlerror();
        if(e_msg)
        {
            //fprintf(stderr, e_msg);
        }
    }
    if(is_default)
    {
        return real_free(ptr);
    }
    buf = (shared_buf_t*)real_malloc(sizeof(shared_buf_t));
    ret = shared_buf_init(buf, PTHREAD_PROCESS_SHARED);
    ret = snprintf(msg, MSG_SIZE, "free() called: address: %#0\n" PRIXPTR, (uintptr_t)ptr);
    log_write(get_logger(), msg, ret, &ts);
    real_free(ptr);
}

void* realloc(void* ptr, size_t size)
{
    void* tmp;
    char* e_msg;
    int err; /* real errno */
    int ret;
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    if(!real_realloc)
    {
        real_realloc = dlsym(RTLD_NEXT, "realloc");
        //e_msg = dlerror();
        if(e_msg)
        {
            //fprintf(stderr, e_msg);
        }
    }
    if(is_default)
    {
        return real_realloc(ptr, size);
    }
    tmp = real_realloc(ptr, size);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "realloc() called: bytes requested: %d, current address: %#0"PRIXPTR", \
allocated address: %#0\n" PRIXPTR, size, (uintptr_t)ptr, (uintptr_t)tmp);
    log_write(get_logger(), msg, ret, &ts);
    errno = err; /* Restore real errno */
    return tmp;
}