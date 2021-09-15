#include <stdio.h>

int main()
{
    int *p = NULL;
    puts("Before invalid access");
    *p = 0;
    puts("After invalid access");
}
