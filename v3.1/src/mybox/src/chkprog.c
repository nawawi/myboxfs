#include "libmybox.h"
#include <time.h>

struct listq_q {
        char prog[150];
        char cmd[150];
	int status;
} listq[200];

#define DIRCHECK "/var/sys/check"
#define DIRQUE "/var/sys/taskq"

static int listcnt=0;
static int taskq=0;

static void set_config(char *prog,char *cmd, int cnt) {
        snprintf(listq[cnt].prog,sizeof(listq[cnt].prog),"%s",prog);
        snprintf(listq[cnt].cmd,sizeof(listq[cnt].cmd),"%s",cmd);
	listq[cnt].status=0;
}

static void __log(char *prog) {
        char msg[256];
        snprintf(msg,sizeof(msg),"Service %s down",prog);
        openlog("CHKPROG", LOG_NDELAY|LOG_PID, LOG_DAEMON);
        syslog(LOG_ALERT,"%s",msg);
        closelog();
}

static int check_config(void) {
        DIR *dir;
        struct dirent *entry;
        char *name;
        int n;
        struct stat sb;
        char status[32];
        char buf[1023];
	listcnt=0;
        if((dir = opendir(DIRCHECK)) == NULL) return(1);
        for(;;) {
                if((entry = readdir(dir)) == NULL) {
                        closedir(dir);
                        return(1);
                }
                name = entry->d_name;
                if(!strcmp(name,".") || !strcmp(name,"..")) continue;
                sprintf(status, "%s/%s", DIRCHECK, name);
                if(stat(status, &sb)) continue;
                if(read_oneline(status, buf) < 0) continue;
                trim(buf);trim(name);
                set_config(name,buf,listcnt);
                listcnt++;
        }
        return(0);
}

// 1=up, 0=down
static void set_check(char *str) {
	int i;
	char fname[150];
	for(i=0; i < listcnt ;i++) {
		if(listq[i].prog) {
			memset(fname,0x0,sizeof(fname));	
			snprintf(fname,sizeof(fname),"%s/%s",DIRQUE,listq[i].cmd);
			if(file_exists(fname)) continue;
			if(strstr(str,listq[i].prog)) {
				listq[i].status=1;
			}
		}
	}
}

static void clear_set(void) {
	int i;
	taskq=0;
	for(i=0; i < listcnt ;i++) {
		listq[i].status=0;
		memset(listq[i].prog,0x0,sizeof(listq[i].prog));
		memset(listq[i].cmd,0x0,sizeof(listq[i].cmd));
	}
	listcnt=0;
}

static int run_daemon(char *pname,char *prog) {
	if(file_exists("/bin/start-stop-daemon")) {
		return xsystem("/bin/start-stop-daemon -S -q -b -m -p /var/run/%s.pid -x %s",pname,prog);
	}
	return 1;
}
static void run_check(void) {
	int i;
	time_t curtime;
        struct tm *loctime;
	char fname[150];
	char info[150];

	if(taskq==0 && file_exists("/bin/taskq-check")) {
		run_daemon("taskq-check","/bin/taskq-check");
	}

	if(!file_exists(DIRQUE)) xmkdir(DIRQUE);
	for(i=0; i < listcnt;i++) {
		if(listq[i].cmd && listq[i].status==0) {
			curtime=time(NULL);
        		loctime=localtime(&curtime);
			memset(fname,0x0,sizeof(fname));
			snprintf(fname,sizeof(fname),"%s/%s",DIRQUE,listq[i].cmd);
			if(file_exists(fname)) continue;
			strftime(info, sizeof(info), "%d/%m/%Y %T",loctime);
			save_to_file(fname,"%s",info);
			//printf("[%s] Service %s down\n",info,listq[i].prog);
			memset(info,0x0,sizeof(info));
			__log(listq[i].prog);
		}
	}
}

static int check_prog(void) {
        DIR *dir;
        struct dirent *entry;
        char *name;
        char *status_tail;
        int pid;
        struct stat sb;
        int n;
        char status[32];
        char buf[1023];
        char cmd[16], scmd[16];
	char str[1024];
	if((dir = opendir("/proc")) == NULL) return(1);
        for(;;) {
                if((entry = readdir(dir)) == NULL) {
                        closedir(dir);
                        return(0);
                }
                name = entry->d_name;
                if (!(*name >= '0' && *name <= '9')) continue;
                pid = atoi(name);
                status_tail = status + sprintf(status, "/proc/%d", pid);
                if(stat(status, &sb)) continue;
                strcpy(status_tail, "/stat");
                n = read_oneline(status, buf);
                if(n < 0) continue;
                sscanf(buf, "%*s (%15c)", cmd);
		splitc(scmd,cmd,')');
		sprintf(str,"%s ",scmd);
                strcpy(status_tail, "/cmdline");
		n = read_oneline(status, buf);
                if(n > 0) {
                        if(buf[n-1]=='\n')  buf[--n] = 0;
                        name = buf;
                        while(n) {
                                if(((unsigned char)*name) < ' ') *name = ' ';
                                name++;
                                n--;
                        }
                        *name = 0;
                        if(buf[0]) sprintf(str,"%s %s ",str, strdup(buf));
                }
		trim(str);
		if(strstr(str,"taskq-check")) {
			taskq=1;
			continue;
		} 
		set_check(str);
        }
        return(0);
}

static int proc_idle() {
	if(file_exists("/var/sys/init_down")) return 1; 
	if(file_exists("/var/sys/chkprog_stop")) return 1;
	return 0;
}

int main(int argc,char **argv) {
	chdir("/");
	for(;;) {
		if(proc_idle()) {
			sleep(100);
			continue;
		}
		clear_set();
		check_config();
		check_prog();
		run_check();
		sleep(10);
	}
        exit(0);
}
