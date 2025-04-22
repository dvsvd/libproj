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

typedef struct {
    int fn_id;
    struct timespec ts;
    pid_t pid;
    union {
        malloc_msg_t malloc_msg;
        free_msg_t free_msg;
        realloc_msg_t realloc_msg;
        open_msg_t open_msg;
        close_msg_t close_msg;
        lseek_msg_t lseek_msg;
        rw_msg_t rw_msg;
    } payload;
#define malloc_msg payload.malloc_msg
#define free_msg payload.free_msg
#define realloc_msg payload.realloc_msg
#define open_msg payload.open_msg
#define close_msg payload.close_msg
#define lseek_msg payload.lseek_msg
#define rw_msg payload.rw_msg
} msg_t;

#endif /* MSG_BUF_H */