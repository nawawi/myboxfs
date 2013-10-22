#include "libmybox.h"
long get_boottime(void) {
        FILE *fp;
        char line[1024];
        long bt;
        if((fp=fopen("/proc/stat","r"))!=NULL) {
                while(fgets (line, sizeof (line), fp)) {
                        if(!strncmp ("btime", line, 5)) {
                                time_t btime;
                                btime = (time_t) atol (&line[6]);
                                bt=btime;
                                continue;
                        }
                }
                fclose(fp);
        }
        return bt;
}

