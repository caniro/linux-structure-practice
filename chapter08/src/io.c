#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <stdbool.h>

#define PART_SIZE (1024 * 1024 * 1024)
#define ACCESS_SIZE (64 * 1024 * 1024)

int main(int argc, char *argv[])
{
    char *progname = argv[0];
    if (argc != 6)
    {
        fprintf(stderr, "usage: %s <filename> <kernel's help> <r/w> \
                        <access pattern> <block size[KiB]>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *filename = argv[1];
    
    bool help;
    if (strcmp(argv[2], "on") == 0)
        help = true;
    else if (strcmp(argv[2], "off") == 0)
        help = false;
    else
    {
        fprintf(stderr, "<kernel's help> must be 'on' or 'off': %s\n", \
                argv[2]);
        exit(EXIT_FAILURE);
    }

    bool write_flag;
    if (strcmp(argv[3], "r") == 0)
        write_flag = false;
    else if (strcmp(argv[3], "w") == 0)
        write_flag = true;
    else
    {
        fprintf(stderr, "<r/w> must be 'r' or 'w': %s\n", \
                argv[3]);
        exit(EXIT_FAILURE);
    }
    
    bool random;
        if (strcmp(argv[4], "seq") == 0)
        random = false;
    else if (strcmp(argv[4], "rand") == 0)
        random = true;
    else
    {
        fprintf(stderr, "<access pattern> must be 'seq' or 'rand': %s\n", \
                argv[4]);
        exit(EXIT_FAILURE);
    }

    int block_size = atoi(argv[5]) * 1024;
    if (block_size == 0)
    {
        fprintf(stderr, "<block size> must be > 0: %s\n", \
                argv[5]);
        exit(EXIT_FAILURE);
    }
    if (ACCESS_SIZE % block_size != 0)
    {
        fprintf(stderr, \
                "<access size(%d)> must be multiple of block size: %s\n", \
                ACCESS_SIZE, argv[5]);
        exit(EXIT_FAILURE);
    }

    int max_count = PART_SIZE / block_size;
    int count = ACCESS_SIZE / block_size;

    int *offset = malloc(max_count * sizeof(int));
    if (offset == NULL)
        err(EXIT_FAILURE, "malloc() failed");

    int flag = O_RDWR | O_EXCL;
    if (!help)
        flag |= O_DIRECT; // ???????????? I/O ??????, ??? ????????? I/O ?????? ?????? ?????? ??????

    int fd = open(filename, flag);
    if (fd == -1)
        err(EXIT_FAILURE, "open() failed");

    for (int i = 0; i < max_count; ++i)
        offset[i] = i;
    
    if (random)
        for (int i = 0; i < max_count; ++i)
        {
            int j = rand() % max_count;
            int tmp = offset[i];
            offset[i] = offset[j];
            offset[j] = tmp;
        }
    
    int sector_size;
    if (ioctl(fd, BLKSSZGET, &sector_size) == -1) // ????????? ?????? ????????? ??????
        err(EXIT_FAILURE, "ioctl() failed");

    // malloc() ?????? posix_memalign()??? ????????????
    // ?????? ????????? ?????? ????????? sector_size??? ????????? ??????(align)??????.
    // ???????????? I/O??? ???????????? ????????? ?????? ????????? ???????????? ?????? ???????????? ???????????? ??????.
    char *buf;
    int e = posix_memalign((void **)&buf, sector_size, block_size);
    if (e)
    {
        errno = e;
        err(EXIT_FAILURE, "posix_memalign() failed");
    }

    for (int i = 0; i < count; ++i)
    {
        if (lseek(fd, offset[i] * block_size, SEEK_SET) == -1)
            err(EXIT_FAILURE, "lseek() failed");
        if (write_flag)
        {
            ssize_t ret = write(fd, buf, block_size);
            if (ret == -1)
                err(EXIT_FAILURE, "write() failed");
        }
        else
        {
            ssize_t ret = read(fd, buf, block_size);
            if (ret == -1)
                err(EXIT_FAILURE, "read() failed");
        }
    }

    // ????????? I/O ????????? ????????? ????????? ????????????.
    // ???????????? I/O??? write() ????????? ????????? ????????? ???????????????,
    // ?????? I/O??? write() ????????? ???????????? ???????????? ?????? ????????????.
    if (fdatasync(fd) == -1)
        err(EXIT_FAILURE, "fdatasync() failed");

    if (close(fd) == -1)
        err(EXIT_FAILURE, "close() failed");
}
