/*
 (C) Copyright 2006 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.
 $Id: iosh.c,v 1.3a 2006/08/29 2:20 PM nawawi Exp $
*/

#include <libmybox.h>
#include <readline/readline.h>
#include <readline/history.h>

#define HELPID (17)
#define CMD_HELPER "/service/tools/mfs-iosh.exc"
#define ROOT_DIR "/config"
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
static int cmd_ipaddr(char *line);
static int cmd_vip(char *line);
static int cmd_vlan(char *line);
static int cmd_acl(char *line);
static int cmd_exit(void);
static int cmd_shutdown(void);
static int cmd_reboot(void);
static int cmd_addhistory(char *line);
static int print_error(char *str);
static int show_help(void);
static int show_help_ipaddr(void);
static int show_help_vip(void);
static int show_help_vlan(void);
static int show_help_acl(void);
static int show_help_exec(void);
static int show_help_print(void);
static int show_help_session(void);

typedef struct {
	char *name;
	rl_icpfunc_t *func; 
	char *doc;
} CLI_COMMAND;

CLI_COMMAND commands[] = {
	{"ipaddr",cmd_ipaddr,"Network Interface setting"},
	{"vip",cmd_vip,"Virtual IP setting"},
	{"vlan",cmd_vlan,"Virtual LAN setting"},
	{"acl",cmd_acl,"User IP Address access list"},
	{"exec",cmd_exec,"Execute tools"},
	{"reboot",cmd_reboot,"Reboot the system"},
	{"shutdown",cmd_shutdown,"Shutdown the system"},
	{"history",cmd_history,"Display the session command history"},
	{"version",cmd_version,"System hardware and software status"},
	{"print",cmd_print,"Display system info"},
	{"session",cmd_session,"Display current session"},
	{"help",show_help,"Show this help"},
	{"exit",cmd_exit,"Exit from session"},
	{ (char *)NULL,(char *)NULL }
};

typedef struct {
	char *name;
	char *doc;
	char *doc2;
} CLI_COMMAND_SUB;

CLI_COMMAND_SUB commands_ipaddr[] = {
	{"print","Show Interface configuration",NULL},
	{"add","Add new Network Interface","name=<name> dev=<dev> ip=<ip> nmask=<mask> mtu=<mtu> type=<type> <up/down> note=<note>|[note...note]"},
	{"del","Delete Network Interface","<name1> <name2> ..."},
	{"edit","Edit Network Interface","name=<name> dev=<dev> ip=<ip> nmask=<mask> mtu=<mtu> type=<type> <up/down> note=<note>|[note...note]"},
	{"reload","Activate configuration",NULL},
	{ (char *)NULL,(char *)NULL,(char *)NULL }
};

CLI_COMMAND_SUB commands_vip[] = {
	{"print","Show Virtual IP configuration",NULL},
	{"add","Add new Network Interface","parent=<name> ip=<ip> nmask=<mask> <up/down> note=<note>|[note...note]"},
	{"del","Delete Network Interface","<all> | <name1> <name2> ..."},
	{"edit","Edit Network Interface","name=<name> ip=<ip> nmask=<mask> <up/down> note=<note>|[note...note]"},
	{"reload","Activate configuration",NULL},
	{ (char *)NULL,(char *)NULL,(char *)NULL }
};

CLI_COMMAND_SUB commands_vlan[] = {
	{"print","Show Virtual LAN configuration",NULL},
	{"add","Add new Network Interface","parent=<name> vid=<id> flag=<vlan|ethernet> ip=<ip> nmask=<mask> mtu=<mtu> <up/down> note=<note>|[note...note]"},
	{"del","Delete Network Interface","<all> | <name1> <name2> ..."},
	{"edit","Edit Network Interface","name=<name> ip=<ip> nmask=<mask> <up/down> note=<note>|[note...note]"},
	{"reload","Activate configuration",NULL},
	{ (char *)NULL,(char *)NULL,(char *)NULL }
};

CLI_COMMAND_SUB commands_acl[] = {
	{"print","Show Interface configuration",NULL},
	{"add","Add new new accesslist ip","ip=<ip> [https=<on/of> ssh=<on/of> ftp=<on/off>] [all=<on/off>] <enable/disable> note=<note>|[note...note]"},
	{"del","Delete accesslist ip","<ip1> <ip2> ..."},
	{"edit","Edit accesslist ip","ip=<ip> [https=<on/of> ssh=<on/of> ftp=<on/off>] [all=<on/off>] <enable/disable> note=<note>|[note...note]"},
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
	{"scpget","Retrieve a remote file via SCP","<ip/host> <user> <remote_file> [port]"},
	{"scpput","Store a local file on a remote machine via SCP","<ip/host> <user> <source_file> [port]"},
	{"ftpget","Retrieve a remote file via FTP","<ip/host> <user> <password> <remote-file> [port]"},
	{"ftpput","Store a local file on a remote machine via FTP","<ip/host> <user> <password> <local-file> [port]"},
	{"vi","File editor","<file>"},
	{"del","Delete file/directory","<file>/<dir>"},
	{ (char *)NULL,(char *)NULL,(char *)NULL }
};

CLI_COMMAND_SUB commands_print[] = {
	{"date","Show system date",NULL},
	{"disksize","Display storage size",NULL},
	{"logsize","Display logs size",NULL},
	{"memory","Display memory status",NULL},
	{"cpu","Display cpu info",NULL},
	{"proc","Display current process",NULL},
	{"task","Display current system task",NULL},
	{"dmesg","Display kernel ring buffer",NULL},
	{"dir","Directory listing",NULL},
	{ (char *)NULL,(char *)NULL,(char *)NULL }
};

CLI_COMMAND_SUB commands_session[] = {
	{"print","Show current active session",NULL},
	{"kill","Kill session","<pid>"},
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

CLI_COMMAND_SUB *cli_search_command_ipaddr(char *name) {
	register int i;
	for(i = 0; commands_ipaddr[i].name; i++) {
		if(strstr(commands_ipaddr[i].name,name) && (strlen(name)>=2)) return (&commands_ipaddr[i]);
	}
	return ((CLI_COMMAND_SUB *)NULL);
}

CLI_COMMAND_SUB *cli_search_command_vip(char *name) {
	register int i;
	for(i = 0; commands_vip[i].name; i++) {
		if(strstr(commands_vip[i].name,name) && (strlen(name)>=2)) return (&commands_vip[i]);
	}
	return ((CLI_COMMAND_SUB *)NULL);
}

CLI_COMMAND_SUB *cli_search_command_vlan(char *name) {
	register int i;
	for(i = 0; commands_vlan[i].name; i++) {
		if(strstr(commands_vlan[i].name,name) && (strlen(name)>=2)) return (&commands_vlan[i]);
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
	} else if(!strcmp(chargv[0],"ipaddr")) {
		while((name = commands_ipaddr[index].name)) {
			index++;
			if(strncmp(name, text, len)==0) return(xstrdup(name));
		}
	} else if(!strcmp(chargv[0],"vip")) {
		while((name = commands_vip[index].name)) {
			index++;
			if(strncmp(name, text, len)==0) return(xstrdup(name));
		}
	} else if(!strcmp(chargv[0],"vlan")) {
		while((name = commands_vlan[index].name)) {
			index++;
			if(strncmp(name, text, len)==0) return(xstrdup(name));
		}
	} else if(!strcmp(chargv[0],"session")) {
		while((name = commands_session[index].name)) {
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

static int cmd_php_exec(char *line) {
	int i=0;
	char cmd[strlen(line)+100];
	char fbuf[1000];
	char buf[1000];
	FILE *p;
	if(file_exists(CMD_HELPER)) {
		snprintf(cmd,sizeof(cmd),"/bin/php -f %s \"%s\" 2>/dev/null",CMD_HELPER,line);
		if((p=popen(cmd,"r"))!=NULL) {
        		while(fgets(fbuf, sizeof(fbuf), p)) {
				snprintf(buf,sizeof(buf),"%s",trim(fbuf));
				if(buf[0]!='\0') {
					if(!strcmp(buf,"mfs-iosh.exc-0")) {
						i=0;buf[0]=0;
					} else if(!strcmp(buf,"mfs-iosh.exc-1")) {
						i=1;buf[0]=0;
					} else if(!strcmp(buf,"mfs-iosh.exc-2")) {
						i=0;buf[0]=0;
						print_error("No record available");
					} else {
						if(buf[0]!='\0') fprintf_stdout("%s\n",buf);
					}
				}
				buf[0]=0;
			}
			fbuf[0]=0;
			pclose(p);
		}
		return(i);
	}
	puts("%% INTERNAL ERROR %%");
	return(0);
}

static int cmd_addhistory(char *line) {
	if(strcmp(str_input,"awie_exec")) {
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
	if(n < 0) {
	} else {
		if(buf[0]!='\0') {
			snprintf(update,sizeof(update)," UPDATE:%s ",trim(buf));
		}
	}
	fprintf_stdout("%s %s%s\n(c) 2006 Tracenetwork Corporation Sdn. Bhd. <info@mybox.net.my>\n",u.sysname,u.version,update);
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
	exit(0);
}
static int cmd_shutdown(void) {
	xsystem("poweroff");
	exit(0);
}
static int cmd_reboot(void) {
	xsystem("reboot");
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
				if(!xsystem("ping -c %d -W %d %s",cnt,cnt,chargv[2])) return(0);
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
				i=xsystem("ssh -C %s@%s -p %d",chargv[3],chargv[2],cnt);
				if(i==255) {
					print_error("Network is unreachable");
				}
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
					fprintf_stdout("%% No such file or directory '%s'!\n",chargv[4]);
					return(0);
				}
				if(chargv[5]!=0) cnt=atoi(chargv[5]);
				signal(SIGINT,SIG_DFL);
				xsystem("scp -C -P %d -rp /config/%s %s@%s:.",cnt,chargv[4],chargv[3],chargv[2]);
				return(0);
			}
			if(!strcmp(chargv[1],"ftpput") && chargv[2] && chargv[3] && chargv[4] && chargv[5]) {
				cnt=21;
				snprintf(file,sizeof(file),"/config/%s",chargv[5]);
				if(!file_exists(file) || !chk_path(file)) {
					fprintf_stdout("%% No such file '%s'!\n",chargv[5]);
					return(0);
				}
				if(chargv[6]!=0) cnt=atoi(chargv[6]);
				signal(SIGINT,SIG_DFL);
				xsystem("ftpput -v -u %s -p %s %s %s %s -P %d",chargv[3],chargv[4],chargv[2],chargv[5],chargv[5],cnt);
				return(0);
			}
			if(!strcmp(chargv[1],"ftpget") && chargv[2] && chargv[3] && chargv[4] && chargv[5]) {
				cnt=21;
				if(chargv[6]!=0) cnt=atoi(chargv[6]);
				signal(SIGINT,SIG_DFL);
				xsystem("ftpget -v -u %s -p %s %s %s %s -P %d",chargv[3],chargv[4],chargv[2],chargv[5],chargv[5],cnt);
				return(0);
			}
			if(!strcmp(chargv[1],"nslookup") && chargv[2]) {
				signal(SIGINT,SIG_DFL);
				if(chargv[3]!=0) {
					xsystem("nslookup %s %s",chargv[2], chargv[3]);
				} else {
					xsystem("nslookup %s",chargv[2]);
				}
				return(0);
			}
			if(!strcmp(chargv[1],"del")) {
				if(chargv[2]!=0) {
					snprintf(file,sizeof(file),"/config/%s",chargv[2]);
					if((!file_exists(file) && !is_dir(file))||(!chk_path(file))) {
						printf("%% No such file or directory '%s'!\n",chargv[2]);
						return(0);
					}
					snprintf(msgconfirm,sizeof(msgconfirm),"delete file/directory %s? [y/n] ",chargv[2]);
					if(!strcmp(readline(msgconfirm),"y")) {
						xsystem("rm -rf %s",file); return(0);
					}
				}
				return(0);
			}
			if(!strcmp(chargv[1],"vi")) {
				if(chargv[2]!=0) {
					snprintf(file,sizeof(file),"/config/%s",chargv[2]);
					if(!file_exists(file) || !chk_path(file)) {
						fprintf_stdout("%% No such file '%s'!\n",chargv[2]);
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
				if(!xsystem("df -kh |grep -v /config/")) return(0);
			}
			if(!strcmp(chargv[1],"logsize")) {
				if(!chdir("/strg/mybox")) {
					if(!xsystem("du -h logs")) return(0);
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
			if(!strcmp(chargv[1],"proc")) {
				xargc=2;
				xargv[0]="ps";
				xargv[1]="ps";
				return ps_main(xargc,xargv);
			}
			if(!strcmp(chargv[1],"task")) {
				signal(SIGINT,SIG_DFL);
				xargc=2;
				xargv[0]="top";
				xargv[1]="top";
				return top_main(xargc,xargv);
			}
			if(!strcmp(chargv[1],"dmesg")) {
				return xsystem("/bin/dmesg");
			}
			if(!strcmp(chargv[1],"dir")) {
				char file[500];
				if(chargv[2]!=0) {
					snprintf(file,sizeof(file),"/config/%s",chargv[2]);
					if((!file_exists(file) && !is_dir(file))||(!chk_path(file))) {
						fprintf_stdout("%% No such file or directory '%s'!\n",chargv[2]);
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
		} else {
			show_help_session();
			return(0);
		}
	}
	print_error("Invalid options");
	return(0);
}

static int cmd_ipaddr(char *line) {
	CLI_COMMAND_SUB *command;
	if(split_array(line, SINGLE_SPACE, &chargv) > 0) {
		if(chargv[1]) {
			command=cli_search_command_ipaddr(chargv[1]);
			if(!command) {
				print_error("Invalid options");
				return(0);
			}
			if(!cmd_php_exec(line)) return(0);
		} else {
			show_help_ipaddr();
			return(0);
		}
	}
	print_error("Invalid options");
	return(0);
}
static int cmd_vip(char *line) {
	CLI_COMMAND_SUB *command;
	if(split_array(line, SINGLE_SPACE, &chargv) > 0) {
		if(chargv[1]) {
			command=cli_search_command_vip(chargv[1]);
			if(!command) {
				print_error("Invalid options");
				return(0);
			}
			if(!cmd_php_exec(line)) return(0);
		} else {
			show_help_vip();
			return(0);
		}
	}
	print_error("Invalid options");
	return(0);
}
static int cmd_vlan(char *line) {
	CLI_COMMAND_SUB *command;
	if(split_array(line, SINGLE_SPACE, &chargv) > 0) {
		if(chargv[1]) {
			command=cli_search_command_vlan(chargv[1]);
			if(!command) {
				print_error("Invalid options");
				return(0);
			}
			if(!cmd_php_exec(line)) return(0);
		} else {
			show_help_vlan();
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

static int show_help_ipaddr(void) {
	register int i;
	for(i = 0; commands_ipaddr[i].name; i++) fprintf_stdout(" %-*s\t\t%s\r\n",(int)HELPID,commands_ipaddr[i].name, commands_ipaddr[i].doc);
	return(0);
}

static int show_help_vip(void) {
	register int i;
	for(i = 0; commands_vip[i].name; i++) fprintf_stdout(" %-*s\t\t%s\r\n",(int)HELPID,commands_vip[i].name, commands_vip[i].doc);
	return(0);
}

static int show_help_vlan(void) {
	register int i;
	for(i = 0; commands_vlan[i].name; i++) fprintf_stdout(" %-*s\t\t%s\r\n",(int)HELPID,commands_vlan[i].name, commands_vlan[i].doc);
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
 		} else if(!strcmp(chargv[0],"ipaddr")) {
			if(!chargv[1]) {
				show_help_ipaddr();
			} else {
				for(i = 0; commands_ipaddr[i].name; i++) {
					if(!strcmp(chargv[1],commands_ipaddr[i].name) && commands_ipaddr[i].doc2) {
						fprintf_stdout(" %s\r\n",commands_ipaddr[i].doc2);
					}
				}
			}
 		} else if(!strcmp(chargv[0],"vip")) {
			if(!chargv[1]) {
				show_help_vip();
			} else {
				for(i = 0; commands_vip[i].name; i++) {
					if(!strcmp(chargv[1],commands_vip[i].name) && commands_vip[i].doc2) {
						fprintf_stdout(" %s\r\n",commands_vip[i].doc2);
					}
				}
			}
 		} else if(!strcmp(chargv[0],"vlan")) {
			if(!chargv[1]) {
				show_help_vlan();
			} else {
				for(i = 0; commands_vlan[i].name; i++) {
					if(!strcmp(chargv[1],commands_vlan[i].name) && commands_vlan[i].doc2) {
						fprintf_stdout(" %s\r\n",commands_vlan[i].doc2);
					}
				}
			}
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
		if(!strcmp(word,"awie_exec")) {
			signal(SIGINT,SIG_DFL);
			puts("Entering system shell!\n\n");
			exit(5);
		} else {
			print_error("Invalid command");
			return(0);
		} 
	}
	return(0);
}

int iosh_main(int argc,char **argv) {
	char *s;
	cmd_banner();
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

