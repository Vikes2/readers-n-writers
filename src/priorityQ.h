#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

typedef struct priority_sem_s {
	int value; // jeżeli liczba jest ujemna to wartosc bezwzgledna jest liczbą wątków w kolejce
	pthread_mutex_t mutex; //semaphore binarny
	pthread_cond_t cv; //zmienna warunkowa
	int* prio_waiting; // liczba czekających(zablokowanych wątków) na każdym priorytecie [priority]
	int* prio_released; // liczba odblokowanych wątków które byly zablokowane na każdym priorytecie
	int threadsCount; // liczba wątków
} priosem_t;

int Post(priosem_t* sem);

int Wait(priosem_t* sem, int prio);

int Lock(pthread_mutex_t* mutex);

int Unlock(pthread_mutex_t* mutex);

int IsThreadWaiting(priosem_t* sem);

int GetHighestWaitingPriority(priosem_t* sem);

int Cond_broadcast(pthread_cond_t* cond, pthread_mutex_t* mutex);

int Cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex);