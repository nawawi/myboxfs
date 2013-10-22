#include <libmybox.h>
static int LCD_PROC=0;
void network_down(void) {
	int pt=0,x=0;
	lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> NETWORK OFF");
	pt=get_eth();
	if(pt!=0) {
		for(x=0;x<pt;x++) {
			xsystem("ip link set %s down",eth_list[x].name);
		}
 		xsystem("ip route flush cache");
                xsystem("ip route flush table main");
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
	if(file_exists("/var/sys/lcd_proc")) {
		LCD_PROC=1;
		xtouch("/var/sys/lcd_shutdown");
	}
	save_to_file("/proc/sys/kernel/printk","0 0 0 0\n");
	save_to_file("/etc/resolv.conf","\n");
	xsystem("killall -9 snortd dhcpd sshd iosh");
	xsystem("ps grep php |xargs kill -9");
	curtime=time(NULL);
	loctime=localtime(&curtime);
	memset(name,0x0,sizeof(name));
	strftime(name, sizeof(name),"system-%Y%m%d.log",loctime);
	memset(buf,0x0,sizeof(buf));
	snprintf(buf,sizeof(buf),"%s/%s",LOGPATH,name);
	memset(cmd,0x0,sizeof(cmd));
	strftime(cmd, sizeof(cmd), "[%d/%m/%Y %T] TYPE=INFO MSG=****** SYSTEM SHUTDOWN ******\n",loctime);
	append_to_file(buf,cmd);
	if(is_dir("/firmware")) xsystem("umount -r /firmware");
	lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> FREE MEMORY");
	unlink("/strg/.mount_strg");
	xsystem("rm -f /strg/mybox/db/system.db-* /strg/mybox/db/blacklist.db-* /strg/mybox/db/ips.db-*");
	umount("/config/download");
	umount("/config/logs");
	umount("/config/localsave");
	xsystem("umount -r /strg");
	lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> STORAGE OFF");
	sleep(1);
	xsystem("swapoff -a");
	lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","-> SWAP OFF");
	network_down();
	lcd_msg(LCD_PROC,"SYSTEM SHUTDOWN.","POWEROFF/REBOOT");
	exit(0);
}
