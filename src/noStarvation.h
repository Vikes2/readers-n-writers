#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include "priorityQ.h"




struct args_structB{
    int readersInside;
    int writersInside;
    int readersInQ;
    int writersInQ;
};

pthread_t *readersThread, *writersThread;

sem_t resourceAccess; // controls access (read/write) to the resource
sem_t readCountAccess; // for syncing changes to shared variable readCount
priosem_t serviceQueue; // FAIRNESS: preserves ordering of requests (signaling must be FIFO)





void bothInit(int* readers,int* writers);
void* readerActionB(void* arg);
void* writerActionB(void* arg);

