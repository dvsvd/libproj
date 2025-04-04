#ifndef SHARED_BUF_H
#define SHARED_BUF_H
#include <pthread.h>
#include <unistd.h> // ssize_t

#define BUF_SIZE 4096U

typedef struct {
    size_t count;
    unsigned char buf[BUF_SIZE];
    pthread_rwlock_t lock;
} shared_buf_t;

/* Library internal shared buf */
extern shared_buf_t lib_shared_buf;

int shared_buf_init(shared_buf_t* buf, int pshared);
int shared_buf_destroy(shared_buf_t* buf);
int shared_buf_write(shared_buf_t* buf, const void* src, size_t n);
int shared_buf_read(const shared_buf_t* buf, void* dst, size_t n);

#endif // SHARED_BUF_H