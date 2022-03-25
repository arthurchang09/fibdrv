#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"
#define LENGTH 22
#define BUFFSIZE 8 * sizeof(int) * LENGTH / 3 + 2

int main()
{
    char buf[BUFFSIZE];
    int offset = 1000; /* TODO: try test something bigger than the limit */

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }


    for (int i = 0; i <= offset; i++) {
        lseek(fd, i, SEEK_SET);
        long long sz = read(fd, buf, BUFFSIZE);
        printf("%d %lld\n", i, sz);

    }


    close(fd);
    return 0;
}
