/* vi: set sw=4 ts=4: */
/*
 * Mini ps implementation(s) for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under the GPL v2, see the file LICENSE in this tarball.
 */
// ported to mybox - 22/02/2007

#include <libmybox.h>

int ps_main(int argc, char **argv) {
	procps_status_t *p;
	int i, len, hide=1;

	int terminal_width=INT_MAX;

	// awie
	if(argv[1]!=NULL && argv[2]!=NULL) {
		if(strcmp(argv[1],"grep")==0) {
			char cmd1[128];
			snprintf(cmd1,sizeof(cmd1),"%s",argv[2]);
			while ((p = procps_scan(1)) != 0)  {
				char *namecmd = p->cmd;
				if(namecmd!=NULL) {
					if(!strstr(namecmd,"ps grep") && strstr(namecmd,cmd1)) printf("%d ",p->pid);
				}
			}
			putchar('\n');
			return 0;
		}
	}
	if(argv[1]!=NULL && strcmp(argv[1],"real")==0) hide=0;

	printf("  PID   STAT  COMMAND\n");

	while ((p = procps_scan(1)) != 0)  {
		char *namecmd=p->cmd;
		len = printf("%5d   %s  ", p->pid,p->state);
		i = terminal_width-len;
		if(namecmd && namecmd[0]) {
			if(i < 0)
				i = 0;
			if(strlen(namecmd) > (size_t)i)
				namecmd[i] = 0;
			// awie
			if(hide==1) {
				char *st;
				if(strstr(namecmd,"/bin/php -f /service/tools/")) {
					st=str_replace(namecmd, "/bin/php -f /service/tools/", "");
				} else if(strstr(namecmd,"/bin/php -Cq /service/init/")) {
					st=str_replace(namecmd, "/bin/php -Cq /service/init/", "");
				} else if(strstr(namecmd,"/bin/php /service/www/")) {
					st=str_replace(namecmd, "/bin/php /service/www/", "");
				} else if(strstr(namecmd,"/bin/php -f ")) {
					st=str_replace(namecmd, "/bin/php -f ", "");
				} else if(strstr(namecmd,"/bin/php -Cq ")) {
					st=str_replace(namecmd, "/bin/php -Cq ", "");
				} else if(strstr(namecmd,"snortd ")) {
					st="snortd";
				} else if(strstr(namecmd,"crond ")) {
					st="crond";
				} else if(strstr(namecmd,"wget ")) {
					st="wget";
				} else if(strstr(namecmd,"ddnsd ")) {
					st="ddnsd";
				} else if(strstr(namecmd,"rrdtool")) {
					st="create-graph";
				} else if(strstr(namecmd,"sh -c")) {
					st="shellexecve";
				} else {
					st=str_replace(namecmd, "/bin/", "");
				}
				if(st) {
					printf("%s\n", st);
				} else {
					printf("%s\n", namecmd);
				}
			} else {
				 printf("%s\n", namecmd);
			}
		} else {
			namecmd = p->short_cmd;
			if(i < 2) i = 2;
			if(strlen(namecmd) > ((size_t)i-2)) namecmd[i-2] = 0;
			printf("[%s]\n", namecmd);
		}
		free(p->cmd);
	}
	return 0;
}
