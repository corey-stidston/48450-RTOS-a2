#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

//Note: ues -lrt flag to compile.

int main()
{
    const int SIZE = sizeof(int);
    const char *name = "shared";
    
    int shm_fd;
    void *ptr;
    
    shm_fd = shm_open(name, O_RDONLY, 0666);
    
    if(shm_fd < 0)
    {
        printf("Failed to obtain file descriptor.\n");
        exit(0);
    }
    
    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    
    if((int*)ptr < 0)
    {
        printf("Failed to create a mapping in the virtual address space\n");
        exit(0);
    }
    
    printf("The running time of Prog_1 was - %s milliseconds.\n", (char *)ptr);
    shm_unlink(name);
    
	return 0;
}
