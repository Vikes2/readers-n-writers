#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include "priorityQ.h"



// Deklaracja struktury zawierającej status czytelni oraz kolejki
struct args_structB{
    int readersInside;
    int writersInside;
    int readersInQ;
    int writersInQ;
};

pthread_t *readersThread, *writersThread;

sem_t resourceAccess; // semaphore liczebny kontrolujący pisarz oraz czytelników wchodzących do biblioteki
sem_t readCountAccess; // semphore liczebny kontrolujący spójność zmiennych dotyczących czytelników
priosem_t serviceQueue; // kolejka priorytetowa aktywująca wątki na podstawie priorytetu / sygnały przesyłane powinny byc FIFO





void bothInit(int* readers,int* writers);
void* readerActionB(void* arg);
void* writerActionB(void* arg);

