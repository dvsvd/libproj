#ifndef MEMFCN_H
#define MEMFCN_H
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdatomic.h>

typedef void* (*malloc_t)(size_t size);
typedef void (*free_t)(void*);
typedef void* (*realloc_t)(void*, size_t);
typedef void (*perror_t)(const char*);

extern malloc_t real_malloc;
extern free_t real_free;
extern realloc_t real_realloc;
extern perror_t real_perror;

extern atomic_bool is_default;

#endif /* MEMFCN_H */
