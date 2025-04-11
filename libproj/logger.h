#ifndef LOGGER_H
#define LOGGER_H
#include "buf_queue.h"
#include "iofcn.h"
#include <stdio.h>
#include <threads.h>

typedef struct
{
    FILE* file;
    buf_queue_t q;
    thrd_t t;
} logger_t;

int logger_init(logger_t* l, const char* pathname);

int log_from_queue(void* data);

int listen_for_messages(logger_t* l);

int logger_destroy(logger_t* l);

#endif /* LOGGER_H */
