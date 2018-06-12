#include "readersStarvation.h"


// funkcja inicjuje rozwiązanie zagładzające czyteników
// param 
// readers - liczba czytelników
// writers - liczba pisarzy
void readersInit(int* readers,int* writers){

    readersThread = (pthread_t*) malloc(*readers * sizeof(pthread_t)); // alokacja pamięci dla wątków czytelników
    writersThread = (pthread_t*) malloc(*writers * sizeof(pthread_t));  // alokacja pamięci dla wątków pisarzy
    struct args_struct *arg =  (struct args_struct *)malloc(sizeof(struct args_struct)); // alokacja pamięci dla struktury przechowującej argumenty
    //wartości początkowe
    arg->readersInside = 0;
    arg->writersInside = 0;
    arg->readersInQ = *readers;
    arg->writersInQ = *writers;
    arg->writers = 0;
    
    
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
    for(int i=0 ;i < *readers; i++){
        pthread_create(&readersThread[i], NULL, &readerAction, (void*)arg);
    }   

    for(int i=0 ;i < *writers; i++){
        pthread_create(&writersThread[i], NULL, &writerAction, (void*)arg);
    }
    
    // funkcja wstrymuje zakonczenie aktualnego wątku(main) oraz oczekuje na zakończenie pracy wątku czytelnika(czeka w nieskończoność)
    pthread_join(readersThread[0],NULL);
}


// funkcja emitująca pracę czytelnika
// param:
//  - struktura argumentów
void* readerAction(void* args){
    
    struct args_struct *arg = (struct args_struct *) args; // rzutowanie na struct
    
    while(1){

        // monitor wprowadzajacy czytelnika
        if(pthread_mutex_lock(&m) !=0) // blokowanie mutexu
            printf("Błąd: %s", strerror(errno));

        //wstrzymanie pracy wątku czytelnika do momentu gdy jest jakikolwiek pisarz w środku czytelni
        while(arg->writersInside > 0){
        
            if (pthread_cond_wait(&readersQ, &m) != 0) 
                printf("Błąd: %s", strerror(errno));
        }
        
        // zmiana zmiennych reprezetujących stan kolejki oraz czytelni
        arg->readersInside++;
        arg->readersInQ--;
        
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writers);

        //wyjscie z monitora
        if (pthread_mutex_unlock(&m) != 0) 
            printf("Błąd: %s", strerror(errno));

        sleep(1); //emitacja czytania

        //monitor wyprowadzający czytelnika
        if(pthread_mutex_lock(&m) !=0)
            printf("Błąd: %s", strerror(errno));
        // wysłanie sygnału do pisarza jeżeli z czytelni wyszedł ostatni czytelnik
        if (--arg->readersInside == 0) { 
            if (pthread_cond_signal(&writersQ) != 0) 
                printf("Błąd: %s", strerror(errno));
        }
        
        //aktualizacja stanu kolejki
        arg->readersInQ++;        

        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writers);

        // wyjście z monitora
        if (pthread_mutex_unlock(&m) != 0) 
            printf("Błąd: %s", strerror(errno));

    }
}

// funkcja emitująca pracę pisarza
// param:
//  - struktura argumentów
void* writerAction(void* args){

    struct args_struct *arg = (struct args_struct *) args; //rzutowanie na struct

    while (1) {

        //monitor wprowadzający pisarza
        if(pthread_mutex_lock(&m) !=0)
            printf("Błąd: %s", strerror(errno));

        arg->writersInside++;
        //wstrzymanie pracy wątku pisarza do momentu gdy jest jakikolwiek pisarz lub czytelnik jest w środku czytelni
        while (!((arg->readersInside == 0) && (arg->writers == 0))) {
            
            if (pthread_cond_wait(&writersQ, &m) != 0) 
                printf("Błąd: %s", strerror(errno));
            
        }

        // aktualizacja zmiennych reprezetujących stan kolejki oraz czytelni
        arg->writers++;
        arg->writersInQ--;

        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writers);
        
        //wyjscie z monitora
        if (pthread_mutex_unlock(&m) != 0) 
            printf("Błąd: %s", strerror(errno));

        sleep(1); //emitacja pisania

        //monitor wyprowadzający pisarza
        if(pthread_mutex_lock(&m) !=0)
            printf("Błąd: %s", strerror(errno));

        // aktualizacja zmiennych reprezetujących stan kolejki oraz czytelni
        arg->writersInside--;
        arg->writers--;
        arg->writersInQ++;
        
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writers);

        
        if (arg->writersInside > 0) {
            if (pthread_cond_signal(&writersQ) != 0) //sygnał informujący kolejnego pisarza o wyjściu jeżeli takowy istawił się w kolejce
                printf("Błąd: %s", strerror(errno));

        } else {
            if (pthread_cond_broadcast(&readersQ) != 0) // jeżeli nie ma czekających pisarzy wysyłany jest sygnał do wszystkich czytelników 
                printf("Błąd: %s", strerror(errno));

        }
        //wyjście z monitora
        if (pthread_mutex_unlock(&m) != 0)
            printf("Błąd: %s", strerror(errno));

    }
}