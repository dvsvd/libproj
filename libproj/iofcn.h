#ifndef IOFCN_H
#define IOFCN_H
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

typedef int (*open_t)(const char*, int, ...);
typedef int (*close_t)(int);
typedef off_t (*lseek_t)(int, off_t, int);
typedef ssize_t (*read_t)(int, void*, size_t);
typedef ssize_t (*write_t)(int, const void*, size_t);

extern open_t real_open;
extern close_t real_close;
extern lseek_t real_lseek;
extern read_t real_read;
extern write_t real_write;

int open(const char* pathname, int flags, ...);
int close(int fd);
off_t lseek(int fd, off_t offset, int whence);
ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);
#endif /* IOFCN_H */
