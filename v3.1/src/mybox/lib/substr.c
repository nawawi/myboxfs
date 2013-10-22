#include "libmybox.h"
// substr
char *substr(const char *string, int start, size_t count) {
        size_t len = strlen(string);
        if(start < 0) start = len + start;
        if(start >= 0 && start < len) {
                if(count == 0 || count > len - start) count = len - start;
                char *smtpstr=xmalloc(count+1);
                memcpy(smtpstr, string + start, count);
                smtpstr[count] = 0;
                return smtpstr;
        }
        char *smtpstr=xmalloc(sizeof(char));
        smtpstr[0] = 0;
        return smtpstr;
}

