#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <libgen.h>
#include <utime.h>
#include <errno.h>
#include <dirent.h>
#include <wait.h>
#include <ctype.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/poll.h>
#include <linux/kernel.h>
#include <linux/sys.h>
#include <linux/sockios.h>
#include <linux/types.h>
#include <linux/errqueue.h>
#include <net/if.h>
#include <stdarg.h>
#include <sqlite.h>
#include <inttypes.h>
#include <netdb.h>
#include <termios.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <sched.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/icmp.h>
#include <netinet/ip.h>

// global
static const char * const DB_NAME="/strg/mybox/db/system.db";
static const char * const SPACE="                                ";
static const char * const SINGLE_SPACE=" ";
static const char * const LOGPATH="/strg/mybox/logs";

char **chargv;
int numtokens;

// login.c
char tty_login[32];
void log_login(char *stat);

// Prototype -- lib.c 
char *trim(char *s);
char *stripshellargv(char *string);
char *stripchar(char *string, char r);
char *replacechar(char *string, char r, char p);
void splitc(char *first, char *rest, char divider);
char split_array(const char *s, const char *delimiters, char ***argvp);
int str_replace_add(char **start, const char *string_add, size_t s);
void *xmalloc(size_t size);
void *xstrndup(char *s, size_t n);
void *xstrdup(char *s);
int xsystem(char *format, ...);
int xmkdir(char *p);
int xtouch(const char *file);
void xflush_stdout(void);
int file_exists(const char *s);
int is_dir(const char *s);
char *base_name(char **argv);
int read_oneline(const char *filename, void *buf);
void print_file(char *fname);
int save_to_file(const char *filename,char *format, ...);
int append_to_file(const char *filename,char *format, ...);
void clear_screen(void);
void log_action(char *type, char *msg);
#define MAX_ETH 100
struct devinfo {
        char name[10];
} eth_list[MAX_ETH];
int get_eth(void);
void print_date(void);
void fprintf_stdout(char *format, ...);
int mk_dev(char *file,int major, int minor);
void fixup_argv(char **argv,char *name);
void print_banner(void);

// prototype -- lcd.c
int lcd_msg(int prog, char *name, char *val);

// chkpro.c
void ips_clear_iptab(void);

// Prototype -- mybox.c
int bootup_main(int argc,char **argv);
int bootdown_main(int argc,char **argv);
int initrc_main(int argc,char **argv);
int logtail_main(int argc,char **argv);
int getkey_main(int argc,char **argv);
int login_main(int argc,char **argv);
int lcdd_main(int argc,char **argv);
int sqlite_main(int argc,char **argv);
int iosh_main(int argc,char **argv);
int uplinkd_main(int argc,char **argv);
int chkprog_main(int argc,char **argv);
int ping_main(int argc, char **argv);
int taskq_main(int argc, char **argv);
