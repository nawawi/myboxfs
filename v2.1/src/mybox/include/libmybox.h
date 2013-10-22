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
#include <linux/kernel.h>
#include <linux/sys.h>
#include <stdarg.h>
#include <sqlite.h>
#include <inttypes.h>
#include <netdb.h>
#include <termios.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>

// procps.c
#ifndef COMM_LEN
#ifdef TASK_COMM_LEN
#define COMM_LEN TASK_COMM_LEN
#else
#define COMM_LEN 16
#endif
#endif

typedef struct  {
        int pid;
        char user[9];
        char state[4];
        unsigned long rss;
        int ppid;
        unsigned pcpu;
        unsigned pscpu;
        unsigned long stime;
        unsigned long utime;
        char *cmd;
        char short_cmd[COMM_LEN];
} procps_status_t;
procps_status_t * procps_scan(int dotask);

// global
static const char * const DB_NAME="/strg/mybox/db/system.db";
static const char * const SPACE="                                ";
static const char * const SINGLE_SPACE=" ";
static const char * const LOGPATH="/strg/mybox/logs";

char **chargv;
int numtokens;

// Prototype/variable -- db.c
#define SQL_COLNAME_SIZE 255
#define SQL_COLVALUE_SIZE 10000
#define SQL_RESULT_MAX 10000
int SQL_NUMROWS;
struct SQL_QUERY {
        char name[SQL_COLNAME_SIZE];
        char value[SQL_COLVALUE_SIZE];
} SQL_RESULT[SQL_RESULT_MAX];
sqlite *db_connect(const char *database);
void db_clean_buffer(void);
void db_flush(sqlite *db_id);
void db_close(sqlite *db_id);
int db_callback(void *args,int numrows, char **colresults, char **colnames);
int db_query(sqlite *db_id,const char *sql);
char *db_get_single(sqlite *db_id,const char *sql);

// Prototype -- lib.c 
char *trim(char *s);
char *stripshellargv(char *string);
char *stripchar(char *string, char r);
void splitc(char *first, char *rest, char divider);
char split_array(const char *s, const char *delimiters, char ***argvp);
int str_replace_add(char **start, const char *string_add, size_t s);
char *str_replace(const char *string, const char *search_string, const char *replace_string);
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


// Prototype/variable -- md5.c 
int MD5Password(char *pass, char *pass1);
char MD5Values[255];
void MDString(char *inString);

// prototype -- lcd.c
int lcd_msg(int prog, char *name, char *val);

// Prototype -- mybox.c
int bootup_main(int argc,char **argv);
int bootdown_main(int argc,char **argv);
int initrc_main(int argc,char **argv);
int logtail_main(int argc,char **argv);
int getkey_main(int argc,char **argv);
int login_main(int argc,char **argv);
int lcdd_main(int argc,char **argv);
int sqlite_main(int argc,char **argv);
int basename_main(int argc,char **argv);
int iosh_main(int argc,char **argv);
int failoverd_main(int argc,char **argv);
int chkprog_main(int argc,char **argv);
int pwd_main(int argc,char **argv);
int ping_main(int argc, char **argv);
int arpscan_main(int argc, char **argv);
int ps_main(int argc, char **argv);
int top_main(int argc, char **argv);
int reset_main(int argc, char **argv);
int clear_main(int argc, char **argv);
int uptime_main(int argc, char **argv);
int sync_main(int argc, char **argv);
int mkdir_main(int argc, char **argv);
int mkdev_main(int argc, char **argv);
int sleep_main(int argc, char **argv);
int usleep_main(int argc, char **argv);
int false_main(int argc, char **argv);
int true_main(int argc, char **argv);

