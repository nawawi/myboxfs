#include "libmybox.h"
// read_oneline -- Get first line from file
int read_oneline(const char *filename, void *buf) {
        int fd;
        ssize_t ret;
        fd=open(filename, O_RDONLY);
        if(fd < 0) return -1;
        ret=read(fd, buf, 1023);
        ((char *)buf)[ret > 0 ? ret : 0]=0;
        close(fd);
        return ret;
}

