#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>

/* Mutual exclusion object */
pthread_mutex_t mutex;

/* Semaphores */
sem_t one, two, master;
pthread_t tidA,tidB,tidC; //Thread ID
pthread_attr_t attr; //Set of thread attributes

void *readData(); // Reads from data.txt
void *passData(); // Passes to ThreadC
void *writeData(); // Writes content to src.text
void initializeData();

int main(int argc, char*argv[])
{
    printf("*****START OF PROG_1*****\n");
    
    initializeData();
    
    pthread_attr_init(&attr); // Get the default attributes
    
    pthread_create(&tidA, &attr, readData, NULL); // Creates thread A
    
    pthread_create(&tidB, &attr, passData, NULL); // Creates thread B
    
    pthread_create(&tidC, &attr, writeData, NULL); // Creates thread C
    
    // send semaphore to thread A to begin the running of threads
    
    // wait for threads to exit
    
    printf("*****END OF PROG_1****\n");
}

/* Thread A will begin control in this function*/
void *readData()
{
    
}

/* Thread B will begin control in this function*/
void *passData()
{

}

/* Thread C will begin control in this function*/
void *writeData()
{
    
}

void initializeData() {

}
