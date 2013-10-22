/*
 (C) Copyright 2002,2003,2004 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.
 $Id: login.c,v 1.00 2003/07/28 1:07 AM nawawi Exp $
 $Id: login.c,v 1.10 2004/08/17 10:10 PM nawawi Exp $
*/

#define _XOPEN_SOURCE
#include "libmybox.h"
#include <pwd.h>

extern int snprintf(char *str, size_t size, const char *format, ...);
extern char *getpass(const char * prompt );

void run_shell(void) {
	pid_t pid,status;
	const char *gt;
	char TERM[50], SSH_CLIENT[150], SSH_CONNECTION[150], SSH_TTY[50], ANYTERM_SHELL[17];
	if(getenv("TERM")!=NULL) {
		snprintf(TERM,sizeof(TERM),"TERM=%s",getenv("TERM"));
	} else {
		snprintf(TERM,sizeof(TERM),"TERM=%s","linux");
	}
	if(getenv("ANYTERM_SHELL")!=NULL) {
		snprintf(ANYTERM_SHELL,sizeof(ANYTERM_SHELL),"ANYTERM_SHELL=%s",getenv("ANYTERM_SHELL"));
	} else {
		snprintf(ANYTERM_SHELL,sizeof(ANYTERM_SHELL),"ANYTERM_SHELL=%d",0);
	}
	snprintf(SSH_CLIENT,sizeof(SSH_CLIENT),"SSH_CLIENT=%s",getenv("SSH_CLIENT"));
	snprintf(SSH_CONNECTION,sizeof(SSH_CONNECTION),"SSH_CONNECTION=%s",getenv("SSH_CONNECTION"));
	snprintf(SSH_TTY,sizeof(SSH_TTY),"SSH_TTY=%s",getenv("SSH_TTY"));
	char *argv_shell[2]={ "-sh", NULL };
	char *envp_shell[11]={ "HOME=/config", TERM, "PATH=/bin:/service/tools:/service/init", "INPUTRC=/etc/inputrc", "PS1=\\h:\\W> ", "USER=sys", "SHELL=/bin/sh", SSH_CLIENT, SSH_CONNECTION, SSH_TTY, NULL };
	chdir("/config");
        pid = fork();
        if(pid==0) {
        	execve("/bin/sh", argv_shell, envp_shell);
		exit(0);
	}
	while(wait(&status)!=pid);
}

void log_login(char *ecode,char *stat) {
	char msg[256];
	if(getenv("ANYTERM_SHELL")!=NULL) {
		snprintf(msg,sizeof(msg),"ECODE=%s TYPE=console USER=console IP=%s MSG=(webshell) %s",ecode,tty_login,stat);
	} else {
		snprintf(msg,sizeof(msg),"ECODE=%s TYPE=console USER=console IP=%s MSG=%s",ecode,tty_login,stat);
	}
	openlog("LOGIN", LOG_NDELAY|LOG_PID, LOG_AUTHPRIV);
	syslog(LOG_INFO,"%s",msg);
	closelog();
}


int password_prompt(void) {
	struct passwd *pw;
	const char *passwd, *passcrypt;
	char *salt, passprompt[150];
	snprintf(passprompt,sizeof(passprompt),"%s> ",gettext("Password"));
        passwd=(char *)getpass(passprompt);
	fflush(stdout);
        if(passwd[0]==0) return(0);
	pw=getpwnam("console");
        if(!pw || !pw->pw_passwd[0] || pw->pw_passwd[0] == '!' || pw->pw_passwd[0] == '*') {
		printf("\r%s\n\n",gettext("No password defined"));
		log_login("01",gettext("No password defined"));
		return(0);
	}
	salt=substr(pw->pw_passwd,0,12);	
	passcrypt=(char *)crypt(passwd,salt);
	if(strcmp(pw->pw_passwd,passcrypt)) {
		printf("\r%s\n\n",gettext("Password incorrect"));
		log_login("01",gettext("Password incorrect"));
		return(0);
        }
        return(1);
}

void clear_screen(void) {
        printf("\033[H\033[J");
}

void do_banner(void) {
	clear_screen();
	print_banner();
	putchar('\n');
}

int login_main(int argc, char **argv) {
	const char *tmp, *tty;
	int count=0;
	init_lang();
	do_banner();
	signal(SIGINT,SIG_IGN);
	if(!isatty(0) || !isatty(1) || !isatty(2)) return 1;
	tmp=ttyname(0);
	if(tmp) {
		strncpy(tty_login, tmp, sizeof(tty_login));
		if(strncmp(tty_login, "/dev/", 5) == 0) {
			tty = tty_login + 5;
			strcpy(tty_login,tty);
		}
		if(strncmp(tty_login, "pts/", 4) == 0) {
			tty=str_replace(tty_login,"pts/","pts");
			strcpy(tty_login,tty);
		}
	}

	strcpy(argv[0],"login               ");
	if(file_exists("/etc/noconsole")==0 && file_exists("/var/sys/do_single")==0) {
		while(1) {
			if(password_prompt()) {
				log_login("00",gettext("Password Accepted"));
				break;
			}
			if(count++ == 3) {
				printf("%s\n\n",gettext("Invalid access password"));
				log_login("01",gettext("Invalid access password"));
				sleep(1);
				return(1);
			}
		}
	} else {
		printf("%s\n",gettext("Console password disabled"));
		log_login("00",gettext("Console password disabled"));	
		sleep(1);
		unlink("/etc/noconsole");
	}
	signal(SIGINT, SIG_DFL);
	iosh_main(argc,argv);
	exit(0);
}
