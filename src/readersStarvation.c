#include "readersStarvation.h"



void readersInit(int* readers,int* writers){

    readersThread = (pthread_t*) malloc(*readers * sizeof(pthread_t));
    writersThread = (pthread_t*) malloc(*writers * sizeof(pthread_t));
    struct args_struct *arg =  (struct args_struct *)malloc(sizeof(struct args_struct));
    arg->readersInside = 0;
    arg->writersInside = 0;
    arg->readersInQ = *readers;
    arg->writersInQ = *writers;
    arg->writers = 0; //currently writing
    
    

    if(pthread_cond_init(&readersQ, NULL) != 0){
        printf("Błąd: %s", strerror(errno));
    }

    if(pthread_cond_init(&writersQ, NULL) != 0){
        printf("Błąd: %s", strerror(errno));
    }

    if(pthread_mutex_init(&m, NULL) != 0){
        printf("Błąd: %s", strerror(errno));
    }
    
    for(int i=0 ;i < *readers; i++){
        pthread_create(&readersThread[i], NULL, &readerAction, (void*)arg);
    }   

    for(int i=0 ;i < *writers; i++){
        pthread_create(&writersThread[i], NULL, &writerAction, (void*)arg);
    }
    

    pthread_join(readersThread[0],NULL);



}
void* readerAction(void* args){
    
    struct args_struct *arg = (struct args_struct *) args;
    printf("============= %d ====================\n",arg->writers);
    
    while(1){

        
        if(pthread_mutex_lock(&m) !=0)
            printf("Błąd: %s", strerror(errno));
        
        while(arg->writersInside > 0){
        
            if (pthread_cond_wait(&readersQ, &m) != 0) 
                printf("Błąd: %s", strerror(errno));
        }
        
        arg->readersInside++;
        arg->readersInQ--;
        
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writers);

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

        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writers);

        
        if (pthread_mutex_unlock(&m) != 0) 
            printf("Błąd: %s", strerror(errno));

    }
}

void* writerAction(void* args){

    struct args_struct *arg = (struct args_struct *) args;

    while (1) {

        
        if(pthread_mutex_lock(&m) !=0)
            printf("Błąd: %s", strerror(errno));

        arg->writersInside++;

        while (!((arg->readersInside == 0) && (arg->writers == 0))) {
            
            if (pthread_cond_wait(&writersQ, &m) != 0) 
                printf("Błąd: %s", strerror(errno));
            
        }

        
        arg->writers++;
        arg->writersInQ--;

        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writers);
        
        
        if (pthread_mutex_unlock(&m) != 0) 
            printf("Błąd: %s", strerror(errno));

        sleep(1);

        if(pthread_mutex_lock(&m) !=0)
            printf("Błąd: %s", strerror(errno));

        arg->writersInside--;
        arg->writers--;
        arg->writersInQ++;
        
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", arg->readersInQ,  arg->writersInQ, arg->readersInside, arg->writers);

        if (arg->writersInside > 0) {
            if (pthread_cond_signal(&writersQ) != 0) 
                printf("Błąd: %s", strerror(errno));

        } else {
            if (pthread_cond_broadcast(&readersQ) != 0)
                printf("Błąd: %s", strerror(errno));

        }

        if (pthread_mutex_unlock(&m) != 0)
            printf("Błąd: %s", strerror(errno));

    }
}