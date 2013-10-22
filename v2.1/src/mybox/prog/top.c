/* vi: set sw=4 ts=4: */
/*
 * A tiny 'top' utility.
O5B *
 * This is written specifically for the linux /proc/<PID>/stat(m)
 * files format.

 * This reads the PIDs of all processes and their status and shows
 * the status of processes (first ones that fit to screen) at given
 * intervals.
 *
 * NOTES:
 * - At startup this changes to /proc, all the reads are then
 *   relative to that.
 *
 * (C) Eero Tamminen <oak at welho dot com>
 *
 * Rewritten by Vladimir Oleynik (C) 2002 <dzo@simtreas.ru>
 */

/* Original code Copyrights */
/*
 * Copyright (c) 1992 Branko Lankester
 * Copyright (c) 1992 Roger Binns
 * Copyright (C) 1994-1996 Charles L. Blake.
 * Copyright (C) 1992-1998 Michael K. Johnson
 * May be distributed under the conditions of the
 * GNU Library General Public License
 */

// 28/02/2007 ported to mybox - nawawi 
#include <libmybox.h>


typedef int (*cmp_t)(procps_status_t *P, procps_status_t *Q);

static procps_status_t *top;
static int ntop;

static int mem_sort(procps_status_t *P, procps_status_t *Q) {
	return (int)(Q->rss - P->rss);
}


#define sort_depth 3
static cmp_t sort_function[sort_depth];

static int pcpu_sort(procps_status_t *P, procps_status_t *Q) {
	return (Q->pcpu - P->pcpu);
}

static int time_sort(procps_status_t *P, procps_status_t *Q) {
	return (int)((Q->stime + Q->utime) - (P->stime + P->utime));
}

static int mult_lvl_cmp(void* a, void* b) {
	int i, cmp_val;

	for (i = 0; i < sort_depth; i++) {
		cmp_val = (*sort_function[i])(a, b);
		if (cmp_val != 0)
			return cmp_val;
	}
	return 0;
}

struct save_hist {
	int ticks;
	int pid;
};

static struct save_hist *prev_hist;
static int prev_hist_count;

static unsigned total_pcpu;

struct jiffy_counts {
	unsigned long long usr,nic,sys,idle,iowait,irq,softirq,steal;
	unsigned long long total;
	unsigned long long busy;
};
static struct jiffy_counts jif, prev_jif;

static void get_jiffy_counts(void) {
	FILE* fp = fopen("stat", "r");
	prev_jif = jif;
	if (fscanf(fp, "cpu  %lld %lld %lld %lld %lld %lld %lld %lld",
			&jif.usr,&jif.nic,&jif.sys,&jif.idle,
			&jif.iowait,&jif.irq,&jif.softirq,&jif.steal) < 4) {
		printf("failed to read 'stat'\n");
	}
	fclose(fp);
	jif.total = jif.usr + jif.nic + jif.sys + jif.idle
			+ jif.iowait + jif.irq + jif.softirq + jif.steal;
	jif.busy = jif.total - jif.idle - jif.iowait;
}

static void do_stats(void) {
	procps_status_t *cur;
	int pid, total_time, i, last_i, n;
	struct save_hist *new_hist;

	get_jiffy_counts();
	total_pcpu = 0;
	new_hist = xmalloc(sizeof(struct save_hist)*ntop);
	i = 0;
	for (n = 0; n < ntop; n++) {
		cur = top + n;
		pid = cur->pid;
		total_time = cur->stime + cur->utime;
		new_hist[n].ticks = total_time;
		new_hist[n].pid = pid;
		cur->pcpu = 0;
		last_i = i;
		if (prev_hist_count) do {
			if (prev_hist[i].pid == pid) {
				cur->pcpu = total_time - prev_hist[i].ticks;
				break;
			}
			i = (i+1) % prev_hist_count;
		} while (i != last_i);
		total_pcpu += cur->pcpu;
	}

	free(prev_hist);
	prev_hist = new_hist;
	prev_hist_count = ntop;
}

static unsigned long display_generic(int scr_width) {
	FILE *fp;
	char buf[80];
	char scrbuf[80];
	char *end;
	unsigned long total, used, mfree, shared, buffers, cached;
	unsigned int needs_conversion = 1;
	fp = fopen("meminfo", "r");
	if (fscanf(fp, "MemTotal: %lu %s\n", &total, buf) != 2) {
		fgets(buf, sizeof(buf), fp);
		fscanf(fp, "Mem: %lu %lu %lu %lu %lu %lu",&total, &used, &mfree, &shared, &buffers, &cached);
	} else {
		needs_conversion = 0;
		fscanf(fp, "MemFree: %lu %s\n", &mfree, buf);
		if (fscanf(fp, "MemShared: %lu %s\n", &shared, buf) != 2) shared = 0;
		fscanf(fp, "Buffers: %lu %s\n", &buffers, buf);
		fscanf(fp, "Cached: %lu %s\n", &cached, buf);

		used = total - mfree;
	}
	fclose(fp);

	/* read load average as a string */
	fp = fopen("loadavg", "r");
	buf[0] = '\0';
	fgets(buf, sizeof(buf), fp);
	end = strchr(buf, ' ');
	if (end) end = strchr(end+1, ' ');
	if (end) end = strchr(end+1, ' ');
	if (end) *end = '\0';
	fclose(fp);

	if (needs_conversion) {
		/* convert to kilobytes */
		used /= 1024;
		mfree /= 1024;
		shared /= 1024;
		buffers /= 1024;
		cached /= 1024;
		total /= 1024;
	}
	if (scr_width > sizeof(scrbuf))
		scr_width = sizeof(scrbuf);
	snprintf(scrbuf, scr_width,
		"Mem: %ldK used, %ldK free, %ldK shrd, %ldK buff, %ldK cached",
		used, mfree, shared, buffers, cached);
	printf("\e[H\e[J%s\n", scrbuf);
	snprintf(scrbuf, scr_width,
		"Load average: %s  (Status: S=sleeping R=running, W=waiting)", buf);
	printf("%s\n", scrbuf);
	return total;
}

static void display_status(int count, int scr_width) {
	enum {
		bits_per_int = sizeof(int)*8
	};

	procps_status_t *s = top;
	char rss_str_buf[8];
	unsigned long total_memory = display_generic(scr_width);
	unsigned pmem_shift, pmem_scale;

	unsigned pcpu_shift, pcpu_scale;

	/* what info of the processes is shown */
	printf("\e[7m%.*s\e[0m", scr_width,
		"  PID     STATUS   RSS  PPID %CPU %MEM COMMAND");
#define MIN_WIDTH sizeof( "  PID     STATUS   RSS  PPID %CPU %MEM C")
	pmem_shift = bits_per_int-11;
	pmem_scale = 1000*(1U<<(bits_per_int-11)) / total_memory;
	while (pmem_scale >= 512) {
		pmem_scale /= 4;
		pmem_shift -= 2;
	}
	pcpu_shift = 6;
	pcpu_scale = (1000*64*(uint16_t)(jif.busy-prev_jif.busy) ? : 1);
	while (pcpu_scale < (1U<<(bits_per_int-2))) {
		pcpu_scale *= 4;
		pcpu_shift += 2;
	}
	pcpu_scale /= ( (uint16_t)(jif.total-prev_jif.total)*total_pcpu ? : 1);
	while (pcpu_scale >= 1024) {
		pcpu_scale /= 4;
		pcpu_shift -= 2;
	}

	while (count--) {
		div_t pmem = div( (s->rss*pmem_scale) >> pmem_shift, 10);
		int col = scr_width+1;
		div_t pcpu;

		if (s->rss >= 100*1024)
			sprintf(rss_str_buf, "%6ldM", s->rss/1024);
		else
			sprintf(rss_str_buf, "%7ld", s->rss);
		pcpu = div((s->pcpu*pcpu_scale) >> pcpu_shift, 10);
		col -= printf("\n%5d %-3s %s %s%6d%3u.%c%3u.%c ",
				s->pid, "", s->state, rss_str_buf, s->ppid,
				pcpu.quot, '0'+pcpu.rem,
				pmem.quot, '0'+pmem.rem);
		if (col>0)
			printf("%.*s", col, s->short_cmd);
		/* printf(" %d/%d %lld/%lld", s->pcpu, total_pcpu,
			jif.busy - prev_jif.busy, jif.total - prev_jif.total); */
		s++;
	}
	putchar('\r');
	fflush(stdout);
}

static void clearmems(void) {
	free(top);
	top = 0;
	ntop = 0;
}

static int chk_loop=1;
static void do_exit(void) {
	chk_loop=0;
	return(0);
}
int top_main(int argc, char **argv) {
	int opt, lines, col, k;
	chdir("/proc");
	sort_function[0] = pcpu_sort;
	sort_function[1] = mem_sort;
	sort_function[2] = time_sort;
	signal(SIGINT,do_exit);
	chk_loop=1;
	while (chk_loop==1) {
		procps_status_t *p;
		/* Default to 25 lines - 5 lines for status */
		lines = 24 - 3;
		col = 79;

		/* read process IDs & status for all the processes */
		while ((p = procps_scan(0)) != 0) {
			int n = ntop;

			top = realloc(top, (++ntop)*sizeof(procps_status_t));
			memcpy(top + n, p, sizeof(procps_status_t));
		}
		if (ntop == 0) {
			printf("Can't find process info in /proc\n");
		}
		if (!prev_hist_count) {
			do_stats();
			sleep(1);
			clearmems();
			continue;
		}
		do_stats();
		qsort(top, ntop, sizeof(procps_status_t), (void*)mult_lvl_cmp);
		opt = lines;
		if (opt > ntop) {
			opt = ntop;
		}
		/* show status for each of the processes */
		display_status(opt, col);
		clearmems();
		sleep(5);
	}
	clearmems();
	putchar('\n');
	return 0;
}

