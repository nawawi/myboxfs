#include "libmybox.h"

// stripchar -- Strip character in string
char *stripchar(char *string, char r) {
        register char *s;
        char *t=xmalloc(1+strlen(string));
        int i, x=0;
        s=string;
        for(i=0;i<strlen(string);i++) {
                if(string[i]==r) continue;
                t[x]=string[i];
                x++;
        }
        t[x]=0;
        strcpy(s,t);
        return s;
}

