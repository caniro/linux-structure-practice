#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#define NLOOP_FOR_ESTIMATION 1000000000UL
#define NSECS_PER_MSEC 1000000UL
#define NSECS_PER_SEC 1000000000UL

/*
    before와 after의 시간 차이를 ns 단위로 리턴
    빠른 실행을 위해 inline 키워드 사용
*/
static inline unsigned long diff_nsec(struct timespec before, struct timespec after)
{
    // unsigned long before_nsec = before.tv_sec * NSECS_PER_SEC + before.tv_nsec;
    // unsigned long after_nsec = after.tv_sec * NSECS_PER_SEC + after.tv_nsec;
    // printf("before : %lu\n", before_nsec);
    // printf("after : %lu\n", after_nsec);
    // printf("diff : %lu\n", after_nsec - before_nsec);
    return ((after.tv_sec * NSECS_PER_SEC + after.tv_nsec) \
            - (before.tv_sec * NSECS_PER_SEC + before.tv_nsec));
}

// 1ms 당 반복문을 몇번 돌 수 있는지 (성능 기준치 측정)
static unsigned long loops_per_msec()
{
    struct timespec before, after;

    clock_gettime(CLOCK_MONOTONIC, &before); // Monotonic system-wide clock id 사용
    for (unsigned long i = 0; i < NLOOP_FOR_ESTIMATION; ++i)
        ;
    clock_gettime(CLOCK_MONOTONIC, &after);

    // 1ms 당 반복 횟수 = 측정 횟수 / 걸린 시간[ms]
    // return NLOOP_FOR_ESTIMATION / (diff_nsec(before, after) / NSECS_PER_MSEC);
    return NLOOP_FOR_ESTIMATION * NSECS_PER_MSEC / diff_nsec(before, after);
}

// 단순히 반복문으로 부하를 주는 함수
static inline void load(unsigned long nloop)
{
    for (unsigned long i = 0; i < nloop; ++i)
        ;
}

static void child_fn(int id, struct timespec *buf, int nrecord, \
                    unsigned long nloop_per_resol, struct timespec start)
{
    for (int i = 0; i < nrecord; ++i)
    {
        struct timespec ts;

        load(nloop_per_resol);
        clock_gettime(CLOCK_MONOTONIC, &ts);
        buf[i] = ts;
    }
    for (int i = 0; i < nrecord; ++i)
        // 프로세스고유ID(0 ~ nproc-1), 프로그램시작시점부터경과한시간, 진행도[%]
        printf("%d\t%lu\t%d\n", id, diff_nsec(start, buf[i]) / NSECS_PER_MSEC, \
            100 * (i + 1) / nrecord);

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    if (argc < 4)       // 끝에 : Success 뜨는 이유??
        err(EXIT_FAILURE, \
            "usage: %s <nproc> <total[ms]> <resolution[ms]>", argv[0]);

    int nproc = atoi(argv[1]);
    int total = atoi(argv[2]);
    int resol = atoi(argv[3]);

    if (nproc < 1 || total < 1 || resol < 1)
        err(EXIT_FAILURE, "parameters must be positive value");
    else if (total % resol)
        err(EXIT_FAILURE, "<total> must be multiple of <resolution>");

    int nrecord = total / resol;
    
    struct timespec *logbuf = malloc(nrecord * sizeof(struct timespec));
    if (!logbuf)
        err(EXIT_FAILURE, "malloc(logbuf) failed");

    // resolution 당 반복 횟수
    unsigned long nloop_per_msec = loops_per_msec();
    unsigned long nloop_per_resol = nloop_per_msec * resol;
    // printf("nloop_per_msec : %lu\n", nloop_per_msec);
    // printf("nloop_per_resol : %lu\n", nloop_per_resol);

    pid_t *pids = malloc(nproc * sizeof(pid_t));
    if (!pids)
        err(EXIT_FAILURE, "malloc(pids) failed");

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int ret = EXIT_SUCCESS;
    int ncreated = 0;
    for (int i = 0; i < nproc; ++i, ++ncreated)
    {
        pids[i] = fork();
        if (pids[i] < 0)
        {
            for (int j = 0; j < ncreated; ++j)
                kill(pids[j], SIGKILL);
            ret = EXIT_FAILURE;
            break ;
        }
        else if (pids[i] == 0)
            child_fn(i, logbuf, nrecord, nloop_per_resol, start);
    }
    
    for (int i = 0; i < ncreated; ++i)
        if (wait(NULL) < 0)
            warn("wait() failed");

    return ret;
}
