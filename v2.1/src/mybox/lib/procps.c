/* vi: set sw=4 ts=4: */
/*
 * Mini ps implementation(s) for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under the GPL v2, see the file LICENSE in this tarball.
 */


#include <libmybox.h>


procps_status_t * procps_scan(int dotask) {
	static DIR *dir;
	struct dirent *entry;
	static procps_status_t ret_status;
	char *name;
	int n;
	char status[32];
	char *status_tail;
	char buf[1024];
	procps_status_t curstatus;
	int pid;
	long tasknice;
	struct stat sb;

	if(!dir) {
                dir=opendir("/proc");
        }
	for(;;) {
		if((entry = readdir(dir)) == NULL) {
			closedir(dir);
			dir = 0;
			return 0;
		}
		name = entry->d_name;
		if(!(*name >= '0' && *name <= '9')) continue;

		memset(&curstatus, 0, sizeof(procps_status_t));
		pid = atoi(name);
		curstatus.pid = pid;

		status_tail = status + sprintf(status, "/proc/%d", pid);
		if(stat(status, &sb)) continue;
		strcpy(status_tail, "/stat");
		n = read_oneline(status, buf);
		if(n < 0) continue;
		name = strrchr(buf, ')');
		if(name == 0 || name[1] != ' ') continue;
		*name = 0;
		sscanf(buf, "%*s (%15c", curstatus.short_cmd);
		n = sscanf(name+2,
		"%c %d "
		"%*s %*s %*s %*s "     /* pgrp, session, tty, tpgid */
		"%*s %*s %*s %*s %*s " /* flags, min_flt, cmin_flt, maj_flt, cmaj_flt */
		"%lu %lu "             /* utime, stime */
		"%*s %*s %*s "         /* cutime, cstime, priority */
		"%ld "                 /* nice */
		"%*s %*s %*s "         /* timeout, it_real_value, start_time */
		"%*s "                 /* vsize */
		"%ld",                 /* rss */
		curstatus.state, &curstatus.ppid,
		&curstatus.utime, &curstatus.stime,
		&tasknice,
                &curstatus.rss);

		if(n != 6)
			continue;

		// paging (not valid since the 2.6.xx kernel)
		//if (curstatus.rss == 0 && curstatus.state[0] != 'Z')
		//	curstatus.state[1] = 'W';
		//else
		//curstatus.state[1] = '';
		if (tasknice < 0) {
			curstatus.state[1] = '<';
		} else if(tasknice > 0) {
			curstatus.state[1] = 'N';
		} else {
			curstatus.state[1] = ' ';
		}
		curstatus.state[2]=' ';
		curstatus.state[3]=' ';
		curstatus.rss *= (getpagesize() >> 10);     /* 2**10 = 1kb */

		if(dotask==1) {
			strcpy(status_tail, "/cmdline");
			n = read_oneline(status, buf);
			if(n > 0) {
				if(buf[n-1]=='\n')
					buf[--n] = 0;
				name = buf;
				while(n) {
					if(((unsigned char)*name) < ' ')
						*name = ' ';
					name++;
					n--;
				}
				*name = 0;
				if(buf[0])
					curstatus.cmd = strdup(buf);
				/* if NULL it work true also */
			}
		}
		return memcpy(&ret_status, &curstatus, sizeof(procps_status_t));
	}
}


