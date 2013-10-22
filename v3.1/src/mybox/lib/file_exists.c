#include "libmybox.h"
// file_exists -- Checks whether a file or directory exists
int file_exists(char *s) {
        struct stat ss;
        int i=stat(s, &ss);
        if (i < 0) return 0;
        if ((ss.st_mode & S_IFREG) || (ss.st_mode & S_IFLNK) || (ss.st_mode & S_IFDIR)) return(1);
        return(0);
}

int file_exists_match(char *format, ...) {
        va_list va;
        int len, x;
        char *ret;
        va_start(va, format);
        len = vsnprintf(0, 0, format, va);
        len++;
        va_end(va);
        ret=xmalloc(len);
        va_start(va, format);
        vsnprintf(ret, len, format, va);
        va_end(va);
        return file_exists(ret);
}

