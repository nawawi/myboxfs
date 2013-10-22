#include <libmybox.h>

static int dhcp_stat=0;
static int ftp_stat=0;
static int ssh_stat=0;
static int auth_ad=0;
static int ddns_stat=0;
static int pptp_stat=0;
static int snmp_stat=0;
static int shaper_stat=0;
static int ips_stat=0;
static int https_stat=0;

static int nonetwork=0;
static int numdev=0;
static int DO_SINGLE=0;

void network_touch(void) {
	int pt=0,x=0;
	fprintf_stdout("* Detecting network interfaces: ");
	pt=get_eth();
	lcd_msg("SYSTEM LOADING..","-> CHECKING NIC");
	if(pt!=0) {
		for(x=0;x<pt;x++) {
			fprintf_stdout("%s ",eth_list[x].name);
		}
		putchar('\n');
		if(pt < 4) pt=4;
		save_to_file("/var/sys/numnet","%d\n",pt);
		numdev=pt;
	} else {
		fprintf_stdout("* No network device found\n");
		lcd_msg("SYSTEM LOADING..","-> NO NIC FOUND");
		xtouch("/var/sys/nonetwork");
		nonetwork=1;
	}
}

static void chk_prog_stat(void) {
        sqlite *db_id;
        int x=0;
        db_id=db_connect(DB_NAME);
        db_query(db_id,"select service_dhcp.stat,service_ftp.stat,service_ssh.stat,auth_ad.stat,service_ddns.stat,pptp_vpn.stat,service_snmp.stat,service_https.stat from service_dhcp,service_ftp,service_ssh,auth_ad,service_ddns,pptp_vpn,service_snmp,service_https");
        if(SQL_NUMROWS!=0) {
                for(x=0;x<SQL_NUMROWS;x++) {
                        if(!strcmp(SQL_RESULT[x].name,"service_dhcp.stat") && atoi(SQL_RESULT[x].value)==1) dhcp_stat=1;
                        if(!strcmp(SQL_RESULT[x].name,"service_ftp.stat") && atoi(SQL_RESULT[x].value)==1) ftp_stat=1;
                        if(!strcmp(SQL_RESULT[x].name,"service_ssh.stat") && atoi(SQL_RESULT[x].value)==1) ssh_stat=1;
                        if(!strcmp(SQL_RESULT[x].name,"auth_ad.stat") && atoi(SQL_RESULT[x].value)==1) auth_ad=1;
                        if(!strcmp(SQL_RESULT[x].name,"service_ddns.stat") && atoi(SQL_RESULT[x].value)==1) ddns_stat=1;
                        if(!strcmp(SQL_RESULT[x].name,"pptp_vpn.stat") && atoi(SQL_RESULT[x].value)==1) pptp_stat=1;
                        if(!strcmp(SQL_RESULT[x].name,"service_snmp.stat") && atoi(SQL_RESULT[x].value)==1) snmp_stat=1;
                        if(!strcmp(SQL_RESULT[x].name,"service_https.stat") && atoi(SQL_RESULT[x].value)==1) https_stat=1;
                }
        }
	db_clean_buffer();
	db_query(db_id,"select * from htb_client");
	if(SQL_NUMROWS!=0) shaper_stat=1;
	db_clean_buffer();
	db_query(db_id,"select val from misc where name='ids_stat'");
	if(SQL_NUMROWS!=0) {
		ips_stat=atoi(SQL_RESULT[0].value);
	}
        db_close(db_id);
}

static void wait_prog(const char *msg, const char *path,const char *prog, const char *argz) {
	FILE *f;
	char spin[]={'-','\\','|','/','-','\\','|','/'};
	int numspin=sizeof(spin);
	int i=0, ok=0, pt=0;
	char filetmp[1024], buf[1024];
	memset(filetmp,0x0,sizeof(filetmp));
	snprintf(filetmp,sizeof(filetmp),"/tmp/%s.log",prog);
	save_to_file(filetmp,"");
	xsystem("php -q %s/%s %s >> %s 2>&1 && rm -f %s >>/dev/null 2>&1 &",path,prog,argz,filetmp,filetmp);
	while(file_exists(filetmp)) {
		for(i=0;i<numspin;i++) {
			fprintf_stdout("\r-> %s [%c]",msg,spin[i]);
			usleep(100000);
		}
		memset(buf,0x0,sizeof(buf));
		if((f=fopen(filetmp,"r"))!=NULL) {
			while(fgets(buf,sizeof(buf),f)!=NULL) {
				trim(buf);
				if(buf[0]=='\0') continue;
				if(strstr(buf,"unlicensed")) {
					fprintf_stdout("\r* %s Unlicensed.\n",msg);
					unlink(filetmp);ok=1;
				} else if(strstr(buf,"Internal error")) {
					fprintf_stdout("\r* %s Internal error.\n",msg);
					unlink(filetmp);ok=1;
				} else if(strstr(buf,"failed") || strstr(buf,"Failed") ) {
					fprintf_stdout("\r* %s Failed.\n",msg);
					unlink(filetmp);ok=1;
				} else if(strstr(buf,"disabled") || strstr(buf,"Disabled")) {
					fprintf_stdout("\r* %s Disabled.\n",msg);
					unlink(filetmp);ok=1;
				}
				memset(buf,0x0,sizeof(buf));	
			}
			fclose(f);
		}
		// more than 1 minutes
		if(pt > 60) {
			fprintf_stdout("\r* %s Timeout.\n",msg);
			unlink(filetmp);ok=1;
		}
		pt++;
		sleep(1);
	}
	if(ok==0) {
		fprintf_stdout("\r* %s Done.\n",msg);
		unlink(filetmp);
	}
}

void startprog(void) {
	char host[40];

	xsystem("mkdir -p /tmp/sessions");
	chmod("/tmp/sessions",S_IREAD | S_IWRITE | S_IEXEC);
	save_to_file("/etc/resolv.conf","");
	xtouch("/var/sys/init_no_restart");
	xtouch("/var/sys/init_start");
	xsystem("ps |grep getkey |awk '{print $1}' |xargs kill -9 >/dev/null 2>&1");
	chk_prog_stat();
	if(file_exists("/service/init/misc.init")) {
		if(xsystem("/service/init/misc.init keymap >/dev/null 2>&1")==0) {
			fprintf_stdout("* Setting keymap. Done.\n");
			lcd_msg("SYSTEM SETTING..","-> KEYMAP");
		}
		if(xsystem("/service/init/misc.init clock >/dev/null 2>&1")==0) {
			fprintf_stdout("* Setting clock: ");
			print_date();
			lcd_msg("SYSTEM SETTING..","-> CLOCK");
		}
		if(xsystem("/service/init/misc.init dnshost >/dev/null 2>&1")==0) {
			gethostname(host,sizeof(host));
			fprintf_stdout("* Setting system hostname: %s\n",host);
			lcd_msg("SYSTEM SETTING..","-> HOSTNAME");
		}
	}
	xsystem("iptables -P INPUT DROP >/dev/null 2>&1");
	xsystem("iptables -P FORWARD DROP >/dev/null 2>&1");
	xsystem("iptables -P OUTPUT DROP >/dev/null 2>&1");
	xsystem("iptables -A INPUT -i lo -s 127.0.0.1 -j ACCEPT >/dev/null 2>&1");
	xsystem("iptables -A INPUT -o lo -s 127.0.0.1 -j ACCEPT >/dev/null 2>&1");
	
	if(nonetwork==0) {
		if(file_exists("/service/init/network.init")) {
			lcd_msg("START SERVICES..","-> NETWORKING");
			wait_prog("Bringing up network interfaces.","/service/init","network.init","start");
		}
		if(file_exists("/service/init/dns.init")) {
			lcd_msg("START SERVICES..","-> DNS RESOLVER");
			wait_prog("Starting DNS Resolver Agent.","/service/init","dns.init","start");
		}
		if(file_exists("/service/init/policy.init")) {
			lcd_msg("START SERVICES..","-> PACKET FILTER");
			wait_prog("Applying packet filter rules.","/service/init","policy.init","start");
		}
	}
	if(file_exists("/service/init/syslog.init")) {
		lcd_msg("START SERVICES..","-> SYSTEM LOGGER");
		wait_prog("Starting Syslog.","/service/init","syslog.init","start");
	}
	if(file_exists("/service/tools/mfs-query.exc")) {
		lcd_msg("START SERVICES..","-> MYBOX AGENTS");
		fprintf_stdout("* Starting Mybox Agents: ");
		xsystem("/service/tools/mfs-query.exc q >/dev/null 2>&1");
		fprintf_stdout(" query");
		xsystem("/service/tools/mfs-query.exc i >/dev/null 2>&1");
		fprintf_stdout(" ips");
		xsystem("/service/tools/mfs-query.exc o >/dev/null 2>&1");
		fprintf_stdout(" others");
		putchar('\n');
	}
	if(nonetwork==0) {
		if(snmp_stat==1) {
			if(file_exists("/service/init/snmp.init")) {
				lcd_msg("START SERVICES..","-> SNMP AGENT");
				wait_prog("Starting SNMP Agent.","/service/init","snmp.init","start");
			}
		}
		if(ips_stat==1) {
			if(file_exists("/service/init/ips.init")) {
				lcd_msg("START SERVICES..","-> IPS AGENT");
				wait_prog("Starting IPS Agent.","/service/init","ips.init","start");
			}
		}
		if(auth_ad==1) {
			if(file_exists("/service/init/auth.init")) {
				lcd_msg("START SERVICES..","-> AUTH AD AGENT");
				wait_prog("Starting Auth AD Agent.","/service/init","auth.init","start");
			}
		}
		if(pptp_stat==1) {
			if(file_exists("/service/init/pptp.init")) {
				lcd_msg("START SERVICES..","-> PPTP VPN");
				wait_prog("Starting PPTP VPN Agent.","/service/init","pptp.init","start");
			}
		}
		if(dhcp_stat==1) {
			if(file_exists("/service/init/dhcp.init")) {
				lcd_msg("START SERVICES..","-> DHCP AGENT");
				wait_prog("Starting DHCP Agent.","/service/init","dhcp.init","start");
			}
		}
		if(ftp_stat==1) {
			if(file_exists("/service/init/ftp.init")) {
				lcd_msg("START SERVICES..","-> FTP AGENT");
				wait_prog("Starting FTP Agent.","/service/init","ftp.init","start");
			}
		}
		if(https_stat==1) {
			if(file_exists("/service/init/https.init")) {
				lcd_msg("START SERVICES..","-> MYADMIN AGENT");
				wait_prog("Starting MyAdmin HTTPS Agent.","/service/init","https.init","start");
			}
		}
		if(ssh_stat==1) {
			if(file_exists("/service/init/ssh.init")) {
				lcd_msg("START SERVICES..","-> SSH AGENT");
				wait_prog("Starting SSH Agent.","/service/init","ssh.init","start");
			}
		}
		if(ddns_stat==1) {
			if(file_exists("/service/init/ddns.init")) {
				lcd_msg("START SERVICES..","-> DYNAMIC DNS");
				wait_prog("Starting Dynamic DNS Agent.","/service/init","ddns.init","start");
			}
		}
		if(shaper_stat==1) {
			if(file_exists("/service/init/shaper.init")) {
				lcd_msg("START SERVICES..","-> SHAPER");
				wait_prog("Starting bandwidth shaper.","/service/init","shaper.init","start");
			}
		}
		if(file_exists("/service/init/misc.init")) {
			lcd_msg("START SERVICES..","-> STATIC ARP");
			wait_prog("Starting static ARP.","/service/init","misc.init","staticarp");
		}
	}
	if(file_exists("/service/tools/mfs-graph.exc")) {
		lcd_msg("START SERVICES..","-> GRAPH");
		wait_prog("Creating diagnostic graphs.","/service/tools","mfs-graph.exc","--");
	}
	if(file_exists("/service/tools/mfs-query.exc")) {
		lcd_msg("START SERVICES..","-> GRAPH");
		wait_prog("Compressing logs","/service/tools","mfs-query.exc","l");
	}
	if(file_exists("/service/init/cron.init")) {
		lcd_msg("START SERVICES..","-> SCHEDULER");
		wait_prog("Starting scheduler Agent.","/service/init","cron.init","start");
	}
	unlink("/var/sys/init_no_restart");
}

int initrc_main(int argc, char **argv) {
	DIR *dp;
	struct dirent *dt;
	char cmd[1024];
	(void) signal(SIGINT,SIG_IGN);
	freopen("/dev/null", "w", stderr);
	putenv("PATH=/bin");
	putenv("TERM=linux");
	umask(0770);
	chdir("/");
	if(file_exists("/strg/.nostrg")) {
		unlink("/strg/.nostrg");
		DO_SINGLE=1;
	}
	save_to_file("/proc/sys/kernel/printk","0 0 0 0\n");
	//clean rc.bootup stuff
	umount("/initrd/sys");
	umount("/initrd");
	xsystem("rm -rf /initrd /bootup");
	save_to_file("/bin/groups","#!/bin/sh\necho \"mfs\"\n");
	chmod("/bin/groups",S_IREAD | S_IWRITE | S_IEXEC);
	if(is_dir("/tmp/tools")) {
		if((dp=opendir("/tmp/tools")) == NULL) {
			perror("opendir");
			exit(1);
        	}
		fprintf_stdout("-> Extracting helper tools.");
		while((dt=readdir(dp))!=NULL) {
			if(!strcmp(dt->d_name,".") || !strcmp(dt->d_name,"..")) continue;
			if(strstr(dt->d_name,".bz2")) {
				fprintf_stdout(".");
				xsystem("tar -C / -jxf /tmp/tools/%s",dt->d_name);
				usleep(10000);
			}
		}
		free(dt);
		closedir(dp);
		fprintf_stdout("\r* Extracting helper tools. Done.%s\n",SPACE);
	}
	memset(cmd,0x0,sizeof(cmd));
	if(is_dir("/tmp/modules")) {
		if((dp=opendir("/tmp/modules")) == NULL) {
			perror("opendir");
			exit(1);
        	}
		fprintf_stdout("-> Extracting system modules.");
		while((dt=readdir(dp))!=NULL) {
			if(!strcmp(dt->d_name,".") || !strcmp(dt->d_name,"..")) continue;
			if(strstr(dt->d_name,".bz2")) {
				fprintf_stdout(".");
				xsystem("tar -C / -jxf /tmp/modules/%s",dt->d_name);
				usleep(10000);
			}
		}
		free(dt);
		closedir(dp);
		fprintf_stdout("\r* Extracting system modules. Done.%s\n",SPACE);
	}
	memset(cmd,0x0,sizeof(cmd));
	if(file_exists("/etc/microcode.dat")) xsystem("/bin/microcode_ctl -Qui");
	// chk network device
	network_touch();
	//default domain
	save_to_file("/proc/sys/kernel/hostname","fw.mybox.local\n");
	xsystem("chmod 700 /service/www/*.html");
	xsystem("chmod 700 /service/tools/*.exc");
	//if(DO_SINGLE==0) startprog();
	if(DO_SINGLE==0 && file_exists("/etc/rc.start/sh")) {
		xsystem("/etc/rc.start/sh");
	}
	// post-boot
	if(file_exists("/strg/mybox/post-boot") && DO_SINGLE==0) {
		chmod("/strg/mybox/post-boot",S_IREAD | S_IWRITE | S_IEXEC);
		fprintf_stdout("* Executing post-boot: /strg/mybox/post-boot\n");
		xsystem("/strg/mybox/post-boot >/dev/null 2>&1");
	}
	// clean space
	xsystem("rm -rf /etc/inittab /etc/microcode.dat /bin/microcode_ctl /strg/mybox/download/* /var/sys/init_no_restart /bin/initrc /tmp/* /usr/share/fonts /var/sys/init_start /etc/rc.start");
	//clean unuse modules
	if(file_exists("/service/tools/mfs-rmmod.exc")) {
		xsystem("/bin/php -f /service/tools/mfs-rmmod.exc >/dev/null 2>&1");
	}
	if(file_exists("/etc/rc.sysctl/sh")) {
		fprintf_stdout("* Setting kernel parameters. Done.\n");
		lcd_msg("SYSTEM LOADING..","-> TUNE KERNEL");
		xsystem("/etc/rc.sysctl/sh >/dev/null 2>&1");
	}
	//console session
	xsystem("mkdir -p /tmp/console.session");
	if(is_dir("/strg/mybox/")) {
		xsystem("dmesg -c -s 131072 > /strg/mybox/boot.msg");
	}
	xflush_stdout();
	(void) signal(SIGINT,SIG_DFL);
	if(DO_SINGLE==1) run_shell();
	if(DO_SINGLE==1) xsystem("/bin/reboot");
	save_to_file("/proc/sys/kernel/printk","6 0 0 0\n");
	xsystem("chmod 700 /*");
	if(file_exists("/dev/lcd") && file_exists("/bin/lcdd")) {
		xsystem("/bin/lcdd");
		xtouch("/var/sys/chk_lcdd");
	}
	exit(0);
}


