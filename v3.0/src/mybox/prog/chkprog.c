#include "libmybox.h"

static int LCD_PROC=-1;

static int syslog_ng=0;
static int ulogd=0;
static int queryo=0;
static int snmpd=0;
static int snortd=0;
static int snortcnt=0;
static int dhcpd=0;
static int dhcpr=0;
static int pptpd=0;
static int lcdd=0;
static int nmbd=0;
static int winbindd=0;
static int crond=0;
static int uplinkd=0;
static int ddnsd=0;
static int named=0;
static int watchd=0;
static int lcds=0;
static int lcdp=0;
static int lcde=0;
static int taskq=0;
static int captived=0;
static int ntpd=0;
static int barnyard=0;
static int clamd=0;
static int squid=0;
static int frox=0;
static int xinetd=0;
static int sophosd=0;
static int spamd=0;
static int p3scan=0;

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
		if(strstr(str,"mfs-query.exc o")) queryo++;
		if(strstr(str,"snmpd")) snmpd=1;
		if(strstr(str,"syslog-ng")) syslog_ng=1;
		if(strstr(str,"ulogd")) ulogd=1;
		if(strstr(str,"snortd")) snortd++;
		if(strstr(str,"barnyard")) barnyard=1;
		if(strstr(str,"dhcpd")) dhcpd=1;
		if(strstr(str,"dhcrelay")) dhcpr=1;
		if(strstr(str,"pptpd")) pptpd=1;
		if(strstr(str,"nmbd")) nmbd=1;
		if(strstr(str,"winbindd")) winbindd=1;
		if(strstr(str,"crond")) crond=1;
		if(strstr(str,"uplinkd")) uplinkd=1;
		if(strstr(str,"ddnsd")) ddnsd=1;
		if(strstr(str,"named")) named=1;
		if(strstr(str,"mfs-watch.exc")) watchd=1;
		if(strstr(str,"taskq")) taskq=1;
		if(strstr(str,"captived")) captived=1;
		if(strstr(str,"ntpd")) ntpd=1;
		if(strstr(str,"clamd")) clamd=1;
		if(strstr(str,"squid")) squid=1;
		if(strstr(str,"frox")) frox=1;
		if(strstr(str,"xinetd")) xinetd=1;
		if(strstr(str,"sophosd")) sophosd=1;
		if(strstr(str,"spamd")) spamd=1;
		if(strstr(str,"p3scan")) p3scan=1;
		if(LCD_PROC==0) {
			if(strstr(str,"lcdd")) lcdd=1;
		} else if(LCD_PROC==1) {
			if(strstr(str,"lcds")) lcds=1;
			if(strstr(str,"lcdp")) lcdp=1;
			if(strstr(str,"lcde")) lcde=1;
		}
        }
        return 0;
}

static void do_task(char *prog,char *task) {
	char txt[256];
	if(strcmp(prog,"none")) {
		snprintf(txt,sizeof(txt),"Service down: %s",prog);
		log_action("WARNING",txt);sleep(1);
	}
	append_to_file("/var/sys/mfsque","%s\n",task);
}

void ips_clear_iptab(void) {
	FILE *fp;
	if((fp=popen("/bin/iptables-restore","w"))!=NULL) {
		fprintf(fp,"*traceips\n");
		fprintf(fp,":PREROUTING ACCEPT [0:0]\n");
		fprintf(fp,":INPUT ACCEPT [0:0]\n");
		fprintf(fp,":FORWARD ACCEPT [0:0]\n");
		fprintf(fp,":OUTPUT ACCEPT [0:0]\n");
		fprintf(fp,":POSTROUTING ACCEPT [0:0]\n");
		fprintf(fp,"COMMIT\n");
		pclose(fp);
	}
}

static void clear_set(void) {
	syslog_ng=0;
	ulogd=0;
	queryo=0;
	snmpd=0;
	snortd=0;
	snortcnt=0;
	dhcpd=0;
	dhcpr=0;
	pptpd=0;
	lcdd=0;
	nmbd=0;
	winbindd=0;
	crond=0;
	uplinkd=0;
	ddnsd=0;
	named=0;
	watchd=0;
	lcds=0;
	lcdp=0;
	lcde=0;
	taskq=0;
	captived=0;
	ntpd=0;
	barnyard=0;
	clamd=0;
	squid=0;
	frox=0;
	xinetd=0;
	sophosd=0;
	spamd=0;
	p3scan=0;
}

static void chkrunsnort(void) {
	snortcnt=0;
	if(file_exists("/strg/mybox/patterns/ips.conf")) snortcnt++;
	if(file_exists("/strg/mybox/patterns/ips-im.conf")) snortcnt++;
	if(file_exists("/strg/mybox/patterns/ips-p2p.conf")) snortcnt++;
}

static void exit_program(int val) {
	exit(val);
}


void run_check(void) {
	/*if(file_exists("/var/sys/chk_syslog-ng") && syslog_ng==0) {
		do_task("Syslog","syslog-restart");
	} else if(file_exists("/var/sys/chk_syslog-ng") && ulogd==0) {
		do_task("Syslog","syslog-restart");
	}*/
	if(file_exists("/var/sys/chk_syslog-ng")) {
		if(syslog_ng==0) {
			do_task("Syslog","syslog-up");
		}
		if(ulogd==0) {
			do_task("Syslog","ulogd-up");
		}
	}

	if(queryo==0) {
		if(file_exists("/var/sys/chk_mfs-query-o")) {
			do_task("Mybox Agent","mfs-query");
		}
	}

	if((snortd==0 || snortd < snortcnt) || (barnyard==0)) {
		if(file_exists("/var/sys/chk_snortd")) {
			ips_clear_iptab();
			do_task("IPS","ips-restart");
		} else {
			if(file_exists("/var/sys/ips_tab.done")) {
				ips_clear_iptab();
				unlink("/var/sys/ips_tab.done");
			}
		}
	}
	if(file_exists("/var/sys/chk_snmpd") && snmpd==0) {
		do_task("SNMP","snmp-restart");
	}
	if(file_exists("/var/sys/chk_captived") && captived==0) {
		do_task("Captive Portal","captive-up");
	}
	if(file_exists("/var/sys/chk_dhcpd") && dhcpd==0) {
		do_task("DHCP","dhcp-restart");
	}
	if(file_exists("/var/sys/chk_dhcp_relay") && dhcpr==0) {
		do_task("DHCP","dhcp_relay-restart");
	}
	if(file_exists("/var/sys/chk_pptpd") && pptpd==0) {
		do_task("PPTP","pptp-restart");
	}
	if(file_exists("/var/sys/chk_ddnsd") && ddnsd==0) {
		do_task("none","ddns-restart");
	}
	if(file_exists("/var/sys/chk_named") && named==0) {
		do_task("none","dns-restart");
	}
	if(file_exists("/var/sys/chk_ntpd") && ntpd==0) {
		do_task("none","ntp-restart");
	}
	if(LCD_PROC==0) {
		if(file_exists("/var/sys/chk_lcdd") && lcdd==0) {
			do_task("none","mfs-lcdd");
		}
	} else {
		if(file_exists("/var/sys/chk_lcdg")) {
			if(lcds==0 || lcdp==0 || lcde==0) do_task("none","mfs-lcdp");
		}
	}
	if(file_exists("/var/sys/chk_nmbd") && nmbd==0) {
		do_task("none","mfs-nmbd");
	}
	if(file_exists("/var/sys/chk_winbindd") && winbindd==0) {
		do_task("none","mfs-winbindd");
	}
	if(file_exists("/var/sys/chk_crond") && crond==0) {
		do_task("CRON","cron-restart");
	}
	if(file_exists("/var/sys/uplink_chk") && uplinkd==0) {
		do_task("none","mfs-uplinkd");
	}
	if(file_exists("/var/sys/ad_join_ko")) {
		do_task("AUTH","auth-ad-restart");
	}
	if(file_exists("/var/sys/chk_clamd") && clamd==0) {
		do_task("CLAM AV","clam-restart");
	}
	if(file_exists("/var/sys/chk_sophosd") && sophosd==0) {
		do_task("SOPHOS AV","sophos-up");
	}
	if(file_exists("/var/sys/chk_frox") && frox==0) {
		do_task("FTP Proxy","ftp_proxy-restart");
	}
	if(file_exists("/var/sys/chk_squid") && squid==0) {
		do_task("HTTP Proxy","http_proxy-up");
	}
	if(file_exists("/var/sys/chk_xinetd") && xinetd==0) {
		do_task("XINETD","xinet-up");
	}
	if(file_exists("/var/sys/chk_spamd") && spamd==0) {
		do_task("SPAM Filter","spam-up");
	}
	if(file_exists("/var/sys/chk_p3scan") && p3scan==0) {
		do_task("MAIL Proxy","mail-up");
	}
	if(taskq==0) {
		xtouch("/var/sys/taskq_down");
	}
}
int chkprog_main(int argc,char **argv) {
	freopen("/dev/null", "r", stdin);
      	freopen("/dev/null", "w", stdout);
      	freopen("/dev/null", "w", stderr);

	strcpy(argv[0],"chkprog    ");

	if(file_exists("/var/sys/lcd_proc")) {
		LCD_PROC=1;
	} else if(file_exists("/var/sys/lcd_com")) {
		LCD_PROC=0;
	}
	chdir("/");
	for(;;) {
		clear_set();
		chkrunsnort();
		chkpid();
		run_check();
		sleep(10);
	}
        exit(0);
}

