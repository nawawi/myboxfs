#include <libmybox.h>

static char BOOT_DEV[11]="/dev/hda1";
static char BOOT_FS[6]="ext3";
static char STRG_DEV[11]="/dev/hda3";
static char STRG_FS[6]="ext3";
static char SWAP_DEV[11]="/dev/hda2";
static char KEY_MAP[150]="";
static char LCD_DEV[8];
static int NUM_NET=0;
static int LCD_PROG=0;

static int TOTALMEM=0;
static int MEMFREE=0;
static int MEMCACHE=0;
static int FOUNDMEM=0;
static int MINSIZE=2000;
static int MINLEFT=16000;
static int MAXSIZE=0;
static int RAMSIZE=0;


static void do_spin(char *msg) {
	char spin[]={'-','\\','|','/','-','\\','|','/'};
	int numspin=sizeof(spin);
	int i, p;
	printf("%s \\",msg);
	for(i=0;i<101;i++) {
		p=p+5;
		fprintf_stdout("\r%s %c %d%%",msg,spin[i%numspin],i);
		usleep(10000);
        }
        fprintf_stdout("\r%s %d%% OK\n",msg,i-1);
}

static void calculate_mem(void) {
	FILE *f, *p;
	char buf[150];
	char p1[50];
	int n;
	if(file_exists("/proc/meminfo")) {
		f=fopen("/proc/meminfo", "r");
		while(fgets(buf, sizeof(buf) - 1, f)) {
			trim(buf);
			if(split_array(buf," ",&chargv) > 0) {
				if(chargv[0]!=NULL && chargv[1]!=NULL) {
					if(!strncmp(chargv[0],"MemTotal:",9)) FOUNDMEM=atoi(chargv[1]);
					if(!strncmp(chargv[0],"MemFree:",8)) MEMFREE=atoi(chargv[1]);
					if(!strncmp(chargv[0],"Cached:",7)) MEMCACHE=atoi(chargv[1]);
				}
			}
		}
		fclose(f);
	}
	if(FOUNDMEM==0) {
		fprintf_stdout("**** MEMORY COUNTING FAILED! ****\n");
		exit(1);
	}
	if(file_exists("/proc/kcore")) {
		memset(buf,0x0,sizeof(buf));
		memset(p1,0x0,sizeof(p1));
		p=popen("du /proc/kcore","r");
		fgets(buf, sizeof(buf) - 1, p);
		if(buf[0]!='\0') {
			splitc(p1,buf,' ');
			n=atoi(p1);
			if(n!=0) fprintf_stdout("* Physical memory size: %d kB\n",n);
			if(n!=0 && n < 184324) {
				fprintf_stdout("**** NOT ENOUGH MEMORY (%d kB) ****\n",n);
				exit(1);
			}
		}
		pclose(p);
	}
	TOTALMEM=MEMFREE + MEMCACHE;
	fprintf_stdout("* Total memory found: %d kB\n",FOUNDMEM);
	do_spin("* Initializing..");
	MAXSIZE=TOTALMEM - MINLEFT;
	RAMSIZE=TOTALMEM / 5;
	if(TOTALMEM > MINLEFT) {
		if(RAMSIZE < 0 ) RAMSIZE=65536;
		RAMSIZE=RAMSIZE * 4;
	}
}

static void read_cmdline(void) {
	char buf[550], name[150];
	char **xchargv;
	int n, k;
	read_oneline("/proc/cmdline",buf);
	trim(buf);
	if(split_array(buf," ",&chargv) > 0) {
		k=numtokens;
		for(n=0;n < k; n++) {
			memset(name,0x0,sizeof(name));
			snprintf(name,sizeof(name),"%s",chargv[n]);
			if(split_array(name,"=",&xchargv) > 0) {
				if(xchargv[0]!=NULL && xchargv[1]!=NULL) {
					if(!strcmp(xchargv[0],"dev_boot")) {
						splitc(BOOT_DEV,xchargv[1],':');
						strcpy(BOOT_FS,xchargv[1]);
					} else if(!strcmp(xchargv[0],"dev_strg")) {
						splitc(STRG_DEV,xchargv[1],':');
						strcpy(STRG_FS,xchargv[1]);
					} else if(!strcmp(xchargv[0],"dev_swap")) {
						splitc(SWAP_DEV,xchargv[1],':');
					} else if(!strcmp(xchargv[0],"keymap")) {
						strcpy(KEY_MAP,xchargv[1]);
					} else if(!strcmp(xchargv[0],"dev_lcd")) {
						strcpy(LCD_DEV,xchargv[1]);
						trim(LCD_DEV);
					} else if(!strcmp(xchargv[0],"lcdprog")) {
						LCD_PROG=atoi(xchargv[1]);						
					} else if(!strcmp(xchargv[0],"numnet")) {
						NUM_NET=atoi(xchargv[1]);
					}
				}
			}
		}
	}
}

void stage1(void) {
	DIR *dp;
	struct dirent *dt;
	calculate_mem();
	fprintf_stdout("* Creating Mybox filesystem (%d kB) on shared memory...\n",RAMSIZE);
	xsystem("mount -t tmpfs -o \"rw,size=%dk\" virtual /ramd",RAMSIZE);
	chmod("/ramd",S_IREAD | S_IWRITE | S_IEXEC);
	chdir("/ramd");
	if((dp=opendir("/")) == NULL) {
		perror("opendir");
		exit(1);
        }
	while((dt=readdir(dp))!=NULL) {
		if(!strcmp(dt->d_name,".") || !strcmp(dt->d_name,"..") ||
			!strcmp(dt->d_name,"lost+found") || 
			!strcmp(dt->d_name,"ramd") || 
			!strcmp(dt->d_name,"proc") || 
			!strcmp(dt->d_name,"dev") || 
			!strcmp(dt->d_name,"sys")) continue;
			xsystem("cp -dpR /%s /ramd/",dt->d_name);
	}
	closedir(dp);
	xmkdir("dev/pts");
	xmkdir("initrd");
	umount("/proc");
	umount("/sys");
}

void stage2(void) {
	DIR *dp;
	struct dirent *dt;
	FILE *fp;
	char **xargv;
	char buf[1024], name[150];
	int rc;
	rc=pivot_root(".","initrd");
	if(rc==-1) {
		perror("pivot_root");
		exit(1);
	}
	chdir("/");
	xmkdir("proc");
	xmkdir("sys");
	xmkdir("config");
	xmkdir("config/download");
	xmkdir("config/localsave");
	xmkdir("config/logs");
	xsystem("mount -t proc -o rw virtual /proc");
	xsystem("mount -t sysfs -o rw virtual /sys");
	save_to_file("/proc/sys/kernel/printk","0 0 0 0\n");
	if((dp=opendir("/tmp")) == NULL) {
		perror("opendir");
		exit(1);
        }
	fprintf_stdout("-> Extracting base tools.");
	while((dt=readdir(dp))!=NULL) {
		if(!strcmp(dt->d_name,".") || !strcmp(dt->d_name,"..")) continue;
		if(strstr(dt->d_name,".bz2")) {
			fprintf_stdout(".");
			xsystem("tar -C / -jxf /tmp/%s",dt->d_name);
			usleep(10000);
		}
	}
	free(dt);
	closedir(dp);
	fprintf_stdout("\r* Extracting base tools. Done.%s\n",SPACE);
	save_to_file("/proc/sys/kernel/modprobe","/bin/modprobe\n");
	xsystem("depmod -a");
	if(file_exists("/lib/modules/drivers.txt")) {
		memset(buf,0x0,sizeof(buf));
		if((fp=fopen("/lib/modules/drivers.txt","r"))!=NULL) {
			while(fgets(buf,sizeof(buf),fp)!=NULL) {
				trim(buf);
				if(buf[0]=='\0') continue;
				xargv[0]=NULL;xargv[1]=NULL;
				memset(name,0x0,sizeof(name));
				xargv[0]=buf;
				xargv[1]=".ko";
				snprintf(name,sizeof(name),"%s",base_name(xargv));
				fprintf_stdout("-> Scanning for %s..%s\r",name,SPACE);
				if(!strcmp(name,"ne")) {
					memset(name,0x0,sizeof(name));
					strncpy(name,"ne io=0x300,0x340",sizeof(name));
				}
				xsystem("modprobe -q -k %s",name);
				usleep(1000);
			}
			fclose(fp);
			fprintf_stdout("%s\r* Scanning. Done.\n",SPACE);
		}
	}
}

static int do_chroot(void) {
	char **xargv;
	if(chroot(".")!=0) return 1;
	chdir("/");
	xargv[0]="/bin/chroot";
	xargv[1]=".";
	xargv[2]="/bin/init";
        execvp("/bin/chroot", xargv);
	return 0;
}

int bootup_main(int argc, char **argv) {
	FILE *fp;
	int t=0;
	char cmd[1024], buf[1024], name[1024];
	time_t curtime;
  	struct tm *loctime;
	signal(SIGINT,SIG_IGN);
	putenv("PATH=/bin");
	putenv("TERM=linux");
	umask(0770);
	chdir("/");
	print_file("/etc/banner");
	xsystem("mount -t proc -o ro virtual /proc");
	xsystem("mount -t sysfs -o ro virtual /sys");
	stage1();
	stage2();

	// STAGE 3
	chdir("/");
	read_cmdline();
	xsystem("mdev -s");
	xsystem("mount -t devpts -o \"rw,gid=0,mode=620\"");
	mk_dev("/dev/ppp",108,0);
	if(LCD_PROG==1 && file_exists("/tmp/tools/lcd/lcd.bz2")) {
		mk_dev("/dev/parport0",99,0);
		mk_dev("/dev/lp0",6,0);
		xsystem("tar -C / -jxf /tmp/tools/lcd/lcd.bz2");
	} else {
		LCD_PROG=0;
	}
	rename("/dev/random","/dev/random-block");
	symlink("/dev/urandom","/dev/random");
	xmkdir("/strg");
	if(LCD_DEV[0]!='\0') {
		snprintf(buf,sizeof(buf),"/dev/%s",LCD_DEV);
		if(file_exists(buf)) {
			save_to_file("/var/sys/lcd_dev",buf);
			if(LCD_PROG==1) save_to_file("/var/sys/lcd_proc","%d",LCD_PROG);
			symlink(buf,"/dev/lcd");
			lcd_msg(LCD_PROG,"SYSTEM LOADING..","-> STORAGE ON");
		}
	}
	memset(buf,0x0,sizeof(buf));
	snprintf(buf,sizeof(buf),"mount -t %s -o \"rw,noatime\" %s /strg",STRG_FS,STRG_DEV);
	if(xsystem("mount -t %s -o \"rw,noatime\" %s /strg",STRG_FS,STRG_DEV)==0) {
		if(file_exists("/strg/.mount_strg")) {
			unlink("/strg/.mount_strg");
			if(xsystem("umount /strg")==0) {
				fprintf_stdout("**** MYBOX SYSTEM APPEARS TO HAVE SHUT DOWN UNCLEANLY ****\n");
				lcd_msg(LCD_PROG,"SYSTEM LOADING..","-> FIX STORAGE");
				xsystem("e2fsck -y %s",STRG_DEV);
				t=1;
			}
		}
		if(t==0) xsystem("umount /strg");
	} else {
		fprintf_stdout("**** MOUNTING STORAGE DISK FAILED! ****\n");
		lcd_msg(LCD_PROG,"SYSTEM LOADING..","STORAGE FAILED !");
		xtouch("/strg/.nostrg");
		xtouch("/var/sys/nolog");
	}
	if(!file_exists("/strg/.nostrg")) {
		if(xsystem("mount -t %s -o \"rw,noatime\" %s /strg",STRG_FS,STRG_DEV)==0) {
			memset(buf,0x0,sizeof(buf));
			snprintf(buf,sizeof(buf),"%s:%s\n",STRG_DEV,STRG_FS);
			save_to_file("/strg/.mount_strg",buf);
			save_to_file("/var/sys/.mount_strg",buf);
			save_to_file("/var/sys/.mount_strg","%s:%s\n",BOOT_DEV,BOOT_FS);
		}
	}
	if(xsystem("swapon %s",SWAP_DEV)==0) {
		memset(buf,0x0,sizeof(buf));
		snprintf(buf,sizeof(buf),"%s:swap\n",SWAP_DEV);
		save_to_file("/var/sys/.mount_swap",buf);
	}
	xsystem("chmod 700 *");
	if((fp=fopen("/etc/inittab","w"))!=NULL) {
		fprintf(fp,"::sysinit:/bin/initrc\n");
		fprintf(fp,"tty1::respawn:/bin/getty -h -n -L tty1 9600 linux\n");
		fprintf(fp,"ttyS0::respawn:/bin/getty -h -n -L ttyS0 9600 vt100\n");
		fprintf(fp,"null::respawn:/bin/chkprog\n");
		fprintf(fp,"::restart:/bin/init\n");
		fprintf(fp,"::ctrlaltdel:/bin/bootdown\n");
		fprintf(fp,"::ctrlaltdel:/bin/reset\n");
		fprintf(fp,"::ctrlaltdel:/bin/reboot\n");
		fprintf(fp,"::shutdown:/bin/bootdown\n");
		fclose(fp);
	}
	if((fp=fopen("/etc/profile","a"))!=NULL) {
		fprintf(fp,"xexit() {\n");
		fprintf(fp,"	if [ \"$PPID\" = \"1\" ]; then\n");
		fprintf(fp,"		local logname=\"/strg/mybox/logs/auth-$(date \"+%s\").log\"\n","%Y%m%d");
		fprintf(fp,"		local msg=\"[$(date \"+%s\")] TYPE=console USER=console IP=$(basename $(tty)) MSG=Session logout.\"\n","%d/%m/%Y %H:%M:%S");
		fprintf(fp,"		echo \"$msg\" >> $logname\n");
		fprintf(fp,"		[ ! -z \"$ME\" -a -f \"$ME\" ] && rm -f /tmp/console.session/console_*\n");
		fprintf(fp,"	fi\n");
		fprintf(fp,"	exit\n");
		fprintf(fp,"}\n");
		fprintf(fp,"alias exit='xexit'\n");
		fprintf(fp,"export HISTFILE=/.consolehistory\n");
		fprintf(fp,"lcdd_msg() {\n");
		fprintf(fp,"	if [ -f \"/bin/lcdd\" -a -c \"/dev/lcd\" ]; then\n");
		fprintf(fp,"		if [ -f \"/var/sys/lcd_proc\" ]; then\n");
		fprintf(fp,"			echo \"$2\" > /var/sys/lcd_msg\n");
		fprintf(fp,"		else\n");
		fprintf(fp,"			/bin/lcdd \"$1\" \"$2\"\n");
		fprintf(fp,"		fi\n");
		fprintf(fp,"	fi\n");	
		fprintf(fp,"}\n");
		fprintf(fp,"if [ -z $DO_SINGLE ]; then\n");
		fprintf(fp,"	if [ -f \"/bin/iosh\" ]; then\n");
		fprintf(fp,"		XTTY=\"SSL\";\n");
		fprintf(fp,"		if [ -f \"/var/sys/init_start\" ]; then\n");
		fprintf(fp,"			trap : 1 2 3 15\n");
		fprintf(fp,"			echo \"System loading in progress..please wait or login back in a minute\"\n");
		fprintf(fp,"			while [ -f \"/var/sys/init_start\" ]; do sleep 1;done\n");
		fprintf(fp,"			trap 1 2 3 15\n");
		fprintf(fp,"		fi\n");
		fprintf(fp,"		if [ \"$PPID\" = \"1\" ]; then\n");
		fprintf(fp,"			export ME=\"/tmp/console.session/console_${PPID}_$(basename $(tty))_$(date \"+%s\")\";\n","%d:%m:%Y_%H:%M:%S");
		fprintf(fp,"			touch $ME\n");
		fprintf(fp,"			XTTY=\"console\";\n");
		fprintf(fp,"		fi\n");
		fprintf(fp,"		/bin/iosh $XTTY\n");
		fprintf(fp,"		if [ $? != 5 ]; then\n");
		fprintf(fp,"			clear;reset\n");
		fprintf(fp,"			exit\n");
		fprintf(fp,"		fi\n");
		fprintf(fp,"	else\n");
		fprintf(fp,"		echo \"** FAILED TO RUN IO SHELL **\"\n");
		fprintf(fp,"		read io\n");
		fprintf(fp,"		exit\n");
		fprintf(fp,"	fi\n");
		fprintf(fp,"else \n");
		fprintf(fp,"	echo \"** MAINTENANCE MODE **\"\n");
		fprintf(fp,"	lcdd_msg \"SYSTEM LOADING.." "-> MAINTENANCE\"\n");
		fprintf(fp,"	read io\n");
		fprintf(fp,"fi\n");
		fclose(fp);
	}
	unlink("/strg/mybox/debug.log");
	curtime=time(NULL);
	loctime=localtime(&curtime);


	memset(name,0x0,sizeof(name));
	strftime(name, sizeof(name),"system-%Y%m%d.log",loctime);
	memset(buf,0x0,sizeof(buf));
	snprintf(buf,sizeof(buf),"%s/%s",LOGPATH,name);
	memset(cmd,0x0,sizeof(cmd));
	strftime(cmd, sizeof(cmd), "[%d/%m/%Y %T] TYPE=INFO MSG=****** SYSTEM LOADING ******\n",loctime);
	append_to_file(buf,cmd);
	if(file_exists("/bin/getkey")) {
		if(system("getkey -c 3 -m \"-> Starting Init: %d\" R")==0) {
			fprintf_stdout("\r*** BYPASS CONSOLE LOGIN ***\n");
			lcd_msg(LCD_PROG,"SYSTEM LOADING..","BYPASS CONSOLE !");
			xtouch("/etc/noconsole");
		} else {
			fprintf_stdout("\r* Starting Init. Done.\n");
		}
	}
	memset(buf,0x0,sizeof(buf));
	snprintf(buf,sizeof(buf),"%s\n","/bin/mdev");
	save_to_file("/proc/sys/kernel/hotplug",buf);
	if(NUM_NET!=0) save_to_file("/var/sys/numnet_veto","%d",NUM_NET);
	do_chroot();
	signal(SIGINT,SIG_DFL);
	lcd_msg(LCD_PROG,"SYSTEM LOADING..","ERROR !");
	fprintf_stdout("You are not supposed to be here, something went wrong!\n");
	fprintf_stdout("Press Ctrl+Alt+Del or switch off/on for reboot.\n");
	while(1);
	exit(0);
}
