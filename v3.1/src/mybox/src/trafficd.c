#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <ipt_ACCOUNT_cl.h>

struct listx {
	char cmd[17];
} listq[10];

static listcnt=0;

char *addr_to_dotted(unsigned int addr) {
	static char buf[17];
	const unsigned char *bytep;
	bytep=(const unsigned char *) &addr;
	snprintf(buf, sizeof(buf), "%u.%u.%u.%u", bytep[0], bytep[1], bytep[2], bytep[3]);
	buf[16]=0;
	return buf;
}

void save_data(const char *table, const char *ip,u_int32_t srcp,u_int32_t srcb,u_int32_t dstp,u_int32_t dstb) {
	time_t timestamp;
        timestamp=time(NULL);
	char dir[255], f[255];
	snprintf(dir,sizeof(dir),"/var/spool/account/%s",table);
	if(!is_dir(dir)) xmkdir(dir);
	/*if(!is_dir(dir)) {
		xmkdir(dir);
		chmod("/var/spool/account",S_IREAD | S_IWRITE | S_IEXEC);
		chmod(dir,S_IREAD | S_IWRITE | S_IEXEC);
	}*/
	snprintf(f,sizeof(f),"%s/%s",dir,ip);
	save_to_file(f,"%d,%s,%u,%u,%u,%u\n",timestamp,ip,srcp,srcb,dstp,dstb);
	//chmod(f,S_IREAD | S_IWRITE | S_IEXEC);
}

static void free_list(void) {
        int i;
        for(i=0;i<listcnt;i++) {
                memset(listq[i].cmd,0x0,sizeof(listq[i].cmd));
        }
        listcnt=0;
}

static int proc_idle() {
        if(file_exists("/var/sys/init_down")) return 1; 
	if(file_exists("/var/sys/trafficd_stop")) return 1;
	if(file_exists("/var/sys/do_single")) return 1;
	// create by policy
	if(!file_exists("/var/sys/init_account") ) return 1;
        return 0;
}

int main(int argc,char **argv) {
	struct ipt_ACCOUNT_context ctx;
	struct ipt_acc_handle_ip *entry;
 	int i=0, listcnt=0, rtn;
        const char *name;
	char doFlush=1;

	//freopen("/dev/null", "r", stdin);
      	//freopen("/dev/null", "w", stdout);
      	//freopen("/dev/null", "w", stderr);

	//strcpy(argv[0],"trafficd    ");

	for(;;) {
		if(proc_idle()) {
			sleep(60);
			continue;
		}
		if(ipt_ACCOUNT_init(&ctx)) {
			sleep(60);
			continue;
		}
        	rtn=ipt_ACCOUNT_get_table_names(&ctx);
        	if(rtn < 0) {
			sleep(60);
			continue;
		}
		listcnt=0;
        	while((name=ipt_ACCOUNT_get_next_name(&ctx)) != 0) {
			snprintf(listq[listcnt].cmd,sizeof(listq[listcnt].cmd),"%s",name);
			listcnt++;
		}
		i=0;
		for(i=0;i<listcnt;i++) {
			if(listq[i].cmd) {
				if(ipt_ACCOUNT_read_entries(&ctx, listq[i].cmd, doFlush)) {
					goto DONE;
				}
				while((entry=ipt_ACCOUNT_get_next_entry(&ctx)) != NULL) {
					save_data(listq[i].cmd,addr_to_dotted(entry->ip),entry->src_packets, entry->src_bytes,entry->dst_packets, entry->dst_bytes);
        			}
				memset(listq[i].cmd,0x0,sizeof(listq[i].cmd));
			}
		}
		DONE:
		free_list();
		ipt_ACCOUNT_deinit(&ctx);
		sleep(10);
	}
	exit(0);
}
