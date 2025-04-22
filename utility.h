#ifndef UTILITY_H
#define UTILITY_H
#include <time.h>
#include <mqueue.h>

//TODO debug library
// switch from MQ to custom shared memory-based mq
// Make a write to non-existent mq a nop
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

#define MEM_MQ_NAME "/4B4A2572-E94D-4448-B52C-509352C4AC3D"
#define IO_MQ_NAME "/81EBD61F-AEA8-46BD-A34E-A0D54B09CB9C"

#define S1(N) #N
#define S2(N) S1(N)
#define LINESTR S2(__LINE__)

mqd_t get_mem_mq(void);
mqd_t get_io_mq(void);
#endif /* UTILITY_H */
