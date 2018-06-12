#include "writersStarvation.h"

// funkcja inicjuje rozwiązanie zagładzające pisarzy
// param 
// readers - liczba czytelników
// writers - liczba pisarzy
void writersInit(int* readers,int* writers){
    readersThread = (pthread_t*) malloc(*readers * sizeof(pthread_t)); // alokacja pamięci dla wątków czytelników
    writersThread = (pthread_t*) malloc(*writers * sizeof(pthread_t)); // alokacja pamięci dla wątków pisarzy
    struct args_structW *arg =  (struct args_structW *)malloc(sizeof(struct args_structW)); // alokacja pamięci dla struktury przechowującej argumenty
    //wartości początkowe
    arg->readersInside = 0; 
    arg->writersInside = 0;
    arg->readersInQ = *readers;
    arg->writersInQ = *writers; 
    
    // inicjacja zmiennych warunkowych oraz mutexu
    if(pthread_cond_init(&readersQ, NULL) != 0){
        printf("Błąd: %s", strerror(errno));
    }

    if(pthread_cond_init(&writersQ, NULL) != 0){
        printf("Błąd: %s", strerror(errno));
    }

    if(pthread_mutex_init(&m, NULL) != 0){
        printf("Błąd: %s", strerror(errno));
    }
    
    //tworzenie wątków oraz nadanie im funkcji odpowiadających za emitację aktywności pisarzy i czytelników
    for(int i=0 ;i < *writers; i++){
        pthread_create(&writersThread[i], NULL, &writerActionW, (void*)arg);
    }
    
    for(int i=0 ;i < *readers; i++){
        pthread_create(&readersThread[i], NULL, &readerActionW, (void*)arg);
    }   

    // funkcja wstrymuje zakonczenie aktualnego wątku(main) oraz oczekuje na zakończenie pracy wątku czytelnika(czeka w nieskończoność)
    pthread_join(readersThread[0],NULL);
}

// funkcja emitująca pracę czytelnika
// param:
//  - struktura argumentów
void* readerActionW(void* args){
    struct args_structW *arg = (struct args_structW *) args; // rzutowanie na struct
    
    while(1){
        // monitor wprowadzający czytelnika
        if(pthread_mutex_lock(&m) !=0)  // blokowanie mutexu
            printf("Błąd: %s", strerror(errno));
        
        //wstrzymanie pracy wątku czytelnika do momentu gdy jest jakikolwiek pisarz w środku czytelni
        while(arg->writersInside > 0){  
        
            if (pthread_cond_wait(&readersQ, &m) != 0) 
                printf("Błąd: %s", strerror(errno));
        }
        // zmiana zmiennych reprezetujących stan kolejki oraz czytelni
        arg->readersInside++;
        arg->readersInQ--;
        
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writersInside);

        // wyjście z monitora
        if (pthread_mutex_unlock(&m) != 0) 
            printf("Błąd: %s", strerror(errno));

        sleep(1); //emitacja czytania

        // monitor wyprowadzający czytelnika z czytelni
        if(pthread_mutex_lock(&m) !=0) // blokowanie mutexu
            printf("Błąd: %s", strerror(errno));

        //zmniejszenie liczby czytelników w czytelni o 1 oraz gdy jest to ostatni czytelnik w środku zostaje wysłany sygnał odblokowywujący pisarza
        if (--arg->readersInside == 0) { 
            if (pthread_cond_signal(&writersQ) != 0) 
                printf("Błąd: %s", strerror(errno));
        }
        
        // uaktualnienie stanu kolejki
        arg->readersInQ++;        
        //prezentacja stanu czytelni oraz kolejki
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writersInside);

        //wyjście z monitora 
        if (pthread_mutex_unlock(&m) != 0) //odblokowanie mutexu
            printf("Błąd: %s", strerror(errno));
    }
}

// funkcja emitująca pracę pisarza
// param:
// -struktura argumentów
void* writerActionW(void* args){
    struct args_structW *arg = (struct args_structW *) args;

    while (1) {
        // monitor wprowadzający pisarza
        if(pthread_mutex_lock(&m) !=0) //blokada mutexu
            printf("Błąd: %s", strerror(errno));

        //wstrzymanie pracy wątku pisarza do momentu gdy ktokolwiek jest w czytelni
        while (!((arg->readersInside == 0) && (arg->writersInside == 0))) {         
            if (pthread_cond_wait(&writersQ, &m) != 0) 
                printf("Błąd: %s", strerror(errno));
        }

        // zmiana zmiennych reprezetujących stan kolejki oraz czytelni
        arg->writersInside++;
        arg->writersInQ--;

        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writersInside);
        
        //wyjście z monitora
        if (pthread_mutex_unlock(&m) != 0) 
            printf("Błąd: %s", strerror(errno));

        sleep(1); // emitacja pisania

        //monitor wyprowadzający pisarza
        if(pthread_mutex_lock(&m) !=0)
            printf("Błąd: %s", strerror(errno));

        // zmiana zmiennych reprezetujących stan kolejki oraz czytelni
        arg->writersInside--;
        arg->writersInQ++;
        
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writersInside);

        // sygnały informujące o wyjściu pisarza (czytelnia jest pusta)
        if (pthread_cond_signal(&writersQ) != 0) // sygnał dla pisarza
                printf("Błąd: %s", strerror(errno));

        if (pthread_cond_broadcast(&readersQ) != 0) //sygnał dla wszystkich czytelników
                printf("Błąd: %s", strerror(errno));

        //wyjście z monitora
        if (pthread_mutex_unlock(&m) != 0)
            printf("Błąd: %s", strerror(errno));
    }
}