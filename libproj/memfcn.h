#ifndef MEMFCN_H
#define MEMFCN_H
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdatomic.h>

typedef struct {
    size_t size;
    void* addr;
} malloc_msg_t;

typedef struct {
    void* addr;
} free_msg_t;

typedef struct {
    size_t size;
    void* cur_addr;
    void* alloc_addr;
} realloc_msg_t;

typedef void* (*malloc_t)(size_t size);
typedef void (*free_t)(void*);
typedef void* (*realloc_t)(void*, size_t);

extern malloc_t real_malloc;
extern free_t real_free;
extern realloc_t real_realloc;

extern atomic_bool is_default;

void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
#endif /* MEMFCN_H */
