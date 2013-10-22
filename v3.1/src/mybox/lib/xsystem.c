#include "libmybox.h"
int xsystem(char *format, ...) {
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
        x=system(ret);
        return x;
}

