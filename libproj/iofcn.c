#include "iofcn.h"
#include "shared_buf.h"
#include <time.h>

static open_t real_open;
static close_t real_close;
static lseek_t real_lseek;
static read_t real_read;
static write_t real_write;

int open(const char* pathname, int flags, ...)
{
    printf("called open()\n");
    if(!real_open)
    {
        dlerror();
        real_open = dlsym(RTLD_NEXT, "open");
        char* msg = dlerror();
        if(msg)
        {
            fprintf(stderr, "%s", msg);
        }
    }
    return real_open(pathname, flags);
}

int close(int fd)
{
    printf("called close()\n");
    if(!real_close)
    {
        real_close = dlsym(RTLD_NEXT, "close");
    }
    return real_close(fd);
}

off_t lseek(int fd, off_t offset, int whence)
{
    printf("called lseek()\n");
    if(!real_lseek)
    {
        real_lseek = dlsym(RTLD_NEXT, "lseek");
    }
    return real_lseek(fd, offset, whence);
}

ssize_t read(int fd, void* buf, size_t count)
{
    printf("called read()\n");
    if(!real_read)
    {
        real_read = dlsym(RTLD_NEXT, "read");
    }
    return real_read(fd, buf, count);
}

ssize_t write(int fd, const void* buf, size_t count)
{
    printf("called write()\n");
    if(!real_write)
    {
        real_write = dlsym(RTLD_NEXT, "write");
    }
    return real_write(fd, buf, count);
}
