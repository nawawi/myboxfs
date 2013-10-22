#include "libmybox.h"

// xmalloc -- Exit unless we can allocate memory.
void *xmalloc(size_t size) {
        void *ptr=malloc(size);
        if(!ptr) {
            printf("malloc() Out of memory, exiting\n");
            exit(1);
        }
        return ptr;
}

// xstrndup -- Exit unless we can allocate a copy of this many bytes of string.
void *xstrndup(char *s, size_t n) {
	void *ret = xmalloc(++n);
	strncpy(ret, s, n);
	return ret;
}

// xstrdup -- Exit unless we can allocate a copy of this string.
void *xstrdup(char *s) {
	return xstrndup(s,strlen(s));
}

// xstrncpy -- safe version of strncpy 
char *xstrncpy(char *dst, const char *src, size_t size) {
        dst[size-1] = '\0';
	return strncpy(dst, src, size-1);
}

// xwrite -- safe version of write
ssize_t xwrite(int fd, const void *buf, size_t count) {
        ssize_t n;
        do {
                n = write(fd, buf, count);
        } while (n < 0 && errno == EINTR);
        return n;
}

void *xrealloc(void *ptr, size_t size) {
        ptr = realloc(ptr, size);
        if (ptr == NULL && size != 0) {
		printf("malloc() Out of memory, exiting\n");
		exit(1);
	}
        return ptr;
}
