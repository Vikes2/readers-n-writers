#include "noStarvation.h"

// funkcja inicjuje optymalne rozwiązanie (brak zagłodzenia)
// param 
// readers - liczba czytelników
// writers - liczba pisarzy
void bothInit(int* readers,int* writers) {
	
	readersThread = (pthread_t*) malloc(*readers * sizeof(pthread_t)); // alokacja pamięci dla wątków czytelników
    writersThread = (pthread_t*) malloc(*writers * sizeof(pthread_t)); // alokacja pamięci dla wątków pisarzy
    
	struct args_structB *arg =  (struct args_structB *)malloc(sizeof(struct args_structB)); // alokacja pamięci dla struktury przechowującej argumenty
    //wartości początkowe
	arg->readersInside = 0;
	arg->writersInside = 0;
	arg->readersInQ = *readers;
	arg->writersInQ = *writers;


	//------------- inicjacja kolejki priorytetowej oraz semaphorów
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
    
    //tworzenie wątków oraz nadanie im funkcji odpowiadających za emitację aktywności pisarzy i czytelników
    for(int i=0 ;i < *writers; i++){
        pthread_create(&writersThread[i], NULL, &writerActionB, (void*)arg);
    }
    
    for(int i=0 ;i < *readers; i++){
        pthread_create(&readersThread[i], NULL, &readerActionB, (void*)arg);
    }   

    // funkcja wstrymuje zakonczenie aktualnego wątku(main) oraz oczekuje na zakończenie pracy wątku czytelnika(czeka w nieskończoność)
    pthread_join(readersThread[0],NULL);

	free(serviceQueue.prio_waiting);
	free(serviceQueue.prio_released);
}

// funkcja emitująca pracę czytelnika
// param:
//  - struktura argumentów
void* readerActionB(void* args){
    struct args_structB *arg = (struct args_structB *) args; // rzutowanie na struct
    
	while (1) {
		if (Wait(&serviceQueue, GetHighestWaitingPriority(&serviceQueue)) == -1) {  //  Czekanie w kolejce do momentu obsłużenia
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		};

		if (sem_wait(&readCountAccess) == -1) { //Blokowanie semaphora dla pozostałych wątków odpowiadającego za dostęp do stanu czytających
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
		// Jeżeli nie ma żadnego czytelnika w czytelni, blokuj dostęp dla pisarzy
		if (arg->readersInside == 0) { 
			if (sem_wait(&resourceAccess) == -1) { 
				printf("Błąd: %s", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}

        // zmiana zmiennych reprezetujących stan kolejki oraz czytelni
		arg->readersInside++; 
		arg->readersInQ--; 
		printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ, arg->writersInQ, arg->readersInside, arg->writersInside);

		if (Post(&serviceQueue) == -1) { //  Zwolnienie kolejki oraz wysłanie sygnału do wszystkich czekających
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (sem_post(&readCountAccess) == -1) { //  odblokowanie semaphora kontrolującego dostęp do stanu czytających
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		sleep(1); // Emitacja czytania
		//Czytelnik wychodzi	

		if (sem_wait(&readCountAccess) == -1) { //  Blokowanie semaphora kontrolującego dostęp do stanu czytających
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		// zmiana zmiennych reprezetujących stan kolejki oraz czytelni
		arg->readersInside--; 
		arg->readersInQ++; 
		printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ, arg->writersInQ, arg->readersInside, arg->writersInside);

		//Jeżeli był to ostatni czytelnik odblokuj dostęp dla pisarzy
		if (arg->readersInside == 0) { 
			if (sem_post(&resourceAccess) == -1) { 
				printf("Błąd: %s", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}

		//  odblokowanie semaphora kontrolującego dostęp do stanu czytających
		if (sem_post(&readCountAccess) == -1) { 
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
}

// funkcja emitująca pracę pisarza
// param:
//  - struktura argumentów
void* writerActionB(void* args){
    struct args_structB *arg = (struct args_structB *) args; // rzutowanie na struct
    
	while (1) {
		if (Wait(&serviceQueue, GetHighestWaitingPriority(&serviceQueue)) == -1) {  //  Czekanie w kolejce do momentu obsłużenia
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		};

		if (sem_wait(&resourceAccess) == -1) { //  blokuje dostęp do czytelni dla wszystkich innych
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (Post(&serviceQueue) == -1) { //  Zwolnienie kolejki oraz wysłanie sygnału do wszystkich czekających
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		// zmiana zmiennych reprezetujących stan kolejki oraz czytelni
		arg->writersInside++; 
		arg->writersInQ--; 

		printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ, arg->writersInQ, arg->readersInside, arg->writersInside);
        
		sleep(1); //Emitacja pisania

		//Wyjście pisarza

		arg->writersInside--; 
		arg->writersInQ++;

		printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ, arg->writersInQ, arg->readersInside, arg->writersInside);
        
		//Zwalniany semaphore dla innych czekających w kolejce
		if (sem_post(&resourceAccess) == -1) { 
			printf("Błąd: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
}

