#include "memfcn.h"
#include "msg_buf.h"
#include "utility.h"
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <mqueue.h>
#include <string.h>

malloc_t real_malloc;
free_t real_free;
realloc_t real_realloc;
perror_t real_perror;
atomic_bool is_default = ATOMIC_VAR_INIT(!!1); /* Default memory functions behaviour flag */

void perror(const char* s)
{
    is_default = !!1;
    real_perror(s);
    is_default = !!0;
}

void* malloc(size_t size)
{
    mqd_t qfd;
    int ret;
    void* tmp;
    int err; /* real errno */
    msg_t m;
    char* e_msg;
    timespec_get(&m.ts, TIME_UTC);
    if(!real_malloc)
    {
        real_malloc = dlsym(RTLD_NEXT, "malloc");
        e_msg = dlerror();
        if(e_msg)
        {
            fprintf(stderr, "%s\n", e_msg);
        }
    }
    if(is_default)
    {
        return real_malloc(size);
    }
    qfd = get_mem_mq();
    tmp = real_malloc(size);
    err = errno;
    m.fn_id = MALLOC;
    m.pid = getpid();
    m.malloc_msg.size = size;
    m.malloc_msg.addr = tmp;
    //memset(&m.malloc_msg + sizeof m.malloc_msg, 0, sizeof m.payload - sizeof m.malloc_msg);
    if(mq_send(qfd, (const char*)&m, sizeof(msg_t), 0) == -1)
    {
        switch (errno)
        {
        case EBADF:
            break;
        default:
            perror("mq_send() failed in "__FILE__" at line "LINESTR);
        }
    }
    errno = err; /* Restore real errno */
    return tmp;
}

void free(void* ptr)
{
    int ret;
    mqd_t qfd;
    msg_t m;
    timespec_get(&m.ts, TIME_UTC);
    char* e_msg;
    if(!real_free)
    {
        real_free = dlsym(RTLD_NEXT, "free");
        e_msg = dlerror();
        if(e_msg)
        {
            fprintf(stderr, "%s\n", e_msg);
        }
    }
    if(is_default)
    {
        return real_free(ptr);
    }
    qfd = get_mem_mq();
    m.fn_id = FREE;
    m.pid = getpid();
    m.free_msg.addr = ptr;
    //memset(&m.free_msg + sizeof m.free_msg, 0, sizeof m.payload - sizeof m.free_msg);
    if(mq_send(qfd, (const char*)&m, sizeof(msg_t), 0) == -1)
    {
        switch (errno)
        {
        case EBADF:
            break;
        default:
            perror("mq_send() failed in "__FILE__" at line "LINESTR);
        }
    }
    real_free(ptr);
}

void* realloc(void* ptr, size_t size)
{
    mqd_t qfd;
    void* tmp;
    int err; /* real errno */
    int ret;
    msg_t m;
    char* e_msg;
    timespec_get(&m.ts, TIME_UTC);
    if(!real_realloc)
    {
        real_realloc = dlsym(RTLD_NEXT, "realloc");
        e_msg = dlerror();
        if(e_msg)
        {
            fprintf(stderr, "%s\n", e_msg);
        }
    }
    if(is_default)
    {
        return real_realloc(ptr, size);
    }
    qfd = get_mem_mq();
    tmp = real_realloc(ptr, size);
    err = errno;
    m.fn_id = REALLOC;
    m.pid = getpid();
    m.realloc_msg.cur_addr = ptr;
    m.realloc_msg.alloc_addr = tmp;
    m.realloc_msg.size = size;
    //memset(&m.realloc_msg + sizeof m.realloc_msg, 0, sizeof m.payload - sizeof m.realloc_msg);
    if(mq_send(qfd, (const char*)&m, sizeof(msg_t), 0) == -1)
    {
        switch (errno)
        {
        case EBADF:
            break;
        default:
            perror("mq_send() failed in "__FILE__" at line "LINESTR);
        }
    }
    errno = err; /* Restore real errno */
    return tmp;
}