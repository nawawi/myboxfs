#include <libmybox.h>

#ifndef FSHIFT
# define FSHIFT 16              /* nr of bits of precision */
#endif
#define FIXED_1         (1<<FSHIFT)     /* 1.0 as fixed-point */
#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)


int uptime_main(int argc, char **argv)
{
        int updays, uphours, upminutes;
        struct sysinfo info;
        struct tm *current_time;
        time_t current_secs;

        time(&current_secs);
        current_time = localtime(&current_secs);

        sysinfo(&info);

        printf(" %02d:%02d:%02d up ",
                        current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
        updays = (int) info.uptime / (60*60*24);
        if (updays)
                printf("%d day%s, ", updays, (updays != 1) ? "s" : "");
        upminutes = (int) info.uptime / 60;
        uphours = (upminutes / 60) % 24;
        upminutes %= 60;
        if(uphours)
                printf("%2d:%02d, ", uphours, upminutes);
        else
                printf("%d min, ", upminutes);

        printf("load average: %ld.%02ld, %ld.%02ld, %ld.%02ld\n",
                        LOAD_INT(info.loads[0]), LOAD_FRAC(info.loads[0]),
                        LOAD_INT(info.loads[1]), LOAD_FRAC(info.loads[1]),
                        LOAD_INT(info.loads[2]), LOAD_FRAC(info.loads[2]));

        return EXIT_SUCCESS;
}
