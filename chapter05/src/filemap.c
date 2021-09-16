#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#define BUFFER_SIZE 1000
#define ALLOC_SIZE (100 * 1024 * 1024)

int main()
{
    pid_t pid;
    char command[BUFFER_SIZE];

    pid = getpid();
    snprintf(command, BUFFER_SIZE, "cat /proc/%d/maps", pid);

    puts("memory map before memory allocation");
    fflush(stdout);
    system(command);

    int fd = open("testfile", O_RDWR);
    if (fd == -1)
        err(EXIT_FAILURE, "open() failed");

    char *file_contents = mmap(NULL, ALLOC_SIZE, PROT_READ | PROT_WRITE, \
                            MAP_SHARED, fd, 0);
    if (file_contents == (void *)-1)
    {
        close(fd);
        err(EXIT_FAILURE, "mmap() failed");
    }

    printf("\nsucceeded to allocate memory : address(%p), size(0x%x)\n\n", \
            file_contents, ALLOC_SIZE);

    puts("memory map after memory allocation");
    fflush(stdout);
    system(command);

    printf("\n file contents before overwrite mapped region: %s\n", file_contents);

    char *overwrite_data = "HELLO";
    memcpy(file_contents, overwrite_data, strlen(overwrite_data));
    
    printf("overwritten mapped region: %s\n", file_contents);
    
    if (munmap(file_contents, ALLOC_SIZE) == -1)
        err(EXIT_FAILURE, "munmap() failed");
    
    close(fd);
}
