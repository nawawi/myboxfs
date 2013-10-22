#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int file_exist(char *s);
extern void rmspace(char *x);
extern void splitc(char *first, char *rest, char divider);

int query=0;
int snmpd=0;
int miniserv=0;
int sshd=0;
int syslogd=0;

void chkpid(void) {
	FILE *f;
	char buff[256];
	if(f=popen("ps ax","r")) {
		while(fgets(buff, sizeof(buff) - 1, f)) {
			rmspace(buff);
			if(strstr(buff,"mfs-query.exc q")) query=1;
			if(strstr(buff,"snmpd")) snmpd=1;
			if(strstr(buff,"mfs-miniserv.exc")) miniserv=1;
			if(strstr(buff,"sshd")) sshd=1;
			if(strstr(buff,"syslogd")) syslogd=1;
		}
		pclose(f);
        }
}

int mysystem(char *cmd) {
	FILE *p;
	char buf[256];
	snprintf(buf,sizeof(buf),"%s >/dev/null 2>&1",cmd);
	if(p=popen(buf,"r")) {
		pclose(p);
		return 0;
	}
	return 1;
}

int do_ping(char *ip, char *dev) {
	char cmd[150];
	snprintf(cmd,sizeof(cmd),"arping -qb %d -c 1 -w 1 -I %s",atoi(ip),dev);
	mysystem(cmd);
	return 0;
}

void chkfailover(void) {
	FILE *f;
	char buff[150];
	char ipgw[25], gwdev[5], ipfa[25], fadev[5];

	if(file_exist("/var/sys/.load_fa")) {
		if(f=fopen("/var/sys/.load_fa","r")) {
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
	}
}

int main(int argc, char **argv, char **envp) {

	strcpy(argv[0],"mfs-check");

	chkfailover();
	chkpid();

	if(file_exist("/var/run/chkprog")) {
		unlink("/var/run/chkprog");
		exit(1);
	}
	system("echo > /var/run/chkprog");
	if(query==0) {
		system("/service/tools/mfs-query.exc q >/dev/null 2>&1 &");
	}
	if(file_exist("/var/sys/.chk_snmp")) {
		if(snmpd==0) {
			mysystem("/service/init/snmp.init restart");
		}
	}
	if(file_exist("/var/sys/.chk_https")) {
		if(miniserv==0) {
			mysystem("/service/init/https.init restart");
		}
	}
	if(file_exist("/var/sys/.chk_ssh")) {
		if(sshd==0) {
			mysystem("/service/init/ssh.init restart");
		}
	}
	if(file_exist("/var/sys/.chk_syslog")) {
		if(syslogd==0) {
			mysystem("/service/init/syslog.init restart");
		}
	}
	unlink("/var/run/chkprog");
	exit(0);
}
