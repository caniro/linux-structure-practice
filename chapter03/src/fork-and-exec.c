#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

static void child()
{
    char *args[] = { "/bin/echo", "hello", NULL }; // 명령어를 2차원 배열로 구성

    printf("자식 프로세스의 PID : %d\n", getpid());
    fflush(stdout);
    execve("/bin/echo", args, NULL); // 3번째 인자는 envp(KEY=VALUE 형식의 환경 변수 배열), 여기서는 필요 없음
    err(EXIT_FAILURE, "exec() failed");
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