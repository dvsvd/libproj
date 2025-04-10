#include "memfcn.h"
#include "shared_buf.h"
#include <time.h>

/* Library internal shared buf */
extern shared_buf_t lib_shared_buf;

malloc_t real_malloc;
free_t real_free;
realloc_t real_realloc;

void* malloc(size_t size)
{
    return real_malloc(size);
}

void free(void* ptr)
{
    return real_free(ptr);
}

void* realloc(void* ptr, size_t size)
{
    return real_realloc(ptr, size);
}