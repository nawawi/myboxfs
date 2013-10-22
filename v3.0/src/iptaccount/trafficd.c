#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <ipt_ACCOUNT_cl.h>

struct listx {
	char cmd[17];
} listq[10];

char *addr_to_dotted(unsigned int addr) {
    static char buf[17];
    const unsigned char *bytep;
    bytep=(const unsigned char *) &addr;
    snprintf(buf, 16, "%u.%u.%u.%u", bytep[0], bytep[1], bytep[2], bytep[3]);
    buf[16]=0;
    return buf;
}

ssize_t xwrite(int fd, const void *buf, size_t count) {
        ssize_t n;
        do {
                n = write(fd, buf, count);
        } while (n < 0 && errno == EINTR);
        return n;
}

void *xmalloc(size_t size) {
        void *ptr=malloc(size);
        if(!ptr) {
            printf("malloc() Out of memory, exiting\n");
            exit(1);
        }
        return ptr;
}

int save_to_file(const char *filename,char *format, ...) {
        va_list va;
        int len, fd;
        char *buf;
	ssize_t ret;
        va_start(va, format);
        len = vsnprintf(0, 0, format, va);
        len++;
        va_end(va);
        buf = xmalloc(len);
        va_start(va, format);
        vsnprintf(buf, len, format, va);
        va_end(va);
        fd=open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
        if(fd < 0) return -1;
        ret=xwrite(fd, buf, strlen(buf));
        close(fd);
        return ret;
}

int is_dir(const char *s) {
        struct stat ss;
        int i=stat(s, &ss);
        if (i < 0) return(0);
        if ((ss.st_mode & S_IFREG) || (ss.st_mode & S_IFDIR)) return(1);
        return(0);
}

int xmkdir(char *p) {
	long mode=0700;
        mode_t mask;
        char *s=xmalloc(strlen(p) * sizeof(char)); 
        char *path=xmalloc(strlen(p) * sizeof(char)); 
        char c;
        struct stat st;
        path=strdup(p);
        s=path;
        mask=umask(0);
	umask(mask & ~0300);

        do {
		c=0;
		while(*s) {
 			if (*s=='/') {
 				do {
 					++s;
 				} while (*s == '/');
				c = *s;
				*s = 0;
				break;
			}
			++s;
		}
                if(mkdir(path, 0700) < 0) {
                        if(errno != EEXIST || (stat(path, &st) < 0 || !S_ISDIR(st.st_mode))) {
				umask(mask);
                                break;
                        }
                        if(!c) {
				umask(mask);
 				return 0;
 			}
                }

                if(!c) {
			umask(mask);
                        if(chmod(path, mode) < 0) {
				break;
			}
			return 0;
                }
                *s=c;
        } while (1);
        return -1;
}

void save_data(const char *table, const char *ip,u_int32_t srcp,u_int32_t srcb,u_int32_t dstp,u_int32_t dstb) {
	char dir[255], f[255];
	snprintf(dir,sizeof(dir),"/var/spool/account/%s",table);
	if(!is_dir(dir)) xmkdir(dir);
	snprintf(f,sizeof(f),"%s/%s",dir,ip);
	save_to_file(f,"%s,%u,%u,%u,%u\n",ip,srcp,srcb,dstp,dstb);
}

static void exit_program(int val) {
	exit(val);
}

int main(int argc, char *argv[]) {
	struct ipt_ACCOUNT_context ctx;
	struct ipt_acc_handle_ip *entry;
 	int i=0, listcnt=0, rtn;
        const char *name, *name2;
	char doFlush=0;

	signal(SIGINT, exit_program);
        signal(SIGTERM, exit_program);
        signal(SIGHUP, exit_program);
        signal(SIGKILL, exit_program);

	if(daemon(1,1)==0) {
		for(;;) {
			if(ipt_ACCOUNT_init(&ctx)) continue;
        		rtn=ipt_ACCOUNT_get_table_names(&ctx);
        		if(rtn < 0) continue;
			listcnt=0;
        		while((name=ipt_ACCOUNT_get_next_name(&ctx)) != 0) {
				snprintf(listq[listcnt].cmd,sizeof(listq[listcnt].cmd),"%s",name);
				listcnt++;
			}
			i=0;
			for(i=0;i<listcnt;i++) {
				if(listq[i].cmd) {
					if(ipt_ACCOUNT_read_entries(&ctx, listq[i].cmd, !doFlush)) {
						goto ER;
					}
 					while((entry=ipt_ACCOUNT_get_next_entry(&ctx)) != NULL) {
						save_data(listq[i].cmd,addr_to_dotted(entry->ip),entry->src_packets, entry->src_bytes,
                                   			entry->dst_packets, entry->dst_bytes);
            				}
					memset(listq[i].cmd,0x0,sizeof(listq[i].cmd));
				}
			}
			ER:
			ipt_ACCOUNT_deinit(&ctx);
			sleep(5);
    		}
	}
	exit(0);
}
