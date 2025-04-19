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
atomic_bool is_default = ATOMIC_VAR_INIT(!!1); /* Default memory functions behaviour flag */

/* Library internal get_logger() */
//logger_t* get_logger();

void* malloc(size_t size)
{
    mqd_t fd;
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
    if((fd = get_mem_mq()) == -1)
    {
        perror("get_mem_mq() failed: ");
        //exit(1337);
    }
    tmp = real_malloc(size);
    err = errno;
    m.fn_id = MALLOC;
    msg.size = size;
    msg.addr = tmp;
    memcpy(m.payload, &msg, sizeof(malloc_msg_t));
    memset(m.payload + sizeof(malloc_msg_t), 0, sizeof m.payload - sizeof(malloc_msg_t));
    if(mq_send(fd, &m, sizeof(msg_t), 0) == -1)
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
    msg_t m;
    free_msg_t msg;
    timespec_get(&m.ts, TIME_UTC);
    if(is_default)
    {
        return real_free(ptr);
    }
    if((fd = get_mem_mq()) == -1)
    {
        perror("get_mem_mq() failed: ");
        //exit(1337);
    }
    m.fn_id = FREE;
    msg.addr = ptr;
    memcpy(m.payload, &msg, sizeof(free_msg_t));
    memset(m.payload + sizeof(free_msg_t), 0, sizeof m.payload - sizeof(free_msg_t));
    if(mq_send(fd, &m, sizeof(msg_t), 0) == -1)
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
    msg_t m;
    realloc_msg_t msg;
    timespec_get(&m.ts, TIME_UTC);
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
    m.fn_id = REALLOC;
    msg.cur_addr = ptr;
    msg.alloc_addr = tmp;
    msg.size = size;
    memcpy(m.payload, &msg, sizeof(realloc_msg_t));
    memset(m.payload + sizeof(realloc_msg_t), 0, sizeof m.payload - sizeof(realloc_msg_t));
    if(mq_send(fd, &m, sizeof(msg_t), 0) == -1)
    {
        perror("mq_send() failed: ");
    }
    errno = err; /* Restore real errno */
    return tmp;
}