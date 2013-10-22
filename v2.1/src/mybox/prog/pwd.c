#include <libmybox.h>
int pwd_main(int argc,char **argv) {
        char *pwd=getcwd(NULL, 0);
        printf("%s\n", pwd);
        return 0;
}
