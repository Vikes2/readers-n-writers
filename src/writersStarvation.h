#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

// Deklaracja zmiennych wątków oraz składowych monitora: mutexu oraz zmiennych warunkowych
pthread_t *readersThread, *writersThread;
pthread_mutex_t m;
pthread_cond_t readersQ, writersQ;

// Deklaracja struktury zawierającej status czytelni oraz kolejki
struct args_structW{
    int readersInside;
    int writersInside;
    int readersInQ;
    int writersInQ;
};

void writersInit(int* readers,int* writers);
void* readerActionW(void* arg);
void* writerActionW(void* arg);

