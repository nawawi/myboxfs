#include <libmybox.h>

static int DO_SINGLE=0;
static int LCD_PROC=0;

void network_touch(void) {
	int pt=0,x=0;
	fprintf_stdout("* Detecting network interfaces: ");
	pt=get_eth();
	lcd_msg(LCD_PROC,"SYSTEM LOADING..","-> CHECKING NIC");
	if(pt!=0) {
		for(x=0;x<pt;x++) {
			fprintf_stdout("%s ",eth_list[x].name);
		}
		putchar('\n');
		if(pt < 4) pt=4;
		save_to_file("/var/sys/numnet","%d\n",pt);
	} else {
		fprintf_stdout("* No network device found\n");
		lcd_msg(LCD_PROC,"SYSTEM LOADING..","-> NO NIC FOUND");
		xtouch("/var/sys/nonetwork");
	}
}


int initrc_main(int argc, char **argv) {
	DIR *dp;FILE *fp;
	struct dirent *dt;
	char cmd[1024];
	signal(SIGINT,SIG_IGN);
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
	if(file_exists("/etc/microcode.dat")) xsystem("/bin/microcode_ctl -Qf /etc/microcode.dat");
	// chk network device
	network_touch();
	//default domain
	save_to_file("/proc/sys/kernel/hostname","fw.mybox.local\n");
	xsystem("chmod 700 /service/www/*.exh");
	xsystem("chmod 700 /service/tools/*.exc");

	if(DO_SINGLE==0 && file_exists("/etc/rc.start/sh")) {
		xmkdir("/tmp/sessions");
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
		lcd_msg(LCD_PROC,"SYSTEM LOADING..","-> TUNE KERNEL");
		xsystem("/etc/rc.sysctl/sh >/dev/null 2>&1");
	}
	//console session
	xmkdir("/tmp/console.session");
	// www session
	xmkdir("/tmp/sessions");
	if(is_dir("/strg/mybox/")) {
		xsystem("dmesg -c -s 131072 > /strg/mybox/boot.msg");
	}
	xflush_stdout();
	(void) signal(SIGINT,SIG_DFL);
	if(DO_SINGLE==1) run_shell();
	if(DO_SINGLE==1) xsystem("/bin/reboot");
	lcd_msg(LCD_PROC,"SYSTEM LOADING..","-> SET STORAGE");
	xsystem("mount --bind /strg/mybox/download /config/download");
	xsystem("mount --bind /strg/mybox/localsave /config/localsave");
	xsystem("mount --bind /strg/mybox/logs /config/logs");
	save_to_file("/proc/sys/kernel/printk","6 0 0 0\n");
	xsystem("chmod 700 /*");
	if(LCD_PROC==0) {
		if(file_exists("/dev/lcd") && file_exists("/bin/lcdd")) {
			xsystem("/bin/lcdd");
			xtouch("/var/sys/chk_lcdd");
		}
	}
	unlink("/var/sys/lcd_msg");
	if((fp=fopen("/var/sys/flushapp","w"))!=NULL) {
		fprintf(fp,"syslog.init\n");
		fprintf(fp,"https.init\n");
		fprintf(fp,"ips.init\n");
		fprintf(fp,"ssh.init\n");
		fprintf(fp,"ftp.init\n");
		fprintf(fp,"dns.init\n");
		fprintf(fp,"dhcp.init\n");
		fprintf(fp,"ddns.init\n");
		fprintf(fp,"snmp.init\n");
		fclose(fp);
	}
	exit(0);
}


