#include "utility.h"
#include <stdlib.h>
#include <mqueue.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>

/*
    ret = snprintf(msg, MSG_SIZE, "malloc() called: bytes requested: %d, allocated address: %#0\n" PRIXPTR, size, (uintptr_t)tmp);

    ret = snprintf(msg, MSG_SIZE, "free() called: address: %#0\n" PRIXPTR, (uintptr_t)ptr);

    ret = snprintf(msg, MSG_SIZE, "realloc() called: bytes requested: %d, current address: %#0"PRIXPTR
        ", allocated address: %#0\n" PRIXPTR, size, (uintptr_t)ptr, (uintptr_t)tmp);

    ret = snprintf(msg, MSG_SIZE, "open() called: filename: %s, flags: %#0X, "
                "fd: %d\n", pathname, flags, tmp);

    ret = snprintf(msg, MSG_SIZE, "close() called: fd: %d, return code: %d\n", fd, tmp);

    ret = snprintf(msg, MSG_SIZE, "lseek() called: filename: %d, requested offset: %d, "
                "whence: %s, resulted offset: %d\n", fd, offset, whence_text(whence), tmp);

    ret = snprintf(msg, MSG_SIZE, "lseek() called: fd: %d, buf pointer: %#0"PRIXPTR
                ", count: %d, bytes read: %d\n", fd, (uintptr_t)buf, count, tmp);
                
    ret = snprintf(msg, MSG_SIZE, "lseek() called: fd: %d, buf pointer: %#0"PRIXPTR
    ", count: %d, bytes written: %d\n", fd, (uintptr_t)buf, count, tmp);
*/

const char* whence_text(int whence)
{
    switch (whence)
    {
    case SEEK_CUR:
        return "SEEK_CUR";
    case SEEK_SET:
        return "SEEK_SET";
    case SEEK_END:
        return "SEEK_END";
    default:
        return "UNKNOWN";
    }
}

//TODO write with timestamp

int write_with_timestamp(FILE* file, const char* src, size_t n, struct timespec* ts)
{
    const size_t timestamp_size = 100;
    int ret;
    int msecs;
    size_t tmsize; // size of the timestamp in the buffer
    //struct timespec ts = {0};
    struct tm gt = {0};
    char timestamp[timestamp_size] = {0};
    // ret = timespec_get(ts, TIME_UTC);
    // if(ret == 0)
    // {
    //     errno = ENOMSG;
    //     return -1;
    // }
    localtime_r(&ts->tv_sec, &gt);
    msecs = ts->tv_nsec / 1000000L;
    tmsize = strftime(timestamp, timestamp_size, "[%F %T.", &gt);
    if(tmsize == 0)
    {
        errno = ENOMEM;
        return -1;
    }
    ret = sprintf(timestamp + tmsize, "%03d] ", msecs);
    if(ret < 0)
    {
        errno = ENOMSG;
        return -1;
    }
    tmsize += ret;
    fwrite(timestamp, sizeof(char), tmsize, file);
    fwrite(src, sizeof(char), n, file);
    return 0;
}

void print_help(void)
{
    printf("Usage: daemon [options]\n"
        "\t-m: start memory functions daemon (starts I/O functions daemon if omitted)\n"
        "\t-t <msecs>: specify message queue poll interval (default: 0)\n"
        "\t-n <name>: log file name (default: \"/var/log/libprojdaemon.log\")\n"
        "\t--help: print this help message");
}

int main(int argc, char* argv[])
{
    _Bool is_mem = !!0; /* Start MEM daemon if true; I/O otherwise */
    char* filename = "log.txt";
    char opt;
    static mqd_t qfd;
    ssize_t size;
    FILE* logfile;
    struct timespec ts = {0};
    char msg[MSG_SIZE] = {0};
    if(argc < 2 || strcmp(argv[1], "--help") == 0)
    {
        print_help();
        return 0;
    }
    while((opt = getopt(argc, argv, "mt:n:")) != -1)
    {
        switch (opt)
        {
        case 'm':
            is_mem = !!1;
            break;
        case 'n':
            logfile = optarg;
            break;
        case 't':
            ts.tv_sec = atoi(optarg);
            break;
        default:
            print_help();
            return 0;
        }
    }
    if(daemon(0, 0) == -1)
    {
        perror("daemon() failed: ");
        return 1;
    }
    if((logfile = fopen(filename, "a")) == NULL)
    {
        perror("fopen() failed: ");
        return 1;
    }
    if((qfd = mq_open(is_mem ? MEM_MQ_NAME : IO_MQ_NAME, O_RDONLY, 0600)) == -1)
    {
        perror("mq_open() failed: ");
        fclose(logfile);
        return 1;
    }
    while(1)
    {
        if(ts.tv_sec != 0 && (size = mq_timedreceive(qfd,  msg, MSG_SIZE, 0, &ts)) == -1)
        {
            switch (errno)
            {
            case ETIMEDOUT:
                continue;
            default:
                perror("mq_timedreceive() failed: ");
                fclose(logfile);
                return 1;
            }
        }
        else if((size = mq_receive(qfd,  msg, MSG_SIZE, 0)) == -1)
        {
            perror("mq_receive() failed: ");
            fclose(logfile);
            return 1;
        }
        fwrite(msg, sizeof(char), size, logfile);
    }
    fclose(logfile);
    return 0;
}