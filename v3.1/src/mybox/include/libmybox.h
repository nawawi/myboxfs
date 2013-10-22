#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <utime.h>
#include <errno.h>
#include <dirent.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <ctype.h>
#include <syslog.h>
#include <libintl.h>
#include <locale.h>

// login.c
#define LANG_DIR "/etc/lang"
char cli_help[50];
char tty_login[32];
void log_login(char *ecode,char *stat);

// str_replace.c
int str_replace_add(char **start, const char *string_add, size_t s);
char *str_replace(const char *string, const char *search_string, const char *replace_string);

// xlib.c
void *xmalloc(size_t size);
void *xstrndup(char *s, size_t n);
void *xstrdup(char *s);
ssize_t xwrite(int fd, const void *buf, size_t count);
char *xstrncpy(char *dst, const char *src, size_t size);
void *xrealloc(void *ptr, size_t size);

// substr.c
char *substr(const char *string, int start, size_t count);

// is_dir.c
int is_dir(const char *s);

// splitc.c
void splitc(char *first, char *rest, char divider);

// save_append_to_file.c
int save_to_file(const char *filename,char *format, ...);
int append_to_file(const char *filename,char *format, ...);

// split_array.c
char **chargv;
int numtokens;
char split_array(const char *s, const char *delimiters, char ***argvp);

// stripchar.c
char *stripchar(char *string, char r);

// trim.c
char *trim(char *s);

// file_exists.c
int file_exists(char *s);
int file_exists_match(char *format, ...);

// xsystem.c
int xsystem(char *format, ...);

// stristr.c
#define NUL '\0'
int stristr(const char *string, const char *pattern);

// xmkdir.c
int xmkdir(char *p);

// read_oneline.c
int read_oneline(const char *filename, void *buf);

// init_lang.c
int init_lang(void);

// get_boottime.c
long get_boottime(void);

// print_banner.c
void print_banner(void);

// prototype -- lcd.c
int lcd_msg(int prog, char *name, char *val);

// Prototype -- mybox.c
int login_main(int argc,char **argv);
