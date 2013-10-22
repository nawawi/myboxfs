#include "libmybox.h"
#define _XOPEN_SOURCE
struct listx {
	char cmd[150];
} listq[150];

static listcnt=0;

typedef struct {
	char name[150];
	char cmd[50];
} QUERY_CMD;

QUERY_CMD qcommands[] = {
	// SNMP
	{"snmp-restart","snmp.init restart"},
	{"snmp-restart-quiet","snmp.init restart quiet"},
	// SYSLOG
	{"syslog-restart","syslog.init restart"},
	{"syslog-restart-quiet","syslog.init restart quiet"},
	{"syslog-up","/bin/syslog-ng"},
	{"ulogd-up","/bin/ulogd -d"},
	// PPTP
	{"pptp-restart","pptp.init restart"},
	{"pptp-start","pptp.init start"},
	{"pptp-restart-quiet","pptp.init restart quiet"},
	{"pptp-start-quiet","pptp.init start quiet"},
	// NETWORK
	{"network-restart","network.init restart"},
	{"network-restart-quiet","network.init restart quiet"},
	{"network-stop","network.init stop"},
	// IPS
	{"ips-restart","ips.init restart"},
	{"ips-restart-quiet","ips.init restart quiet"},
	// POLICY
	{"policy-restart","policy.init restart"},
	{"policy-restart-quiet","policy.init restart quiet"},
	{"policy-restart-nomod","policy.init restart nomod"},
	{"policy-clear","policy.init clear"},
	{"policy-dos","policy.init dos"},
	{"policy-blacklist","policy.init blacklist"},
	{"policy-ips","policy.init ips"},
	{"policy-acl","policy.init acl"},
	{"policy-pscan","policy.init pscan"},
	{"policy-ips-all","policy.init ips-all"},
	{"policy-mod","policy.init mod"},
	{"policy-lb","policy.init lb"},
	{"policy-nat","policy.init nat"},
	// CRON
	{"cron-restart","cron.init restart"},
	{"cron-restart-quiet","cron.init restart quiet"},
	// SHAPER
	{"shaper-restart","shaper.init restart"},
	{"shaper-restart-quiet","shaper.init restart quiet"},
	// ROUTE FLUSH
	{"route-flush-cache","ip route flush cache"},
	// MISC
	{"update-dnshost","initconf.init dnshost"},
	{"update-dnshost-quiet","initconf.init dnshost quiet"},
	{"update-clock","initconf.init clock"},
	{"update-clock-quiet","initconf.init clock quiet"},
	// DHCP
	{"dhcp-restart","dhcp.init restart"},
	{"dhcp-restart-quiet","dhcp.init restart quiet"},
	// DHCP Relay
	{"dhcp_relay-restart","dhcp_relay.init restart"},
	{"dhcp_relay-restart-quiet","dhcp_relay.init restart quiet"},
	// AUTH
	{"auth-ad-restart","auth.init ad_restart"},
	{"auth-ad-restart-quiet","auth.init ad_restart quiet"},
	// DDNS
	{"ddns-restart","ddns.init restart"},
	{"ddns-restart-quiet","ddns.init restart quiet"},
	// DNS
	{"dns-restart","dns.init restart"},
	{"dns-restart-quiet","dns.init restart quiet"},
	// ARP FLUSH
	{"arp-flush-cache","mfs-query.exc f"},
	// CAPTIVE PORTAL
	{"captive-restart","captive.init restart"},
	{"captive-restart-quite","captive.init restart quite"},
	{"captive-up","/bin/captived /etc/captived.cnf"},
	// NTP
	{"ntp-restart","ntp.init restart"},
	{"ntp-restart-quiet","ntp.init restart quiet"},
	// CLAM
	{"clam-restart","clam.init restart"},
	{"clam-restart-quiet","clam.init restart quiet"},
	// SQUID
	{"http_proxy-restart","http_proxy.init restart"},
	{"http_proxy-restart-quiet","http_proxy.init restart quiet"},
	{"http_proxy-up","/bin/squid -DY"},
	// FROX
	{"ftp_proxy-restart","ftp_proxy.init restart"},
	{"ftp_proxy-restart-quiet","ftp_proxy.init restart quiet"},
	// P3SCAN
	{"mail_proxy-restart","mail_proxy.init restart"},
	{"mail_proxy-restart-quiet","mail_proxy.init restart quiet"},
	{"mail-up","rm /var/run/p3scan.pid && /bin/p3scan"},
	// XINET
	{"xinet-restart","xinet.init restart"},
	{"xinet-restart-quiet","xinet.init restart quiet"},
	{"xinet-reload","xinet.init reload"},
	{"xinet-reload-quiet","xinet.init reload quiet"},
	{"xinet-up","rm /var/run/xinetd.pid && /bin/xinetd -stayalive -pidfile /var/run/xinetd.pid"},
	// SOPHOS
	{"sophos-restart","sophos.init restart"},
	{"sophos-restart-quiet","sophos.init restart quiet"},
	{"sophos-reload","sophos.init reload"},
	{"sophos-reload-quiet","sophos.init reload quiet"},
	{"sophos-update","sophos_update"},
	{"sophos-up","/bin/sophosd -D"},
	// SPAMD
	{"spam-restart","spam.init restart"},
	{"spam-restart-quiet","spam.init restart quiet"},
	{"spam-reload","spam.init reload"},
	{"spam-reload-quiet","spam.init reload quiet"},
	{"spam-up","rm /var/run/spam.pid && /bin/spamd -d -r /var/run/spam.pid"},
	// SYSTEM
	{"mfs-reboot","reboot -d 5"},
	{"mfs-shutdown","poweroff -d 5"},
	{"mfs-query","mfs-query.exc o"},
	{"mfs-lcdd","lcdd"},
	{"mfs-lcdp","/etc/lcreset"},
	{"mfs-nmbd","nmbd -D"},
	{"mfs-winbindd","winbindd"},
	{"mfs-uplinkd","uplinkd"},
	{"mfs-firmware-update","mfs-update.exc update-firmware"},
	{"mfs-firmware-download","mfs-update.exc download-firmware"},
	{"mfs-chk-update","mfs-update.exc check"},
	{ (char *)NULL,(char *)NULL }
};

static void run_query(char *prog) {
	int i=0, ret=0;
	char whatcmd[150];
	for(i = 0; qcommands[i].name; i++) {
		if(!strcmp(qcommands[i].name,prog)) {
			snprintf(whatcmd,sizeof(whatcmd),"%s",qcommands[i].cmd);
			ret=1;
			break;
		}
	}
	if(ret==1) {
		system(whatcmd);
	}
}

static void run_list(void) {
	int i=0;
	for(i=0;i<listcnt;i++) {
		run_query(listq[i].cmd);
		usleep(72500); // cpu tune
	}
}

static int read_config(void) {
	FILE *f;
	char buff[150];
	int i=0;
	if(!file_exists("/var/sys/mfsque")) return 0;
	if(rename("/var/sys/mfsque","/var/sys/mfsque.p")==0) {
		if((f=fopen("/var/sys/mfsque.p","r"))!=NULL) {
			while(fgets(buff, sizeof(buff), f)) {
				trim(buff);
				if(buff[0]!='\0') {
					snprintf(listq[i].cmd,sizeof(listq[i].cmd),"%s",buff);
					i++;
				}
				buff[0]=0;
			}
			fclose(f);
		}	
	}
	unlink("/var/sys/mfsque.p");
	return i;
}

static void exit_program(int val) {
	exit(val);
}

static void free_list(void) {
	int i;
	for(i=0;i<listcnt;i++) {
		memset(listq[i].cmd,0x0,sizeof(listq[i].cmd));
	}
	listcnt=0;
}

int taskq_main(int argc,char **argv) {
	putenv("PATH=/bin:/service/tools:/service/init");

	signal(SIGINT, exit_program);
        signal(SIGTERM, exit_program);
        signal(SIGHUP, exit_program);
        signal(SIGKILL, exit_program);

	freopen("/dev/null", "r", stdin);
      	freopen("/dev/null", "w", stdout);
      	freopen("/dev/null", "w", stderr);

	strcpy(argv[0],"taskq     ");

	chdir("/");
	if(daemon(1,1)==0) {
		for(;;) {
			listcnt=read_config();
			if(listcnt!=0) {
				run_list();
				free_list();	
			}
			sleep(1);
		}
	}
	exit(0);
}
