#include "memfcn.h"
#include "shared_buf.h"
#include <time.h>

static malloc_t real_malloc;
static free_t real_free;
static realloc_t real_realloc;

void* malloc(size_t size)
{
    //printf("called malloc()\n");
    if(!real_malloc)
    {
        dlerror();
        real_malloc = dlsym(RTLD_NEXT, "malloc");
        char* msg = dlerror();
        if(msg)
        {
            //fprintf(stderr, "%s", msg);
        }
    }
    return real_malloc(size);
}

void free(void* ptr)
{
    //printf("called free()\n");
    if(!real_free)
    {
        real_free = dlsym(RTLD_NEXT, "free");
    }
    return real_free(ptr);
}

void* realloc(void* ptr, size_t size)
{
    //printf("called realloc()\n");
    if(!real_realloc)
    {
        real_realloc = dlsym(RTLD_NEXT, "realloc");
    }
    return real_realloc(ptr, size);
}