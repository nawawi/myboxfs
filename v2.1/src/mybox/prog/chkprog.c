#include <libmybox.h>

static int queryq=0;
static int queryi=0;
static int queryo=0;
static int snmpd=0;
static int httpsd=0;
static int sshd=0;
static int syslogd=0;
static int snortd=0;
static int dhcpd=0;
static int pptpd=0;
static int ftpd=0;
static int mfssysp=0;
static int mfssys=0;
static int lcdd=0;
static int nmbd=0;
static int winbindd=0;
static int crond=0;
static int failoverd=0;
static int ddnsd=0;
static int dns_resolver=0;
static int watchd=0;
static int LCD_PROC=0;
static int lcds=0;
static int lcdp=0;
static int lcde=0;

static int chkpid(void) {
        DIR *dir;
        struct dirent *entry;
        char *name;
        char *status_tail;
        int pid;
        struct stat sb;
        int n;
        char status[32];
        char buf[1023];
        char cmd[16], scmd[16];
	char str[1024];
	if((dir = opendir("/proc")) == NULL) return 1;
        for(;;) {
                if((entry = readdir(dir)) == NULL) {
                        closedir(dir);
                        return 0;
                }
                name = entry->d_name;
                if (!(*name >= '0' && *name <= '9')) continue;
                pid = atoi(name);
                status_tail = status + sprintf(status, "/proc/%d", pid);
                if(stat(status, &sb)) continue;
                strcpy(status_tail, "/stat");
                n = read_oneline(status, buf);
                if(n < 0) continue;
                sscanf(buf, "%*s (%15c)", cmd);
		splitc(scmd,cmd,')');
		sprintf(str,"%s ",scmd);
                strcpy(status_tail, "/cmdline");
		n = read_oneline(status, buf);
                if(n > 0) {
                        if(buf[n-1]=='\n')  buf[--n] = 0;
                        name = buf;
                        while(n) {
                                if(((unsigned char)*name) < ' ') *name = ' ';
                                name++;
                                n--;
                        }
                        *name = 0;
                        if(buf[0]) sprintf(str,"%s %s ",str, strdup(buf));
                }
		trim(str);
		if(strstr(str,"mfs-query.exc q")) queryq++;
		if(strstr(str,"mfs-query.exc i")) queryi++;
		if(strstr(str,"mfs-query.exc o")) queryo++;
		if(strstr(str,"snmpd")) snmpd=1;
		if(strstr(str,"httpsd")) httpsd=1;
		if(strstr(str,"sshd")) sshd=1;
		if(strstr(str,"syslogd")) syslogd++;
		if(strstr(str,"mfs-system.exc p")) mfssysp++;
		if(strstr(str,"mfs-system.exc")) mfssys++;
		if(strstr(str,"klogd")) syslogd++;
		if(strstr(str,"mfs-system.exc")) syslogd++;
		if(strstr(str,"mfs-kernel.exc")) syslogd++;
		if(strstr(str,"snortd")) snortd++;
		if(strstr(str,"dhcpd")) dhcpd++;
		if(strstr(str,"pptpd")) pptpd++;
		if(strstr(str,"ftpd")) ftpd++;
		if(strstr(str,"nmbd")) nmbd++;
		if(strstr(str,"winbindd")) winbindd++;
		if(strstr(str,"crond")) crond++;
		if(strstr(str,"failoverd")) failoverd++;
		if(strstr(str,"ddnsd")) ddnsd++;
		if(strstr(str,"dns_resolver")) dns_resolver++;
		if(strstr(str,"mfs-watch.exc")) watchd++;

		if(LCD_PROC==0) {
			if(strstr(str,"lcdd")) lcdd++;
		} else {
			if(strstr(str,"lcds")) lcds++;
			if(strstr(str,"lcdp")) lcdp++;
			if(strstr(str,"lcde")) lcde++;
		}
        }
        return 0;
}

static void fork_script(char *file,char *arg) {
	char txt[256], **xargv=xmalloc(250 * sizeof(char **));
	pid_t pid,status; 
	pid = fork();
        if(pid==0) {
		if(file_exists(file)) {
			xargv[0]=file;xargv[1]=NULL;
			snprintf(txt,sizeof(txt),"Service down: Executing %s %s",base_name(xargv),arg);
			log_action("WARNING",txt);sleep(1);
                	xsystem("/bin/php -f %s %s",file,arg);
			sleep(1);
		}
                exit(0);
        }
	while(wait(&status)!=pid);
}

static void fork_exec(char *prog,char *arg) {
	char txt[256], **xargv=xmalloc(250 * sizeof(char **));
	pid_t pid,status; 
	pid = fork();
        if(pid==0) {
		if(file_exists(prog)) {
			xargv[0]=prog;xargv[1]=NULL;
			snprintf(txt,sizeof(txt),"Service down: Executing %s %s",base_name(xargv),arg);
			log_action("WARNING",txt);sleep(1);
                	xsystem("%s %s",prog,arg);
			sleep(1);
		}
                exit(0);
        }
	while(wait(&status)!=pid);
}

static void chksnortd(void) {
	FILE *f;
	char buff[150];
	int cnt=0;
	if(file_exists("/var/sys/chk_snortd")) {
		if((f=fopen("/var/sys/chk_snortd","r"))!=NULL) {
			fgets(buff, sizeof(buff) - 1, f);
			trim(buff);
			cnt=atoi(buff);
			fclose(f);
		}
		if(snortd >= cnt) {
			snortd=1;
		} else {
			snortd=0;
		}
	}
}

int chkprog_main(int argc,char **argv) {
	freopen("/dev/null", "r", stdin);
      	freopen("/dev/null", "w", stdout);
      	freopen("/dev/null", "w", stderr);
	if(file_exists("/var/sys/lcd_proc")) LCD_PROC=1;
	chkpid();
	chksnortd();

	if(file_exists("/var/sys/chk_syslogd") && syslogd==0) {
		fork_script("/service/init/syslog.init","restart");
		snortd=1;
	}
	if(file_exists("/var/sys/chk_syslogd") && syslogd!=0) {
		if(mfssysp==0 || mfssys==0) {
			fork_script("/service/init/syslog.init","restart");
			snortd=1;
		}
	}
	if(queryq==0) {
		if(!file_exists("/var/sys/chkprog_wait")) fork_script("/service/tools/mfs-query.exc","q");
	}
	if(queryi==0) {
		if(!file_exists("/var/sys/chkprog_wait")) fork_script("/service/tools/mfs-query.exc","i");
	}
	if(queryo==0) {
		if(!file_exists("/var/sys/chkprog_wait")) fork_script("/service/tools/mfs-query.exc","o");
	}
	if(file_exists("/var/sys/chk_snortd") && snortd==0) {
		fork_script("/service/init/ips.init","restart");
	}
	if(file_exists("/var/sys/chk_snmpd") && snmpd==0) {
		fork_script("/service/init/snmp.init","restart");
	}
	if(file_exists("/var/sys/chk_httpsd") && httpsd==0) {
		fork_script("/service/init/https.init","restart");
	}
	if(file_exists("/var/sys/chk_sshd") && sshd==0) {
		fork_script("/service/init/ssh.init","restart");
	}
	if(file_exists("/var/sys/chk_dhcpd") && dhcpd==0) {
		fork_script("/service/init/dhcp.init","restart");
	}
	if(file_exists("/var/sys/chk_pptpd") && pptpd==0) {
		fork_script("/service/init/pptp.init","restart");
	}
	if(file_exists("/var/sys/chk_ftpd") && ftpd==0) {
		fork_script("/service/init/ftp.init","restart");
	}
	if(file_exists("/var/sys/chk_ddnsd") && ddnsd==0) {
		fork_script("/service/init/ddns.init","restart");
	}
	if(file_exists("/var/sys/chk_dns_resolver") && dns_resolver==0) {
		fork_script("/service/init/dns.init","restart");
	}
	if(file_exists("/var/sys/chk_bawal") && watchd==0) {
		fork_script("/service/tools/mfs-watch.exc","");
	}
	if(LCD_PROC==0) {
		if(file_exists("/var/sys/chk_lcdd") && lcdd==0) {
			fork_exec("/bin/lcdd","");
		}
	} else {
		if(file_exists("/var/sys/chk_lcdg")) {
			if(lcds==0 || lcdp==0 || lcde==0) xsystem("/etc/lcreset");
		}
	}
	if(file_exists("/var/sys/chk_nmbd") && nmbd==0) {
		fork_exec("/bin/nmbd","-D");
	}
	if(file_exists("/var/sys/chk_winbindd") && winbindd==0) {
		fork_exec("/bin/winbindd","");
	}
	if(file_exists("/var/sys/chk_crond") && crond==0) {
		fork_script("/service/init/cron.init","restart");
	}
	if(file_exists("/var/sys/load_balancing") && failoverd==0) {
		fork_exec("/bin/failoverd","");
	}
	sleep(10);
        exit(0);
}
