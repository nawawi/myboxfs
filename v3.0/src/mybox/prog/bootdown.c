#include "libmybox.h"

static int LCD_PROC=-1;
void network_down(void) {
	int pt=0,x=0;
	lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> NETWORK OFF");
	if(file_exists("/var/sys/ipconfig.cache")) printf("* Shutting down network interfaces\n");
	pt=get_eth();
	if(pt!=0) {
		for(x=0;x<pt;x++) {
			xsystem("ip link set %s down",eth_list[x].name);
		}
 		xsystem("ip route flush cache");
                xsystem("ip route flush table main");
	}
}

static void clear_iptab(void) {
        FILE *fp;
        if((fp=popen("/bin/iptables-restore","w"))!=NULL) {
                fprintf(fp,"*traceips\n");
                fprintf(fp,":PREROUTING ACCEPT [0:0]\n");
                fprintf(fp,":INPUT ACCEPT [0:0]\n");
                fprintf(fp,":FORWARD ACCEPT [0:0]\n");
                fprintf(fp,":OUTPUT ACCEPT [0:0]\n");
                fprintf(fp,":POSTROUTING ACCEPT [0:0]\n");
                fprintf(fp,"COMMIT\n");
                fprintf(fp,"*nat\n");
                fprintf(fp,":PREROUTING ACCEPT [0:0]\n");
                fprintf(fp,":POSTROUTING ACCEPT [0:0]\n");
                fprintf(fp,":OUTPUT ACCEPT [0:0]\n");
                fprintf(fp,"COMMIT\n");
                fprintf(fp,"*mangle\n");
                fprintf(fp,":PREROUTING ACCEPT [0:0]\n");
                fprintf(fp,":INPUT ACCEPT [0:0]\n");
                fprintf(fp,":FORWARD ACCEPT [0:0]\n");
                fprintf(fp,":OUTPUT ACCEPT [0:0]\n");
                fprintf(fp,":POSTROUTING ACCEPT [0:0]\n");
                fprintf(fp,"COMMIT\n");
                fprintf(fp,"*raw\n");
                fprintf(fp,":PREROUTING ACCEPT [0:0]\n");
                fprintf(fp,":OUTPUT ACCEPT [0:0]\n");
                fprintf(fp,"COMMIT\n");
                fprintf(fp,"*filter\n");
                fprintf(fp,":INPUT DROP [0:0]\n");
                fprintf(fp,":FORWARD DROP [0:0]\n");
                fprintf(fp,":OUTPUT DROP [0:0]\n");
        	fprintf(fp,"-A INPUT -s 127.0.0.1 -i lo -j ACCEPT\n");
        	fprintf(fp,"-A OUTPUT -s 127.0.0.1 -j ACCEPT\n");
        	fprintf(fp,"COMMIT\n");
                pclose(fp);
        }
}

static void run_php_exec(char *cmd) {
	char fbuf[1000];
	char buf[1000];
	FILE *p;
	if((p=popen(cmd,"r"))!=NULL) {
        	while(fgets(fbuf, sizeof(fbuf), p)) {
			snprintf(buf,sizeof(buf),"%s",trim(fbuf));
			if(buf[0]!='\0') {
				printf("%s\n",buf);
			}
			buf[0]=0;
		}
		fbuf[0]=0;
		pclose(p);
	}
}

int bootdown_main(int argc, char **argv) {
	char cmd[1024], buf[1024], name[1024];
	time_t curtime;
  	struct tm *loctime;
	signal(SIGINT,SIG_IGN);
	putenv("PATH=/bin");
	putenv("TERM=linux");
	chdir("/");
	xsystem("/etc/init.boot/bootdown_alert");
	xtouch("/var/sys/init_no_restart");
	ips_clear_iptab();
 	clear_iptab();
	xsystem("/bin/lanbypass > /dev/null 2>/dev/null");
	if(file_exists("/var/sys/lcd_proc")) {
		LCD_PROC=1;
	} else if(file_exists("/var/sys/lcd_com")) {
		LCD_PROC=0;
	}
	save_to_file("/proc/sys/kernel/printk","0 0 0 0\n");
	save_to_file("/etc/resolv.conf","\n");
	
	putchar('\n');
	if(file_exists("/var/sys/chk_crond")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> SCHEDULER");
		run_php_exec("/service/init/cron.init stop 2>/dev/null");
	}
	if(file_exists("/var/sys/chk_mfs-query-o")) {
		unlink("/var/sys/chk_mfs-query-o");
		xsystem("pkill -9 -f \"mfs-query.exc o\" > /dev/null 2>/dev/null"); 
	}
	xsystem("pkill -9 -f taskq > /dev/null 2>/dev/null");

	if(file_exists("/var/sys/chk_nmbd") || file_exists("/var/sys/chk_winbindd")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> AUTH AD");
		run_php_exec("/service/init/auth.init ad_stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_snortd")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> IPS");
		run_php_exec("/service/init/ips.init stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_pptpd")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> PPTP VPN");
		run_php_exec("/service/init/pptp.init stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_ntpd")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> NTP");
		run_php_exec("/service/init/ntp.init stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_snmpd")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> SNMP");
		run_php_exec("/service/init/snmp.init stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_captived")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> CAPTIVE PORTAL");
		run_php_exec("/service/init/captive.init stop 2>/dev/null");
	}
	
	if(file_exists("/var/sys/chk_ddnsd")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> DYNAMIC DNS");
		run_php_exec("/service/init/ddns.init stop 2>/dev/null");
	}
	if(file_exists("/var/sys/chk_named")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> DNS RESOLVER");
		run_php_exec("/service/init/dns.init stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_dhcpd")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> DHCP");
		run_php_exec("/service/init/dhcp.init stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_dhcp_relay")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> DHCP RELAY");
		run_php_exec("/service/init/dhcp_relay.init stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_squid")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> HTTP PROXY");
		run_php_exec("/service/init/http_proxy.init stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_frox")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> FTP PROXY");
		run_php_exec("/service/init/ftp_proxy.init stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_p3scan")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> MAIL PROXY");
		run_php_exec("/service/init/mail_proxy.init stop 2>/dev/null");
	}

	
	if(file_exists("/var/sys/chk_clamd")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> CLAMAV");
		run_php_exec("/service/init/clam.init stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_sophosd")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> SOPHOS AV");
		run_php_exec("/service/init/sophos.init stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_spamd")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> SPAM Filter");
		run_php_exec("/service/init/spam.init stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_xinetd")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> XINETD");
		run_php_exec("/service/init/xinet.init stop 2>/dev/null");
	}

	if(file_exists("/var/sys/chk_syslog-ng")) {
		lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> SYSTEM LOGGER");
		run_php_exec("/service/init/syslog.init stop 2>/dev/null");
	}

	//lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> PACKET FILTER");
	//run_php_exec("/service/init/policy.init stop 2>/dev/null");
	xsystem("/bin/hwclock -w");
	lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> MYBOX");
	xsystem("pkill -9 -f php >/dev/null 2>&1");
	network_down();
	curtime=time(NULL);
	loctime=localtime(&curtime);
	memset(name,0x0,sizeof(name));
	strftime(name, sizeof(name),"system-%Y%m%d.log",loctime);
	memset(buf,0x0,sizeof(buf));
	snprintf(buf,sizeof(buf),"%s/%s",LOGPATH,name);
	memset(cmd,0x0,sizeof(cmd));
	strftime(cmd, sizeof(cmd), "[%d/%m/%Y %T] TYPE=INFO MSG=****** SYSTEM SHUTDOWN ******\n",loctime);
	append_to_file(buf,cmd);
	umount("/config/logs");
	umount("/config/backup");
	umount("/config/update");
	lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> STORAGE OFF");
	xsystem("/etc/init.boot/bootdown");
	xsystem("umount -r /boot");
	xsystem("umount -r /strg");
	lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> SWAP OFF");
	xsystem("swapoff -a");
	lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","POWEROFF/REBOOT");
	exit(0);
}
