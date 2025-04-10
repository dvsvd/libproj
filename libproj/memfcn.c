#include "memfcn.h"
#include "shared_buf.h"
#include <time.h>

/* Library internal shared buf */
extern shared_buf_t lib_shared_buf;

static malloc_t real_malloc;
static free_t real_free;
static realloc_t real_realloc;

void* malloc(size_t size)
{
    if(!real_malloc)
    {
        dlerror();
        real_malloc = dlsym(RTLD_NEXT, "malloc");
        char* msg = dlerror();
        if(msg)
        {
        }
    }
    return real_malloc(size);
}

void free(void* ptr)
{
    if(!real_free)
    {
        real_free = dlsym(RTLD_NEXT, "free");
    }
    return real_free(ptr);
}

void* realloc(void* ptr, size_t size)
{
    if(!real_realloc)
    {
        real_realloc = dlsym(RTLD_NEXT, "realloc");
    }
    return real_realloc(ptr, size);
}