#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#define CACHE_LINE_SIZE 64
#define NLOOP (4 * 1024UL * 1024 * 1024) // 여러 번 반복해서 평균 시간을 구함
#define NSECS_PER_SEC 1000000000UL

static inline long diff_nsec(struct timespec before, struct timespec after)
{
    return ((after.tv_sec - before.tv_sec) * NSECS_PER_SEC + \
            (after.tv_nsec - before.tv_nsec));
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <size[KB]>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else if (atoi(argv[1]) < 1)
    {
        fprintf(stderr, "size must be >= 1: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    register int size = atoi(argv[1]) * 1024;
    char *buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, \
        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (buffer == (void *)-1)
        err(EXIT_FAILURE, "mmap() failed");

    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (int i = 0; i < NLOOP / (size / CACHE_LINE_SIZE); ++i)
        for (long j = 0; j < size; j += CACHE_LINE_SIZE)
            buffer[j] = 0;
    clock_gettime(CLOCK_MONOTONIC, &after);

    printf("%d[KB] : %f\n", atoi(argv[1]), (double)diff_nsec(before, after) / NLOOP); // 평균 시간 출력

    if (munmap(buffer, size) == -1)
        err(EXIT_FAILURE, "munmap() failed");
}
