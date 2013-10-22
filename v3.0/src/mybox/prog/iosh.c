/*
 (C) Copyright 2006-2007 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.
 $Id: iosh.c,v 1.4 2007/10/27 01:56:10 nawawi Exp $
*/

#include "libmybox.h"
#include <readline/readline.h>
#include <readline/history.h>

#define HELPID (17)
static const char * const CMD_HELPER="/service/tools/mfs-iosh.exc";
static const char * const ROOT_DIR="/config";
static char host[40];
static char msgconfirm[100];
static int done=0;
static char *str_input;

static int cmd_php_exec(char *line);
static int cmd_history(char *line);
static int cmd_version(void);
static int cmd_print_memory(void);
static int cmd_print_cpu(void);
static int cmd_exec(char *line);
static int cmd_print(char *line);
static int cmd_session(char *line);
static int cmd_update(char *line);
static int cmd_acl(char *line);
static int cmd_exit(void);
static int cmd_shutdown(void);
static int cmd_reboot(void);
static int cmd_addhistory(char *line);
static int print_error(char *str);
static int show_help(void);
static int show_help_acl(void);
static int show_help_exec(void);
static int show_help_print(void);
static int show_help_session(void);
static int show_help_update(void);

typedef struct {
	char *name;
	rl_icpfunc_t *func; 
	char *doc;
} CLI_COMMAND;

CLI_COMMAND commands[] = {
	{"acl",cmd_acl,"User IP Address access list"},
	{"exec",cmd_exec,"Execute tools"},
	{"reboot",cmd_reboot,"Reboot the system"},
	{"shutdown",cmd_shutdown,"Shutdown the system"},
	{"history",cmd_history,"Display the session command history"},
	{"version",cmd_version,"System hardware and software status"},
	{"print",cmd_print,"Display system info"},
	{"session",cmd_session,"Display current session"},
	{"update",cmd_update,"System update"},
	{"help",show_help,"Show this help"},
	{"exit",cmd_exit,"Exit from session"},
	{ (char *)NULL,(char *)NULL }
};

typedef struct {
	char *name;
	char *doc;
	char *doc2;
} CLI_COMMAND_SUB;

CLI_COMMAND_SUB commands_acl[] = {
	{"print","Show current configuration",NULL},
	{"add","Add new new accesslist ip","ip=<ip> [https=<on/off> ssh=<on/off> ftp=<on/off>] [all=<on/off>] <enable/disable> note=<note>|\"note...note\""},
	{"del","Delete accesslist ip","[<ip1> <ip2> ...] all"},
	{"edit","Edit accesslist ip","ip=<ip> [https=<on/off> ssh=<on/off> ftp=<on/off>] [all=<on/off>] <enable/disable> note=<note>|\"note...note\""},
	{"passwd","Access password","type=<admin|look|console|ssh|ftp> passwd=<password>"},
	{"reload","Activate configuration",NULL},
	{ (char *)NULL,(char *)NULL,(char *)NULL }
};

CLI_COMMAND_SUB commands_exec[] = {
	{"ping","send ICMP ping","<ip/host> [cnt]"},
	{"telnet","TELNET user interface","<ip/host> [port]"},
	{"nslookup","Name server lookup","<host> [name_server]"},
	{"traceroute","Display route packets to network host","<host>"},
	{"ssh","SSH user interface","<ip/host> <user> [port]"},
	{"ftp","File transfer program","<put/get> <ip/host> <user> <file> [port]"},
	{"scpget","Retrieve a remote file via SCP","<ip/host> <user> <remote_file> [port]"},
	{"scpput","Store a local file on a remote machine via SCP","<ip/host> <user> <source_file> [port]"},
	{"edit","Edit file","<file>"},
	{"delete","Delete file/directory","<file>/<dir>"},
	{ (char *)NULL,(char *)NULL,(char *)NULL }
};

CLI_COMMAND_SUB commands_print[] = {
	{"date","Show system date",NULL},
	{"disksize","Display storage size",NULL},
	{"logsize","Display logs size",NULL},
	{"memory","Display memory status",NULL},
	{"cpu","Display cpu info",NULL},
	{"process","Display a snapshot of the current processes",NULL},
	{"task","Display current system task",NULL},
	{"dir","Directory listing",NULL},
	{ (char *)NULL,(char *)NULL,(char *)NULL }
};

CLI_COMMAND_SUB commands_session[] = {
	{"print","Show current active session",NULL},
	{"kill","Kill session","<sid>"},
	{ (char *)NULL,(char *)NULL,(char *)NULL }
};

CLI_COMMAND_SUB commands_update[] = {
	{"check","Check update status",NULL},
	{"update-firmware","Update firmware",NULL},
	{"download-firmware","Download firmware",NULL},
	{ (char *)NULL,(char *)NULL,(char *)NULL }
};

CLI_COMMAND *cli_search_command(char *name) {
	register int i;
	for(i = 0; commands[i].name; i++) {
		if(strstr(commands[i].name,name) && (strlen(name)>=2)) return (&commands[i]);
	}
	return ((CLI_COMMAND *)NULL);
}

CLI_COMMAND_SUB *cli_search_command_exec(char *name) {
	register int i;
	for(i = 0; commands_exec[i].name; i++) {
		if(strstr(commands_exec[i].name,name) && (strlen(name)>=2)) return (&commands_exec[i]);
	}
	return ((CLI_COMMAND_SUB *)NULL);
}

CLI_COMMAND_SUB *cli_search_command_print(char *name) {
	register int i;
	for(i = 0; commands_print[i].name; i++) {
		if(strstr(commands_print[i].name,name) && (strlen(name)>=2)) return (&commands_print[i]);
	}
	return ((CLI_COMMAND_SUB *)NULL);
}

CLI_COMMAND_SUB *cli_search_command_session(char *name) {
	register int i;
	for(i = 0; commands_session[i].name; i++) {
		if(strstr(commands_session[i].name,name) && (strlen(name)>=2)) return (&commands_session[i]);
	}
	return ((CLI_COMMAND_SUB *)NULL);
}

CLI_COMMAND_SUB *cli_search_command_update(char *name) {
	register int i;
	for(i = 0; commands_update[i].name; i++) {
		if(strstr(commands_update[i].name,name) && (strlen(name)>=2)) return (&commands_update[i]);
	}
	return ((CLI_COMMAND_SUB *)NULL);
}

CLI_COMMAND_SUB *cli_search_command_acl(char *name) {
	register int i;
	for(i = 0; commands_acl[i].name; i++) {
		if(strstr(commands_acl[i].name,name) && (strlen(name)>=2)) return (&commands_acl[i]);
	}
	return ((CLI_COMMAND_SUB *)NULL);
}

static char *command_generator(const char *text, int state) {
	static int len;
	static char *name = NULL;
	static int index = 0;
	if(!state) {
		index = 0;
		len = strlen (text);
	}
	str_input= split_array(rl_line_buffer, SINGLE_SPACE, &chargv);      
	if(str_input == NULL) return NULL;
	if(!strcmp(chargv[0],"print")) {
		while((name = commands_print[index].name)) {
			index++;
			if(strncmp(name, text, len)==0) return(xstrdup(name));
		}
	} else if(!strcmp(chargv[0],"exec")) {
		while((name = commands_exec[index].name)) {
			index++;
			if(strncmp(name, text, len)==0) return(xstrdup(name));
		}
	} else if(!strcmp(chargv[0],"acl")) {
		while((name = commands_acl[index].name)) {
			index++;
			if(strncmp(name, text, len)==0) return(xstrdup(name));
		}
	} else if(!strcmp(chargv[0],"session")) {
		while((name = commands_session[index].name)) {
			index++;
			if(strncmp(name, text, len)==0) return(xstrdup(name));
		}
	} else if(!strcmp(chargv[0],"update")) {
		while((name = commands_update[index].name)) {
			index++;
			if(strncmp(name, text, len)==0) return(xstrdup(name));
		}
	} else if(!chargv[1]) {
		while((name = commands[index].name)) {
			index++;
			if(strncmp(name, text, len)==0) return(xstrdup(name));
		}
	}
    	return NULL;
}

char **word_completion (char *text, int start, int end) {
	char **matches;
	matches = (char **)NULL;
	matches = rl_completion_matches(text, command_generator);
	rl_attempted_completion_over = 1;
	return (matches);
}


// --------- cmd
static void fork_exec(const char *cmd, const char *arg) {
        pid_t pid,status;
        pid = fork();
        if(pid==0) {
                execl(cmd,cmd,arg,NULL);
                exit(0);
        }
        while(wait(&status)!=pid);
}

static int cmd_php_exec(char *line) {
	int i=0;
	char cmd[strlen(line)+100];
	char fbuf[1000];
	FILE *p;
	if(file_exists(CMD_HELPER)) {
		snprintf(cmd,sizeof(cmd),"/bin/php -f %s %s 2>/dev/null",CMD_HELPER,line);
		if((p=popen(cmd,"r"))!=NULL) {
        		while(fgets(fbuf, sizeof(fbuf), p)) {
				trim(fbuf);
				if(fbuf[0]!='\0') {
					if(!strcmp(fbuf,"mfs-iosh.exc-0")) {
						i=0;fbuf[0]=0;
					} else if(!strcmp(fbuf,"mfs-iosh.exc-1")) {
						i=1;fbuf[0]=0;
					} else if(!strcmp(fbuf,"mfs-iosh.exc-2")) {
						i=0;fbuf[0]=0;
						print_error("No record available");
					} else {
						if(fbuf[0]!='\0') fprintf_stdout("%s\n",fbuf);
					}
				}
				fbuf[0]=0;
			}
			pclose(p);
		}
		return(i);
	}
	puts("%% INTERNAL ERROR %%");
	return(0);
}

static int cmd_php_exec_script(char *file,char *line) {
	int i=0;
	char cmd[strlen(line)+100];
	char fbuf[1000];
	FILE *p;
	if(file_exists(file)) {
		snprintf(cmd,sizeof(cmd),"/bin/php -f %s \"%s\" 2>/dev/null",file,line);
		if((p=popen(cmd,"r"))!=NULL) {
        		while(fgets(fbuf, sizeof(fbuf), p)) {
				trim(fbuf);
				if(fbuf[0]!='\0') fprintf_stdout("%s\n",fbuf);
				fbuf[0]=0;
			}
			pclose(p);
		}
		return(1);
	}
	puts("%% INTERNAL ERROR %%");
	return(1);
}

static int cmd_addhistory(char *line) {
	if(strcmp(str_input,"awie_exec") && strcmp(str_input,"sysexec")) {
		add_history(line);
	}
}

extern HIST_ENTRY **history_list();
static int cmd_history(char *line) {
	HIST_ENTRY **list;
	register int i;
	list = history_list();
	if(list) {
        	for(i = 0; list[i]; i++) fprintf_stdout("  %s\r\n",list[i]->line);
    	}
    	return(0);
}

void cmd_banner(void) {
	char buf[50],update[101]="";
	int n;
	struct utsname u;
	uname(&u);
	n=read_oneline("/etc/firmware", buf);
	if(n > 0 && buf[0]!='\0') {
		snprintf(update,sizeof(update),"%s",trim(buf));
	}
	fprintf_stdout("%s %s %s\n(c) Tracenetwork Corporation Sdn. Bhd.\n",u.sysname,u.version,update);
}
static int cmd_version(void) {
	const long minute = 60;
	const long hour = minute * 60;
	const long day = hour * 24;
	const double megabyte = 1024 * 1024;
	struct sysinfo si;
	sysinfo(&si);
	cmd_banner();
	fprintf_stdout("System uptime %ld days, %ld:%02ld:%02ld\n", si.uptime / day, (si.uptime % day) / hour, (si.uptime % hour) / minute, si.uptime % minute);
	fprintf_stdout("total RAM %5.1f MB\n",si.totalram / megabyte);
	fprintf_stdout("free  RAM %5.1f MB\n",si.freeram / megabyte);
	fprintf_stdout("process count %d\r\n",si.procs);
	return(0);    
}

static int cmd_print_memory(void) {
	const double megabyte = 1024 * 1024;
	struct sysinfo si;
	sysinfo(&si);
	fprintf_stdout("Total  RAM  %5.1f MB\n",si.totalram / megabyte);
	fprintf_stdout("Free   RAM  %5.1f MB\n",si.freeram / megabyte);
	fprintf_stdout("Shared RAM  %5.1f MB\n",si.sharedram / megabyte);
	fprintf_stdout("Total  SWAP %5.1f MB\n",si.totalswap / megabyte);
	fprintf_stdout("Free   SWAP %5.1f MB\n",si.freeswap / megabyte);
	return(0);
}

static int cmd_print_cpu(void) {
	char buf[250];
	int nums, x=0;
	float idle;
	FILE *p;
	if((p=popen("/bin/procinfo 2>/dev/null","r"))!=NULL) {
		while(fgets(buf,sizeof(buf),p)!=NULL) {
			if(buf[0]!='\0') {
				if(strstr(buf,"idle")) {
					nums=split_array(buf, SINGLE_SPACE, &chargv);
					if(nums > 0) {
						for(x=0;x<nums;x++) {
							if(strstr(chargv[x],"%")) idle=atof(stripchar(chargv[x],'%'));
						}
					}
					break;
				}
			}
		}
		buf[0]=0;
		pclose(p);
        }
	fprintf_stdout("Usage\t: %05.2f%%\nIdle\t: %05.2f%%\n",100 - idle, idle);
	return(0);
}

static int cmd_exit(void) {
	puts("Exit!\n\n");
	if(tty_login[0]!=0) {
	 	if(!strstr(tty_login,"pts") && !strstr(tty_login,"ttyp")) {
			log_login("Session logout");
		} 
	}
	exit(0);
}
static int cmd_shutdown(void) {
	xsystem("/bin/exec_reboot shutdown");
	exit(0);
}
static int cmd_reboot(void) {
	xsystem("/bin/exec_reboot reboot");
	exit(0);
}
static int chk_path(char *f) {
	char *d,*dc,*b;
	if(!chdir(f)) {
		b=get_current_dir_name();
		if(!strcmp(b,"/")) {
			chdir("/config");
			return 0;
		}
		if(strlen(b) < 7) {
			chdir("/config");
			return 0;
		}
		b[7]=0;
		if(strcmp(b,"/config")) {
			chdir("/config");
			return 0;
		}
	} else {
		if(strlen(f) < 7) return 0;
		d=strdup(f);
		dc=dirname(d);
		if(dc!=NULL) {
			return chk_path(dc);	
		}
		f[7]=0;
		if(strcmp(f,"/config")) return 0;
	}
	chdir("/config");
	return 1;
}
static int cmd_exec(char *line) {
	int cnt=5,i=0, t;
	char file[500], *xline;
	xline=stripshellargv(line);
	CLI_COMMAND_SUB *command;
	if(split_array(xline, SINGLE_SPACE, &chargv) > 0) {
		if(chargv[1]) {
			command=cli_search_command_exec(chargv[1]);
			if(!command) {
				print_error("Invalid options");
				return(0);
			}
			if(!strcmp(chargv[1],"ping") && chargv[2]) {
                                if(chargv[3]!=0) cnt=atoi(chargv[3]);
                                signal(SIGINT,SIG_DFL);
                               	xsystem("ping -c %d -w %d %s",cnt,cnt,chargv[2]);
				return(0);
                        }
			if(!strcmp(chargv[1],"traceroute") && chargv[2]) {
				char aopt[50];
				if(chargv[3]!=0) strcpy(aopt,chargv[3]);
				signal(SIGINT,SIG_DFL);
				xsystem("traceroute %s %s",chargv[2],aopt);
				return(0);
			}
			if(!strcmp(chargv[1],"telnet") && chargv[2]) {
				cnt=23;
				if(chargv[3]!=0) cnt=atoi(chargv[3]);
				signal(SIGINT,SIG_DFL);
				xsystem("telnet %s %d",chargv[2],cnt);
				return(0);
			}
			if(!strcmp(chargv[1],"ssh") && chargv[2] && chargv[3]) {
				cnt=22;
				if(chargv[4]!=0) cnt=atoi(chargv[4]);
				signal(SIGINT,SIG_DFL);
				xsystem("exec_ssh %s %s %d",chargv[2],chargv[3],cnt);
				return(0);
			}
                        if(!strcmp(chargv[1],"ftp") && chargv[2] && chargv[3] && chargv[4] && chargv[5]) {
                                cnt=21;
                                if(chargv[6]!=0) cnt=atoi(chargv[6]);
                                signal(SIGINT,SIG_DFL);
                                xsystem("exec_ftp %s %s %s %s %d",chargv[2],chargv[3],chargv[4],chargv[5],cnt);
                                return(0);
                        }
			if(!strcmp(chargv[1],"scpget") && chargv[2] && chargv[3] && chargv[4]) {
				cnt=22;
				if(chargv[5]!=0) cnt=atoi(chargv[5]);
				signal(SIGINT,SIG_DFL);
				xsystem("scp -C -P %d -rp %s@%s:%s /config/.",cnt,chargv[3],chargv[2],chargv[4]);
				return(0);
			}
			if(!strcmp(chargv[1],"scpput") && chargv[2] && chargv[3] && chargv[4]) {
				cnt=22;
				snprintf(file,sizeof(file),"/config/%s",chargv[4]);
				if((!file_exists(file) && !is_dir(file))||(!chk_path(file))) {
					fprintf_stdout("%% No such file or directory\n");
					return(0);
				}
				if(chargv[5]!=0) cnt=atoi(chargv[5]);
				signal(SIGINT,SIG_DFL);
				xsystem("scp -C -P %d -rp /config/%s %s@%s:.",cnt,chargv[4],chargv[3],chargv[2]);
				return(0);
			}
			if((!strcmp(chargv[1],"nslookup")||!strcmp(chargv[1],"ns")) && chargv[2]) {
				signal(SIGINT,SIG_DFL);
				if(chargv[3]!=0) {
					xsystem("nslookup %s %s",chargv[2], chargv[3]);
				} else {
					xsystem("nslookup %s",chargv[2]);
				}
				return(0);
			}
			if(!strcmp(chargv[1],"delete") || !strcmp(chargv[1],"del")) {
				if(chargv[2]!=0) {
					snprintf(file,sizeof(file),"/config/%s",chargv[2]);
					if((!file_exists(file) && !is_dir(file))||(!chk_path(file))) {
						fprintf_stdout("%% No such file or directory\n");
						return(0);
					}
					snprintf(msgconfirm,sizeof(msgconfirm),"delete file/directory %s? [y/n] ",chargv[2]);
					if(!strcmp(readline(msgconfirm),"y")) {
						xsystem("rm -rf %s",file); return(0);
					}
				}
				return(0);
			}
			if(!strcmp(chargv[1],"edit")) {
				if(chargv[2]!=0) {
					snprintf(file,sizeof(file),"/config/%s",chargv[2]);
					if(!file_exists(file) || !chk_path(file)) {
						fprintf_stdout("%% No such file\n");
						return(0);
					}
					if(!xsystem("vi %s",chargv[2])) return(0);
				}
			}
		} else {
			show_help_exec();
			return(0);
		}
	}
	print_error("Invalid options");
	return(0);
}

static int cmd_print(char *line) {
	int xargc;
	char **xargv=xmalloc(10 * sizeof(char *));
	CLI_COMMAND_SUB *command;
	if(split_array(line, SINGLE_SPACE, &chargv) > 0) {
		if(chargv[1]) {
			command=cli_search_command_print(chargv[1]);
			if(!command) {
				print_error("Invalid options");
				return(0);
			}
			if(!strcmp(chargv[1],"date")) {
				print_date();
				return(0);
			}
			if(!strcmp(chargv[1],"disksize")) {
				if(!xsystem("df -x rootfs -kh |grep -v /config/")) return(0);
			}
			if(!strcmp(chargv[1],"logsize")) {
				if(!chdir("/strg/mybox")) {
					if(!xsystem("du -h logs |awk '{print $1}'")) return(0);
				}
			}
			if(!strcmp(chargv[1],"memory")) {
				cmd_print_memory();
				return(0);
			}
			if(!strcmp(chargv[1],"cpu")) {
				cmd_print_cpu();
				return(0);
			}
			if(!strcmp(chargv[1],"task")) {
				signal(SIGINT,SIG_DFL);
				//return xsystem("top");
				fork_exec("/bin/top",NULL);
				return(0);
			}
			if(!strcmp(chargv[1],"proc")||!strcmp(chargv[1],"process")) {
				signal(SIGINT,SIG_DFL);
				//return xsystem("ps |grep -v \"sh -c ps\" |grep -v ps |grep -v grep");
				fork_exec("/bin/ps",NULL);
				return(0);
			}
			if(!strcmp(chargv[1],"dir")) {
				char file[500];
				if(chargv[2]!=0) {
					snprintf(file,sizeof(file),"/config/%s",chargv[2]);
					if((!file_exists(file) && !is_dir(file))||(!chk_path(file))) {
						fprintf_stdout("%% No such file or directory\n");
						return(0);
					}
					xsystem("ls -lash %s",file);return(0);
				} else {
					xsystem("ls -lash");return(0);
				}
			}
		} else {
			show_help_print();
			return(0);
		}
	}
	print_error("Invalid options");
	return(0);
}

static int cmd_session(char *line) {
	CLI_COMMAND_SUB *command;
	if(split_array(line, SINGLE_SPACE, &chargv) > 0) {
		if(chargv[1]) {
			command=cli_search_command_session(chargv[1]);
			if(!command) {
				print_error("Invalid options");
				return(0);
			}
			if(!cmd_php_exec(line)) return(0);
			return(1);
		} else {
			show_help_session();
			return(0);
		}
	}
	print_error("Invalid options");
	return(0);
}

static int cmd_update(char *line) {
	CLI_COMMAND_SUB *command;
	if(split_array(line, SINGLE_SPACE, &chargv) > 0) {
		if(chargv[1]) {
			command=cli_search_command_update(chargv[1]);
			if(!command) {
				print_error("Invalid options");
				return(0);
			}
			cmd_php_exec_script("/service/tools/mfs-update.exc",chargv[1]);
			return(1);
		} else {
			show_help_update();
			return(0);
		}
	}
	print_error("Invalid options");
	return(0);
}

static int cmd_acl(char *line) {
	CLI_COMMAND_SUB *command;
	if(split_array(line, SINGLE_SPACE, &chargv) > 0) {
		if(chargv[1]) {
			command=cli_search_command_acl(chargv[1]);
			if(!command) {
				print_error("Invalid options");
				return(0);
			}
			if(!cmd_php_exec(line)) return(0);
		} else {
			show_help_acl();
			return(0);
		}
	}
	print_error("Invalid options");
	return(0);
}
//------- cmd end

// help
static int print_error(char *str) {
	fprintf_stdout("%% %s\r\n",str);
	return(0);
}
static int show_help(void) {
	register int i;
	for(i = 0; commands[i].name; i++) fprintf_stdout(" %-*s\t\t%s\r\n",(int)HELPID,commands[i].name, commands[i].doc);
	return(0);
}

static int show_help_acl(void) {
	register int i;
	for(i = 0; commands_acl[i].name; i++) fprintf_stdout(" %-*s\t\t%s\r\n",(int)HELPID,commands_acl[i].name, commands_acl[i].doc);
	return(0);
}

static int show_help_exec(void) {
	register int i;
	for(i = 0; commands_exec[i].name; i++) fprintf_stdout(" %-*s\t\t%s\r\n",(int)HELPID,commands_exec[i].name, commands_exec[i].doc);
	return(0);
}

static int show_help_print(void) {
	register int i;
	for(i = 0; commands_print[i].name; i++) fprintf_stdout(" %-*s\t\t%s\r\n",(int)HELPID,commands_print[i].name, commands_print[i].doc);
	return(0);
}

static int show_help_session(void) {
	register int i;
	for(i = 0; commands_session[i].name; i++) fprintf_stdout(" %-*s\t\t%s\r\n",(int)HELPID,commands_session[i].name, commands_session[i].doc);
	return(0);
}

static int show_help_update(void) {
	register int i;
	for(i = 0; commands_update[i].name; i++) fprintf_stdout(" %-*s\t\t%s\r\n",(int)HELPID,commands_update[i].name, commands_update[i].doc);
	return(0);
}
static int print_help(void) {
	register int i;

	char buff[strlen(rl_line_buffer)+1];
	strcpy(buff,rl_line_buffer);
	str_input=split_array(rl_line_buffer, SINGLE_SPACE, &chargv);
	if(str_input == NULL) {
		putchar('\n');
		show_help();
		rl_delete_text (0, rl_end);
		rl_point = rl_end = 0;
		rl_on_new_line();
	} else if (rl_end && isspace((int) rl_line_buffer[rl_end - 1])) {
		putchar('\n');
 		if(!strcmp(chargv[0],"help")) {
			show_help();
		} else if(!strcmp(chargv[0],"acl")) {
			if(!chargv[1]) {
				show_help_acl();
			} else {
				for(i = 0; commands_acl[i].name; i++) {
					if(!strcmp(chargv[1],commands_acl[i].name) && commands_acl[i].doc2) {
						fprintf_stdout(" %s\r\n",commands_acl[i].doc2);
					}
				}
			}
		} else if(!strcmp(chargv[0],"exec")) {
			if(!chargv[1]) {
				show_help_exec();
			} else {
				for(i = 0; commands_exec[i].name; i++) {
					if(!strcmp(chargv[1],commands_exec[i].name) && commands_exec[i].doc2) {
						fprintf_stdout(" %s\r\n",commands_exec[i].doc2);
					}
				}
			}
 		} else if(!strcmp(chargv[0],"print")) {
			if(!chargv[1]) {
				show_help_print();
			} else {
				for(i = 0; commands_print[i].name; i++) {
					if(!strcmp(chargv[1],commands_print[i].name) && commands_print[i].doc2) {
						fprintf_stdout(" %s\r\n",commands_print[i].doc2);
					}
				}
			}
 		} else if(!strcmp(chargv[0],"session")) {
			if(!chargv[1]) {
				show_help_session();
			} else {
				for(i = 0; commands_session[i].name; i++) {
					if(!strcmp(chargv[1],commands_session[i].name) && commands_session[i].doc2) {
						fprintf_stdout(" %s\r\n",commands_session[i].doc2);
					}
				}
			}
 		} else if(!strcmp(chargv[0],"update")) {
			if(!chargv[1]) {
				show_help_update();
			} else {
				for(i = 0; commands_update[i].name; i++) {
					if(!strcmp(chargv[1],commands_update[i].name) && commands_update[i].doc2) {
						fprintf_stdout(" %s\r\n",commands_update[i].doc2);
					}
				}
			}
		} 
		rl_delete_text (0, rl_end);
		rl_point = rl_end = 0;
		rl_on_new_line();
		rl_insert_text(buff);
	} 
	return(0);
}

// init
void cli_readline_init() {
	rl_terminal_name = getenv("TERM"); 
	setenv("TERM", rl_terminal_name, 0);
	rl_bind_key ('?', (Function *)print_help);
	rl_attempted_completion_function =  (CPPFunction *)word_completion;
	rl_completion_append_character = '\0';
}

static int execute_line(char *line) {
	char *word, cmd[256];
	CLI_COMMAND *command;
	if(split_array(line, SINGLE_SPACE, &chargv) > 0) {
		word=chargv[0];
		strncpy(cmd,word,sizeof(cmd));
		if(cmd[0]=='/' && cmd[1]=='/') {
			word="print";
			line=str_replace(line,"//","print ");
		} else if(cmd[0]=='/' && cmd[1]!='/') {
			word="exec";
			line=str_replace(line,"/","exec ");
		}
		command=cli_search_command(word);
		if(command) {
			return ((*(command->func)) (line));
		}
		if(!strcmp(word,"awie_exec") || !strcmp(word,"sysexec")) {
			signal(SIGINT,SIG_DFL);
			puts("Entering system shell!\n\n");
			//printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
			//printf("@ NOTE: ANY MODIFICATIONS DONE USING SYSTEM SHELL WILL VOID YOUR SUPPORT @\n");
			//printf("@       PLEASE USE MYADMIN FOR ANY CONFIGURATION CHANGES                 @\n");
			//printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n");
			run_shell();
		} else {
			print_error("Invalid command");
			return(0);
		} 
	}
	return(0);
}

int iosh_main(int argc,char **argv) {
	char *s, xcmd[150];
	const char *tmp, *tty;

	if(tty_login[0]==0) {
		if(!isatty(0) || !isatty(1) || !isatty(2)) return 1;
        	tmp=ttyname(0);
        	if(tmp) {
                	strncpy(tty_login, tmp, sizeof(tty_login));
                	if(strncmp(tty_login, "/dev/", 5) == 0) {
                        	tty = tty_login + 5;
                        	strcpy(tty_login,tty);
                	}
        	}
	}	
	if(strstr(tty_login,"pts") || strstr(tty_login,"ttyp")) {
		snprintf(xcmd,sizeof(xcmd),"iosh %s","SSL");
	} else {
		snprintf(xcmd,sizeof(xcmd),"iosh %s","console");
	}

	strcpy(argv[0],xcmd);

	cmd_banner();
	fprintf_stdout("\nEnter help/? for command\n");
	putchar('\n');
	cli_readline_init();
	signal(SIGQUIT,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	for( ; done == 0; ) {
		signal(SIGINT,SIG_IGN);
		chdir(ROOT_DIR);
		gethostname(host,sizeof(host));
		if(!strcmp(host,"(none")) strncpy(host,"mybox",sizeof(host));
		strcat(host,"> ");              
		str_input=readline(host);
		if(!str_input) break;
		s=trim(str_input);
		if(*s) {
	  		cmd_addhistory(s);
	  		execute_line(s);
		}
		free(str_input);
    	}
	exit(0);
}
