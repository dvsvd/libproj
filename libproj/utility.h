#ifndef UTILITY_H
#define UTILITY_H
#include "shared_buf.h"
#include "logger.h"
#include <time.h>

#define MSG_SIZE 256
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

logger_t* get_logger();
int write_with_timestamp(shared_buf_t* buf, const void* src, size_t n, struct timespec* ts);
#endif /* UTILITY_H */
