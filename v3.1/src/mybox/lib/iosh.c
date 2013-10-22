/*
 (C) Copyright 2006-2007 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.
 $Id: iosh.c,v 1.4 2007/10/27 01:56:10 nawawi Exp $
 Update: 05-May-2008, using clish library for easier maintence
*/

#include "libmybox.h"
#include "lub/c_decl.h"
#include "clish/shell.h"
#include "lub/string.h"
#include "lub/argv.h"

extern void clish_startup(int argc, const char **argv);
extern void clish_shutdown(void);
extern struct termios clish_default_tty_termios;
extern clish_shell_access_fn_t clish_access_callback;
extern clish_shell_script_fn_t clish_script_callback;

// protoype
static int do_confirm=0;
static int awie_login(void);
static bool_t awie_banner(void);
static int awie_version(void);
static bool_t awie_shell(void);
static bool_t awie_confirm(const clish_shell_t *shell,const lub_argv_t *argv);
static void awie_close(void);
static void awie_vstorage_pwd(void);
static int awie_more(const clish_shell_t *shell,const lub_argv_t *argv);
static int awie_vstorage_dochdir(char *dir);
static int awie_vstorage_chdir(const clish_shell_t *shell,const lub_argv_t *argv);
static int awie_exec_pass(const clish_shell_t *shell,const lub_argv_t *argv);
static int awie_help(void);
static clish_shell_builtin_t awie_cmd_list[] = {
	{"awie_close", awie_close},
	{"awie_banner", awie_banner},
	{"awie_version", awie_version},
	{"awie_shell", awie_shell},
	{"awie_more", awie_more},
	{"awie_vstorage_chdir", awie_vstorage_chdir},
	{"awie_vstorage_pwd", awie_vstorage_pwd},
	{"awie_help", awie_help},
	{"awie_exec_pass", awie_exec_pass},
	{"awie_confirm", awie_confirm},
	{NULL,NULL}
};

static clish_shell_hooks_t my_hooks = {
	NULL, /* init callback */
	clish_access_callback,
	NULL, /* cmd_line callback */
	clish_script_callback,
	NULL, /* fini callback */
	awie_cmd_list  /* builtin functions */
};


static int awie_version(void) {
	if(file_exists("/bin/exec_cli")) {
		return system("/bin/exec_cli version 2>/dev/null");
	}
	return 1;
} 


static bool_t awie_banner(void) {
	char btime[256], bv[20]="08.12";
        time_t curtime;
        struct tm *loctime;
        struct utsname u;
        uname(&u);
        curtime=time(NULL);
        loctime=localtime(&curtime);
        strftime(btime,sizeof(btime), "%a %d-%b-%Y %H:%M:%S %Z\n", loctime);
	read_oneline("/etc/firmware", bv);
	trim(bv);
	//fprintf(stdout,"[*] %s/Linux kernel %s\n",u.sysname,u.release);
	fprintf(stdout,"[*] %s build-%s\n",u.version,bv);
	fprintf(stdout,"[*] %s\n",u.nodename);
	fprintf(stdout,"[*] %s\n",btime);
	fflush(stdout);
	return BOOL_TRUE;
}

void log_sysexec(char *stat) {
        char msg[256];
     	if(getenv("SSH_CLIENT")!=NULL) {
		char *ip=strtok(getenv("SSH_CLIENT")," ");
		if(ip) {
			snprintf(msg,sizeof(msg),"ECODE=00 TYPE=sysexec USER=console IP=%s MSG=%s",ip,stat);
		} else {
			snprintf(msg,sizeof(msg),"ECODE=00 TYPE=sysexec USER=console IP=%s MSG=%s",tty_login,stat);
		}
	} else {
		snprintf(msg,sizeof(msg),"ECODE=00 TYPE=sysexec USER=console IP=%s MSG=%s",tty_login,stat);
	}
        openlog("LOGIN", LOG_NDELAY|LOG_PID, LOG_AUTHPRIV);
        syslog(LOG_INFO,"%s",msg);
        closelog();
}

static bool_t awie_shell(void) {
	if(do_confirm==1) {
               if(awie_login()!=0) {
                        return BOOL_FALSE;
                }
	}
	printf("%s!\n\n",gettext("Entering system shell"));
	log_sysexec(gettext("Entering system shell"));
	run_shell();
	return BOOL_TRUE;
}

static void awie_close(void) {
	char sysfile[23]="/etc/clish/sysexec.xml";
	if(tty_login[0]!=0) {
		if(!strstr(tty_login,"pts") && !strstr(tty_login,"ttyp")) {
			log_login("01",gettext("Session logout"));
		}
	}
	if(file_exists(sysfile)) {
		unlink(sysfile);
	}
	exit(1);
}

static int awie_more(const clish_shell_t *shell,const lub_argv_t *argv) {
	int result=1;
	const char *filename = lub_argv__get_arg(argv,0);
	if(file_exists(filename)) {
		if(strstr(filename,".gz")) {
			result=xsystem("/bin/zcat %s |more -dl",filename);
		} else {
			result=xsystem("/bin/more -dl %s",filename);
		}
	}
	return result ? 0 : 1;
}

// 1 = true, 0 = false
static int awie_vstorage_dochdir(char *dir) {
	if(chdir(dir)==0) {
		char *pt=(char *)get_current_dir_name();
		if(strncmp(pt,"/config",7)) {
			return 0;
		}
		return 1;
	}
	return 0;
}

// 1 = true, 0 = false
static int awie_vstorage_chdir(const clish_shell_t *shell,const lub_argv_t *argv) {
	const char *dirname = lub_argv__get_arg(argv,0);
	int chk=0;
	char *pp;
	char *buf=xmalloc(10+strlen(dirname));
	sprintf(buf,"/config/%s",dirname);
	if(awie_vstorage_dochdir(buf)) chk=1;
	if(chk==0 && awie_vstorage_dochdir(dirname)) chk=1;

	if(chk==0) {
		printf("%s '%s'\n",gettext("Invalid directory"),dirname);
		return 0;
	}
	pp=(char *)str_replace(get_current_dir_name(),"/config","\0");
	if(pp && pp[0]!='\0') {
		printf("%s '%s'\n",gettext("Change to"),pp);
	} else {
		printf("%s '%s'\n",gettext("Change to"),dirname);
	}
	return 1;
}

static void awie_vstorage_pwd(void) {
	char *pp=(char *)str_replace(get_current_dir_name(),"/config","\0");
	if(pp && pp[0]!='\0') {
		printf("%s\n",pp);
	} else {
		printf("/\n");
	}
}

static int awie_help(void) {
	if(cli_help[0]!='\0' && file_exists_match("/etc/clish/cli.help.%s",cli_help)) {
		return xsystem("/bin/more -dl /etc/clish/cli.help.%s",cli_help);
	}
	if(file_exists("/etc/clish/cli.help")) {
		return xsystem("/bin/more -dl /etc/clish/cli.help");
	}
	return 1;
}

static int awie_login(void) {
	int count=0;
	printf("%s\n\n",gettext("Please enter console password.."));
	while(1) {
		if(password_prompt()) {
			puts(gettext("Password Accepted"));
			return(0);
		}
		if(count++ == 3) {
			printf("%s\n",gettext("Invalid access password"));
			sleep(1);
			return(1);
		}
	}
	return(1);
}

static int awie_exec_pass(const clish_shell_t *shell,const lub_argv_t *argv) {
	int result=1;
	const char *cmd=lub_argv__get_arg(argv,0);
	const char *opt=lub_argv__get_arg(argv,1);
	if(!file_exists(cmd)) return 0;
        if(do_confirm==1) {
               	if(awie_login()!=0) {
                        return 0;
                }
        }
	result=xsystem("%s %s",cmd,opt);
	return result ? 0 : 1;
}

static bool_t awie_confirm(const clish_shell_t *shell,const lub_argv_t *argv) {
	if(do_confirm==1) {
		if(awie_login()!=0) {
                        return BOOL_FALSE;
                }
	}
	if(argv!=NULL) {
		printf("\n%s\n\n",lub_argv__get_arg(argv,0));
	}
	return BOOL_TRUE;
}

int iosh_main(int argc,char **argv) {
        char *s, xcmd[150], host[40];
        const char *tmp, *tty, *h;
        int result=1;
	init_lang();

        if(tty_login[0]==0) {
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
        }

        if(getenv("SSH_TTY")!=NULL && getenv("ANYTERM_SHELL")==NULL) {
                snprintf(xcmd,sizeof(xcmd),"iosh %s","SSL");
		do_confirm=1;
		print_banner();
       		putchar('\n');
        } else {
		if(getenv("ANYTERM_SHELL")!=NULL) {
			do_confirm=1;
		}
                snprintf(xcmd,sizeof(xcmd),"iosh %s","console");
        }

	gethostname(host,sizeof(host));
	if(!strcmp(host,"(none)")) {
		memset(host,0x0,sizeof(host));
		strncpy(host,"trace",sizeof(host));
	} else {
		h=strtok(host,".");
		strncpy(host,h,sizeof(host));
	}
	setenv("SYSTEM_NAME",host,1);
	putenv("PATH=/bin:/service/init:/service/tools");
	putenv("CLISH_PATH=/etc/clish");
        strcpy(argv[0],xcmd);

	awie_banner();
	if(!file_exists("/etc/clish/sysexec.xml") && file_exists("/etc/clish/sysexec.cli")) {
		xsystem("/bin/cp -f /etc/clish/%s /etc/clish/%s","sysexec.cli","sysexec.xml");
	}
	chdir("/config");
	clish_startup(argc,argv);
	result = clish_shell_spawn_and_wait(&my_hooks,NULL);
	clish_shutdown();
	return result ? 0 : 1;
}
