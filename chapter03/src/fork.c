#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

static void child()
{
    printf("자식 프로세스의 PID : %d\n", getpid());
    exit(EXIT_SUCCESS);
}

static void parent(pid_t pid_c)
{
    printf("부모 프로세스의 PID : %d, 자식 프로세스의 PID : %d\n", getpid(), pid_c);
    exit(EXIT_SUCCESS);
}

int main()
{
    // 실패 시 -1, 성공 시 자식 프로세스는 0, 부모 프로세스는 자식 프로세스의 PID 리턴
    pid_t ret = fork();

    if (ret == -1)
        err(EXIT_FAILURE, "fork() failed");
    else if (ret == 0)
        child();
    else
        parent(ret);
    err(EXIT_FAILURE, "shouldn't reach here");
}
