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
/* Library internal get_logger() */
//logger_t* get_logger();

void* malloc(size_t size)
{
    void* tmp;
    int err; /* real errno */
    int ret;
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    tmp = real_malloc(size);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "malloc() called: bytes requested: %d, allocated address: %#0\n" PRIXPTR, size, (uintptr_t)tmp);
    log_write(get_logger(), msg, ret, &ts);
    errno = err; /* Restore real errno */
    return tmp;
}

void free(void* ptr)
{
    int ret;
    struct timespec ts = {0};
    shared_buf_t* buf;
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    buf = (shared_buf_t*)real_malloc(sizeof(shared_buf_t));
    ret = shared_buf_init(buf, PTHREAD_PROCESS_SHARED);
    ret = snprintf(msg, MSG_SIZE, "free() called: address: %#0\n" PRIXPTR, (uintptr_t)ptr);
    log_write(get_logger(), msg, ret, &ts);
    real_free(ptr);
}

void* realloc(void* ptr, size_t size)
{
    void* tmp;
    int err; /* real errno */
    int ret;
    struct timespec ts = {0};
    unsigned char msg[MSG_SIZE] = {0};
    timespec_get(&ts, TIME_UTC);
    tmp = real_realloc(ptr, size);
    err = errno;
    ret = snprintf(msg, MSG_SIZE, "realloc() called: bytes requested: %d, current address: %#0"PRIXPTR", \
allocated address: %#0\n" PRIXPTR, size, (uintptr_t)ptr, (uintptr_t)tmp);
    log_write(get_logger(), msg, ret, &ts);
    errno = err; /* Restore real errno */
    return tmp;
}