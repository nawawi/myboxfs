/*
 * Copyright (c) 2004 Alexander Eremin <xyzyx@rambler.ru>
 *
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <limits.h>
#include <features.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define BLANK_STRING  " "
#define FALSE ((int) 0)
#define TRUE  ((int) 1)
#define MAX_LINE_LEN 256

#define VERSION "0.4"
#define ZNAK "!\n"

extern char *makeargv(const char *s, const char *delimiters, char ***argvp);
extern char *safe_strncpy(char *dst, const char *src, size_t size);
extern FILE *xfopen(const char *path, const char *mode);
extern void *xmalloc (size_t size);
extern char *dupstr (char *s);
extern struct hostent *xgethostbyname(const char *name);
extern int INET_rresolve(char *name, size_t len, struct sockaddr_in *s_in, int numeric, unsigned int netmask);
extern int create_icmp_socket(void);
extern void vopros(void);
extern char **word_completion (char *text, int start, int end);
extern int resolve (void);
extern int intverify(char *ifname);
extern int cli_run_command (char *cli_input);
extern int cli_conf_init();
extern void cli_readline_init();
extern int cli_telnet(char *name);
extern void cli_help_vopros();
extern char *cli_chomp(char *str);
extern void cli_show_vopros();
extern void cli_show_ip();
extern void cli_show_no();
extern void cli_acl_help();
extern void cli_help_second(char *name);
extern int showtime();
extern int showhist();
extern int showversion();
extern int cli_arp_show();
extern int cli_main();
extern int cli_show_run();
extern int cli_show_start();
extern int ethshow(char *ifname);
extern int ethshowall();
extern void showroute();
extern int ethshow();
extern int cli_acl_show();
extern int ping_main(int argc, char **argv);
extern int tarce_main(int argc, char **argv);
extern int telnet_main(int argc, char **argv);
extern int trace_main(int argc, char *argv[]);
extern int route_main(int argc, char **argv);
extern int hostname_main(int argc, char **argv);
extern int cli_route_def();
extern int ifcon(int argc, char **argv);
extern int check_secret();
extern int secret();
extern int tm();
extern int startup();
extern int noacl();
extern int info(char *iface);
extern int infoall(char *iface);
extern int ipcheck(char *arg);
extern char* makenetmask(unsigned int, char*);
extern void printdata(char*, char*, char*);
extern int bitcount(unsigned int);
extern int nmtoc_main(char*, unsigned int, unsigned int, unsigned int, unsigned int );
extern int validatemask(unsigned int);
extern int wildcard(int argc, char **argv);
extern int cli_acl_st(char *name);
extern int cli_acl_ext(char *name);
extern int cli_access(char *name);
extern int cli_addr(char *name);
extern int acl_stand(int argc, char **argv);
extern int xconnect(struct sockaddr_in *s_addr);
extern unsigned short lookup_port(const char *port, const char *protocol, unsigned short default_port);
extern void lookup_host(struct sockaddr_in *s_in, const char *host);
extern void get_terminal_width_height(int fd, int *width, int *height);
extern int ripd(int argc, char *argv[]);
extern int setclock(int argc, char **argv);
