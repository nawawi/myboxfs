/*
 (C) Copyright 2002,2003,2004 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.
 $Id: login.c,v 1.00 2003/07/28 1:07 AM nawawi Exp $
 $Id: login.c,v 1.10 2004/08/17 10:10 PM nawawi Exp $
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sqlite.h>
#include <signal.h>
#include <syslog.h>

extern char *getpass( const char * prompt );
extern int MD5Password(char *pass, char *pass1);
extern char MD5Values[255];
extern void MDString(char *inString);

static char tty_login[150];

static char SQL_result[256];
static char SQL_error[256];

//#define SHELL "/service/tools/shell.exc"
#define SHELL "/bin/bash"
#define DB "/strg/mybox/db/system.db"

int file_exist(char *s) {
        struct stat ss;
        int i = stat(s, &ss);
        if (i < 0) return 0;
        if ((ss.st_mode & S_IFREG) || (ss.st_mode & S_IFLNK)) return(1);
        return(0);
}

void clear_screen(void) {
        printf("\033[H\033[J");
}

void rmspace(char *x) {
	char *t;
   	for(t=x+strlen(x)-1; (((*t=='\x20')||(*t=='\x0D')||(*t=='\x0A')||(*t=='\x09'))&&(t >= x)); t--);
   	if(t!=x+strlen(x)-1) *(t+1)=0;
   	for(t=x; (((*t=='\x20')||(*t=='\x0D')||(*t=='\x0A')||(*t=='\x09'))&&(*t)); t++);
   	if(t!=x) strcpy(x,t);
}

int SQL_callback(void *args, int numCols, char **results, char **columnNames) {
        snprintf(SQL_result,sizeof(SQL_result),"%s",results[0]);
        return(1);
}

int SQL_query(char *database,char *sql) {
	int ret=0;
        char **errmsg=NULL;
        sqlite *db=NULL;
	memset(SQL_result,0x0,sizeof(SQL_result));
	memset(SQL_error,0x0,sizeof(SQL_error));
	if(file_exist(database)==0) {
		snprintf(SQL_error,sizeof(SQL_error),"db_error: %s not found",database);
		return(1);
	}
        db=sqlite_open(database, 0, errmsg);
        if(!db) {
		snprintf(SQL_error,sizeof(SQL_error),"SQL_error: %s",errmsg);
                free(errmsg);
                return(1);
        }
        ret=sqlite_exec(db, sql, &SQL_callback, NULL, NULL);
	if(ret==1) {
		snprintf(SQL_error,sizeof(SQL_error),"SQL_error");
	}
        sqlite_close(db);
	return ret;
}

void print_text(char *text, int wait) {
	printf("%s\n",text);
	sleep(wait);
}

void do_banner(void) {
	char hostname[255], buf[150];
	char version[255], version2[255];
	//char build[150];
	clear_screen();
	gethostname(hostname, 255);
	/*
	memset(version,0x0,sizeof(version));
	memset(build,0x0,sizeof(build));
	memset(SQL_result,0x0,sizeof(SQL_result));
	if(SQL_query(DB,"select val from mybox_version where name='version'")!=1) {
		snprintf(version,sizeof(version),"%s",SQL_result);
	}
	if(SQL_query(DB,"select val from mybox_version where name='build'")!=1) {
		snprintf(build,sizeof(build),"%s",SQL_result);
	}
	*/
	FILE *f;
	version[0]=0;
        if((f=popen("uname -v","r"))!=NULL) {
		if(fgets(buf,sizeof(buf),f)!=NULL) {
			sprintf(version,"%s",buf);
		}
		pclose(f);
        }
	rmspace(version);
	version2[0]=0;
	if(file_exist("/etc/firmware")) {
		if((f=fopen("/etc/firmware","r"))!=NULL) {
			if(fgets(buf,sizeof(buf),f)!=NULL) {
				sprintf(version2,"%s",buf);
			}
			fclose(f);
        	}
	}
	rmspace(version2);
	printf("+=============================================================================+\n");
	printf("| MYBOX FIREWALL SYSTEM (c) Tracenetwork Corporation Sdn. Bhd.                |\n");
	printf("|                           http://www.mybox.net.my info@mybox.net.my         |\n");
	printf("+=============================================================================+\n");

	if(version[0]!=0) {
	printf(" %s UPDATE:%s\n",version,version2);
	printf("+-----------------------------------------------------------------------------+\n");
	}

	/*printf("Mybox Firewall System ");
	if(version[0]!=0) {
		printf(" %s UPDATE:%s\n",version,version2);
		printf("==========================================================");
	}*/
	printf("\n\n");
}

int chk_password(char *pass) {
	int ret=0, x=0;
	memset(MD5Values,0x0,sizeof(MD5Values));
	if(SQL_query(DB,"select pass from auth_login where name='console'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	return MD5Password(pass,SQL_result);
}


void log_login(char *stat) {
	char msg[256];
	snprintf(msg,sizeof(msg),"AUTH_CONSOLE %s %s",tty_login,stat);
	openlog("MYBOX_LOGIN", LOG_NDELAY|LOG_PID, LOG_NOTICE);
	syslog(LOG_NOTICE,"%s",msg);
	closelog();
}


int password_prompt(void) {
	char passwd[25];
	char *pp=(char *)getpass("Password> ");
	snprintf(passwd,sizeof(passwd),"%s",pp);
	if(passwd[0]==0) return(0);
	if(chk_password(passwd)==0) {
		print_text("\rPassword incorrect",1);
		log_login("Password incorrect");
		return(0);
	}
	return(1);
}

int main(int argc, char **argv, char **envp) {
	char dv[50], *tty;
	int prompt=0;
	clear_screen();

	tty=ttyname(0);
	snprintf(dv,sizeof(dv),"%s",tty);
	snprintf(tty_login,sizeof(tty_login),"%s",dv);

	strcpy(argv[0],"mfs-login");
	if(file_exist("/etc/noconsole")==0) {
		(void) signal(SIGINT,SIG_IGN);
		do {
			clear_screen();
			do_banner();
			prompt=password_prompt();
		} while(!prompt);
		print_text("\rWelcome!",1);
		log_login("Password Accepted");
	} else {
		log_login("Bypass console password");
		sleep(1);
		unlink("/etc/noconsole");
	}
	//(void) signal(SIGINT,SIG_DFL);
	do_banner();
	system(SHELL);
	_exit(1);
}