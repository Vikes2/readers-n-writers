#include "noStarvation.h"


void bothInit(int* readers,int* writers) {
    struct args_structB *arg =  (struct args_structB *)malloc(sizeof(struct args_structB));
    
	arg->readersInside = 0;
	arg->writersInside = 0;
	arg->readersInQ = *readers;
	arg->writersInQ = *writers;

	readersThread = (pthread_t*) malloc(*readers * sizeof(pthread_t));
    writersThread = (pthread_t*) malloc(*writers * sizeof(pthread_t));

	//-------------
    serviceQueue.value = 1;
	serviceQueue.threadsCount = *readers + *writers;
	serviceQueue.prio_waiting = malloc(serviceQueue.threadsCount * sizeof(int));
	serviceQueue.prio_released = malloc(serviceQueue.threadsCount * sizeof(int));

	serviceQueue.prio_waiting[0] = serviceQueue.threadsCount;
	serviceQueue.prio_released[0] = 0;

	for (int i = 1; i < serviceQueue.threadsCount; i++) {
		serviceQueue.prio_waiting[i] = 0;
		serviceQueue.prio_released[i] = 0;
	}

	if (pthread_mutex_init(&(serviceQueue.mutex), NULL) == -1) {
        printf("Błąd: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (pthread_cond_init(&(serviceQueue.cv), NULL) == -1) {
        printf("Błąd: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (sem_init(&resourceAccess, 0, 1) == -1) {
        printf("Błąd: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (sem_init(&readCountAccess, 0, 1) == -1) {
        printf("Błąd: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
    //---------------------
    
	/* Create threads */
    for(int i=0 ;i < *writers; i++){
        pthread_create(&writersThread[i], NULL, &writerActionB, (void*)arg);
    }
    
    for(int i=0 ;i < *readers; i++){
        pthread_create(&readersThread[i], NULL, &readerActionB, (void*)arg);
    }   

    pthread_join(readersThread[0],NULL);

	free(serviceQueue.prio_waiting);
	free(serviceQueue.prio_released);
}

void* readerActionB(void* args){
    struct args_structB *arg = (struct args_structB *) args;
    
	while (1) {
		if (Wait(&serviceQueue, GetHighestWaitingPriority(&serviceQueue)) == -1) {  //  Wait in line to be serviced
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		};

		if (sem_wait(&readCountAccess) == -1) { //  Request exclusive access to readCount
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (arg->readersInside == 0) { //  if there are no readers already reading
			if (sem_wait(&resourceAccess) == -1) { //   request resource access for readers (writers blocked)
				printf("Błąd: %s", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}

		arg->readersInside++; //  Increase the number of readers inside
		arg->readersInQ--; //  Decrease the number of readers in queue
		printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ, arg->writersInQ, arg->readersInside, arg->writersInside);

		if (Post(&serviceQueue) == -1) { //  let next in line be serviced
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (sem_post(&readCountAccess) == -1) { //  release access to readCount
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		sleep(1); // Simulate reading

		if (sem_wait(&readCountAccess) == -1) { //  request exclusive access to readCount
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		arg->readersInside--; //  Increase the number of readers inside
		arg->readersInQ++; //  Increase the number of readers in queue
		printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ, arg->writersInQ, arg->readersInside, arg->writersInside);

		if (arg->readersInside == 0) { // if there are no readers left
			if (sem_post(&resourceAccess) == -1) { //  request exclusive access to readCount
				printf("Błąd: %s", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}

		if (sem_post(&readCountAccess) == -1) { //  release access to readCount
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
}

void* writerActionB(void* args){
    struct args_structB *arg = (struct args_structB *) args;
    
	while (1) {
		if (Wait(&serviceQueue, GetHighestWaitingPriority(&serviceQueue)) == -1) {  //  Wait in line to be serviced
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		};

		if (sem_wait(&resourceAccess) == -1) { //  request exclusive access to resource
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (Post(&serviceQueue) == -1) { //  let next in line be serviced
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		arg->writersInside++; //  Increase the number of writers inside
		arg->writersInQ--; //  Decrease the number of writers in queue

		printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ, arg->writersInQ, arg->readersInside, arg->writersInside);
        
		sleep(1);

		arg->writersInside--; //  Decrease the number of writers inside
		arg->writersInQ++; //  Increase the number of writers in queue

		printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ, arg->writersInQ, arg->readersInside, arg->writersInside);
        

		if (sem_post(&resourceAccess) == -1) { // release resource access for next reader/writer
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
}

