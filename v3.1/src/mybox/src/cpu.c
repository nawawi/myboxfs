/*
 (C) Copyright 2008 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.
*/

#include "libmybox.h"
#include <time.h>

unsigned long long cpu_usr,cpu_nice,cpu_sys,cpu_idle,cpu_iowait,cpu_irq,cpu_softirq,cpu_steal;
unsigned long long cpu_total,cpu_busy;

static int get_cpu(void) {
	FILE *fp;
	int x=0;
	unsigned total_diff;

	typedef struct jiffy_counts_t {
		unsigned long long usr,nic,sys,idle,iowait,irq,softirq,steal;
		unsigned long long total;
		unsigned long long busy;
	} jiffy_counts_t;
	jiffy_counts_t jif, prev_jif;

	for(x=0;x<2;x++) {
		prev_jif = jif;
		if((fp=fopen("/proc/stat","r"))!=NULL) {
			if(fscanf(fp, "cpu  %lld %lld %lld %lld %lld %lld %lld %lld",
				&jif.usr,&jif.nic,&jif.sys,&jif.idle,
				&jif.iowait,&jif.irq,&jif.softirq,&jif.steal) < 4) {
					exit(1);
			}
			fclose(fp);
			jif.total = jif.usr + jif.nic + jif.sys + jif.idle + jif.iowait + jif.irq + jif.softirq + jif.steal;
			jif.busy = jif.total - jif.idle - jif.iowait;
		}
		sleep(1);
	}
	total_diff = ((unsigned)(jif.total - prev_jif.total) ? : 1);
#define CALC_STAT(xxx) unsigned xxx = 100 * (unsigned)(jif.xxx - prev_jif.xxx) / total_diff
	{ /* need block: CALC_STAT are declarations */
		CALC_STAT(usr);CALC_STAT(sys);
		CALC_STAT(nic);CALC_STAT(idle);
		CALC_STAT(iowait);CALC_STAT(irq);
		CALC_STAT(softirq);
		CALC_STAT(busy);
		cpu_usr=usr;cpu_sys=sys;
		cpu_nice=nic;cpu_idle=idle;
		cpu_iowait=iowait;cpu_irq=irq;
		cpu_softirq=softirq;cpu_total=busy;
		cpu_busy=busy;
       }
	return(0);
}

int main(int argc,char **argv) {
	int x;
	get_cpu();
	if(argc > 1) {
		for(x=1;x < argc;x++) {
			if(!strcmp(argv[x],"all")) {
				printf("%u,%u,%u,%u,%u,%u,%u,%u\n",cpu_usr,cpu_nice,cpu_sys,cpu_idle,cpu_iowait,cpu_irq,cpu_softirq,cpu_steal);
				exit(0);
			}
			if(!strcmp(argv[x],"usage")) {
				printf("%u,",100 - cpu_idle);
				printf("%u\n",cpu_idle);
				exit(0);
			}
		}
	}
	time_t curtime;
        struct tm *loctime;
	char buf[50];
	while(1) {
		curtime=time(NULL);
                loctime=localtime(&curtime);
		strftime(buf, sizeof(buf), "%d-%b-%Y %T",loctime);
		get_cpu();
		printf("%s\n",buf);
		printf("====================\n");
		printf("User     : %4u%%\n",cpu_usr);
		printf("Nice     : %4u%%\n",cpu_nice);
		printf("System   : %4u%%\n",cpu_sys);
		printf("Idle     : %4u%%\n",cpu_idle);
		printf("IO wait  : %4u%%\n",cpu_iowait);
		printf("IRQ      : %4u%%\n",cpu_irq);
		printf("Soft IRQ : %4u%%\n",cpu_softirq);
		printf("Steal    : %4u%%\n",cpu_steal);
		putchar('\n');
		sleep(1);
	}
	exit(0);
}
