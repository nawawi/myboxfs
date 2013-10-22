#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <utime.h>
#include <errno.h>
#include <unistd.h>

#include "common.h"
#include "popen.h"

int query1=0;
int query2=0;
int query3=0;
int snmpd=0;
int miniserv=0;
int sshd=0;
int syslogd=0;
int klogd=0;
int systemd=0;
int kerneld=0;
int ipsd=0;
int dhcpd=0;

int mytouch(char *file) {
	int fd;
	fd=open(file, O_RDWR | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if(fd >=0) return 0;
	return 1;
}


int file_exist(char *s) {
        struct stat ss;
        int i = stat(s, &ss);
        if (i < 0) return 0;
        if ((ss.st_mode & S_IFREG) || (ss.st_mode & S_IFLNK)) return(1);
        return(0);
}

void rmspace(char *x) {
	char *t;
   	for(t=x+strlen(x)-1; (((*t=='\x20')||(*t=='\x0D')||(*t=='\x0A')||(*t=='\x09'))&&(t >= x)); t--);
   	if(t!=x+strlen(x)-1) *(t+1)=0;
   	for(t=x; (((*t=='\x20')||(*t=='\x0D')||(*t=='\x0A')||(*t=='\x09'))&&(*t)); t++);
   	if(t!=x) strcpy(x,t);
}

void splitc(char *first, char *rest, char divider) {
	char *p;
   	p=strchr(rest, divider);
   	if(p==NULL) {
      		if((first != rest) && (first != NULL))
	 	first[0] = 0;
      		return;
   	}
   	*p=0;
   	if(first != NULL) strcpy(first, rest);
   	if(first != rest) strcpy(rest, p + 1);
}

void chkpid(void) {
        char buff[MAX_INPUT_BUFFER];
        if((child_process = spopen("/bin/ps ax")) == NULL) return 1;
        while(fgets(buff, MAX_INPUT_BUFFER - 1, child_process)) {
		rmspace(buff);
		if(strstr(buff,"mfs-query.exc q")) query1=1;
		if(strstr(buff,"mfs-query.exc i")) query2=1;
		if(strstr(buff,"mfs-query.exc o")) query3=1;
		if(strstr(buff,"snmpd")) snmpd=1;
		if(strstr(buff,"mfs-miniserv.exc")) miniserv=1;
		if(strstr(buff,"sshd")) sshd=1;
		if(strstr(buff,"syslogd")) syslogd=1;
		if(strstr(buff,"klogd")) klogd=1;
		if(strstr(buff,"mfs-system.exc")) systemd=1;
		if(strstr(buff,"mfs-kernel.exc")) kerneld=1;
		if(strstr(buff,"ipsd")) ipsd++;
		if(strstr(buff,"dhcpd")) dhcpd++;
	}
	spclose(child_process);
}

void fork_script(const char *file,const char *arg) {
	pid_t pid,status; 
	pid = fork();
        if(pid==0) {
                execl("/bin/php","/bin/php","-q", file, arg, NULL);
		sleep(1);
                exit(EXIT_SUCCESS);
        }
	while(wait(&status)!=pid);
}

void do_ping(const char *ip, const char *dev) {
	pid_t pid,status; 
	pid = fork();
        if(pid==0) {
		execl("/bin/arping","/bin/arping","-qb",ip,"-c","3","-w","3","-I",dev,NULL);
		sleep(1);
                exit(EXIT_SUCCESS);
	}
	while(wait(&status)!=pid);
}

void chkfailover(void) {
	FILE *f;
	char buff[150];
	char ipgw[25], gwdev[5], ipfa[25], fadev[5];
	pid_t pid,status; 
	if(file_exist("/var/sys/load_balancing")) {
		pid = fork();
        	if(pid==0) {
			if(f=fopen("/var/sys/load_balancing","r")) {
				while(fgets(buff, sizeof(buff) - 1, f)) {
					rmspace(buff);
					splitc(ipgw,buff,'|');
					splitc(gwdev,buff,'|');
					splitc(ipfa,buff,'|');
					splitc(fadev,buff,'|');
				}
				fclose(f);
				do_ping(ipgw,gwdev);
				do_ping(ipfa,fadev);
			}
                	exit(EXIT_SUCCESS);
		}
		while(wait(&status)!=pid);
	}
}

void chkipsd(void) {
	FILE *f;
	char buff[150];
	int cnt=0;
	if(file_exist("/var/sys/.chk_ipsd")) {
		if(f=fopen("/var/sys/.chk_ipsd","r")) {
			fgets(buff, sizeof(buff) - 1, f);
			rmspace(buff);
			cnt=atoi(buff);
			fclose(f);
		}
	}
	if(ipsd >= cnt) {
		ipsd=1;
	} else {
		ipsd=0;
	}
}

int main(void) {
	freopen("/dev/null", "r", stdin);
      	freopen("/dev/null", "w", stdout);
      	freopen("/dev/null", "w", stderr);

	chkfailover();
	chkpid();
	chkipsd();

	if(query1==0) {
		fork_script("/service/tools/mfs-query.exc","q");
	}
	if(query2==0) {
		fork_script("/service/tools/mfs-query.exc","i");
	}
	if(query3==0) {
		fork_script("/service/tools/mfs-query.exc","o");
	}
	if(ipsd==0) {
		fork_script("/service/init/ips.init","restart");
	}
	if(file_exist("/var/sys/.chk_snmp") && snmpd==0) {
		fork_script("/service/init/snmp.init","restart");
	}
	if(file_exist("/var/sys/.chk_https") && miniserv==0) {
		fork_script("/service/init/https.init","restart");
	}
	if(file_exist("/var/sys/.chk_ssh") && sshd==0) {
		fork_script("/service/init/ssh.init","restart");
	}
	if(file_exist("/var/sys/.chk_syslog")&& (syslogd==0 || klogd==0 || systemd==0 || kerneld==0)) {
		fork_script("/service/init/syslog.init","restart");
	}
	if(file_exist("/var/sys/.chk_dhcp") && dhcpd==0) {
		fork_script("/service/init/dhcp.init","restart");
	}
	sleep(1);
        exit(EXIT_SUCCESS);
}
