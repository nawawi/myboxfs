#include <libmybox.h>

int WAN1_PING=0;
int WAN2_PING=0;
int LISTOK=0;
int MODE=0;

static int ip_route(char *opt,char *ip, char *gw) {
	char cmd[80];
	snprintf(cmd,sizeof(cmd),"/bin/ip route %s %s via %s",opt,ip,gw);
	return system(cmd);
}

static int ip_route_default(char *gw, char *dev) {
	char cmd[80];
	snprintf(cmd,sizeof(cmd),"/bin/ip route add default via %s dev %s metric 1",gw,dev);
	return system(cmd);
}

static int update_arp(char *ip, char *dev) {
	char cmd[80];
	snprintf(cmd,sizeof(cmd),"/bin/arping -qb %s -c 3 -w 3 -i %s",ip,dev);
	return system(cmd);
}

static int update_ping(char *gw, char *ip) {
	char cmd[250];
	snprintf(cmd,sizeof(cmd),"/bin/ping -q -c 1 -W 5 -I %s %s",gw,ip);
	return system(cmd);
}

static int exec_ping(char *ip) {
	char cmd[250];
	snprintf(cmd,sizeof(cmd),"/bin/ping -q -c 1 -W 5 %s",ip);
	return system(cmd);
}

static void do_ping(char *ip, char *dev, char *myip, int cnt) {
	char filename[31]="/var/sys/load_balancing_link";
	char buf[1024];
	FILE *f;
	system("/bin/ip route flush cache");
	if(file_exists(filename)) {
		if((f=fopen(filename,"r"))!=NULL) {
			while(fgets(buf, sizeof(buf), f)) {
				trim(buf);
				if(buf[0]!='\0') {
					LISTOK=1;
					ip_route("add",buf,ip);
					if(update_ping(myip,buf)==0) {
						if(cnt==1 && WAN1_PING==0) {
							WAN1_PING=1;
							xtouch("/var/sys/load_balancing_WAN1_UP");
						}
						if(cnt==2 && WAN2_PING==0) {
							WAN2_PING=1;
							xtouch("/var/sys/load_balancing_WAN2_UP");
						}
					}
					update_arp(ip,dev);
					exec_ping(buf);
					ip_route("del",buf,ip);
				}
			}
			fclose(f);
		}
	}
	if(LISTOK==0) {
		if(update_arp(ip,dev)==0) {
			if(cnt==1 && WAN1_PING==0) {
				WAN1_PING=1;
				xtouch("/var/sys/load_balancing_WAN1_UP");
			}
			if(cnt==2 && WAN2_PING==0) {
				WAN2_PING=1;
				xtouch("/var/sys/load_balancing_WAN2_UP");
			}
		}
               	if(exec_ping(ip)==0) {
 			if(cnt==1 && WAN1_PING==0) {
				WAN1_PING=1;
				xtouch("/var/sys/load_balancing_WAN1_UP");
			}
			if(cnt==2 && WAN2_PING==0) {
				WAN2_PING=1;
				xtouch("/var/sys/load_balancing_WAN2_UP");
			}
		}
	}
	sleep(1);
}

static void chkfailover(void) {
	char buff[150];
	char ipgw[25], gwdev[10], ipfa[25], fadev[10], txt[256];
	char ip1[25], ip2[25];
	pid_t pid,status;
	int n;
	char filename[25]="/var/sys/load_balancing";
	if(file_exists("/var/sys/load_balancing_mode1")) {
		MODE=1;
	} else {
		MODE=2;
	}
	if(file_exists(filename) && MODE!=0) {
		pid = fork();
        	if(pid==0) {
			n = read_oneline(filename, buff);
			if(n < 0) exit(0);
			WAN1_PING=0;WAN2_PING=0;LISTOK=0;
			trim(buff);
			splitc(ipgw,buff,'|');
			splitc(gwdev,buff,'|');
			splitc(ipfa,buff,'|');
			splitc(fadev,buff,'|');
			splitc(ip1,buff,'|');
			strcpy(ip2,buff);
			do_ping(ipgw,gwdev,ip1,1);
			do_ping(ipfa,fadev,ip2,2);
			if(MODE==2) {
				if(WAN1_PING==0) {
					unlink("/var/sys/load_balancing_WAN1_UP");
					snprintf(txt,sizeof(txt),"Load balancing (active->active), primary gateway %s seems to be down",ipgw);
					log_action("ERROR",txt);sleep(1);
				}
				if(WAN2_PING==0) {
					unlink("/var/sys/load_balancing_WAN2_UP");
					snprintf(txt,sizeof(txt),"Load balancing (active->active), secondary gateway %s seems to be down",ipfa);
					log_action("ERROR",txt);sleep(1);
				}
			}
			if(MODE==1) {
				if(WAN1_PING==0 && WAN2_PING==1) {
					unlink("/var/sys/load_balancing_WAN1_UP");
					system("/bin/ip route del 0/0");
					ip_route_default(ipfa,fadev);
					snprintf(txt,sizeof(txt),"Load balancing (active->cold), primary gateway %s seems to be down, switching to secondary gateway %s",ipgw,ipfa);
					log_action("INFO",txt);sleep(1);
				}
				if(WAN2_PING==0 && WAN1_PING==1) {
					unlink("/var/sys/load_balancing_WAN2_UP");
					system("/bin/ip route del 0/0");
					ip_route_default(ipgw,gwdev);
					snprintf(txt,sizeof(txt),"Load balancing (active->cold), secondary gateway %s seems to be down, switching to primary gateway %s",ipfa,ipgw);
					log_action("INFO",txt);sleep(1);
				}
				if(WAN2_PING==0 && WAN1_PING==0) {
					unlink("/var/sys/load_balancing_WAN1_UP");
					unlink("/var/sys/load_balancing_WAN2_UP");
					system("/bin/ip route del 0/0");
					ip_route_default(ipgw,gwdev);
					log_action("ERROR","Load balancing (active->cold), all gateway seems to be down!");sleep(1);	
				}
			}
                	exit(0);
		}
		while(wait(&status)!=pid);
	}
}

int failoverd_main(int argc,char **argv) {
	freopen("/dev/null", "r", stdin);
      	freopen("/dev/null", "w", stdout);
      	freopen("/dev/null", "w", stderr);
	if(daemon(1,1)==0) {
		while(1){
			chkfailover();
			if(file_exists("/var/sys/load_balancing_mode1")) {
				sleep(10);
			} else {
				sleep(60);
			}
		}
		exit(0);
	}
	exit(0);
}
