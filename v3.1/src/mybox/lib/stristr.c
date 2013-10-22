#include "libmybox.h"
// case-in-sensitive strstr
int stristr(const char *string, const char *pattern) {
        const char *pptr, *sptr, *start;
        for(start = string; *start != NUL; start++) {
                for( ; ((*start!=NUL) && (toupper(*start) != toupper(*pattern))); start++);
                if(NUL == *start) return -1;
                pptr = pattern;
                sptr = start;
                while(toupper(*sptr) == toupper(*pptr)) {
                        sptr++;
                        pptr++;
                        if(NUL == *pptr) return (start-string);
                }
        }
        return -1;
}

