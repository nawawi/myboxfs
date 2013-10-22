/*
 (C) Copyright 2002,2003,2004 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.
 $Id: login.c,v 1.00 2003/07/28 1:07 AM nawawi Exp $
 $Id: login.c,v 1.10 2004/08/17 10:10 PM nawawi Exp $
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sqlite.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <newt.h>
#include <syslog.h>

#ifndef __WHATTIME_H
#define __WHATTIME_H
extern void print_uptime(void);
extern void echo_uptime(char *ret);
#endif /* __WHATTIME_H */

extern char space[];
extern char *getpass( const char * prompt );
char *crypt(const char *key, const char *salt);
extern int file_exist(char *s);
extern int MD5Password(char *pass, char *pass1);
extern char MD5Values[255];
extern void MDString(char *inString);

static char SQL_result[256];
static char SQL_error[256];
static char tty_login[150];

#define SHELL "/bin/bash"
#define DB "/strg/mybox/db/system.db"
#define LOGDB "/strg/mybox/db/ulog.db"

void log_date(char *ret) {
	char date[13];
	char xmon[3];
	char xsec[3];
	char xmin[3];
	char xhour[3];
	char xday[3];
	struct tm *xtm;
	time_t tm;
	int year, month, day, hour, sec, min;
	time(&tm);
	xtm=localtime(&tm);
	year=xtm->tm_year;
        month=xtm->tm_mon;
        day=xtm->tm_mday;
        hour=xtm->tm_hour;
        sec=xtm->tm_sec;
        min=xtm->tm_min;
	month=month + 1;
        if(month<10) {
                sprintf(xmon,"0%d",month);
        } else {
                sprintf(xmon,"%d",month);
        }
        if(day<10) {
                sprintf(xday,"0%d",day);
        } else {
                sprintf(xday,"%d",day);
        }
        if(hour<10) {
                sprintf(xhour,"0%d",hour);
        } else {
                sprintf(xhour,"%d",hour);
        }
        if(min<10) {
                sprintf(xmin,"0%d",min);
        } else {
                sprintf(xmin,"%d",min);
        }
        if(sec<10) {
                sprintf(xsec,"0%d",sec);
        } else {
                sprintf(xsec,"%d",sec);
        }

        if(year<80) year=year+2000;
        if(year<1900) year=year+1900;
	sprintf(date,"%s/%s/%d %s:%s:%s",xmon,xday,year,xhour,xmin,xsec);
	strcpy(ret,date);
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
	char hostname[255], version[255];
	char build[150];
	clear_screen();
	gethostname(hostname, 255);
	memset(version,0x0,sizeof(version));
	memset(build,0x0,sizeof(build));
	memset(SQL_result,0x0,sizeof(SQL_result));
	if(SQL_query(DB,"select val from mybox_version where name='version'")!=1) {
		snprintf(version,sizeof(version),"%s",SQL_result);
	}
	if(SQL_query(DB,"select val from mybox_version where name='build'")!=1) {
		snprintf(build,sizeof(build),"%s",SQL_result);
	}
	printf("Mybox Firewall");
	if(version[0]!=0) printf(" v%s",version);
	if(build[0]!=0) printf(" (BUILD:%d)",atoi(build));
	printf("\n");
	if(hostname[0]!=0) printf("%s\n",hostname);
	printf("\n");
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

/*
void log_login(char *stat) {
	char date[13], sql[255];
	log_date(date);
	snprintf(sql,sizeof(sql),"insert into log ('ltime','who','host','status') values ('%s','console','%s','%s')",date,tty_login,stat);
	SQL_query(LOGDB,sql);
}

*/

void log_login(char *stat) {
	char msg[256];
	snprintf(msg,sizeof(msg),"AUTH (console@%s) %s",tty_login,stat);
	openlog("MYBOX_LOGIN", LOG_NDELAY|LOG_PID, LOG_NOTICE);
	syslog(LOG_NOTICE,"%s",msg);
	closelog();
}


int password_prompt(void) {
	char passwd[25];
	char *pp=(char *)getpass("Password: ");
	snprintf(passwd,sizeof(passwd),"%s",pp);
	if(passwd[0]==0) return(0);
	if(chk_password(passwd)==0) {
		print_text("\rPassword incorrect",1);
		log_login("Password incorrect");
		return(0);
	}
	return(1);
}

/** Newt Stuff ***/
const struct newtColors awieColorPalette = {
        "white", "black",                       /* root fg, bg */
        "white", "blue",                        /* border fg, bg */
        "cyan", "blue",                         /* window fg, bg */
        "white", "black",                       /* shadow fg, bg */
        "white", "blue",                        /* title fg, bg */
        "black", "cyan",                        /* button fg, bg */
        "white", "cyan",                        /* active button fg, bg */
        "yellow", "black",                      /* checkbox fg, bg */
        "black", "yellow",                      /* active checkbox fg, bg */
        "red", "lightgray",                   /* entry box fg, bg */
        "white", "blue",                        /* label fg, bg */
        "white", "blue",                        /* listbox fg, bg */
        "yellow", "black",                      /* active listbox fg, bg */
        "white", "blue",                        /* textbox fg, bg */
        "red", "black",                        /* active textbox fg, bg */
        "red", "lightgray",                     /* help line */
        "red", "lightgray",                      /* root text */
        "blue",                                 /* scale full */
        "red",                                  /* scale empty */
        "blue", "lightgray",                    /* disabled entry fg, bg */
        "white", "black",                       /* compact button fg, bg */
        "yellow", "cyan",                       /* active & sel listbox */
        "black", "brown"                        /* selected listbox */
};

void errorbox(char *message) {
	newtBell();
	newtWinMessage("ERROR", "OK", message);
}

void get_version(char *ret) {
	char hostname[255], version[255];
	char text[255], build[255];
	clear_screen();
	gethostname(hostname, 255);
	if(SQL_query(DB,"select val from mybox_version where name='version'")!=1) {
		snprintf(version,sizeof(version)," v%s",SQL_result);
	}
	strcpy(text," Mybox Firewall");
	if(version[0]!=0) strcat(text,version);
	
	if(SQL_query(DB,"select val from mybox_version where name='build'")!=1) {
		snprintf(build,sizeof(build)," (BUILD:%s)",SQL_result);
	}
	if(build[0]!=0) strcat(text,build);
	strcpy(ret,text);
}

int gui_password_prompt(void) {
	char title[550],title2[850],uptime[128];
	char hostname[255];
	int prompt=0, ret=1;
        char passwd[25],version[250];

        newtComponent f, l1, entry, b1, b2, b3, timeLabel;
	struct newtExitStruct pl;
        const char *entryValue;

    	char buf[20];
    	const char * spinner = "-\\|/\\|/";
    	const char * spinState;

        newtInit();
        newtCls();
	newtSetColors(awieColorPalette);
	gethostname(hostname, 255);
	echo_uptime(uptime);
	get_version(version);
	snprintf(title2,sizeof(title2),"%s%s",uptime,space);
	snprintf(title,sizeof(title),"%s %s%s",version,hostname,space);
       	newtDrawRootText(0, 0, title);
       	newtDrawRootText(0, -1, title2);
	newtCenteredWindow(37,6,NULL);
	l1 = newtLabel(2, 0, "Enter Password Below: ");
        entry = newtEntry(2, 2, "", 33, &entryValue,NEWT_FLAG_PASSWORD);
        b1 = newtCompactButton(1, 4, " Login ");
        b2 = newtCompactButton(11, 4, " Reboot ");
        b3 = newtCompactButton(22, 4, " Shutdown ");

    	spinState = spinner;
    	timeLabel = newtLabel(24, 0, "-");

        f = newtForm(NULL, NULL, 0);
	newtFormAddComponents(f, l1, entry, b1, b2, b3, NULL);

    	newtFormAddComponents(f, timeLabel, NULL);
    	newtRefresh();
    	newtFormSetTimer(f, 200);

    	do {
		newtFormRun(f, &pl);
		if(pl.reason == NEWT_EXIT_TIMER) {
	    		spinState++;
	    		if(!*spinState) spinState = spinner;
	    		sprintf(buf, "%c", *spinState);
	    		newtLabelSetText(timeLabel, buf);
		}
		echo_uptime(uptime);
		snprintf(title2,sizeof(title2),"%s%s",uptime,space);
		newtDrawRootText(0, -1, title2);
		if(pl.u.co==b1) prompt=1;
		if(pl.u.co==b2) prompt=2;
		if(pl.u.co==b3) prompt=3;

    	} while(prompt==0);
	if(prompt==2) {
		newtCls();
        	newtFormDestroy(f);
        	newtFinished();
		system("reboot");
		exit(1);
	} else if(prompt==3) {
		newtCls();
        	newtFormDestroy(f);
        	newtFinished();
		system("poweroff");
		exit(1);
	}
        sprintf(passwd,"%s",entryValue);
	if(passwd[0]==0) return(0);
        if(chk_password(passwd)==0) {
                errorbox("Password Incorrect");
		log_login("Password incorrect");
		ret=0;
        }
	newtCls();
        newtFormDestroy(f);
        newtFinished();
        return ret;
}

/** end Newt Stuff **/

void handle_ctrlc(int sig) {
}

int main(int argc, char **argv, char **envp) {
	char dv[50], *tty;
	char *rhost;
	
	int prompt=0,login_style=1;
	if(!isatty(0) || !isatty(1) || !isatty(2)) {
		puts("not a terminal");
		exit(0);
	}
	tty=ttyname(0);
	snprintf(dv,sizeof(dv),"%s",tty);
	snprintf(tty_login,sizeof(tty_login),"%s",dv);
	clear_screen();
	login_style=1;
	if(strstr(dv,"/dev/tts") || strstr(dv,"/dev/pts")) {
		login_style=0;
	}
	strcpy(argv[0],"mfs-login");
	(void) signal(SIGINT,handle_ctrlc);
	do {
		clear_screen();
		if(login_style==1) {
			prompt=gui_password_prompt();
		} else {
			do_banner();
			prompt=password_prompt();
		}
	} while(!prompt);
	print_text("\rAccess granted",1);
	log_login("Password Accepted");
	do_banner();
	system(SHELL);
	exit(1);
}