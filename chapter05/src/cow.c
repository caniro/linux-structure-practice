#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#define BUFFER_SIZE (100 * 1024 * 1024)
#define PAGE_SIZE (4 * 1024)
#define COMMAND_SIZE (4 * 1024)

static void child_fn(char *p)
{
    char command[COMMAND_SIZE];

    printf("child ps info before memory access:\n");
    fflush(stdout);
    snprintf(command, COMMAND_SIZE, \
        "bash -c 'ps -eo pid,comm,vsz,rss,maj_flt,min_flt | grep %d'", getpid());
    system(command);

    printf("free memory info before memory access:\n");
    fflush(stdout);
    system("free");

    for (int i = 0; i < BUFFER_SIZE; i += PAGE_SIZE)
        p[i] = 0;

    printf("child ps info after memory access:\n");
    fflush(stdout);
    system(command);

    printf("free memory info after memory access:\n");
    fflush(stdout);
    system("free");

    exit(EXIT_SUCCESS);
}

int main()
{
    char *p = malloc(BUFFER_SIZE);
    if (p == NULL)
        err(EXIT_FAILURE, "malloc() failed");

    // 확보한 메모리 전체에 쓰기 작업 (디맨드 페이징으로 물리 메모리 매핑)
    for (int i = 0; i < BUFFER_SIZE; i += PAGE_SIZE)
        p[i] = 0;

    printf("free memory info before fork():\n");
    fflush(stdout);
    system("free");

    pid_t ret = fork();
    if (ret == -1)
        err(EXIT_FAILURE, "fork() failed");
    else if (ret == 0)
        child_fn(p);
    else
        wait(NULL);
}
