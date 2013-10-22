#include <libmybox.h>

int mkdev_main(int argc, char **argv) {
	int major, minor;
	if(argc!=4) exit(1);
	major=atoi(argv[2]);
	minor=atoi(argv[3]);
	return mk_dev(argv[1],major,minor);
}



