#include <stdio.h>
#include <stdlib.h>
#include "readersStarvation.h"
#include "writersStarvation.h"



void initParams(int argc, char** argv);

//funkcja inicjujaca parametry programu
// param: 
//      -nazwa funkcji
//      - liczba czytelników
//      - liczba pisarzy
//      - option [-R/-W/-N] readersStarvation, writerStarvation, noStarvation
void initParams(int argc, char** argv)
{
    int opt;

    int *readers = (int*)malloc(sizeof(int));
    int *writers = (int*)malloc(sizeof(int));
    *readers = atoi(argv[1]);
    *writers = atoi(argv[2]);
    
    // Walidacja parametrów
    if(argc < 4) //at least 4 arguments are necessary, e.g. ./lib 10 4 -R
    {
        fprintf(stderr, "Usage: %s amount_readers amount_writers [-R/-W/-N]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(*readers < 0)
    {
        fprintf(stderr, "%d is not a valid data\n", *readers);
        exit(EXIT_FAILURE);
    }

    if(*writers < 0)
    {
        fprintf(stderr, "%d is not a valid data\n", *writers);
        exit(EXIT_FAILURE);
    }

    // wybranie opcji
    while ((opt = getopt(argc, argv, "RWN")) != -1)
    {
        switch(opt)
        {
            case 'R':
                readersInit(readers,writers);
                break;
            case 'W':
                writersInit(readers,writers);
                break;
            case 'N':
                bothInit(readers,writers);
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s amount_readers amount_writers [-R/-W/-N]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    return;
}




int main(int argc, char **argv)
{
    initParams(argc, argv);
    

    return 0;
}