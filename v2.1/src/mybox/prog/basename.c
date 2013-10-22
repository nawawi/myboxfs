#include <libmybox.h>

int basename_main(int argc, char **argv) {
        if(argc==1) exit(1);
        *argv++;
       	printf("%s\n",(char *)base_name(argv));
        exit(0);
}


