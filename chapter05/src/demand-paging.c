#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#define BUFFER_SIZE (100 * 1024 * 1024)
#define NCYCLE 10
#define PAGE_SIZE (4 * 1024)

static void set_time(time_t *t, char **s)
{
    *t = time(NULL);
    *s = ctime(t);
}

int main()
{
    time_t t;
    char *s;

    set_time(&t, &s);

    printf("%.*s: before allocation, press Enter key\n", \
            (int)(strlen(s) - 1), s);
    getchar();

    char *p = malloc(BUFFER_SIZE);
    if (p == NULL)
        err(EXIT_FAILURE, "malloc() failed");

    set_time(&t, &s);
    printf("%.*s: allocated %dMiB, press Enter key\n", \
            (int)(strlen(s) - 1), s, BUFFER_SIZE / (1024 * 1024));
    getchar();

    for (int i = 0; i < BUFFER_SIZE; i += PAGE_SIZE)
    {
        p[i] = 0;

        // i가 10MiB단위(10MiB, 20MiB, ...)일 때만 출력
        int cycle = i / (BUFFER_SIZE / NCYCLE); // 10MiB 이하일 때 출력 방지용
        if (cycle != 0 && i % (BUFFER_SIZE / NCYCLE) == 0)
        {
            set_time(&t, &s);
            printf("%.*s: touched %dMiB\n", \
                    (int)(strlen(s) - 1), s, i / (1024 * 1024));
            sleep(1);
        }
    }
    
    set_time(&t, &s);
    printf("%.*s: touched %dMiB, press Enter key\n", \
            (int)(strlen(s) - 1), s, BUFFER_SIZE / (1024 * 1024));
    getchar();
}
