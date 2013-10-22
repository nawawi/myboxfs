/*
 (C) Copyright 2008 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.
*/

#include "libmybox.h"

#ifndef FSHIFT
#define FSHIFT 16
#endif
#define FIXED_1 (1<<FSHIFT)
#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)

int main(int argc,char **argv) {
	struct sysinfo si;
        sysinfo(&si);
	printf("%ld.%02ld,%ld.%02ld,%ld.%02ld\n",
                        LOAD_INT(si.loads[0]), LOAD_FRAC(si.loads[0]),
                        LOAD_INT(si.loads[1]), LOAD_FRAC(si.loads[1]),
                        LOAD_INT(si.loads[2]), LOAD_FRAC(si.loads[2]));
	exit(0);
}
