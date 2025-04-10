#ifndef SHARED_BUF_H
#define SHARED_BUF_H
#include <pthread.h>
#include <unistd.h> /* ssize_t */

#define BUF_SIZE 4096U

typedef struct {
    size_t count;
    unsigned char buf[BUF_SIZE];
    pthread_rwlock_t lock;
} shared_buf_t;

//TODO: return values of functions
// shared buf write pos
// write with timestamp

/* Initialize shared buffer buf and pass pshared attribute to its rwlock */
int shared_buf_init(shared_buf_t* buf, int pshared);
/* Deinitialize shared buffer buf */
int shared_buf_destroy(shared_buf_t* buf);
/* Write min(BUF_SIZE - pos, n) from src to buf from pos */
ssize_t shared_buf_write_pos(shared_buf_t* buf, size_t pos, const void* src, size_t n);
/* Write min(buf->count, n) from src to buf from start */
ssize_t shared_buf_write(shared_buf_t* buf, const void* src, size_t n);
/* Append min(BUF_SIZE - buf->count, n) bytes from src to buf */
ssize_t shared_buf_append(shared_buf_t* buf, const void* src, size_t n);
/* Read min(buf->count, n) bytes from buf to dst */
ssize_t shared_buf_read(const shared_buf_t* buf, void* dst, size_t n);

#endif /* SHARED_BUF_H */