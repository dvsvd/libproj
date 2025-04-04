#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <fcntl.h>
#include "shared_buf.h"

/* Library internal shared buf */
shared_buf_t lib_shared_buf;

__attribute__((constructor)) static void setup(void)
{
    /* Initialize library shared buf */
    shared_buf_init(&lib_shared_buf, PTHREAD_PROCESS_SHARED);
}

__attribute__((destructor)) static void deinit(void)
{
    /* Initialize library shared buf */
    shared_buf_destroy(&lib_shared_buf);
}