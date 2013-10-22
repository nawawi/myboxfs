#include "libmybox.h"
// splitc -- Split string
void splitc(char *first, char *rest, char divider) {
        char *p;
        p=strchr(rest, divider);
        if(p==NULL) {
                if((first != rest) && (first != NULL))
                first[0]=0;
                return;
        }
        *p=0;
        if(first != NULL) strcpy(first, rest);
        if(first != rest) strcpy(rest, p + 1);
}

