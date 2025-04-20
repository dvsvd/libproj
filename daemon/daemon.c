#include "utility.h"
#include "msg_buf.h"
#include <stdlib.h>
#include <mqueue.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <signal.h>

// add-symbol-file /home/a/repos/libproj/build/libproj/liblibprojd.so

static void exit_handler(void)
{

}

static void handler(int sig, siginfo_t* info, void* ucontext)
{
    FILE* logfile = (FILE*)ucontext;
    fflush(logfile);
    fclose(logfile);
}

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

int write_msg(FILE* file, const msg_t* msg)
{
    int ret;
    switch (msg->fn_id)
    {
    case MALLOC:
    {
        const malloc_msg_t* m = (const malloc_msg_t*)msg->payload;
        ret = fprintf(file, "malloc() called: bytes requested: %ld, allocated address: %#0"PRIXPTR"\n", m->size, (uintptr_t)m->addr);
        break;
    }
    case FREE:
    {
        const free_msg_t* m = (const free_msg_t*)msg->payload;
        ret = fprintf(file, "free() called: address: %#0"PRIXPTR"\n", (uintptr_t)m->addr);
        break;
    }
    case REALLOC:
    {
        const realloc_msg_t* m = (const realloc_msg_t*)msg->payload;
        ret = fprintf(file, "realloc() called: bytes requested: %ld, current address: %#0"PRIXPTR
            ", allocated address: %#0"PRIXPTR"\n", m->size, (uintptr_t)m->cur_addr, (uintptr_t)m->alloc_addr);
        break;
    }
    case OPEN:
    {
        const open_msg_t* m = (const open_msg_t*)msg->payload;
        ret = fprintf(file, "open() called: filename: %s, flags: %#0"PRIX32", "
            "fd: %d\n", m->pathname, m->flags, m->fd);
        break;
    }
    case CLOSE:
    {
        const close_msg_t* m = (const close_msg_t*)msg->payload;
        ret = fprintf(file, "close() called: fd: %d, return code: %d\n", m->fd, m->ret);
        break;
    }
    case LSEEK:
    {
        const lseek_msg_t* m = (const lseek_msg_t*)msg->payload;
        ret = fprintf(file, "lseek() called: fd: %d, requested offset: %ld, "
            "whence: %s, resulted offset: %ld\n", m->fd, m->offset, whence_text(m->whence), m->new_pos);
        break;
    }
    case READ:
    {
        const rw_msg_t* m = (const rw_msg_t*)msg->payload;
        ret = fprintf(file, "read() called: fd: %d, buffer pointer: %#0"PRIXPTR
            ", count: %ld, bytes read: %ld\n", m->fd, (uintptr_t)m->buf_ptr, m->count, m->bytes_transmitted);
        break;
    }
    case WRITE:
    {
        const rw_msg_t* m = (const rw_msg_t*)msg->payload;
        ret = fprintf(file, "write() called: fd: %d, buffer pointer: %#0"PRIXPTR
            ", count: %ld, bytes written: %ld\n", m->fd, (uintptr_t)m->buf_ptr, m->count, m->bytes_transmitted);
        break;
    }
    default:
        return -1;
    }
    return ret;
}

int write_with_timestamp(FILE* file, const msg_t* msg)
{
#define timestamp_size 100
    int ret;
    int msecs;
    size_t tmsize; /* size of the timestamp in the buffer */
    struct tm gt = {0};
    char timestamp[timestamp_size] = {0};
    localtime_r(&msg->ts.tv_sec, &gt);
    msecs = msg->ts.tv_nsec / 1000000L;
    tmsize = strftime(timestamp, timestamp_size, "[%F %T.", &gt);
    if(tmsize == 0)
    {
        errno = ENOMEM;
        return -1;
    }
    ret = sprintf(timestamp + tmsize, "%03d]: ", msecs);
    if(ret < 0)
    {
        errno = ENOMSG;
        return -1;
    }
    tmsize += ret;
    fwrite(timestamp, sizeof(char), tmsize, file);
    ret = write_msg(file, msg);
    return ret;
#undef timestamp_size
}

void print_help(void)
{
    printf("Usage: daemon [options]\n"
        "\t-m: start memory functions daemon (starts I/O functions daemon if omitted)\n"
        "\t-t <msecs>: specify message queue poll interval in milliseconds. "
        "If 0 or omitted, goes into blocking mode\n"
        "\t-n <name>: log file name (default: \"libprojdaemon-date-time.log\")\n"
        "\t--help: print this help message\n");
}

void get_default_name(char* buf, size_t size, _Bool is_mem)
{
    time_t t = time(NULL);
    struct tm lt;
    localtime_r(&t, &lt);
    strftime(buf, size, "libprojdaemon-%F-%H-%M-%S-", &lt);
    strcat(buf, is_mem ? "mem.log" : "io.log");
}

int main(int argc, char* argv[])
{
    int ret;
    _Bool is_mem = !!0; /* Start MEM daemon if true; I/O otherwise */
    char buf[NAME_MAX] = {0};
    char* filename = buf;
    char opt;
    static mqd_t qfd;
    ssize_t size;
    FILE* logfile;
    struct timespec interval_ts;
    msg_t msg = {0};
    struct sigaction sa = {0};
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
            filename = optarg;
            break;
        case 't':
        {
            int val = atoi(optarg);
            interval_ts.tv_sec = val / 1000;
            interval_ts.tv_nsec = (val % 1000) * 1000000;
            break;
        }
        default:
            print_help();
            return 0;
        }
    }
    get_default_name(buf, sizeof buf, is_mem);
    if(daemon(0, 0) == -1)
    {
        perror("daemon() failed: ");
        return 1;
    }
    atexit(exit_handler);
    if((logfile = fopen(filename, "a")) == NULL)
    {
        perror("fopen() failed in "__FILE__" at line  "LINESTR);
        return 1;
    }
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    if((qfd = mq_open(is_mem ? MEM_MQ_NAME : IO_MQ_NAME, O_RDONLY)) == -1)
    {
        perror("mq_open() failed in "__FILE__" at line  "LINESTR);
        fclose(logfile);
        return 1;
    }
    while(1)
    {
        if(interval_ts.tv_sec != 0 || interval_ts.tv_nsec != 0)
        {
            if((size = mq_timedreceive(qfd,  (char*)&msg, sizeof msg, 0, &interval_ts)) == -1)
            {
                switch (errno)
                {
                case ETIMEDOUT:
                    continue;
                default:
                    perror("mq_timedreceive() failed in "__FILE__" at line  "LINESTR);
                    fclose(logfile);
                    return 1;
                }
            }
        }
        else if((size = mq_receive(qfd, (char*)&msg, sizeof msg, 0)) == -1)
        {
            perror("mq_receive() failed in "__FILE__" at line  "LINESTR);
            fclose(logfile);
            return 1;
        }
        ret = write_with_timestamp(logfile, &msg);
        if(ret == -1)
        {
            perror("write_with_timestamp() failed in "__FILE__" at line  "LINESTR);
            fclose(logfile);
            return 1;
        }
    }
    fflush(logfile);
    fclose(logfile);
    return 0;
}