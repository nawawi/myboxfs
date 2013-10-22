#include "libmybox.h"

int xmkdir(char *p) {
	long mode=0700;
        mode_t mask;
        char *s=xmalloc(strlen(p) * sizeof(char)); 
        char *path=xmalloc(strlen(p) * sizeof(char)); 
        char c;
        struct stat st;
        path=strdup(p);
        s=path;
        mask=umask(0);
	umask(mask & ~0300);

        do {
		c=0;
		while(*s) {
 			if (*s=='/') {
 				do {
 					++s;
 				} while (*s == '/');
				c = *s;
				*s = 0;
				break;
			}
			++s;
		}
                if(mkdir(path, 0700) < 0) {
                        if(errno != EEXIST || (stat(path, &st) < 0 || !S_ISDIR(st.st_mode))) {
				umask(mask);
                                break;
                        }
                        if(!c) {
				umask(mask);
 				return 0;
 			}
                }

                if(!c) {
			umask(mask);
                        if(chmod(path, mode) < 0) {
				break;
			}
			return 0;
                }
                *s=c;
        } while (1);
        return -1;
}

