#ifndef MSG_BUF_H
#define MSG_BUF_H
#include "../utility.h"
#include <unistd.h> /* ssize_t */
#include <time.h>

enum FunctionID {
    MALLOC = 1,
    REALLOC,
    FREE,
    OPEN,
    CLOSE,
    LSEEK,
    READ,
    WRITE
};

typedef struct {
    int fn_id;
    struct timespec ts;
    size_t count;
    unsigned char payload[BUF_SIZE];
} msg_t;

/* Write min(buf->count, n) from src to buf from start */
ssize_t msg_buf_write(msg_t* buf, const void* src, size_t n);

#endif /* MSG_BUF_H */