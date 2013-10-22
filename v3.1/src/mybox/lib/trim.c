#include "libmybox.h"
char *trim(char *s) {
        size_t len=strlen(s);
        size_t lws;
        while (len && isspace(s[len-1])) --len;
        if(len) {
                lws=strspn(s, " \n\r\t\v\f");
                memmove(s, s + lws, len -= lws);
        }
        s[len]=0;
        return s;
}

