#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <fcntl.h>

__attribute__((constructor)) static void setup(void)
{
    printf("called setup()\n");
}