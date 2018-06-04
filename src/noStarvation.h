#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

void bothInit(int* readers,int* writers);
void* readerAction(void* arg);
void* writerAction(void* arg);

