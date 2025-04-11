#ifndef BUF_QUEUE_H
#define BUF_QUEUE_H
#include "shared_buf.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

typedef struct node_t {
    shared_buf_t* data;
    struct node_t* next;
} node_t;

typedef struct {
    node_t* front;
    node_t* rear;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} buf_queue_t;

int buf_queue_init(buf_queue_t *q, int pshared);

int push(buf_queue_t *q, shared_buf_t* data);

/*  Returns pointer to the data of the front element and pops it.
*   The caller is responsible for clearing out the returned data */
shared_buf_t* pop(buf_queue_t *q);

int buf_queue_destroy(buf_queue_t *q);
#endif /* BUF_QUEUE_H */
