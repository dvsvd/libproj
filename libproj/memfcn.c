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
    malloc_msg_t msg;
    timespec_get(&m.ts, TIME_UTC);
    if(is_default)
    {
        return real_malloc(size);
    }
    if((qfd = get_mem_mq()) == -1)
    {
        perror("get_mem_mq() failed in "__FILE__" at line "LINESTR);
    }
    tmp = real_malloc(size);
    err = errno;
    m.fn_id = MALLOC;
    msg.size = size;
    msg.addr = tmp;
    memcpy(m.payload, &msg, sizeof msg);
    memset(m.payload + sizeof msg, 0, sizeof m.payload - sizeof msg);
    if(mq_send(qfd, (const char*)&m, sizeof(msg_t), 0) == -1)
    {
        perror("mq_send() failed in "__FILE__" at line "LINESTR);
    }
    errno = err; /* Restore real errno */
    return tmp;
}

void free(void* ptr)
{
    int ret;
    mqd_t qfd;
    msg_t m;
    free_msg_t msg;
    timespec_get(&m.ts, TIME_UTC);
    if(is_default)
    {
        return real_free(ptr);
    }
    if((qfd = get_mem_mq()) == -1)
    {
        perror("get_mem_mq() failed in "__FILE__" at line "LINESTR);
    }
    m.fn_id = FREE;
    msg.addr = ptr;
    memcpy(m.payload, &msg, sizeof msg);
    memset(m.payload + sizeof msg, 0, sizeof m.payload - sizeof msg);
    if(mq_send(qfd, (const char*)&m, sizeof(msg_t), 0) == -1)
    {
        perror("mq_send() failed in "__FILE__" at line "LINESTR);
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
    realloc_msg_t msg;
    timespec_get(&m.ts, TIME_UTC);
    if(is_default)
    {
        return real_realloc(ptr, size);
    }
    if((qfd = get_mem_mq()) == -1)
    {
        perror("get_mem_mq() failed in "__FILE__" at line "LINESTR);

    }
    tmp = real_realloc(ptr, size);
    err = errno;
    m.fn_id = REALLOC;
    msg.cur_addr = ptr;
    msg.alloc_addr = tmp;
    msg.size = size;
    memcpy(m.payload, &msg, sizeof msg);
    memset(m.payload + sizeof msg, 0, sizeof m.payload - sizeof msg);
    if(mq_send(qfd, (const char*)&m, sizeof(msg_t), 0) == -1)
    {
        perror("mq_send() failed in "__FILE__" at line "LINESTR);
    }
    errno = err; /* Restore real errno */
    return tmp;
}