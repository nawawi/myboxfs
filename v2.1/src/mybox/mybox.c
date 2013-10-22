#include <libmybox.h>

int main(int argc, char **argv) {
	char *cmd;
  	if(argc == 0) {
		puts("no argv[0]?\n");
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
		if(!strcmp(cmd, "initrc")) return initrc_main(argc, argv);
		if(!strcmp(cmd, "logtail")) return logtail_main(argc, argv);
		if(!strcmp(cmd, "getkey")) return getkey_main(argc, argv);
		if(!strcmp(cmd, "login")) return login_main(argc, argv);
		if(!strcmp(cmd, "lcdd")) return lcdd_main(argc, argv);
		if(!strcmp(cmd, "sqlite")) return sqlite_main(argc, argv);
		if(!strcmp(cmd, "basename")) return basename_main(argc, argv);
		if(!strcmp(cmd, "iosh")) return iosh_main(argc, argv);
		if(!strcmp(cmd, "failoverd")) return failoverd_main(argc, argv);
		if(!strcmp(cmd, "chkprog")) return chkprog_main(argc, argv);
		if(!strcmp(cmd, "pwd")) return pwd_main(argc, argv);
		if(!strcmp(cmd, "ping")) return ping_main(argc, argv);
		if(!strcmp(cmd, "arpscan")) return arpscan_main(argc, argv);
		if(!strcmp(cmd, "ps")) return ps_main(argc, argv);
		if(!strcmp(cmd, "top")) return top_main(argc, argv);
		if(!strcmp(cmd, "reset")) return reset_main(argc, argv);
		if(!strcmp(cmd, "clear")) return clear_main(argc, argv);
		if(!strcmp(cmd, "uptime")) return uptime_main(argc, argv);
		if(!strcmp(cmd, "sync")) return sync_main(argc, argv);
		if(!strcmp(cmd, "mkdir")) return mkdir_main(argc, argv);
		if(!strcmp(cmd, "mkdev")) return mkdev_main(argc, argv);
		if(!strcmp(cmd, "sleep")) return sleep_main(argc, argv);
		if(!strcmp(cmd, "usleep")) return usleep_main(argc, argv);
		if(!strcmp(cmd, "false")) return false_main(argc, argv);
		if(!strcmp(cmd, "true")) return true_main(argc, argv);
		if(cmd[0]=='.') {
    			printf("MyboxOS utilities: unknown applet\n");
		} else {
    			printf("MyboxOS utilities: unknown applet name %s\n",cmd);
		}
		exit(1);
	}
}

