#include "libmybox.h"

static char LCD_DEV[8];
static int LCD_PROG=-1;
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
		fprintf_stdout("#### ERROR: MEMORY COUNTING FAILED!\n");
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
				fprintf_stdout("#### ERROR: NOT ENOUGH MEMORY (%d kB)\n",n);
				exit(1);
			}
		}
		pclose(p);
	}
	TOTALMEM=MEMFREE + MEMCACHE;
	fprintf_stdout("* Total memory found: %d kB\n",FOUNDMEM);
	MAXSIZE=TOTALMEM - MINLEFT;
	RAMSIZE=TOTALMEM / 5;
	if(TOTALMEM > MINLEFT) {
		if(RAMSIZE < 0 ) RAMSIZE=65536;
		RAMSIZE=RAMSIZE * 4;
	}
}


static int do_chroot(void) {
	char **xargv;
	if(chroot(".")!=0) {
		fprintf_stdout("#### ERROR: Change root directory failed!\n");
		return 1;
	}
	chdir("/");
	xargv[0]="/bin/chroot";
	xargv[1]=".";
	xargv[2]="/bin/init";
        execvp("/bin/chroot", xargv);
	fprintf_stdout("#### ERROR: Running init failed!\n");
	return 0;
}

int bootup_main(int argc, char **argv) {
	FILE *fp;
	DIR *dp;
	struct dirent *dt;
	int t=0, rc;
	char cmd[1024], buf[1024], name[1024];
	time_t curtime;
  	struct tm *loctime;
	signal(SIGINT,SIG_IGN);
	putenv("PATH=/bin");
	putenv("TERM=linux");
	umask(0770);
	chdir("/");
	putchar('\n');
	print_banner();
	putchar('\n');
	xsystem("mount -t proc -o ro virtual /proc");
	xsystem("mount -t sysfs -o ro virtual /sys");

	// STAGE 1
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

	// STAGE 2
	rc=pivot_root(".","initrd");
	if(rc==-1) {
		fprintf_stdout("#### ERROR: Change root file system failed!\n");
		exit(1);
	}
	chdir("/");
	xmkdir("proc");
	xmkdir("sys");
	xsystem("mount -t proc -o rw virtual /proc");
	xsystem("mount -t sysfs -o rw virtual /sys");
	save_to_file("/proc/sys/kernel/printk","0 0 0 0\n");
	if((dp=opendir("/tmp")) == NULL) {
		perror("opendir");
		exit(1);
        }
	fprintf_stdout("-> Extracting base tools: ");
	while((dt=readdir(dp))!=NULL) {
		if(!strcmp(dt->d_name,".") || !strcmp(dt->d_name,"..")) continue;
		if(strstr(dt->d_name,".mpk")) {
			fprintf_stdout("#");
			xsystem("tar -C / -axf /tmp/%s",dt->d_name);
		}
	}
	free(dt);
	closedir(dp);
	fprintf_stdout("\r* Extracting base tools. Done.%s\n",SPACE);
	save_to_file("/proc/sys/kernel/modprobe","/bin/modprobe\n");
	xsystem("depmod -a");

	// STAGE 3
	chdir("/");
	xsystem("mdev -s");
	xsystem("mount -t devpts /dev/devpts /dev/pts -o \"rw,gid=0,mode=620\"");
	rename("/dev/random","/dev/random-block");
	symlink("/dev/urandom","/dev/random");
	xsystem("chmod 700 *");
	if((fp=fopen("/etc/inittab","w"))!=NULL) {
		fprintf(fp,"::sysinit:/etc/init.boot/rc.init\n");
		fprintf(fp,"tty1::respawn:/bin/getty -h -n -L tty1 115200 linux\n");
		fprintf(fp,"ttyS0::respawn:/bin/getty -h -n -L ttyS0 115200 vt100\n");
		fprintf(fp,"tty7::respawn:/bin/chkprog\n");
		fprintf(fp,"tty8::respawn:/bin/trafficd\n");
		fprintf(fp,"::restart:/bin/init\n");
		fprintf(fp,"::ctrlaltdel:/bin/bootdown\n");
		fprintf(fp,"::ctrlaltdel:/bin/reset\n");
		fprintf(fp,"::ctrlaltdel:/bin/reboot\n");
		fprintf(fp,"::shutdown:/bin/bootdown\n");
		fclose(fp);
	}
	curtime=time(NULL);
	loctime=localtime(&curtime);
	strftime(cmd, sizeof(cmd), "[%d/%m/%Y %T] TYPE=INFO MSG=****** SYSTEM LOADING ******\n",loctime);
	append_to_file("/tmp/bootup",cmd);
	if(file_exists("/bin/getkey")) {
		if(system("getkey -c 3 -m \"-> Starting Init: %d\" R")==0) {
			fprintf_stdout("\r#### WARNING: LOGIN DISABLED\n");
			xtouch("/etc/noconsole");
		} else {
			fprintf_stdout("\r* Starting Init. Done.\n");
		}
	}
	memset(buf,0x0,sizeof(buf));
	snprintf(buf,sizeof(buf),"%s\n","/bin/mdev");
	save_to_file("/proc/sys/kernel/hotplug",buf);
	do_chroot();
	signal(SIGINT,SIG_DFL);
	fprintf_stdout("#### ERROR: Failed to boot file system!\n");
	fprintf_stdout("#### ERROR: Press Ctrl+Alt+Del or switch off/on for reboot.\n");
	while(1);
	exit(0);
}
