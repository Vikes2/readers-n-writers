#include "writersStarvation.h"



void writersInit(int* readers,int* writers){

    readersThread = (pthread_t*) malloc(*readers * sizeof(pthread_t));
    writersThread = (pthread_t*) malloc(*writers * sizeof(pthread_t));
    struct args_structW *arg =  (struct args_structW *)malloc(sizeof(struct args_structW));
    arg->readersInside = 0;
    arg->writersInside = 0;
    arg->readersInQ = *readers;
    arg->writersInQ = *writers;
    
    

    if(pthread_cond_init(&readersQ, NULL) != 0){
        printf("Błąd: %s", strerror(errno));
    }

    if(pthread_cond_init(&writersQ, NULL) != 0){
        printf("Błąd: %s", strerror(errno));
    }

    if(pthread_mutex_init(&m, NULL) != 0){
        printf("Błąd: %s", strerror(errno));
    }
    

    for(int i=0 ;i < *writers; i++){
        pthread_create(&writersThread[i], NULL, &writerActionW, (void*)arg);
    }
    
    for(int i=0 ;i < *readers; i++){
        pthread_create(&readersThread[i], NULL, &readerActionW, (void*)arg);
    }   

    pthread_join(readersThread[0],NULL);



}
void* readerActionW(void* args){
    
    struct args_structW *arg = (struct args_structW *) args;
    printf("============= %d ====================\n",arg->writersInside);
    
    while(1){

        
        if(pthread_mutex_lock(&m) !=0)
            printf("Błąd: %s", strerror(errno));
        
        while(arg->writersInside > 0){
        
            if (pthread_cond_wait(&readersQ, &m) != 0) 
                printf("Błąd: %s", strerror(errno));
        }
        
        arg->readersInside++;
        arg->readersInQ--;
        
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writersInside);

        if (pthread_mutex_unlock(&m) != 0) 
            printf("Błąd: %s", strerror(errno));

        sleep(1);

        if(pthread_mutex_lock(&m) !=0)
            printf("Błąd: %s", strerror(errno));

        if (--arg->readersInside == 0) {
            if (pthread_cond_signal(&writersQ) != 0) 
                printf("Błąd: %s", strerror(errno));
        }
        
        arg->readersInQ++;        

        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writersInside);

        
        if (pthread_mutex_unlock(&m) != 0) 
            printf("Błąd: %s", strerror(errno));

    }
}

void* writerActionW(void* args){

    struct args_structW *arg = (struct args_structW *) args;

    while (1) {

        
        if(pthread_mutex_lock(&m) !=0)
            printf("Błąd: %s", strerror(errno));


        while (!((arg->readersInside == 0) && (arg->writersInside == 0))) {
            
            if (pthread_cond_wait(&writersQ, &m) != 0) 
                printf("Błąd: %s", strerror(errno));
            
        }

        
        arg->writersInside++;
        arg->writersInQ--;

        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writersInside);
        
        
        if (pthread_mutex_unlock(&m) != 0) 
            printf("Błąd: %s", strerror(errno));

        sleep(1);

        if(pthread_mutex_lock(&m) !=0)
            printf("Błąd: %s", strerror(errno));

        arg->writersInside--;
        arg->writersInQ++;
        
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writersInside);

        if (pthread_cond_signal(&writersQ) != 0) 
                printf("Błąd: %s", strerror(errno));

        if (pthread_cond_broadcast(&readersQ) != 0)
                printf("Błąd: %s", strerror(errno));

        if (pthread_mutex_unlock(&m) != 0)
            printf("Błąd: %s", strerror(errno));

    }
}