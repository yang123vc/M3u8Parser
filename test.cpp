#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "M3U8Parser.h"
#include "MediaSegment.h"

char buf[1024*1024];

int read_data(const char* path) {
    int fd = -1;
    if((fd = open(path, O_RDONLY)) < 0)
        return -1;
    int size = (int)read(fd, buf, sizeof(buf));
    close(fd);
    return size;
}

int main(int argc, char **argv) {
    M3U8Parser parser;
    int size = 0;
    if((size = read_data(argv[1])) > 0) {
        parser.parser(buf, size);
    }
    return -1;
}
