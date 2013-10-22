/*
 (C) Copyright 2002,2003,2004 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.
 $Id: login.c,v 1.00 2003/07/28 1:07 AM nawawi Exp $
 $Id: login.c,v 1.10 2004/08/17 10:10 PM nawawi Exp $
*/

#define _XOPEN_SOURCE
#include "libmybox.h"
#include <syslog.h>
#include <pwd.h>

extern char *getpass(const char * prompt );


int ndelay_on(int fd) {
        return fcntl(fd, F_SETFL, fcntl(fd,F_GETFL) | O_NONBLOCK);
}

int ndelay_off(int fd) {
        return fcntl(fd, F_SETFL, fcntl(fd,F_GETFL) & ~O_NONBLOCK);
}


static void alarm_handler(int sig) {
	//ndelay_on(1);
	//ndelay_on(2);
	//printf("\r\nLogin timed out after 60 seconds\r\n");
	fprintf_stdout("Session timed out\n");
	exit(1);
}

void run_shell(void) {
	pid_t pid,status;
	char *argv_shell[2]={ "-sh", NULL };
	char *envp_shell[8]={ "HOME=/config", "TERM=linux", "PATH=/bin:/service/tools:/service/init", "INPUTRC=/etc/inputrc", "PS1=\\h:\\W> ", "USER=mfs", "SHELL=/bin/sh", NULL };
	chdir("/config");
        pid = fork();
        if(pid==0) {
        	execve("/bin/sh", argv_shell, envp_shell);
		exit(0);
	}
	while(wait(&status)!=pid);
}
/*
void run_shell(void) {
	putenv("ENV=/etc/profile");
	chdir("/config");
	system("/bin/sh");
}
*/
void log_login(char *stat) {
	char msg[256];
	snprintf(msg,sizeof(msg),"AUTH_CONSOLE %s %s",tty_login,stat);
	openlog("MYBOX_LOGIN", LOG_NDELAY|LOG_PID, LOG_NOTICE);
	syslog(LOG_NOTICE,"%s",msg);
	closelog();
}


static int password_prompt(void) {
	struct passwd *pw;
	const char *passwd, *passcrypt;
        passwd=(char *)getpass("Password> ");
	fflush(stdout);
        if(passwd[0]==0) return(0);
	pw=getpwnam("console");
        if(!pw || !pw->pw_passwd[0] || pw->pw_passwd[0] == '!' || pw->pw_passwd[0] == '*') {
		puts("\rNo password defined\n");
		log_login("No password defined");
		return(0);
	}
	passcrypt=(char *)crypt(passwd,passwd);
	if(strcmp(pw->pw_passwd,passcrypt)) {
		puts("\rPassword incorrect\n");
		log_login("Password incorrect");
		return(0);
        }
        return(1);
}

void do_banner(void) {
	clear_screen();
	print_banner();
	putchar('\n');
}

int login_main(int argc, char **argv) {
	const char *tmp, *tty;
	int count=0;
	do_banner();
	signal(SIGINT,SIG_IGN);
	//signal(SIGALRM, alarm_handler);
	if(!isatty(0) || !isatty(1) || !isatty(2)) return 1;
	tmp=ttyname(0);
	if(tmp) {
		strncpy(tty_login, tmp, sizeof(tty_login));
		if(strncmp(tty_login, "/dev/", 5) == 0) {
			tty = tty_login + 5;
			strcpy(tty_login,tty);
		}
	}

	strcpy(argv[0],"login               ");
	//alarm(60);
	if(file_exists("/etc/noconsole")==0 && file_exists("/var/sys/do_single")==0) {
		while(1) {
			if(password_prompt()) {
				log_login("Password Accepted");
				break;
			}
			if(count++ == 3) {
				puts("Invalid access password\n");
				log_login("Invalid access password");
				sleep(1);
				return(1);
			}
		}
	} else {
		puts("Console password disabled");
		log_login("Console password disabled");		
		sleep(1);
		unlink("/etc/noconsole");
	}
	//alarm(0);
	//signal(SIGALRM, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	iosh_main(argc,argv);
	exit(0);
}
