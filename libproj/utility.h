#ifndef UTILITY_H
#define UTILITY_H
#include "shared_buf.h"

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

int write_with_timestamp(shared_buf_t* buf, const void* src, size_t n);
#endif /* UTILITY_H */
