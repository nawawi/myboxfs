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

#ifndef __WHATTIME_H
#define __WHATTIME_H
extern void print_uptime(void);
extern void echo_uptime(char *ret);
#endif /* __WHATTIME_H */

extern char space[];
extern char *getpass( const char * prompt );
char *crypt(const char *key, const char *salt);
extern void rmspace(char *x);
extern void splitc(char *first, char *rest, char divider);
extern int file_exist(char *s);
extern int MD5Password(char *pass, char *pass1);
extern char MD5Values[255];
extern void MDString(char *inString);
extern get_network(char *ip, char *mask, char *ret);
extern get_bcast(char *ip, char *mask, char *ret);
extern void get_prefix(char *ip, char *mask, char *ret);

static char SQL_result[256];
static char SQL_error[256];

static char PASSWORD_console[33];
static char PASSWORD_look[33];
static char PASSWORD_admin[33];
static char PASSWORD_ssh[33];

static char DEVICE_wan[5];
static char DEVICE_lan[5];
static char DEVICE_dmz[5];
static char DEVICE_exp[5];

static char NETIP_wan[16];
static char NETMASK_wan[16];
static char NETNET_wan[16];
static char NETBCAST_wan[16];
static char NETPREFIX_wan[3];
static char NETIP_lan[16];
static char NETMASK_lan[16];
static char NETNET_lan[16];
static char NETBCAST_lan[16];
static char NETPREFIX_lan[3];
static char NETIP_dmz[16];
static char NETMASK_dmz[16];
static char NETNET_dmz[16];
static char NETBCAST_dmz[16];
static char NETPREFIX_dmz[3];
static char NETIP_exp[16];
static char NETMASK_exp[16];
static char NETNET_exp[16];
static char NETBCAST_exp[16];
static char NETPREFIX_exp[3];

static int ONBOOT_wan=0;
static int ONBOOT_lan=0;
static int ONBOOT_dmz=0;
static int ONBOOT_exp=0;

static char GENERAL_hostname[150];
static char GENERAL_dns1[150];
static char GENERAL_dns2[150];
static char GENERAL_dns3[150];
static char GENERAL_gateway[16];
static char GENERAL_ntpserver[150];
static int GENERAL_ntptime=5;
static char GENERAL_timezone[150];
static int GENERAL_timegmt;
static char GENERAL_kbdmap[55];
static int GENERAL_login_style=1;
static int SERVICE_httpsport=443;
static int SERVICE_sshport=23;
static int SERVICE_snmpport=161;
static int SERVICE_httpsstat=1;
static int SERVICE_sshstat=0;
static int SERVICE_snmpstat=0;
static char SERVICE_snmpcom[150];

static char tty_login[150];
static int remote_user=0;

#define DB "/strg/mybox/db/system.db"
#define LOGDB "/strg/mybox/db/ulog.db"
#define SHELL "/bin/login_shell"
#define RESET_SCRIPT "/bin/login_reset"
#define GET_TIMEZONE "/usr/share/zoneinfo/ZONELIST.TXT"
#define GET_KBDMAP "/usr/share/kbd/KBDLIST.TXT"
#define NETWORK_RESTART "/etc/init.d/network restart && /etc/init.d/route && /etc/init.d/policy restart && /etc/init.d/ips restart && /etc/init.d/failover restart"

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

        //sprintf(date,"%d%s%s%s%s%s",year,xmon,xday,xhour,xmin,xsec);
	sprintf(date,"%s/%s/%d %s:%s:%s",xmon,xday,year,xhour,xmin,xsec);
	strcpy(ret,date);
}


struct {
	char host[256];
	char note[25];
	int https;
	int ssh;
	int snmp;
	int cnt;
} ACCESS_LIST[101];

int get_accesslist(void) {
	sqlite *db=NULL;
	const char *query_tail=NULL;
	sqlite_vm *vm=NULL;
	char **errmsg=NULL;
	int result=0;
	int numCols=0;
	const char **results=NULL;
	const char **columnNames=NULL;
	int i=0;
	int x=1;
	char query[256];

        db=sqlite_open(DB, 0, errmsg);
        if(!db) {
		snprintf(SQL_error,sizeof(SQL_error),"SQL_error: %s",errmsg);
                free(errmsg);
                return(0);
        }
	snprintf(query,sizeof(query),"%s","select * from accesslist");
	query_tail=query;

	while(query_tail[0]!='\0') {
		if(x >= 101) break;
		result=sqlite_compile(db, query, &query_tail, &vm, errmsg);
		if(result!=SQLITE_OK) {
			snprintf(SQL_error,sizeof(SQL_error),"SQL_error: %s",errmsg);
			free(errmsg);
			return(0);
		}
		result=sqlite_step(vm, &numCols, &results, &columnNames);
		if(result==SQLITE_DONE || result==SQLITE_ROW){
			if(result==SQLITE_ROW){
				sprintf(ACCESS_LIST[x].host,"%s",results[0]);
				ACCESS_LIST[x].https=atoi(results[1]);
				ACCESS_LIST[x].ssh=atoi(results[2]);
				ACCESS_LIST[x].snmp=atoi(results[3]);
				sprintf(ACCESS_LIST[x].note,"%s",results[4]);
				x++;
			}
		}
		while(result==SQLITE_ROW){
			result=sqlite_step(vm, &numCols, &results, &columnNames);
			if(result==SQLITE_ROW){
				sprintf(ACCESS_LIST[x].host,"%s",results[0]);
				ACCESS_LIST[x].https=atoi(results[1]);
				ACCESS_LIST[x].ssh=atoi(results[2]);
				ACCESS_LIST[x].snmp=atoi(results[3]);
				sprintf(ACCESS_LIST[x].note,"%s",results[4]);
				x++;
			}
		}
		result=sqlite_finalize(vm, errmsg);
		if(result!=SQLITE_OK) {
			snprintf(SQL_error,sizeof(SQL_error),"SQL_error: %s",errmsg);
			free(errmsg);
			return(0);
		}
	}
	ACCESS_LIST->cnt=x;
	sqlite_close(db);
	return(1);
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

int load_config(void) {
	// password
	if(SQL_query(DB,"select val from nameval where name='console'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(PASSWORD_console,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='admin'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(PASSWORD_admin,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='look'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(PASSWORD_look,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='ssh'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(PASSWORD_ssh,SQL_result);
	// netdevice
	if(SQL_query(DB,"select val from nameval where name='WAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(DEVICE_wan,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='LAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(DEVICE_lan,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='DMZ'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(DEVICE_dmz,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='EXP'")==1) {
		print_text(SQL_error,1);
		return(1);
	}
	strcpy(DEVICE_exp,SQL_result);
	// netip
	/* start wan */
	if(SQL_query(DB,"select ip from ipaddr where name='WAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETIP_wan,SQL_result);

	if(SQL_query(DB,"select mask from ipaddr where name='WAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETMASK_wan,SQL_result);

	if(SQL_query(DB,"select network from ipaddr where name='WAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETNET_wan,SQL_result);

	if(SQL_query(DB,"select bcast from ipaddr where name='WAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETBCAST_wan,SQL_result);

	if(SQL_query(DB,"select prefix from ipaddr where name='WAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETPREFIX_wan,SQL_result);

	if(SQL_query(DB,"select onboot from ipaddr where name='WAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	ONBOOT_wan=atoi(SQL_result);
	/* end wan */
	/* start lan */
	if(SQL_query(DB,"select ip from ipaddr where name='LAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETIP_lan,SQL_result);

	if(SQL_query(DB,"select mask from ipaddr where name='LAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETMASK_lan,SQL_result);

	if(SQL_query(DB,"select network from ipaddr where name='LAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETNET_lan,SQL_result);

	if(SQL_query(DB,"select bcast from ipaddr where name='LAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETBCAST_lan,SQL_result);

	if(SQL_query(DB,"select prefix from ipaddr where name='LAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETPREFIX_lan,SQL_result);

	if(SQL_query(DB,"select onboot from ipaddr where name='LAN'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	ONBOOT_lan=atoi(SQL_result);
	/* end lan */
	/* start dmz */
	if(SQL_query(DB,"select ip from ipaddr where name='DMZ'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETIP_dmz,SQL_result);

	if(SQL_query(DB,"select mask from ipaddr where name='DMZ'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETMASK_dmz,SQL_result);

	if(SQL_query(DB,"select network from ipaddr where name='DMZ'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETNET_dmz,SQL_result);

	if(SQL_query(DB,"select bcast from ipaddr where name='DMZ'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETBCAST_dmz,SQL_result);

	if(SQL_query(DB,"select prefix from ipaddr where name='DMZ'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETPREFIX_dmz,SQL_result);

	if(SQL_query(DB,"select onboot from ipaddr where name='DMZ'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	ONBOOT_dmz=atoi(SQL_result);
	/* end dmz */
	/* start exp */
	if(SQL_query(DB,"select ip from ipaddr where name='EXP'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETIP_exp,SQL_result);

	if(SQL_query(DB,"select mask from ipaddr where name='EXP'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETMASK_exp,SQL_result);

	if(SQL_query(DB,"select network from ipaddr where name='EXP'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETNET_exp,SQL_result);

	if(SQL_query(DB,"select bcast from ipaddr where name='EXP'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETBCAST_exp,SQL_result);

	if(SQL_query(DB,"select prefix from ipaddr where name='EXP'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(NETPREFIX_exp,SQL_result);

	if(SQL_query(DB,"select onboot from ipaddr where name='EXP'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	ONBOOT_exp=atoi(SQL_result);
	/* end exp */
	// general setting
	if(SQL_query(DB,"select val from nameval where name='hostname'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(GENERAL_hostname,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='dns1'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(GENERAL_dns1,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='dns2'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(GENERAL_dns2,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='dns3'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(GENERAL_dns3,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='gateway'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(GENERAL_gateway,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='ntpserver'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(GENERAL_ntpserver,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='ntptime'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	GENERAL_ntptime=atoi(SQL_result);
	if(SQL_query(DB,"select val from nameval where name='timezone'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(GENERAL_timezone,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='timegmt'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	GENERAL_timegmt=atoi(SQL_result);
	if(SQL_query(DB,"select val from nameval where name='kbdmap'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(GENERAL_kbdmap,SQL_result);
	if(SQL_query(DB,"select val from nameval where name='login_style'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	GENERAL_login_style=atoi(SQL_result);
	// services
	if(SQL_query(DB,"select val from nameval where name='https_port'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	SERVICE_httpsport=atoi(SQL_result);
	if(SQL_query(DB,"select val from nameval where name='https_stat'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	SERVICE_httpsstat=atoi(SQL_result);
	if(SQL_query(DB,"select val from nameval where name='ssh_port'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	SERVICE_sshport=atoi(SQL_result);
	if(SQL_query(DB,"select val from nameval where name='ssh_stat'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	SERVICE_sshstat=atoi(SQL_result);

	if(SQL_query(DB,"select val from nameval where name='snmp_port'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	SERVICE_snmpport=atoi(SQL_result);
	if(SQL_query(DB,"select val from nameval where name='snmp_stat'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	SERVICE_snmpstat=atoi(SQL_result);
	if(SQL_query(DB,"select val from nameval where name='snmp_community'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(SERVICE_snmpcom,SQL_result);
	return(1);
}

void do_banner(void) {
	char hostname[255], version[255];
	char build[150];
	clear_screen();
	gethostname(hostname, 255);
	memset(version,0x0,sizeof(version));
	memset(build,0x0,sizeof(build));
	memset(SQL_result,0x0,sizeof(SQL_result));
	if(SQL_query(DB,"select val from mybox_version where name='mybox_version'")!=1) {
		snprintf(version,sizeof(version),"%s",SQL_result);
	}
	if(SQL_query(DB,"select val from mybox_version where name='mybox_build'")!=1) {
		snprintf(build,sizeof(build),"%s",SQL_result);
	}
	printf("Mybox Firewall");
	if(version[0]!=0) printf(" v%s",version);
	if(build[0]!=0) printf(" (BUILD:%d)",atoi(build));
	printf("\n");
	if(hostname[0]!=0) printf("%s\n",hostname);
	printf("\n");
}

int line_top(char *title) {
	int i;
	do_banner();
	for(i=0;i < 77;i++) {
		printf("=");
	}
	printf("\n %s\n",title);
	for(i=0;i < 77;i++) {
		printf("=");
	}
	printf("\n");
	return(1);
}

int line_bottom(void) {
	int i;
	for(i=0;i < 77;i++) {
		printf("=");
	}
	return(1);
}

void add_space(char *val) {
	int len,x;
	char buf[30], ret[30];
	len=30 - strlen(val);
	strcpy(buf,"");
	for(x=0;x<len;x++) {
		strcat(buf," ");
	}
	snprintf(ret,sizeof(ret),"%s%s",val,buf);
	strcpy(val,ret);
}
int get_input(const char *text, char *value) {
	char *input;
	input=(char *)readline(text);
	strcpy(value,input);
	rmspace(value);
	return(1);
}

int do_shell(int act) {
	char input[2], text[250];
	if(act==0) {
		snprintf(text,sizeof(text),"This action will immediately reboot the system.\nAll services will be re-started.\nDo you want to proceed? [y/N] ");
		get_input(text, input);
		if(input[0]=='y') {
			clear_screen();
			system("reboot");
			exit(1);
		}
		return(1);
	} else if(act==1) {
		snprintf(text,sizeof(text),"This action will immediately shutdown the system.\nAll services will be stopped and the system powered off.\nDo you want to proceed? [y/N] ");
		get_input(text, input);
		if(input[0]=='y') {
			clear_screen();
			system("poweroff");
			exit(1);
		}
		return(1);
	} else if(act==2) {
		snprintf(text,sizeof(text),"This action will reset current firewall configuration to defaults setting.\nDo you want to proceed? [y/N] ");
		get_input(text, input);
		if(input[0]=='y') {
			printf("Please wait..\n");
			system(RESET_SCRIPT);
			print_text("Setting updated!",2);
			return(1);
		}
		return(1);
	} else if(act==3) {
		clear_screen();
		do_banner();
		system(SHELL);
	}
	return(1);
}

int do_password(void) {
	int x=0;
	char input[3], pass[33], db[150];
	char pass1[33];
	for(;;) {
		x=0;
		memset(MD5Values,0x0,sizeof(MD5Values));
		memset(pass,0x0,sizeof(pass));
		memset(pass1,0x0,sizeof(pass1));
		clear_screen();
		line_top("Reset access passwords");
		printf("\n");
		printf(" (1) Console access password\n");
		printf(" (2) Admin access password\n");
		printf(" (3) Look access password\n");
		printf(" (4) SSH access password\n\n");
		printf(" (x) Main menu\n\n");
		line_bottom();
		get_input("\nCommand: ",input);
		if(input[0]=='x' || input[0]=='X') return(0);
		x=atoi(input);
		if(x==1) {
			char *pp=(char *)getpass("Password console: ");
			snprintf(pass,sizeof(pass),"%s",pp);
			if(pass[0]!=0) {
				if(strlen(pass) < 6) {
					print_text("Password must greater than 6 character",1);
				} else {
					snprintf(pass1,sizeof(pass1),"%s",(char *)crypt(pass,pass));
					MDString(pass);
					strcpy(pass,MD5Values);
					if((!strcmp(pass,PASSWORD_admin)) || (!strcmp(pass,PASSWORD_look)) || (!strcmp(pass1,PASSWORD_ssh))) {
						print_text("Password already exist",1);
					} else {
						snprintf(db,sizeof(db),"update nameval set val='%s' where name='console'",pass);
						if(SQL_query(DB,db)==1) {
							print_text(SQL_error,1);
							return(0);
						}
						print_text("Console password updated!",1);
					}
				}
			} else {
				print_text("Invalid input",1);
			}
		} else if(x==2) {
			char *pp=(char *)getpass("Password admin: ");
			snprintf(pass,sizeof(pass),"%s",pp);
			if(pass[0]!=0) {
				if(strlen(pass) < 6) {
					print_text("Password must greater than 6 character",1);
				} else {
					snprintf(pass1,sizeof(pass1),"%s",(char *)crypt(pass,pass));
					MDString(pass);
					strcpy(pass,MD5Values);
					if((!strcmp(pass,PASSWORD_console)) || (!strcmp(pass,PASSWORD_look)) || (!strcmp(pass1,PASSWORD_ssh))) {
						print_text("Password already exist",1);
					} else {
						snprintf(db,sizeof(db),"update nameval set val='%s' where name='admin'",pass);
						if(SQL_query(DB,db)==1) {
							print_text(SQL_error,1);
							return(0);
						}
						print_text("Admin password updated!",1);
					}
				}
			} else {
				print_text("Invalid input",1);
			}
		} else if(x==3) {
			char *pp=(char *)getpass("Password look: ");
			snprintf(pass,sizeof(pass),"%s",pp);
			if(pass[0]!=0) {
				if(strlen(pass) < 6) {
					print_text("Password must greater than 6 character",1);
				} else {
					snprintf(pass1,sizeof(pass1),"%s",(char *)crypt(pass,pass));
					MDString(pass);
					strcpy(pass,MD5Values);
					if((!strcmp(pass,PASSWORD_console)) || (!strcmp(pass,PASSWORD_admin)) || (!strcmp(pass1,PASSWORD_ssh))) {
						print_text("Password already exist",1);
					} else {
						snprintf(db,sizeof(db),"update nameval set val='%s' where name='look'",pass);
						if(SQL_query(DB,db)==1) {
							print_text(SQL_error,1);
							return(0);
						}
						print_text("Look password updated!",1);
					}
				}
			} else {
				print_text("Invalid input",1);
			}
		} else if(x==4) {
			char *pp=(char *)getpass("Password SSH: ");
			snprintf(pass,sizeof(pass),"%s",pp);
			if(pass[0]!=0) {
				if(strlen(pass) < 6) {
					print_text("Password must greater than 6 character",1);
				} else {
					MDString(pass);
					strcpy(pass1,MD5Values);
					if((!strcmp(pass1,PASSWORD_console)) || (!strcmp(pass1,PASSWORD_admin)) || (!strcmp(pass1,PASSWORD_look))) {
						print_text("Password already exist",1);
					} else {
						snprintf(db,sizeof(db),"update nameval set val='%s' where name='ssh'",(char *)crypt(pass,pass));
						if(SQL_query(DB,db)==1) {
							print_text(SQL_error,1);
							return(0);
						}
						printf("Please wait..\n");
						system("/etc/init.d/ssh passwd");
						print_text("SSH password updated!",1);
					}
				}
			} else {
				print_text("Invalid input",1);
			}
		}
		load_config();
	}
	return(1);
}

int set_netdev(char *dev, char *devn) {
	int x=0;
	char buff[250], chk[5];
	char input[3];
	get_input("Set to eth[0-3]: ",input);
	if(input[0]==0) return(0);
	x=atoi(input);
	if(x > 3) {
		print_text("Invalid input",1);
		return(0);
	}
	memset(buff,0x0,sizeof(buff));
	snprintf(buff,sizeof(buff),"select name from nameval where val='eth%d'",x);
	if(SQL_query(DB,buff)==1) {
		print_text(SQL_error,1);
		return(0);
	}
	strcpy(chk,SQL_result);
	memset(buff,0x0,sizeof(buff));
	snprintf(buff,sizeof(buff),"update nameval set val='eth%d' where name='%s'",x,dev);
	if(SQL_query(DB,buff)==1) {
		print_text(SQL_error,1);
		return(0);
	}
	memset(buff,0x0,sizeof(buff));
	snprintf(buff,sizeof(buff),"update nameval set val='%s' where name='%s'",devn,chk);
	if(SQL_query(DB,buff)==1) {
		print_text(SQL_error,1);
		return(0);
	}
	load_config();
	return(1);
}

int do_netdev(void) {
	int x=0;
	char input[3];
	char text[255];
	for(;;) {
		x=0;
		clear_screen();
		line_top("Assign ZONE Interfaces");
		printf("\n");
		printf(" (1) WAN -> %s\n",DEVICE_wan);
		printf(" (2) LAN -> %s\n",DEVICE_lan);
		printf(" (3) DMZ -> %s\n",DEVICE_dmz);
		printf(" (4) EXP -> %s\n\n",DEVICE_exp);
		printf(" (t) Apply setting\n");
		printf(" (x) Main menu\n\n");
		line_bottom();
		get_input("\nCommand: ",input);
		if(input[0]=='t' || input[0]=='T') {
			snprintf(text,sizeof(text),"This action will immediately restart the network. During this action, All connection will be stoppped.\nDo you want to proceed? [y/N] ");
			get_input(text, input);
			if(input[0]=='y') {
				printf("Please wait..\n");
				system(NETWORK_RESTART);
				print_text("Setting Updated!",2);
			}
		}
		if(input[0]=='x' || input[0]=='X') return(0);
		x=atoi(input);
		if(x==1) {
			set_netdev("WAN",DEVICE_wan);
		} else if(x==2) {
			set_netdev("LAN",DEVICE_lan);
		} else if(x==3) {
			set_netdev("DMZ",DEVICE_dmz);
		} else if(x==4) {
			set_netdev("EXP",DEVICE_exp);
		}
	}
	return(1);
}

int set_netip(char *dev, char *devip, char *devmask) {
	char buff[250];
	char input[16], prompt[150];
	char ipaddr[16], ipmask[16];
	char ipnet[16], ipbcast[16], ipprefix[3];

	snprintf(prompt,sizeof(prompt),"Set %s IP address to: ",dev);
	get_input(prompt,input);
	strcpy(ipaddr,input);
	if(ipaddr[0]==0) {
		strcpy(ipaddr,devip);
	}
	snprintf(prompt,sizeof(prompt),"Set %s netmask address to: ",dev);
	get_input(prompt,input);
	strcpy(ipmask,input);
	if(ipmask[0]==0) {
		strcpy(ipmask,devmask);
	}
	
	snprintf(buff,sizeof(buff),"update ipaddr set ip='%s' where name='%s'",ipaddr,dev);
	if(SQL_query(DB,buff)==1) {
		print_text(SQL_error,1);
		return(0);
	}
	snprintf(buff,sizeof(buff),"update ipaddr set mask='%s' where name='%s'",ipmask,dev);
	if(SQL_query(DB,buff)==1) {
		print_text(SQL_error,1);
		return(0);
	}
	get_network(ipaddr,ipmask,ipnet);
	if(ipnet[0]!=0) {
		snprintf(buff,sizeof(buff),"update ipaddr set network='%s' where name='%s'",ipnet,dev);
		if(SQL_query(DB,buff)==1) {
			print_text(SQL_error,1);
			return(0);
		}
	}
	get_bcast(ipaddr,ipmask,ipbcast);
	if(ipbcast[0]!=0) {
		snprintf(buff,sizeof(buff),"update ipaddr set bcast='%s' where name='%s'",ipbcast,dev);
		if(SQL_query(DB,buff)==1) {
			print_text(SQL_error,1);
			return(0);
		}
	}
	get_prefix(ipaddr,ipmask,ipprefix);
	if(ipprefix[0]!=0) {
		snprintf(buff,sizeof(buff),"update ipaddr set prefix='%s' where name='%s'",ipprefix,dev);
		if(SQL_query(DB,buff)==1) {
			print_text(SQL_error,1);
			return(0);
		}
	}
	load_config();
	return(1);
}

int active_net(void) {
	int x=0, yes=0;
	char input[256], prompt[150], buff[256], dev[4];
	snprintf(prompt,sizeof(prompt),"Enter Interface number: ");
	get_input(prompt,input);
	x=atoi(input);
	if(x==1) {
		strcpy(dev,"WAN");
	} else if(x==2) {
		strcpy(dev,"LAN");
	} else if(x==3) {
		strcpy(dev,"DMZ");
	} else if(x==4) {
		strcpy(dev,"EXP");
	} else {
		print_text("No such device",1);
		return(0);
	}
	snprintf(prompt,sizeof(prompt),"Activate Interface [y/N]: ");
	get_input(prompt,input);
	if(input[0]=='y') yes=1;
	snprintf(buff,sizeof(buff),"update ipaddr set onboot='%d' where name='%s'",yes,dev);
	if(SQL_query(DB,buff)==1) {
		print_text(SQL_error,1);
		return(0);
	}
	load_config();
	return(1);
}

int do_netip(void) {
	int x=0;
	char input[3];
	char boot1[4],boot2[4],boot3[4],boot4[4];
	char text[255];
	for(;;) {
		x=0;
		clear_screen();
		line_top("Set up IP addresses");
		printf("\n");
		if(ONBOOT_wan==0) {
			sprintf(boot1,"%s","no");
		} else {
			sprintf(boot1,"%s","yes");
		}
		if(ONBOOT_lan==0) {
			sprintf(boot2,"%s","no");
		} else {
			sprintf(boot2,"%s","yes");
		}
		if(ONBOOT_dmz==0) {
			sprintf(boot3,"%s","no");
		} else {
			sprintf(boot3,"%s","yes");
		}
		if(ONBOOT_exp==0) {
			sprintf(boot4,"%s","no");
		} else {
			sprintf(boot4,"%s","yes");
		}
		printf(" (1) WAN -> %s/%s (active:%s)\n",NETIP_wan,NETMASK_wan,boot1);
		printf(" (2) LAN -> %s/%s (active:%s)\n",NETIP_lan,NETMASK_lan,boot2);
		printf(" (3) DMZ -> %s/%s (active:%s)\n",NETIP_dmz,NETMASK_dmz,boot3);
		printf(" (4) EXP -> %s/%s (active:%s)\n\n",NETIP_exp,NETMASK_exp,boot4);
		printf(" (e) Active (On/Off)\n");
		printf(" (t) Apply setting\n");
		printf(" (x) Main menu\n\n");
		line_bottom();
		get_input("\nCommand: ",input);
		if(input[0]=='x' || input[0]=='X') {
			return(0);
		} else if(input[0]=='t' || input[0]=='T') {
			snprintf(text,sizeof(text),"This action will immediately restart the network. During this action, All connection will be stoppped.\nDo you want to proceed? [y/N] ");
			get_input(text, input);
			if(input[0]=='y') {
				printf("Please wait..\n");
				system(NETWORK_RESTART);
				print_text("Setting Updated!",2);
			}
		} else if(input[0]=='e' || input[0]=='E') {
			active_net();
		}
		x=atoi(input);
		if(x==1) {
			set_netip("WAN",NETIP_wan,NETMASK_wan);
		} else if(x==2) {
			set_netip("LAN",NETIP_lan,NETMASK_lan);
		} else if(x==3) {
			set_netip("DMZ",NETIP_dmz,NETMASK_dmz);
		} else if(x==4) {
			set_netip("EXP",NETIP_exp,NETMASK_exp);
		} 
	}
	return(1);
}

int add_acl(void) {
	char input[256], prompt[150], buff[256];
	char host[256];
	int https=0,ssh=0,snmp=0;
	snprintf(prompt,sizeof(prompt),"Enter new host/ip: ");
	get_input(prompt,input);
	if(input[0]==0) {
		print_text("Invalid input",1);
		return(0);
	}
	strcpy(host,input);
	snprintf(prompt,sizeof(prompt),"Allow https access [y/N]: ");
	get_input(prompt,input);
	if(input[0]=='y' || input[0]=='Y') https=1;
	snprintf(prompt,sizeof(prompt),"Allow ssh access [y/N]: ");
	get_input(prompt,input);
	if(input[0]=='y' || input[0]=='Y') ssh=1;
	snprintf(prompt,sizeof(prompt),"Allow snmp query [y/N]: ");
	get_input(prompt,input);
	if(input[0]=='y' || input[0]=='Y') snmp=1;

	snprintf(buff,sizeof(buff),"insert into accesslist ('ip','ssh','https','snmp') values ('%s','%d','%d','%d')",host,ssh,https,snmp);
	if(SQL_query(DB,buff)==1) {
		print_text(SQL_error,1);
		return(0);
	}
	return(1);
}

int edit_acl(void) {
	int host=0;
	int https=0,ssh=0,snmp=0;
	char input[256], prompt[150], buff[256];

	snprintf(prompt,sizeof(prompt),"Enter host/ip number: ");
	get_input(prompt,input);
	host=atoi(input);
	snprintf(prompt,sizeof(prompt),"Allow https access [y/N]: ");
	get_input(prompt,input);
	if(input[0]=='y' || input[0]=='Y') https=1;
	snprintf(prompt,sizeof(prompt),"Allow ssh access [y/N]: ");
	get_input(prompt,input);
	if(input[0]=='y' || input[0]=='Y') ssh=1;
	snprintf(prompt,sizeof(prompt),"Allow snmp query [y/N]: ");
	get_input(prompt,input);
	if(input[0]=='y' || input[0]=='Y') snmp=1;
	snprintf(buff,sizeof(buff),"update accesslist set ssh='%d',https='%d',snmp='%d' where ip='%s'",ssh,https,snmp,ACCESS_LIST[host].host);
	if(SQL_query(DB,buff)==1) {
		print_text(SQL_error,1);
		return(0);
	}
	return(1);
}

int del_acl(void) {
	int x=0;
	int https=0,ssh=0;
	char input[256], prompt[150], buff[256], host[256];
	snprintf(prompt,sizeof(prompt),"Enter host/ip number: ");
	get_input(prompt,input);
	if(input[0]!=0) {
		x=atoi(input);
		snprintf(buff,sizeof(buff),"delete from accesslist where ip='%s'",ACCESS_LIST[x].host);
		if(SQL_query(DB,buff)==1) {
			print_text(SQL_error,1);
			return(0);
		}
	}
	return(1);
}

int do_acl(void) {
	int x=0,i=1;
	char input[3], list[256], prog[150], host[30];
	for(;;) {
		clear_screen();
		line_top("Set up access control list");
		printf("\n");
		if(get_accesslist()==0) {
			print_text(SQL_error,1);
			return(0);
		}
		for(i=1;i<ACCESS_LIST->cnt;i++) {
			memset(prog,0x0,sizeof(prog));
			memset(host,0x0,sizeof(host));
			strcpy(prog,"");
			if(ACCESS_LIST[i].https==1){
				strcat(prog,"HTTPS");
			}
			if(ACCESS_LIST[i].ssh==1){
				strcat(prog,":SSH");
			} 
			if(ACCESS_LIST[i].snmp==1){
				strcat(prog,":SNMP");
			}
			rmspace(prog);
			if(prog[0]==0) strcpy(prog,"none");
			strcpy(host,ACCESS_LIST[i].host);
			add_space(host);
			printf(" [%d] %s -> (%s) (%s)\n",i,host,prog,ACCESS_LIST[i].note);
		}
		if(i==1) {
			printf(" Access list empty\n");
		}
		printf("\n");
		printf(" (a) Add new list\n");
		printf(" (d) Delete list\n");
		printf(" (e) Enable/Disable service\n");
		printf(" (t) Apply setting\n");
		printf(" (x) Main menu\n\n");
		line_bottom();
		get_input("\nCommand: ",input);
		if(input[0]=='x' || input[0]=='X') {
			return(0);
		} else if(input[0]=='a' || input[0]=='A') {
			add_acl();
		} else if(input[0]=='d' || input[0]=='D') {
			del_acl();
		} else if(input[0]=='t' || input[0]=='t') {
			system("/etc/init.d/policy service");
			print_text("Setting Updated!",2);
		} else if(input[0]=='e' || input[0]=='e') {
			edit_acl();
		}
	}
	return(1);
}

void addtab(char *txt, int len) {
        int i=0,x=0;
        x=len - strlen(txt);
        for(i=0;i<x;i++) {
		strcat(txt," ");
        }
}

struct {
	char name[150];
	int cnt;
} ZONE_LIST[600];

int scan_timezone(void) {
        FILE *f;
        char buf[255];
        int x=1;
        f=fopen(GET_TIMEZONE, "r");
        if(!f) {
                print_text("cannot scan zoneinfo",1);
                return(0);
        }
        while(fgets(buf, sizeof(buf) - 1, f)) {
                rmspace(buf);
		if(x <= 600) {
                	sprintf(ZONE_LIST[x].name,"%s",buf);
                	x++;
		}
        }
        ZONE_LIST->cnt=x;
        fclose(f);
	return(1);
}

int list_timezone(void) {
	FILE *f;
        int i=1,x=0;
	char input[3];
        if(scan_timezone()==0) return(0);
	f=fopen("/tmp/.zoneinfolist","w");
	if(!f) {
                print_text("error creating zone info list",1);
                return(0);
	}
        for(i=1;i<ZONE_LIST->cnt;i++) {
                addtab(ZONE_LIST[i].name,30);
                if(i<10) {
                        fprintf(f,"(  %d) %s",i,ZONE_LIST[i].name);
                } else if(i<100) {
                        fprintf(f,"( %d) %s",i,ZONE_LIST[i].name);
                } else {
                        fprintf(f,"(%d) %s",i,ZONE_LIST[i].name);
                }
                x++;
                if(x==2) {
                        fprintf(f,"\n");
                        x=0;
                }
        }
	fprintf(f,"\n");
	fclose(f);
	if(file_exist("/tmp/.zoneinfolist")==0) {
                print_text("error creating zone info list",1);
                return(0);
	}
	system("more /tmp/.zoneinfolist");
	unlink("/tmp/.zoneinfolist");
        return(1);	
}

struct {
	char name[150];
	int cnt;
} KBD_LIST[600];

int scan_kbdmap(void) {
        FILE *f;
        char buf[255];
        int x=1;
        f=fopen(GET_KBDMAP, "r");
        if(!f) {
                print_text("cannot scan keymap",1);
                return(0);
        }
        while(fgets(buf, sizeof(buf) - 1, f)) {
                rmspace(buf);
		if(x <= 600) {
                	sprintf(KBD_LIST[x].name,"%s",buf);
                	x++;
		}
        }
        KBD_LIST->cnt=x;
        fclose(f);
	return(1);
}

int list_kbdmap(void) {
	FILE *f;
        int i=1,x=0;
	char input[3];
        if(scan_kbdmap()==0) return(0);
	f=fopen("/tmp/.kbdlist","w");
	if(!f) {
		print_text("error creating keymap list",1);
		return(0);
	}
        for(i=1;i<KBD_LIST->cnt;i++) {
                addtab(KBD_LIST[i].name,30);
                if(i<10) {
                        fprintf(f,"(  %d) %s",i,KBD_LIST[i].name);
                } else if(i<100) {
                        fprintf(f,"( %d) %s",i,KBD_LIST[i].name);
                } else {
                        fprintf(f,"(%d) %s",i,KBD_LIST[i].name);
                }
                x++;
                if(x==2) {
                        fprintf(f,"\n");
                        x=0;
                }
        }
	fprintf(f,"\n");
	fclose(f);
	if(file_exist("/tmp/.kbdlist")==0) {
		print_text("error creating keymap list",1);
		return(0);
	}
	system("more /tmp/.kbdlist");
	unlink("/tmp/.kbdlist");
        return(1);	
}

int set_general(int opt) {
	int x=0,ct=0,cn=0;
	char input[256];
	char ret[256],buff[256];
	int gmt=0, ntptime;
	if(opt==1) {
		get_input("Set Hostame: ",input);
		if(input[0]!=0) {
			snprintf(buff,sizeof(buff),"update nameval set val='%s' where name='hostname'",input);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
	} else if(opt==2) {
		get_input("Set DNS1: ",input);
		if(input[0]!=0) {
			snprintf(buff,sizeof(buff),"update nameval set val='%s' where name='dns1'",input);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
		get_input("Set DNS2: ",input);
		if(input[0]!=0) {
			snprintf(buff,sizeof(buff),"update nameval set val='%s' where name='dns2'",input);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
		get_input("Set DNS1: ",input);
		if(input[0]!=0) {
			snprintf(buff,sizeof(buff),"update nameval set val='%s' where name='dns3'",input);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
	} else if(opt==3) {
		get_input("Set Gateway: ",input);
		if(input[0]!=0) {
			snprintf(buff,sizeof(buff),"update nameval set val='%s' where name='gateway'",input);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
	} else if(opt==4) {
		get_input("Set NTP server host/ip: ",input);
		if(input[0]!=0) {
			snprintf(buff,sizeof(buff),"update nameval set val='%s' where name='ntpserver'",input);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
			memset(GENERAL_ntpserver,0x0,sizeof(GENERAL_ntpserver));
			strcpy(GENERAL_ntpserver,input);
		}
		get_input("Set NTP query interval in minute(s): ",input);
		if(input[0]!=0) {
			ntptime=atoi(input);
			ct=ntptime;
			if(ct >= 44640) {
				ct=ct / 44640;
				if(ct > 12) {
					print_text("Invalid input!",1);
					return(0);
				}
			} else if(ct >= 1440) {
				ct=ct / 1440;
				if(ct > 31) {
					print_text("Invalid input!",1);
					return(0);
				}
			} else if(ct >= 60) {
				ct=ct / 60;
				if(ct > 23) {
					print_text("Invalid input!",1);
					return(0);
				}
			}
			snprintf(buff,sizeof(buff),"update nameval set val='%d' where name='ntptime'",ntptime);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
			GENERAL_ntptime=ntptime;
		}
		memset(buff,0x0,sizeof(buff));
		// 60 = minute = 1 hour
		// 1440 = minute = 24 hour = 1 day
		// 44640 = minute = 31 day = 1 month
		ct=GENERAL_ntptime;
		if(ct!=0) {
			if(ct >= 44640) {
				ct=ct / 44640;
				if(ct > 12) {
					print_text("Invalid input!",1);
					return(0);
				}
				snprintf(buff,sizeof(buff),"update crontab set val='* * * */%d * /bin/ntpdate %s' where name='ntp'",ct,GENERAL_ntpserver);
			} else if(ct >= 1440) {
				ct=ct / 1440;
				if(ct > 31) {
					print_text("Invalid input!",1);
					return(0);
				}
				snprintf(buff,sizeof(buff),"update crontab set val='* * */%d * * /bin/ntpdate %s' where name='ntp'",ct,GENERAL_ntpserver);
			} else if(ct >= 60) {
				ct=ct / 60;
				if(ct > 23) {
					print_text("Invalid input!",1);
					return(0);
				}
				snprintf(buff,sizeof(buff),"update crontab set val='* */%d * * * /bin/ntpdate %s' where name='ntp'",ct,GENERAL_ntpserver);
			} else {
				snprintf(buff,sizeof(buff),"update crontab set val='*/%d * * * * /bin/ntpdate %s' where name='ntp'",GENERAL_ntptime,GENERAL_ntpserver);
			}
		} else {
			snprintf(buff,sizeof(buff),"update crontab set val='# ntp disabled' where name='ntp'");
		}
		if(SQL_query(DB,buff)==1) {
			print_text(SQL_error,1);
			return(0);
		}
	} else if(opt==5) {
		if(list_timezone()==0) return(0);
		get_input("Set Time zone from list number: ",input);
		if(input[0]!=0) {
			x=atoi(input);
			snprintf(buff,sizeof(buff),"update nameval set val='%s' where name='timezone'",ZONE_LIST[x].name);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
		get_input("Hardware clock set to GMT [y/N]: ",input);
		if(input[0]!=0) {
			if(input[0]=='y' || input[0]=='Y') gmt=1;
			snprintf(buff,sizeof(buff),"update nameval set val='%d' where name='timegmt'",gmt);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
	} else if(opt==6) {
		if(list_kbdmap()==0) return(0);
		get_input("Set keymap from list number: ",input);
		if(input[0]!=0) {
			x=atoi(input);
			snprintf(buff,sizeof(buff),"update nameval set val='%s' where name='kbdmap'",KBD_LIST[x].name);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
	} else if(opt==7) {
		get_input(" (1) Gui Login\n (2) Console Login \n Select [1/2]: ",input);
		if(input[0]!=0) {
			x=atoi(input);
			snprintf(buff,sizeof(buff),"update nameval set val='%s' where name='login_style'",input);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
	}
	load_config();
	return(1);
}
int do_general(void) {
	int x=0,i=1,ct;
	char input[3], gmt[4], login[25], ntptime[150];
	for(;;) {
		if(GENERAL_timegmt==1) {
			snprintf(gmt,sizeof(gmt),"yes");
		} else {
			snprintf(gmt,sizeof(gmt),"no");
		}
		if(GENERAL_login_style==1) {
			snprintf(login,sizeof(login),"Gui");
		} else {
			snprintf(login,sizeof(login),"Console");
		}
		ct=GENERAL_ntptime;
		if(ct==0) {
			snprintf(ntptime,sizeof(ntptime)," %s","(Disabled)");
		} else if(ct >= 44640) {
			ct=ct / 44640;
			snprintf(ntptime,sizeof(ntptime),"%d month(s)",ct);
		} else if(ct >= 1440) {
			ct=ct / 1440;
			snprintf(ntptime,sizeof(ntptime),"%d day(s)",ct);
		} else if(ct >= 60) {
			ct=ct / 60;
			snprintf(ntptime,sizeof(ntptime),"%d hour(s)",ct);
		} 
		clear_screen();
		line_top("General Settings");
		printf("\n");
		printf(" (1) Hostname\t\t: %s\n",GENERAL_hostname);
		printf(" (2) DNS\t\t: %s\n",GENERAL_dns1);
		if(GENERAL_dns2[0]!=0) {
		printf("     Secondary DNS\t: %s\n",GENERAL_dns2);
		}
		if(GENERAL_dns3[0]!=0) {
		printf("     Tertiary DNS\t: %s\n",GENERAL_dns3);
		}
		printf(" (3) Gateway\t\t: %s\n",GENERAL_gateway);
		printf(" (4) NTP server\t\t: %s\n",GENERAL_ntpserver);
		printf("     Query interval\t: %d minute(s) %s\n",GENERAL_ntptime,ntptime);
		printf(" (5) Timeconfig\t\t: %s (gmt=%s)\n",GENERAL_timezone,gmt);
		printf(" (6) Keymap\t\t: %s\n",GENERAL_kbdmap);
		printf(" (7) Login style\t: %s\n",login);
		printf("\n");
		printf(" (t) Apply setting\n");
		printf(" (x) Main menu\n\n");
		line_bottom();
		get_input("\nCommand: ",input);
		if(input[0]=='t' || input[0]=='T') {
			printf("Please wait..\n");
			system("/etc/init.d/cron restart && /etc/init.d/network set_general && /etc/init.d/clock && /etc/init.d/keymap");
			print_text("Setting Updated!",2);
		}
		if(input[0]=='x' || input[0]=='X') return(0);
		x=atoi(input);
		if(x>=1) {
			set_general(x);
		}
	}
	return(1);
}

int set_services(int s) {
	char input[101], buff[256];
	int stat=0;
	if(s==1) {
		get_input("Set HTTPS port: ",input);
		if(input[0]!=0) {
			snprintf(buff,sizeof(buff),"update nameval set val='%d' where name='https_port'",atoi(input));
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
		get_input("Enabled HTTPS service [y/N]: ",input);
		if(input[0]!=0) {
			if(input[0]=='y' || input[0]=='Y') stat=1;
			snprintf(buff,sizeof(buff),"update nameval set val='%d' where name='https_stat'",stat);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
	} else if(s==2) {
		get_input("Set SSH port: ",input);
		if(input[0]!=0) {
			snprintf(buff,sizeof(buff),"update nameval set val='%d' where name='ssh_port'",atoi(input));
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
		get_input("Enabled SSH service [y/N]: ",input);
		if(input[0]!=0) {
			if(input[0]=='y' || input[0]=='Y') stat=1;
			snprintf(buff,sizeof(buff),"update nameval set val='%d' where name='ssh_stat'",stat);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
	} else if(s==3) {
		get_input("Set SNMP port: ",input);
		if(input[0]!=0) {
			snprintf(buff,sizeof(buff),"update nameval set val='%d' where name='snmp_port'",atoi(input));
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
		get_input("Set SNMP community string: ",input);
		if(input[0]!=0) {
			snprintf(buff,sizeof(buff),"update nameval set val='%s' where name='snmp_community'",input);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
		get_input("Enabled SNMP service [y/N]: ",input);
		if(input[0]!=0) {
			if(input[0]=='y' || input[0]=='Y') stat=1;
			snprintf(buff,sizeof(buff),"update nameval set val='%d' where name='snmp_stat'",stat);
			if(SQL_query(DB,buff)==1) {
				print_text(SQL_error,1);
				return(0);
			}
		}
	}
	load_config();
	return(1);
}

int do_services(void) {
	int x=0;
	char input[3],stat1[4],stat2[4],stat3[4];
	for(;;) {
		clear_screen();
		line_top("System services");
		printf("\n");
		if(SERVICE_httpsstat==1) {
			strcpy(stat1,"On");
		} else {
			strcpy(stat1,"Off");
		}
		if(SERVICE_sshstat==1) {
			strcpy(stat2,"On");
		} else {
			strcpy(stat2,"Off");
		}
		if(SERVICE_snmpstat==1) {
			strcpy(stat3,"On");
		} else {
			strcpy(stat3,"Off");
		}
		printf(" (1) HTTPS\t-> Port listen\t: %d\t(%s)\n",SERVICE_httpsport,stat1);
		printf(" (2) SSH\t-> Port listen\t: %d\t(%s)\n",SERVICE_sshport,stat2);
		printf(" (3) SNMP\t-> Port listen\t: %d\t(%s)\n",SERVICE_snmpport,stat3);
		printf("         \t   Community\t: %s\n\n",SERVICE_snmpcom);
		printf(" (t) Apply Setting\n");
		printf(" (x) Main menu\n\n");
		line_bottom();
		get_input("\nCommand: ",input);
		if(input[0]=='t' || input[0]=='T') {
			printf("Please wait..\n");
			system("/etc/init.d/https reload && /etc/init.d/ssh restart && /etc/init.d/snmp restart");
			print_text("Setting Updated!",2);
		}
		if(input[0]=='x' || input[0]=='X') return(0);
		x=atoi(input);
		if(x >= 1) {
			set_services(x);
		}
	}
	return(1);
}

int print_menu(void) {
	int x=0;
	char input[3];
	for(;;) {
		clear_screen();
		line_top("Firewall Setup");
		printf("\n");
		printf(" (1) Assign ZONE Interfaces\n");
		printf(" (2) Set up IP addresses\n");
		printf(" (3) Set up access control list\n");
		printf(" (4) Reset access passwords\n");
		printf(" (5) Reset to factory defaults\n");
		printf(" (6) General settings\n");
		printf(" (7) System services\n\n");
		printf(" (8) Reboot\n");
		printf(" (9) Shutdown\n\n");
		printf(" (s) Shell\n");
		printf(" (x) Logout\n\n");
		line_bottom();
		get_input("\nCommand: ",input);
		if(input[0]=='x' || input[0]=='X') return(0);
		if(input[0]=='s' || input[0]=='S') do_shell(3);
		x=atoi(input);
		if(x==1) {
			do_netdev();
		} else if(x==2) {
			do_netip();
		} else if(x==3) {
			do_acl();
		} else if(x==4) {
			do_password();
		} else if(x==5) {
			do_shell(2);
		} else if(x==6) {
			do_general();
		} else if(x==7) {
			do_services();
		} else if(x==8) {
			do_shell(0);
		} else if(x==9) {
			do_shell(1);
		}
	}
	return(1);
}

int chk_password(char *pass) {
	int ret=0, x=0;
	memset(MD5Values,0x0,sizeof(MD5Values));
	if(SQL_query(DB,"select val from nameval where name='console'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	return MD5Password(pass,SQL_result);
}

void log_login(char *stat) {
	char date[13], sql[255];
	log_date(date);
	snprintf(sql,sizeof(sql),"insert into log ('ltime','who','host','status') values ('%s','console','%s','%s')",date,tty_login,stat);
	SQL_query(LOGDB,sql);
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
	if(SQL_query(DB,"select val from mybox_version where name='mybox_version'")!=1) {
		snprintf(version,sizeof(version)," v%s",SQL_result);
	}
	strcpy(text," Mybox Firewall");
	if(version[0]!=0) strcat(text,version);
	
	if(SQL_query(DB,"select val from mybox_version where name='mybox_build'")!=1) {
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
		system("shutdown");
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
	(void) signal(SIGINT,handle_ctrlc);
	if(SQL_query(DB,"select val from nameval where name='login_style'")==1) {
		print_text(SQL_error,1);
		return(0);
	}
	login_style=atoi(SQL_result);
	do {
		clear_screen();
		if((strstr(dv,"/dev/vc") || strstr(dv,"/dev/tty")) && (login_style==1)) {
			prompt=gui_password_prompt();
		} else {
			if(strstr(dv,"/dev/pts")) remote_user=1;
			do_banner();
			prompt=password_prompt();
		}
	} while(!prompt);
	print_text("\rAccess granted",1);
	log_login("Access granted");
	load_config();
	print_menu();
	exit(1);
}
