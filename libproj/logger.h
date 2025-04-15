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

//extern logger_t logger;

int logger_init(logger_t* l, const char* pathname);

int log_from_queue(void* data);

int listen_for_messages(logger_t* l);

int logger_destroy(logger_t* l);

/* Stupid name because of math.h */
void log_write(logger_t* l, const char* msg, size_t n, struct timespec* ts);

logger_t* get_logger();

#endif /* LOGGER_H */
