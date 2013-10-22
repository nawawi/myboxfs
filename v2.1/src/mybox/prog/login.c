/*
 (C) Copyright 2002,2003,2004 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.
 $Id: login.c,v 1.00 2003/07/28 1:07 AM nawawi Exp $
 $Id: login.c,v 1.10 2004/08/17 10:10 PM nawawi Exp $
*/

#include <libmybox.h>
#include <syslog.h>

extern char *getpass(const char * prompt );
static char tty_login[150];

void run_shell(void) {
	char *argv_shell[2]={ "-sh", NULL };
	char *envp_shell[8]={ "HOME=/config", "TERM=linux", "PATH=/bin:/service/tools:/service/init", "INPUTRC=/etc/inputrc", "PS1=\\h:\\W> ", "USER=mfs", "SHELL=/bin/sh", NULL };
	chdir("/config");
        execve("/bin/sh", argv_shell, envp_shell);
}

static int chk_password(char *pass) {
	sqlite *db_id=NULL;
	memset(MD5Values,0x0,sizeof(MD5Values));
	db_id=db_connect(DB_NAME);
	if(!db_id) return 0;
	db_query(db_id,"select pass from auth_login where name='console'");
	if(SQL_NUMROWS==0) return 0;
	return MD5Password(pass,SQL_RESULT[0].value);
}


static void log_login(char *stat) {
	char msg[256];
	snprintf(msg,sizeof(msg),"AUTH_CONSOLE %s %s",tty_login,stat);
	openlog("MYBOX_LOGIN", LOG_NDELAY|LOG_PID, LOG_NOTICE);
	syslog(LOG_NOTICE,"%s",msg);
	closelog();
}


static int password_prompt(void) {
	char passwd[25];
	char *pp=(char *)getpass("Password> ");
	snprintf(passwd,sizeof(passwd),"%s",pp);
	if(passwd[0]==0) return(0);
	if(chk_password(passwd)==0) {
		puts("\rPassword incorrect\n");
		log_login("Password incorrect");
		return(0);
        }
        return(1);
}

void do_banner(void) {
	clear_screen();
	print_file("/etc/banner");
}

int login_main(int argc, char **argv) {
	char *tty, **xargv=xmalloc(10 * sizeof(char *)), xtty[50];
	int prompt=0, i=0;
	do_banner();
	(void) signal(SIGINT,SIG_IGN);
	if(file_exists(DB_NAME)==0) {
		puts("Error 100: Database not found");
		sleep(1);
		run_shell();
		_exit(1);
	}
	tty=ttyname(0);
	snprintf(xtty,sizeof(xtty),"%s",tty);
	xargv[0]=xtty;
	xargv[1]=NULL;
	snprintf(tty_login,sizeof(tty_login),"%s",(char *)base_name(xargv));
	free(xargv);
	strcpy(argv[0],"mfs-login");
	if(file_exists("/etc/noconsole")==0) {
		do {
			if(i >= 5) {
				i=0;do_banner();
			}
			prompt=password_prompt();
			i++;
		} while(!prompt);
		puts("\rWelcome!\n");
		log_login("Password Accepted");
	} else {
		log_login("Bypass console password");
		sleep(1);
		unlink("/etc/noconsole");
	}
	(void) signal(SIGINT,SIG_DFL);
	run_shell();
	_exit(1);
}
