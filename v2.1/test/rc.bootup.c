#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#ifndef MS_MGC_VAL
#define MS_MGC_VAL 0xc0ed0000
#endif
#ifndef MS_RDONLY
#define MS_RDONLY       1
#endif
#ifndef MS_NOSUID
#define MS_NOSUID	2
#endif
#ifndef MS_NODEV
#define MS_NODEV 	4
#endif
#ifndef MS_NOEXEC
#define MS_NOEXEC	8
#endif
#ifndef MS_SYNCHRONOUS
#define MS_SYNCHRONOUS	16
#endif
#ifndef MS_REMOUNT
#define MS_REMOUNT	32
#endif

int TOTALMEM=0;
int MEMFREE=0;
int MEMCACHE=0;
int FOUNDMEM=0;
int MINSIZE=2000;
int MINLEFT=16000;
int MAXSIZE=0;
int RAMSIZE=0;

static void show_banner(void) {
	printf("+=============================================================================+\n");
        printf("| MYBOX FIREWALL SYSTEM (c) Tracenetwork Corporation Sdn. Bhd.                |\n");
        printf("|                           http://www.mybox.net.my info@mybox.net.my         |\n");
        printf("+=============================================================================+\n");
}

static int file_exist(char *s) {
        struct stat ss;
        int i = stat(s, &ss);
        if (i < 0) return 0;
        if ((ss.st_mode & S_IFREG) || (ss.st_mode & S_IFLNK)) return(1);
        return(0);
}

static void rmspace(char *x) {
	char *t;
   	for(t=x+strlen(x)-1; (((*t=='\x20')||(*t=='\x0D')||(*t=='\x0A')||(*t=='\x09'))&&(t >= x)); t--);
   	if(t!=x+strlen(x)-1) *(t+1)=0;
   	for(t=x; (((*t=='\x20')||(*t=='\x0D')||(*t=='\x0A')||(*t=='\x09'))&&(*t)); t++);
   	if(t!=x) strcpy(x,t);
}

static void splitc(char *first, char *rest, char divider) {
	char *p;
   	p=strchr(rest, divider);
   	if(p==NULL) {
      		if((first != rest) && (first != NULL))
	 	first[0] = 0;
      		return;
   	}
   	*p=0;
   	if(first != NULL) strcpy(first, rest);
   	if(first != rest) strcpy(rest, p + 1);
	rmspace(rest);
}

static char *base_name(char *path) {
	static const char null_or_empty[] = ".";
	char *first = path;
	char *last;

        if(!path || !*path) {
		return (char *) null_or_empty;
        }

        last=path - 1;
        while(*path) {
		if((*path != '/') && (path > ++last)) last=first=path;
		++path;
	}

	if(*first=='/') last=first;
	last[1]=0;
	return first;
}

static void do_spin(char *msg) {
	char spin[]={'-','\\','|','/','-','\\','|','/'};
	int numspin=sizeof(spin);
	int i, p;
	printf("%s \\",msg);
	for(i=0;i<101;i++) {
		p=p+5;
		printf("\r%s %c %d%%",msg,spin[i%numspin],i);
		fflush(stdout);
		usleep(5);
        }
        printf("\r%s %d%% OK\n",msg,i-1);
}

static void calculate_mem(void) {
	FILE *f;
	char buf[150];
	char p1[50],p2[50];
	if(file_exist("/proc/meminfo")) {
		f=fopen("/proc/meminfo", "r");
		while(fgets(buf, sizeof(buf) - 1, f)) {
			rmspace(buf);
			splitc(p1,buf,' ');
			splitc(p2,buf,' ');
			if(!strncmp(p1,"MemTotal:",9)) FOUNDMEM=atoi(p2);
			if(!strncmp(p1,"MemFree:",8)) MEMFREE=atoi(p2);
			if(!strncmp(p1,"Cached:",7)) MEMCACHE=atoi(p2);
		}
		fclose(f);
	}
	if(FOUNDMEM==0) {
		fprintf(stderr,"**** MEMORY COUNTING FAILED! ****\n");
		exit(1);
	}

	TOTALMEM=MEMFREE + MEMCACHE;
	printf("* Total memory found: %d kB\n",FOUNDMEM);
	do_spin("* Initializing..");
	if(FOUNDMEM==0) exit(1);
	MAXSIZE=TOTALMEM - MINLEFT;
	RAMSIZE=TOTALMEM / 5;
	if(TOTALMEM > MINLEFT) {
		if(RAMSIZE < 0 ) RAMSIZE=65536;
		RAMSIZE=RAMSIZE * 4;
	}
}

void do_mount(char *fs,char *opt, char *dev, char *mpoint) {
	int rc;
	unsigned long fl=MS_MGC_VAL;
	if(opt!=NULL) {
		if(strstr(opt,"ro")) {
			fl|=MS_RDONLY;
			opt=NULL;
		} else if(strstr(opt,"rw")) {
			fl&=~MS_RDONLY;
			opt=NULL;
		} else if(strstr(opt,"nosuid")) {
			fl|=MS_NOSUID;
			opt=NULL;
		} else if(strstr(opt,"suid")) {
			fl&=~MS_NOSUID;
			opt=NULL;
		} else if(strstr(opt,"nodev")) {
			fl|=MS_NODEV;
			opt=NULL;
		} else if(strstr(opt,"dev")) {
			fl&=~MS_NODEV;
			opt=NULL;
		} else if(strstr(opt,"noexec")) {
			fl|=MS_NOEXEC;
			opt=NULL;
		} else if(strstr(opt,"exec")) {
			fl&=~MS_NOEXEC;
			opt=NULL;
		} else if(strstr(opt,"sync")) {
			fl|=MS_SYNCHRONOUS;
			opt=NULL;
		} else if(strstr(opt,"nosync")) {
			fl&=~MS_SYNCHRONOUS;
			opt=NULL;
		} else if(strstr(opt,"remount")) {
			fl|=MS_REMOUNT;
			opt=NULL;
		}
	}
	rc=mount(dev,mpoint,fs,fl,opt);
 	if(rc==EACCES) {
		/* Read-only filesystem */
		rc=mount(dev,mpoint,fs,fl|MS_RDONLY,opt);
	}
 	if(rc==-1) { 
		perror("mount");
   		exit(1);
  	}
}

static int write_file(char *f, char *va) {
	FILE *fp;
	if((fp=fopen(f,"w"))!=NULL) {
		fprintf(fp,"%s",va);
		fclose(fp);
		return 1;
	}
	return 0;
}

static void run_init_process(char *init_filename) {
	char *envp_init[4]={"HOME=/", "TERM=linux", "PATH=/bin", NULL};
	execve(init_filename, init_filename, envp_init);
}

int main(int argc, char **argv, char **envp) {
	DIR *dp;
	struct dirent *dt;
	FILE *fp;
	char cmd[150];
	char buf[255];
	char opt[3]="rw";
	int i, rc;
	(void) signal(SIGINT,SIG_IGN);
	putenv("PATH=/bin");
	putenv("TERM=linux");
	show_banner();
	fflush(stdout);
	do_mount("proc",NULL,"virtual","/proc");
	do_mount("sysfs",NULL,"virtual","/sys");

	calculate_mem();
	printf("* Creating Mybox filesystem (%d kB) on shared memory...\n",RAMSIZE);
	memset(cmd,0x0,sizeof(cmd));
	sprintf(cmd,"size=%dk",RAMSIZE);
	do_mount("tmpfs",cmd,"virtual","/ramd");

	chmod("/ramd",S_IREAD | S_IWRITE | S_IEXEC);
	chdir("/ramd");
	memset(cmd,0x0,sizeof(cmd));
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
			memset(cmd,0x0,sizeof(cmd));
			snprintf(cmd,sizeof(cmd),"/bin/cp -dpR /%s /ramd/",dt->d_name);
			system(cmd);
	}
	closedir(dp);
	mkdir("dev",S_IREAD | S_IWRITE | S_IEXEC);
	mkdir("initrd",S_IREAD | S_IWRITE | S_IEXEC);
	umount("/proc");
	umount("/sys");
	rc=pivot_root(".","initrd");
	if(rc==-1) {
		perror("pivot_root");
		exit(1);
	}
	write_file("/proc/sys/kernel/printk","0 0 0 0");
	chdir("/");
	mkdir("proc",S_IREAD | S_IWRITE | S_IEXEC);
	mkdir("sys",S_IREAD | S_IWRITE | S_IEXEC);

	do_mount("proc",NULL,"virtual","/proc");
	do_mount("sysfs",NULL,"virtual","/sys");

	system("/bin/mdev -s > /dev/null 2>&1");
	printf("* Extracting common tools..");
	memset(cmd,0x0,sizeof(cmd));
	if((dp=opendir("/tmp")) == NULL) {
		perror("opendir");
		exit(1);
        }
	while((dt=readdir(dp))!=NULL) {
		if(!strcmp(dt->d_name,".") || !strcmp(dt->d_name,"..")) continue;
			if(strstr(dt->d_name,".bz2")) {
				memset(cmd,0x0,sizeof(cmd));
				snprintf(cmd,sizeof(cmd),"/bin/tar -C / -jxf /tmp/%s",dt->d_name);
				if(system(cmd)!=-1) {
					putchar('.');
					fflush(stdout);usleep(5);
				}
				
			}
	}
	closedir(dp);
	if(write_file("/proc/sys/kernel/modprobe","/bin/modprobe")) {
		if(system("/bin/depmod -qa >> /dev/null 2>&1")!=-1) {
			printf("\n* Searching modules dependency..");
        		if((fp=fopen("/lib/modules/drivers.txt","r"))!=NULL) {
				while(fgets(buf,sizeof(buf),fp)!=NULL) {
					rmspace(buf);
					printf("-> Scanning for %s..                                \r",(char *)base_name(buf));
					fflush(stdout);
					memset(cmd,0x0,sizeof(cmd));
					sprintf(cmd,"/bin/modprobe -q -k %s >> /dev/null 2>&1",buf);
					system(cmd);usleep(5);

				}
				fclose(fp);
				printf("                                  \r\n* Scanning done.\n");
			}
		}
	} else {
		fprintf(stderr,"**** LOADING MODULES FAILED! ****\n");
		exit(1);
	}
	system("/bin/echo \"tty1::respawn:-/bin/sh\" > /etc/inittab");
	if(chroot(".")==-1) {
		perror("chroot");
		exit(1);
	}
	chdir("/");
	/*run_init_process("/bin/init");

	fprintf(stderr,"********** ERROR! **********\n");
	fprintf(stderr,"You are not supposed to be here, something went wrong!\n");
	fprintf(stderr,"calling chroot failed?\n");
	fprintf(stderr,"Press Ctrl+Alt+Del or switch off/on for reboot.\n");
	sleep(1);
	system("/bin/sh");
	*/
	// bye bye ..past to kernel init
	exit(1);
}


