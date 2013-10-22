#include "libmybox.h"

struct conf1 {
	char dev[25];
	char gw[50];
	char ip[50];
	int prio;
	int status;
	int down;
} counter[25];

struct conf2 {
	char ip[50];
} list[1024];

static char mode_type[150]="Failover (active->passive)";
static int mode=1;
static int interval=60;
static int numcont=0;
static int listcont=0;
static int singlestate=0;
static int multistate=0;

static int ip_route_flush(void) {
	return xsystem("ip route flush cache");
}

static int ip_route(char *opt,char *ip, char *gw) {
	return xsystem("ip route %s %s via %s",opt,ip,gw);
}

static int ip_route_del_default(void) {
	return xsystem("ip route del default");
}

static int ip_route_default(char *gw, char *dev) {
	return xsystem("ip route add default via %s dev %s metric 0",gw,dev);
}

static int update_arp(char *ip, char *dev) {
	//return xsystem("arping -qb %s -c 3 -w 3 -i %s",ip,dev);
	return xsystem("arping -qb %s -c 3 -w 3 -I %s",ip,dev);
}

static int update_ping(char *gw, char *ip) {
	return xsystem("ping -q -c 1 -W 5 -I %s %s",gw,ip);
}

static int exec_ping(char *ip) {
	return xsystem("ping -q -c 1 -W 5 %s",ip);
}

static int do_ping(char *gw, char *dev, char *ip) {
	int i=0, x=1;
	ip_route_flush();
	for(i=0; i < listcont; i++) {
		if(x==0) break;
		ip_route("add",list[i].ip,gw);
		x=update_ping(ip,list[i].ip);
		exec_ping(list[i].ip);
		ip_route("del",list[i].ip,gw);
		usleep(72500); // cpu tune
	}
	update_arp(ip,dev);
	return x;
}

static void do_single_route(void) {
	char txt[256];
	int i=0, x=1;
	for(i=0; i < numcont; i++) {
		// check if counter with priority 1 is up, just skip
		if(counter[i].prio==1 && counter[i].status==1) {
			// if previously down, up and skip
			if(singlestate==1) {
				ip_route_del_default();
				ip_route_default(counter[i].gw, counter[i].dev);
				singlestate=0;
				memset(txt,0x0,sizeof(txt));
				snprintf(txt,sizeof(txt),"Uplink [%s] switching back to primary gateway %s",mode_type,counter[i].gw);
				log_action("INFO",txt);
			}
			break;
		} else {
			singlestate=1;
			// next counter, close if found
			if(counter[i].status==1) {
				ip_route_del_default();
				ip_route_default(counter[i].gw, counter[i].dev);
				memset(txt,0x0,sizeof(txt));
				snprintf(txt,sizeof(txt),"Uplink [%s] primary gateway seems to be down, switching to %s",mode_type,counter[i].gw);
				log_action("INFO",txt);
				break;	
			}
		}
	}
	usleep(72500); // cpu tune
}

// check if just one gateway is up
static void do_tune_route(void) {
	char txt[256];
	int i=0, x=0, p;
	for(i=0; i < numcont; i++) {
		if(counter[i].status==1) {
			if(x > 1) break;
			p=i;x++;
		}
	}
	if(x==1) {
		if(multistate==0) {
			xsystem("/var/sys/uplink_stop");
			ip_route_del_default();
			ip_route_default(counter[p].gw, counter[p].dev);
			memset(txt,0x0,sizeof(txt));
			snprintf(txt,sizeof(txt),"Uplink [%s] only one gateway %s is up, setting as default",mode_type,counter[p].gw);
			log_action("INFO",txt);sleep(1);
			multistate=1;
		}	
	} else {
		if(multistate==1) {
			xsystem("/var/sys/uplink_start");
			memset(txt,0x0,sizeof(txt));
			snprintf(txt,sizeof(txt),"Uplink [%s] more than one gateway is up, recovering from previous error",mode_type);
			log_action("INFO",txt);sleep(1);
			multistate=0;
		}
	}
	usleep(72500); // cpu tune	
}

static void chkfailover(void) {
	char txt[256];
	int i=0;
	for(i=0; i < numcont; i++) {
		counter[i].status=0;
		if(do_ping(counter[i].gw, counter[i].dev, counter[i].ip)==0) {
			counter[i].status=1;
			counter[i].down=0;
		} else {
			if(counter[i].down==0) {
				counter[i].down=1;
				memset(txt,0x0,sizeof(txt));
				snprintf(txt,sizeof(txt),"Uplink [%s] gateway %s seems to be down",mode_type,counter[i].gw);
				log_action("ERROR",txt);sleep(1);
				// log one
				counter[i].down=3;
			}
		}
	}
	if(mode==1) do_single_route();
	if(mode==2) do_tune_route();
}

static void write_status(void) {
	FILE *f;
	int i=0;
	if((f=fopen("/var/sys/uplink_status.tmp","w"))!=NULL) {
		for(i=0; i < numcont; i++) {
			fprintf(f,"%s|%d\n",counter[i].dev,counter[i].status);
		}
		fclose(f);
	}
	rename("/var/sys/uplink_status.tmp","/var/sys/uplink_status");
}
// 1 =true, 0 = false;
int read_config(void) {
	FILE *f;
	char buff[150];
	int n, i=0;
	if(!file_exists("/var/sys/uplink_config") 
		|| !file_exists("/var/sys/uplink_list") 
		|| !file_exists("/var/sys/uplink_chk")
		|| !file_exists("/var/sys/uplink_start")
		|| !file_exists("/var/sys/uplink_stop")) return 0;

	n = read_oneline("/var/sys/uplink_config", buff);
	if(n < 0) return 0;
	trim(buff);
	sscanf(buff,"%d|%d",&mode,&interval);
	memset(buff,0x0,sizeof(buff));
	if((f=fopen("/var/sys/uplink_chk","r"))!=NULL) {
		while(fgets(buff, sizeof(buff), f)) {
			trim(buff);
			if(buff[0]!='\0') {
				sscanf(buff,"%d %s %s %s",&counter[i].prio,counter[i].dev,counter[i].ip,counter[i].gw);
				counter[i].status=0;
				counter[i].down=0;
				i++;
			}
		}
		fclose(f);
	}
	numcont=i;
	i=0;
	memset(buff,0x0,sizeof(buff));
	if((f=fopen("/var/sys/uplink_list","r"))!=NULL) {
		while(fgets(buff, sizeof(buff), f)) {
			trim(buff);
			if(buff[0]!='\0') {
				snprintf(list[i].ip,sizeof(list[i].ip),"%s",buff);
				i++;
			}
		}
		fclose(f);
	}
	listcont=i;
	return(1);
}

static void exit_program(int val) {
	exit(val);
}

int uplinkd_main(int argc,char **argv) {
	char txt[256];
	signal(SIGINT, exit_program);
        signal(SIGTERM, exit_program);
        signal(SIGHUP, exit_program);
        signal(SIGKILL, exit_program);
	freopen("/dev/null", "r", stdin);
      	freopen("/dev/null", "w", stdout);
      	freopen("/dev/null", "w", stderr);
	read_config();
	if(numcont==0 || listcont==0) {
		snprintf(txt,sizeof(txt),"Failed to running Uplink daemon");
		log_action("ERROR",txt);
		exit(1);
	}
	// default
	xsystem("/var/sys/uplink_stop");
	write_status();
	if(daemon(1,1)==0) {
		if(mode==2) {
			snprintf(mode_type,sizeof(mode_type),"Balancer (active->active)");
			xsystem("/var/sys/uplink_start");
		}
		snprintf(txt,sizeof(txt),"Starting Uplink [%s] daemon",mode_type);
		log_action("ERROR",txt);
		for(;;) {
			chkfailover();
			write_status();
			sleep(interval);
		}
		exit(0);
	} else {
		snprintf(txt,sizeof(txt),"Failed to running Uplink daemon");
		log_action("ERROR",txt);
		exit(1);
	}
	exit(0);
}
