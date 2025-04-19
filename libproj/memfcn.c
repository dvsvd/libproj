#include "memfcn.h"
#include "utility.h"
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <mqueue.h>

malloc_t real_malloc;
free_t real_free;
realloc_t real_realloc;
atomic_bool is_default = ATOMIC_VAR_INIT(!!1); /* Default memory functions behaviour flag */

/* Library internal get_logger() */
//logger_t* get_logger();

void* malloc(size_t size)
{
    mqd_t fd;
    int ret;
    void* tmp;
    int err; /* real errno */
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    fd = get_mem_mq();
    if(is_default)
    {
        return real_malloc(size);
    }
    if((fd = get_mem_mq()) == -1)
    {
        perror("get_mem_mq() failed: ");
        exit(1337);
    }
    tmp = real_malloc(size);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "malloc() called: bytes requested: %d, allocated address: %#0\n" PRIXPTR, size, (uintptr_t)tmp);
    if(mq_send(fd, msg, ret, 0) == -1)
    {
        perror("mq_send() failed: ");
    }
    errno = err; /* Restore real errno */
    return tmp;
}

void free(void* ptr)
{
    int ret;
    mqd_t fd;
    struct timespec ts = {0};
    shared_buf_t* buf;
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    if(is_default)
    {
        return real_free(ptr);
    }
    if((fd = get_mem_mq()) == -1)
    {
        perror("get_mem_mq() failed: ");
        exit(1337);
    }
    ret = snprintf(msg, MSG_SIZE, "free() called: address: %#0\n" PRIXPTR, (uintptr_t)ptr);
    if(mq_send(fd, msg, ret, 0) == -1)
    {
        perror("mq_send() failed: ");
    }
    real_free(ptr);
}

void* realloc(void* ptr, size_t size)
{
    mqd_t fd;
    void* tmp;
    int err; /* real errno */
    int ret;
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    if(is_default)
    {
        return real_realloc(ptr, size);
    }
    if((fd = get_mem_mq()) == -1)
    {
        perror("get_mem_mq() failed: ");
        exit(1337);
    }
    tmp = real_realloc(ptr, size);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "realloc() called: bytes requested: %d, current address: %#0"PRIXPTR
                    ", allocated address: %#0\n" PRIXPTR, size, (uintptr_t)ptr, (uintptr_t)tmp);
    if(mq_send(fd, msg, ret, 0) == -1)
    {
        perror("mq_send() failed: ");
    }
    errno = err; /* Restore real errno */
    return tmp;
}