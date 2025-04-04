#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

typedef void* (*malloc_t)(size_t size);
typedef void (*free_t)(void*);
typedef void* (*realloc_t)(void*, size_t);

void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
