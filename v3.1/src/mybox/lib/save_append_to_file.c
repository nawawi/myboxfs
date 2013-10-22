#include "libmybox.h"

// save_to_file -- Truncate string to file
int save_to_file(const char *filename,char *format, ...) {
        va_list va;
        int len, fd;
        char *buf;
        ssize_t ret;
        va_start(va, format);
        len = vsnprintf(0, 0, format, va);
        len++;
        va_end(va);
        buf = xmalloc(len);
        va_start(va, format);
        vsnprintf(buf, len, format, va);
        va_end(va);
        fd=open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
        if(fd < 0) return -1;
        ret=xwrite(fd, buf, strlen(buf));
        close(fd);
        return ret;
}

// append_to_file -- Append string to file
int append_to_file(const char *filename,char *format, ...) {
        va_list va;
        int len, fd;
        char *buf;
        ssize_t ret;
        va_start(va, format);
        len = vsnprintf(0, 0, format, va);
        len++;
        va_end(va);
        buf = xmalloc(len);
        va_start(va, format);
        vsnprintf(buf, len, format, va);
        va_end(va);
        fd=open(filename, O_RDWR | O_CREAT | O_APPEND, S_IREAD | S_IWRITE);
        if(fd < 0) return -1;
        ret=xwrite(fd, buf, strlen(buf));
        close(fd);
        return ret;
}

