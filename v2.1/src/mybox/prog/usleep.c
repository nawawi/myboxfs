#include <libmybox.h>

int usleep_main(int argc, char **argv) {
	int interval;
	if(argv[1]!=NULL) {
		interval=atoi(argv[1]);
		usleep(interval);
		return 0;
	}
	return 1;
}



