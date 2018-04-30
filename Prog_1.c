#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>

// Note: use -pthread and -lrt to compile

#define BUFFER_SIZE 1024
#define DATA_FILENAME "data.txt"
#define SRC_FILENAME "src.txt"
#define END_OF_HEADER "end_header"
#define SHARED_MEMORY_NAME "shared"

typedef struct {
    char buffer[BUFFER_SIZE];
} buffer_t;

typedef struct {
    int *fd;
    char const * const filename;
} reading_args_t;

typedef struct {
    int *fd;
    buffer_t *shared_buffer;
} passing_args_t;

typedef struct {
    buffer_t *shared_buffer;
    char const * const filename;
} writing_args_t;

sem_t readSem, passSem, writeSem; // Semaphores
pthread_t tidA, tidB, tidC; // Thread IDs
pthread_attr_t attr; // Thread attributes

void *readData(); // Reads from data.txt
void *passData(); // Passes to ThreadC
void *writeData(); // Writes content to src.text
void initializeData(); // Initializes Data

void writeRunningTimetoSharedMemory(int runningTimeInMilliseconds)
{
    int shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    
    ftruncate(shm_fd, sizeof(int));
    void *ptr = mmap(0, sizeof(int), PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    sprintf(ptr, "%i", runningTimeInMilliseconds);
}

int main(int argc, char*argv[])
{
    struct timeval start, end;
    
    gettimeofday(&start, NULL);
    
    int fd[2];
    
    if(pipe(fd) < 0)
    {
        perror("Pipe error");
        exit(0);
    }
    
    initializeData();
    
    // Initialize data structures for each thread
    buffer_t sharedBuffer = {0};
    reading_args_t reading_args = {fd, DATA_FILENAME};
    passing_args_t passing_args = {fd, &sharedBuffer};
    writing_args_t writing_args = {&sharedBuffer, SRC_FILENAME};
    
    pthread_attr_init(&attr); // Get the default attributes
    
    pthread_create(&tidA, &attr, readData, &reading_args); // Creates thread A
    pthread_create(&tidB, &attr, passData, &passing_args); // Creates thread B
    pthread_create(&tidC, &attr, writeData, &writing_args); // Creates thread C
    
    if(pthread_join(tidA, NULL) != 0)
        printf("Issue joining Thread A\n");
    if(pthread_join(tidB, NULL) != 0)
        printf("Issue joining Thread B\n");
    if(pthread_join(tidC, NULL) != 0)
        printf("Issue joining Thread C\n");
        
    gettimeofday(&end, NULL);
    int mtime = ((end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec)/1000.0);
    printf("Elapsed Time: %i milliseconds\n", mtime);
    
    writeRunningTimetoSharedMemory(mtime);
    
    return(0);
}

/* Thread A will begin control in this function */
void *readData(void *param)
{
    reading_args_t *parameters = param;
    char line[BUFFER_SIZE];
    
    FILE* readFile = fopen(parameters->filename, "r");
    if(!readFile)
    {
        perror("Invalid File\n");
        exit(0);
    }
    
    while(!sem_wait(&readSem) && fgets(line, BUFFER_SIZE, readFile) != NULL)
    {
        write(parameters->fd[1], line, strlen(line)+1); // write into the pipe
        sem_post(&passSem);
    }

    /* Close pipe and FILE* */
    close(parameters->fd[1]);
    fclose(readFile);
    
    /* Cancel the execution of threads */
    pthread_cancel(tidA);
    pthread_cancel(tidB);
    pthread_cancel(tidC);
}

/* Thread B will begin control in this function */
void *passData(void *param)
{
    passing_args_t *parameters = param;
    
    while(!sem_wait(&passSem))
    {
        read(parameters->fd[0], parameters->shared_buffer, BUFFER_SIZE); // read from the pipe
        sem_post(&writeSem);
    }
    
    close(parameters->fd[0]); // Close pipe
}

/* Thread C will begin control in this function */
void *writeData(void *param)
{
    writing_args_t *parameters = param;
    FILE* writeFile = fopen(parameters->filename, "w");
    if(!writeFile)
    {
        perror("Invalid File\n");
        exit(0);
    }
    
    char line[BUFFER_SIZE];
    int eoh_flag = 0;
    
    while(!sem_wait(&writeSem))
    {
        if(eoh_flag)
        {
            fputs(parameters->shared_buffer->buffer, writeFile);
        }
        else if(strstr(parameters->shared_buffer->buffer, END_OF_HEADER))
        {
            eoh_flag = 1;
        }

        sem_post(&readSem);
    }
    
    fclose(writeFile); // Close FILE*
}

void initializeData() {
    if(sem_init(&readSem, 0, 1))
    {
        perror("Error initializing semaphore.");
        exit(0);
    }
    
    if(sem_init(&passSem, 0, 0))
    {
        perror("Error initializing semaphore.");
        exit(0);
    }
    
    if(sem_init(&writeSem, 0, 0))
    {
        perror("Error initializing semaphore.");
        exit(0);
    }

    pthread_attr_init(&attr);
}
