#include <libmybox.h>

int main(int argc, char **argv) {
	char *cmd;
  	if(argc == 0) {
		printf("no argv[0]?\n");
		exit(1);
	} else {
		cmd=(char *) base_name(argv);
		if(!strcmp(cmd, "mybox")) {
			*argv++;
			argc--;
			cmd=(char *)base_name(argv);
		}
		if(!strcmp(cmd, "bootup")) return bootup_main(argc, argv);
		if(!strcmp(cmd, "bootdown")) return bootdown_main(argc, argv);
		//if(!strcmp(cmd, "initrc")) return initrc_main(argc, argv);
		//if(!strcmp(cmd, "logtail")) return logtail_main(argc, argv);
		if(!strcmp(cmd, "getkey")) return getkey_main(argc, argv);
		if(!strcmp(cmd, "login")) return login_main(argc, argv);
		if(!strcmp(cmd, "lcdd")) return lcdd_main(argc, argv);
		if(!strcmp(cmd, "sqlite")) return sqlite_main(argc, argv);
		if(!strcmp(cmd, "iosh") || !strcmp(cmd, "-iosh")) return iosh_main(argc, argv);
		if(!strcmp(cmd, "uplinkd")) return uplinkd_main(argc, argv);
		if(!strcmp(cmd, "chkprog")) return chkprog_main(argc, argv);
		if(!strcmp(cmd, "ping")) return ping_main(argc, argv);
		if(!strcmp(cmd, "taskq")) return taskq_main(argc, argv);
		if(!strcmp(cmd, "banner")) {
			print_banner();
			exit(0);
		}
		if(cmd[0]=='.') {
    			printf("TraceNetOS utilities: unknown applet\n");
		} else {
    			printf("TraceNetOS utilities: unknown applet name %s\n",cmd);
		}
		exit(1);
	}
}

