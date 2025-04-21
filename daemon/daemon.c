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
#include <sys/stat.h>
#include <syslog.h>
#include <stdatomic.h>

// add-symbol-file /home/a/repos/libproj/build/libproj/liblibprojd.so

atomic_bool is_stopped = ATOMIC_VAR_INIT(!!0);

static void skeleton_daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    //chdir(dir == NULL ? "/" : dir);

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog ("logdaemon", LOG_PID, LOG_DAEMON);
}

static void handler(int)
{
    is_stopped = !!1;
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
    _Bool is_mem = !!0; /* Start MEM daemon if true; I/O daemon otherwise */
    char buf[NAME_MAX] = {0};
    char* filename = NULL;
    char* que_name;
    char* daemon_type;
    char opt;
    static mqd_t qfd;
    ssize_t size;
    FILE* logfile;
    time_t interval;
    msg_t msg = {0};
    struct sigaction sa = {0};
    struct mq_attr a = {0};
    int mqflags = O_RDONLY | O_CREAT;

    if(argc < 2 || strcmp(argv[1], "--help") == 0)
    {
        print_help();
        return 0;
    }

    /* Parse options */
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
            /* Multiply by 1000 to get interval in usecs */
            interval = atoi(optarg) * 1000;
            break;
        default:
            print_help();
            return 0;
        }
    }

    /* Initialize log file name */
    daemon_type = is_mem ? "MEM" : "I/O";
    if(filename == NULL)
    {
        get_default_name(buf, sizeof buf, is_mem);
        filename = buf;
    }

    /* Daemonize */
    skeleton_daemon();

    if((ret = open(filename, O_CREAT | O_APPEND | O_WRONLY | O_EXCL, 0666)) == -1)
    {
        syslog(LOG_ERR, "open() failed in "__FILE__" at line "LINESTR": %s", strerror(errno));
        goto cleanup_base;
    }
    if((logfile = fdopen(ret, "a")) == NULL)
    {
        syslog(LOG_ERR, "fdopen() failed in "__FILE__" at line "LINESTR": %s", strerror(errno));
        goto cleanup_base;
    }

    /* Set up termination handler */
    sa.sa_handler = handler;
    sigaction(SIGTERM, &sa, NULL);
    //sigaction(SIGINT, &sa, NULL);

    /* Set up mq attributes */
    a.mq_maxmsg = 10;
    a.mq_msgsize = sizeof(msg_t);
    que_name = is_mem ? MEM_MQ_NAME : IO_MQ_NAME;
    if(interval != 0) mqflags |= O_NONBLOCK;

    /* Open mq */
    mq_unlink(que_name);
    if((qfd = mq_open(que_name, mqflags, 0666, &a)) == -1)
    {
        syslog(LOG_ERR, "mq_open() failed in "__FILE__" at line "LINESTR": %s", strerror(errno));
        goto cleanup_file;
    }

    /* Begin main loop */
    while(!is_stopped)
    {
        if((size = mq_receive(qfd, (char*)&msg, sizeof msg, 0)) == -1)
        {
            switch (errno)
            {
            case EAGAIN:
                usleep(interval);
            case EINTR:
                continue;
            default:
                syslog(LOG_ERR, "mq_receive() failed in "__FILE__" at line "LINESTR": %s", strerror(errno));
                goto cleanup_full;
            }
        }
        if(write_with_timestamp(logfile, &msg) == -1)
        {
            syslog(LOG_ERR, "write_with_timestamp() failed in "__FILE__" at line "LINESTR": %s", strerror(errno));
            goto cleanup_full;
        }
        memset((char*)&msg, 0, size);
    }

    /* Perform cleanup */
cleanup_full:
    mq_close(qfd);
    mq_unlink(que_name);

cleanup_file:
    fflush(logfile);
    fclose(logfile);

cleanup_base:
    syslog(LOG_INFO, "%s daemon (PID: %d) successfully shut down", daemon_type, getpid());
    closelog();
    return 0;
}