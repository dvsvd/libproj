#include "iofcn.h"
#include "shared_buf.h"
#include <time.h>
#include <stdarg.h>

static open_t real_open;
static close_t real_close;
static lseek_t real_lseek;
static read_t real_read;
static write_t real_write;

/* Library internal shared buf */
extern shared_buf_t lib_shared_buf;

int open(const char* pathname, int flags, ...)
{
    mode_t mode = 0;
    if(!real_open)
    {
        dlerror();
        real_open = dlsym(RTLD_NEXT, "open");
        char* msg = dlerror();
        if(msg)
        {
            fprintf(stderr, "%s", msg);
            exit(0x1337);
        }
    }
    if(flags & O_CREAT || flags & O_TMPFILE)
    {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }
    return real_open(pathname, flags, mode);
}

int close(int fd)
{
    if(!real_close)
    {
        real_close = dlsym(RTLD_NEXT, "close");
    }
    return real_close(fd);
}

off_t lseek(int fd, off_t offset, int whence)
{
    if(!real_lseek)
    {
        real_lseek = dlsym(RTLD_NEXT, "lseek");
    }
    return real_lseek(fd, offset, whence);
}

ssize_t read(int fd, void* buf, size_t count)
{
    if(!real_read)
    {
        real_read = dlsym(RTLD_NEXT, "read");
    }
    return real_read(fd, buf, count);
}

ssize_t write(int fd, const void* buf, size_t count)
{
    if(!real_write)
    {
        real_write = dlsym(RTLD_NEXT, "write");
    }
    return real_write(fd, buf, count);
}
