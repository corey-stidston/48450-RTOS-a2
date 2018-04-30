#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>

typedef struct {
    int *fd_write;
    char const * const file;
} reading_arguments_t;

sem_t *readSem, *passSem, *writeSem; // Semaphores
pthread_t tidA, tidB, tidC; // Thread IDs
pthread_attr_t attr; // Set of thread attributes

void *readData(); // Reads from data.txt
void *passData(); // Passes to ThreadC
void *writeData(); // Writes content to src.text
void initializeData(); // Initializes Data

int main(int argc, char*argv[])
{
    printf("***** START OF PROG_1 *****\n");
    
    initializeData();
    
    pthread_attr_init(&attr); // Get the default attributes
    
    pthread_create(&tidA, &attr, readData, NULL); // Creates thread A
    pthread_create(&tidB, &attr, passData, NULL); // Creates thread B
    pthread_create(&tidC, &attr, writeData, NULL); // Creates thread C
    
    sem_post(readSem); // Begin reading thread and the execution of the program
    
    if(pthread_join(tidA, NULL) != 0)
        printf("Issue with Thread A\n");
    if(pthread_join(tidB, NULL) != 0)
        printf("Issue with Thread B\n");
    if(pthread_join(tidC, NULL) != 0)
        printf("Issue with Thread C\n");
    
    printf("****** END OF PROG_1 *****\n");
}

/* Thread A will begin control in this function*/
void *readData()
{
    printf("%i", sem_wait(readSem)); // sem_wait for read semaphore
    printf("Reading Data \n");
    // read one line from data.txt
    // write into the pipe
    sem_post(passSem); // sem_post for pass semaphore
}

/* Thread B will begin control in this function*/
void *passData()
{
    printf("%i", sem_wait(passSem)); // sem_wait for pass semaphore
    printf("Passing Data \n");
    // read from the pipe
    // pass to thread C, the method of passing the data is unspecified
    sem_post(writeSem); // sem_post for write semaphore
}

/* Thread C will begin control in this function*/
void *writeData()
{
    
    printf("%i", sem_wait(writeSem)); // sem_wait for write semaphore
    printf("Writing Data \n");
    // read from wherever...
    // Determine whether it is from content or header
    // write content to src.txt
    // if last eof then exit. otherwise sem_post(readSem)
    
}

void initializeData() {
    readSem = sem_open("/readSem", O_CREAT, 0644, 1);
    passSem = sem_open("/passSem", O_CREAT, 0644, 1);
    writeSem = sem_open("/writeSem", O_CREAT, 0644, 1);
    
    pthread_attr_init(&attr);
}
