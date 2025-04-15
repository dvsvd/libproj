#define _GNU_SOURCE
#include "buf_queue.h"
#include "utility.h"
#include "memfcn.h"
#include "iofcn.h"
#include "logger.h"
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdatomic.h>

#define SHMEM_NAME "/4B4A2572-E94D-4448-B52C-509352C4AC3D"

__attribute__((constructor)) static void setup(void)
{
    char* msg;
    int shmfd;
    int ret;
    volatile atomic_bool* is_default;
    logger_t* loggermem;
    real_open = dlsym(RTLD_NEXT, "open");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, msg);
        return;
    }
    real_close = dlsym(RTLD_NEXT, "close");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, msg);
        return;
    }
    real_lseek = dlsym(RTLD_NEXT, "lseek");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, msg);
        return;
    }
    real_read = dlsym(RTLD_NEXT, "read");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, msg);
        return;
    }
    real_write = dlsym(RTLD_NEXT, "write");
    msg = dlerror();
    if(msg)
    {
        fprintf(stderr, msg);
        return;
    }
    shmfd = shm_open(SHMEM_NAME, O_RDWR | O_CREAT, 0600);
    if(shmfd == -1)
    {
        perror("shm_open() failed: ");
        return;
    }
    if(ftruncate(shmfd, sizeof(logger_t)) == -1)
    {
        perror("ftruncate() failed: ");
        return;
    }
    /* Allocate shared memory for the message queue */
    loggermem = (logger_t*)mmap(NULL, sizeof(logger_t) + sizeof(atomic_bool),
    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_32BIT, shmfd, 0);
    if(loggermem == MAP_FAILED)
    {
        perror("mmap() failed: ");
        return;
    }
    if(logger_init(loggermem, "log.txt") == -1)
    {
        perror("logger_init() failed: ");
    }
    is_default = !!0;
}

__attribute__((destructor)) static void deinit(void)
{
    if(shm_unlink(SHMEM_NAME) == -1)
    {
        perror("shm_unlink() failed: ");
    }
    logger_destroy(get_logger());
}