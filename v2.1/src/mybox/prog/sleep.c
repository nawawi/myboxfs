#include <libmybox.h>

int sleep_main(int argc, char **argv) {
	int interval;
	if(argv[1]!=NULL) {
		interval=atoi(argv[1]);
		sleep(interval);
		return 0;
	}
	return 1;
}



