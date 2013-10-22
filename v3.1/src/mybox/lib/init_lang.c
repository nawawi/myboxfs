#include "libmybox.h"
int init_lang(void) {
        char buff[50];
        unsetenv("LANG");
        setlocale(LC_ALL, "");
        cli_help[0]='\0';
        if(read_oneline(LANG_DIR"/default", buff) < 0) {
                return 0;
        }
        trim(buff);
        if(buff[0]!='\0') {
                setenv("LANG",buff,1);
                setlocale(LC_ALL, "");
                textdomain("traceos");
                bindtextdomain("traceos",LANG_DIR);
                strncpy(cli_help,buff,sizeof(cli_help));
                return 1;
        }
        return 0;
}

