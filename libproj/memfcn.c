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
atomic_bool is_default = !!1; /* Default memory functions behaviour flag */

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

    /* Get call time */
    timespec_get(&m.ts, TIME_UTC);

    /* Get real_malloc if not found already */
    if(!real_malloc)
    {
        real_malloc = dlsym(RTLD_NEXT, "malloc");
        e_msg = dlerror();
        if(e_msg)
        {
            fprintf(stderr, "%s\n", e_msg);
        }
    }

    /* Do not perform logging in certain situations */
    if(is_default)
    {
        return real_malloc(size);
    }

    /* Get mq */
    qfd = get_mem_mq();

    /* Execute real function and save its errno */
    tmp = real_malloc(size);
    err = errno;

    /* Fill in the message */
    m.fn_id = MALLOC;
    m.pid = getpid();
    m.malloc_msg.size = size;
    m.malloc_msg.addr = tmp;

    /* Send the message */
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

    /* Restore real errno */
    errno = err;
    return tmp;
}

void free(void* ptr)
{
    int ret;
    mqd_t qfd;
    msg_t m;
    char* e_msg;

    /* Get call time */
    timespec_get(&m.ts, TIME_UTC);

    /* Get real_free if not found already */
    if(!real_free)
    {
        real_free = dlsym(RTLD_NEXT, "free");
        e_msg = dlerror();
        if(e_msg)
        {
            fprintf(stderr, "%s\n", e_msg);
        }
    }

    /* Do not perform logging in certain situations */
    if(is_default)
    {
        return real_free(ptr);
    }

    /* Get mq */
    qfd = get_mem_mq();

    /* Fill in the message */
    m.fn_id = FREE;
    m.pid = getpid();
    m.free_msg.addr = ptr;

    /* Send the message */
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

    /* Get call time */
    timespec_get(&m.ts, TIME_UTC);

    /* Get real_realloc if not found already */
    if(!real_realloc)
    {
        real_realloc = dlsym(RTLD_NEXT, "realloc");
        e_msg = dlerror();
        if(e_msg)
        {
            fprintf(stderr, "%s\n", e_msg);
        }
    }

    /* Do not perform logging in certain situations */
    if(is_default)
    {
        return real_realloc(ptr, size);
    }

    /* Get mq */
    qfd = get_mem_mq();

    /* Execute real function and save its errno */
    tmp = real_realloc(ptr, size);
    err = errno;

    /* Fill in the message */
    m.fn_id = REALLOC;
    m.pid = getpid();
    m.realloc_msg.cur_addr = ptr;
    m.realloc_msg.alloc_addr = tmp;
    m.realloc_msg.size = size;

    /* Send the message */
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
    
    /* Restore real errno */
    errno = err;
    return tmp;
}