#include "memfcn.h"
#include "buf_queue.h"
#include <time.h>

/* Library internal shared buf queue */
extern buf_queue_t q;

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