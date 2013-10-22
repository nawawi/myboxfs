/*
 (C) Copyright 2008 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.
*/

#include "libmybox.h"
#include <time.h>

int main(int argc,char **argv) {
        char booted[40];
        long btime=get_boottime();
	if(argc > 1) {
		if(!strcmp(argv[1],"log")) {
			strftime(booted, sizeof(booted), "%d-%b-%Y %T",localtime(&btime));
			puts(booted);
			exit(0);
		}
	}
	strftime(booted, sizeof(booted), "%a %b %e %H:%M:%S %Z %Y",localtime(&btime));
	puts(booted);
	exit(0);
}
