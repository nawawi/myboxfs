#include "libmybox.h"
int is_dir(const char *s) {
        struct stat ss;
        int i=stat(s, &ss);
        if (i < 0) return(0);
        if ((ss.st_mode & S_IFREG) || (ss.st_mode & S_IFDIR)) return(1);
        return(0);
}

