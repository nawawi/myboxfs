#include <libmybox.h>

int mkdir_main(int argc, char **argv) {
        int status=0;
	*argv++;
        do {
		if(*argv[0]!='-') {
			if(xmkdir(*argv)) {
				status = 1;
			}
		}
        } while (*++argv);
        exit(status);
}



