#ifndef MSG_BUF_H
#define MSG_BUF_H
#include "utility.h"
#include <unistd.h> /* ssize_t */
#include <time.h>
#include <limits.h>

enum FunctionID {
    MALLOC = 1,
    FREE,
    REALLOC,
    OPEN,
    CLOSE,
    LSEEK,
    READ,
    WRITE
};

typedef struct {
    size_t size;
    void* addr;
} malloc_msg_t;

typedef struct {
    void* addr;
} free_msg_t;

typedef struct {
    size_t size;
    void* cur_addr;
    void* alloc_addr;
} realloc_msg_t;

typedef struct {
    int flags;
    int fd;
    size_t pathlen;
    char pathname[PATH_MAX];
} open_msg_t;

typedef struct {
    int ret;
    int fd;
} close_msg_t;

typedef struct {
    off_t new_pos;
    off_t offset;
    int fd;
    int whence;
} lseek_msg_t;

typedef struct {
    size_t count;
    ssize_t bytes_transmitted;
    void* buf_ptr;
    int fd;
} rw_msg_t;

#define PAYLOAD_SIZE \
    MAX(\
        MAX(\
            MAX(\
                MAX(\
                    MAX(\
                        MAX(\
                            sizeof(rw_msg_t), sizeof(lseek_msg_t)),\
                        sizeof(close_msg_t)),\
                    sizeof(open_msg_t)),\
                sizeof(realloc_msg_t)),\
            sizeof(free_msg_t)),\
        sizeof(malloc_msg_t))

typedef struct {
    int fn_id;
    struct timespec ts;
    //size_t count;
    unsigned char payload[PAYLOAD_SIZE];
} msg_t;

#endif /* MSG_BUF_H */