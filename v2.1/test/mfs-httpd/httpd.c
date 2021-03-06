#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>

#include "match.h"
#include "tdate_parse.h"


#include <sys/sendfile.h>

#ifdef USE_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif /* USE_SSL */


#if defined(AF_INET6) && defined(IN6_IS_ADDR_V4MAPPED)
#define USE_IPV6
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#ifndef SHUT_WR
#define SHUT_WR 1
#endif

#ifndef SIZE_T_MAX
#define SIZE_T_MAX 2147483647L
#endif

#ifndef HAVE_INT64T
typedef long long int64_t;
#endif

#ifdef __CYGWIN__
#define timezone  _timezone
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


#ifndef DEFAULT_HTTP_PORT
#define DEFAULT_HTTP_PORT 5051
#endif /* DEFAULT_HTTP_PORT */

#ifndef DEFAULT_HTTP_CONFIG
#define DEFAULT_HTTP_CONFIG "/etc/httpsd.cnf"
#endif /* DEFAULT_HTTP_CONFIG */

#ifdef USE_SSL

#ifndef DEFAULT_HTTPS_PORT
#define DEFAULT_HTTPS_PORT 5051
#endif /* DEFAULT_HTTPS_PORT */

#ifndef DEFAULT_CERTFILE
#define DEFAULT_CERTFILE "/etc/openssl.pem"
#endif /* DEFAULT_CERTFILE */

#endif /* USE_SSL */

#ifndef DEFAULT_USER
#define DEFAULT_USER "mfs"
#endif /* DEFAULT_USER */

#ifndef CGI_NICE
#define CGI_NICE 10
#endif /* CGI_NICE */

#ifndef CGI_PATH
#define CGI_PATH "/bin:/service/www:/service/tools:/service/init"
#endif /* CGI_PATH */

#ifndef CGI_LD_LIBRARY_PATH
#define CGI_LD_LIBRARY_PATH "/lib"
#endif /* CGI_LD_LIBRARY_PATH */

#ifndef READ_TIMEOUT
#define READ_TIMEOUT 120
#endif /* READ_TIMEOUT */

#ifndef WRITE_TIMEOUT
#define WRITE_TIMEOUT 600
#endif /* WRITE_TIMEOUT */

#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET "iso-8859-1"
#endif /* DEFAULT_CHARSET */

#define METHOD_UNKNOWN 0
#define METHOD_GET 1
#define METHOD_HEAD 2
#define METHOD_POST 3

#define SERVER_SOFTWARE_VERSION "\nMyBox Firewall System MyADMIN HTTPS Agent\n\nBased on Mini_httpd. This is a Mybox strip down version of mini_httpd\nAct as a simple https service.\n\nFor original source copy you can download from\nhttp://www.acme.com/software/mini_httpd/mini_httpd-1.19.tar.gz\n\n"
#define SERVER_SOFTWARE "MWS/2.1"

/* A multi-family sockaddr. */
typedef union {
    struct sockaddr sa;
    struct sockaddr_in sa_in;
#ifdef USE_IPV6
    struct sockaddr_in6 sa_in6;
    struct sockaddr_storage sa_stor;
#endif /* USE_IPV6 */
    } usockaddr;

typedef struct {
	int			cpid;	/* child PID - 0 if unused */
	in_addr_t	caddr;	/* client address */
} conninfo;

static char *argv0;
static int debug;
static unsigned short port;
static int maxproc;
static int currproc;
static sigset_t sigchildset;
static conninfo* clients;
static int maxperip;
static char *dir;
static char *user;
static char *cgi_pattern;
static char *hostname;
static char hostname_buf[500];
static char *pidfile;
static char *charset;
static int max_age;
static FILE* logfp;
static int listen4_fd, listen6_fd;
static int do_syslog;
#ifdef USE_SSL
static int do_ssl;
static char *certfile;
static char *cipher;
static SSL_CTX* ssl_ctx;
#endif /* USE_SSL */
static char cwd[MAXPATHLEN];
static int got_hup;


/* Request variables. */
static int conn_fd;
#ifdef USE_SSL
static SSL* ssl;
#endif /* USE_SSL */
static usockaddr client_addr;
static char *request;
static size_t request_size, request_len, request_idx;
static int method;
static char *path;
static char *file;
static char *pathinfo;
struct stat sb;
static char *query;
static char *protocol;
static int status;
static off_t bytes;
static char *req_hostname;

static char *authorization;
static size_t content_length;
static char *content_type;
static char *cookie;
static char *host;
static time_t if_modified_since;
static char *referer;
static char *useragent;

static char *remoteuser;


/* Forwards. */
static void usage(void);
static void read_config(char *filename);
static void value_required(char *name,char *value);
static void no_value_required(char *name,char *value);
static int initialize_listen_socket(usockaddr* usaP);
static void handle_request(void);
static void de_dotdot(char *file);
static int get_pathinfo(void);
static void do_file(void);
static void do_cgi(void);
static void cgi_interpose_input(int wfd);
static void post_post_garbage_hack(void);
static void cgi_interpose_output(int rfd, int parse_headers);
static char** make_argp(void);
static char** make_envp(void);
static char *build_env(char *fmt,char *arg);
static void send_error(int s,char *title,char *extra_header,char *text);
static void send_error_body(int s,char *title,char *text);
static void add_headers(int s,char *title,char *extra_header,char *me,char *mt, off_t b, time_t mod);
static void start_request(void);
static void add_to_request(char *str, size_t len);
static char *get_request_line(void);
static void start_response(void);
static void add_to_response(char *str, size_t len);
static void send_response(void);
static void send_via_write(int fd, off_t size);
static ssize_t my_read(char *buf, size_t size);
static ssize_t my_write(char *buf, size_t size);
static int my_sendfile(int fd, int socket, off_t offset, size_t nbytes);
static void add_to_buf(char** bufP, size_t* bufsizeP, size_t* buflenP,char *str, size_t len);
static void syslog_log_entry(int status,char *text);
static char *get_method_str(int m);
static void init_mime(void);
static const char *figure_mime(char *name,char *me, size_t me_size);
static void handle_sigterm(int sig);
static void handle_sighup(int sig);
static void handle_sigchld(int sig);
static void handle_read_timeout(int sig);
static void handle_write_timeout(int sig);
static void lookup_hostname(usockaddr* usa4P, size_t sa4_len, int* gotv4P, usockaddr* usa6P, size_t sa6_len, int* gotv6P);
static char *ntoa(usockaddr* usaP);
static int sockaddr_check(usockaddr* usaP);
static size_t sockaddr_len(usockaddr* usaP);
static void strdecode(char *to,char *from);
static int hexit(char c);
static int b64_decode(const char *str, unsigned char *space, int size);
static void set_ndelay(int fd);
static void clear_ndelay(int fd);
static void *e_malloc(size_t size);
static void *e_realloc(void *optr, size_t size);
static char *e_strdup(char *ostr);
static int file_exist(char *s);

int main(int argc,char **argv) {
	int argn;
	struct passwd* pwd;
	uid_t uid=32767;
	gid_t gid=32767;
	usockaddr host_addr4;
	usockaddr host_addr6;
	int gotv4, gotv6;
	fd_set lfdset;
	int maxfd;
	usockaddr usa;
	int sz, r;
	char *cp;

	/* Parse args. */
	argv0=argv[0];
	debug=0;
	port=0;
	maxproc=16;
	maxperip=5;
	currproc=0;
	dir=(char*) 0;
	cgi_pattern="**.exh|**.exc|**.exl";
	charset=DEFAULT_CHARSET;
	max_age=-1;
	user=DEFAULT_USER;
	hostname=(char*) 0;
	pidfile=(char*) 0;
	logfp=(FILE*) 0;
	do_syslog=1;
#ifdef USE_SSL
	do_ssl=0;
	certfile=DEFAULT_CERTFILE;
	cipher=(char*) 0;
#endif /* USE_SSL */
	argn=1;
	while (argn < argc && argv[argn][0] == '-') {
		if (strcmp(argv[argn], "-V") == 0) {
			(void) printf("%s\n", SERVER_SOFTWARE_VERSION);
			exit(0);
		} else if (strcmp(argv[argn], "-C") == 0 && argn + 1 < argc) {
			++argn;
			read_config(argv[argn]);
		} else if (strcmp(argv[argn], "-D") == 0) {
			debug=1;
#ifdef USE_SSL
		} else if (strcmp(argv[argn], "-S") == 0) {
			do_ssl=1;
		} else if (strcmp(argv[argn], "-E") == 0 && argn + 1 < argc) {
			++argn;
			certfile=argv[argn];
		} else if (strcmp(argv[argn], "-Y") == 0 && argn + 1 < argc) {
			++argn;
			cipher=argv[argn];
		
#endif /* USE_SSL */
		} else if (strcmp(argv[argn], "-p") == 0 && argn + 1 < argc) {
			++argn;
			port=(unsigned short) atoi(argv[argn]);
		} else if (strcmp(argv[argn], "-maxproc") == 0 && argn + 1 < argc) {
			++argn;
			maxproc=(unsigned short) atoi(argv[argn]);
			if(maxproc <= 0) usage();
		} else if (strcmp(argv[argn], "-maxperip") == 0 && argn + 1 < argc) {
			++argn;
			maxperip = (unsigned short) atoi(argv[argn]);
			if(maxperip < 0) usage();
		} else if (strcmp(argv[argn], "-d") == 0 && argn + 1 < argc) {
			++argn;
			dir=argv[argn];
		} else if (strcmp(argv[argn], "-c") == 0 && argn + 1 < argc) {
			++argn;
			cgi_pattern=argv[argn];
		} else if (strcmp(argv[argn], "-u") == 0 && argn + 1 < argc) {
			++argn;
			user=argv[argn];
		} else if (strcmp(argv[argn], "-h") == 0 && argn + 1 < argc) {
			++argn;
			hostname=argv[argn];
		} else if (strcmp(argv[argn], "-i") == 0 && argn + 1 < argc) {
			++argn;
			pidfile=argv[argn];
		} else if (strcmp(argv[argn], "-T") == 0 && argn + 1 < argc) {
			++argn;
			charset=argv[argn];
		} else if (strcmp(argv[argn], "-M") == 0 && argn + 1 < argc) {
			++argn;
			max_age=atoi(argv[argn]);
		} else if (strcmp(argv[argn], "-nosyslog") == 0) {
			do_syslog=0;
		} else {
			usage();
		}
		++argn;
	}
	if(argn != argc) usage();
	if(file_exist(DEFAULT_HTTP_CONFIG)) read_config(DEFAULT_HTTP_CONFIG);
	if(maxproc < maxperip) usage();

	cp=strrchr(argv0, '/');
	if (cp != (char*) 0) {
		++cp;
	} else {
		cp=argv0;
	}
	if(do_syslog) openlog(cp, LOG_NDELAY|LOG_PID, LOG_DAEMON);

	if(maxperip != 0) {
		int i;
		clients = e_malloc(sizeof(conninfo) * maxproc);
		for (i = 0; i < maxproc; i++) clients[i].cpid = 0;
		sigemptyset(&sigchildset);
		sigaddset(&sigchildset, SIGCHLD);
	}

	if (port == 0) {
#ifdef USE_SSL
		if(do_ssl) {
			port=DEFAULT_HTTPS_PORT;
		} else {
			port=DEFAULT_HTTP_PORT;
		}
#else /* USE_SSL */
		port=DEFAULT_HTTP_PORT;
#endif /* USE_SSL */
	}

	/* If we're root and we're going to become another user, get the uid/gid
	** now.
	*/
	if (getuid() == 0) {
		pwd=getpwnam(user);
		if(pwd == (struct passwd*) 0) {
			if(do_syslog) syslog(LOG_CRIT, "unknown user - '%s'", user);
			if(debug) (void) fprintf(stderr, "%s: unknown user - '%s'\n", argv0, user);
			exit(1);
		}
		uid=pwd->pw_uid;
		gid=pwd->pw_gid;
	}

	/* Look up hostname. */
	lookup_hostname(&host_addr4, sizeof(host_addr4), &gotv4,&host_addr6, sizeof(host_addr6), &gotv6);
	if (hostname == (char*) 0) {
		(void) gethostname(hostname_buf, sizeof(hostname_buf));
		hostname=hostname_buf;
	}
	if (! (gotv4 || gotv6)) {
		if(do_syslog) syslog(LOG_CRIT, "can't find any valid address");
		if(debug) (void) fprintf(stderr, "%s: can't find any valid address\n", argv0);
		exit(1);
	}

	/* Initialize listen sockets.  Try v6 first because of a Linux peculiarity;
	** like some other systems, it has magical v6 sockets that also listen for
	** v4, but in Linux if you bind a v4 socket first then the v6 bind fails.
	*/
	if (gotv6) {
		listen6_fd=initialize_listen_socket(&host_addr6);
	} else {
		listen6_fd=-1;
	}
	if (gotv4) {
		listen4_fd=initialize_listen_socket(&host_addr4);
	} else {
		listen4_fd=-1;
	}
    /* If we didn't get any valid sockets, fail. */
	if (listen4_fd == -1 && listen6_fd == -1) {
		if(do_syslog) syslog(LOG_CRIT, "can't bind to any address");
		if(debug) (void) fprintf(stderr, "%s: can't bind to any address\n", argv0);
		exit(1);
	}

#ifdef USE_SSL
	if(do_ssl) {
		SSL_load_error_strings();
		SSLeay_add_ssl_algorithms();
		ssl_ctx=SSL_CTX_new(SSLv23_server_method());
		if (certfile[0] != '\0') {
			if (SSL_CTX_use_certificate_file(ssl_ctx, certfile, SSL_FILETYPE_PEM) == 0 || 
				SSL_CTX_use_PrivateKey_file(ssl_ctx, certfile, SSL_FILETYPE_PEM) == 0 ||
				SSL_CTX_check_private_key(ssl_ctx) == 0) {
					ERR_print_errors_fp(stderr);
					exit(1);
			}
		}
		if (cipher != (char*) 0) {
			if (SSL_CTX_set_cipher_list(ssl_ctx, cipher) == 0) {
				ERR_print_errors_fp(stderr);
				exit(1);
			}
		}
	}
#endif /* USE_SSL */

	if (! debug) {
		/* Make ourselves a daemon. */
		if (daemon(1, 1) < 0) {
			if(do_syslog) syslog(LOG_CRIT, "daemon - %m");
			perror("daemon");
			exit(1);
		}
	} else {
		/* Even if we don't daemonize, we still want to disown our parent
		** process.
		*/

		(void) setsid();

	}

	if (pidfile != (char*) 0) {
		/* Write the PID file. */
		FILE *pidfp=fopen(pidfile, "w");
		if (pidfp == (FILE*) 0) {
			if(do_syslog) syslog(LOG_CRIT, "%s - %m", pidfile);
			perror(pidfile);
			exit(1);
		}
		(void) fprintf(pidfp, "%d\n", (int) getpid());
		(void) fclose(pidfp);
	}

	/* Read zone info now, in case we chroot(). */
	tzset();

	/* If we're root, start becoming someone else. */
	if (getuid() == 0) {
		/* Set aux groups to null. */
		if (setgroups(0, (gid_t*) 0) < 0) {
			if(do_syslog) syslog(LOG_CRIT, "setgroups - %m");
			perror("setgroups");
			exit(1);
		}
		/* Set primary group. */
		if (setgid(gid) < 0) {
			if(do_syslog) syslog(LOG_CRIT, "setgid - %m");
			perror("setgid");
			exit(1);
		}
		/* Try setting aux groups correctly - not critical if this fails. */
		if (initgroups(user, gid) < 0) {
			if(do_syslog) syslog(LOG_ERR, "initgroups - %m");
			perror("initgroups");
		}
	}

	/* Switch directories if requested. */
	if (dir != (char*) 0) {
		if (chdir(dir) < 0) {
			if(do_syslog) syslog(LOG_CRIT, "chdir - %m");
			perror("chdir");
			exit(1);
		}
	}

	/* Get current directory. */
	(void) getcwd(cwd, sizeof(cwd) - 1);
	if (cwd[strlen(cwd) - 1] != '/') (void) strcat(cwd, "/");

	/* If we're root, become someone else. */
	if (getuid() == 0) {
		/* Set uid. */
		if (setuid(uid) < 0) {
			if(do_syslog) syslog(LOG_CRIT, "setuid - %m");
			perror("setuid");
			exit(1);
		}
	}

	/* Catch various signals. */
	(void) signal(SIGTERM, handle_sigterm);
	(void) signal(SIGINT, handle_sigterm);
	(void) signal(SIGUSR1, handle_sigterm);
	(void) signal(SIGHUP, handle_sighup);
	(void) signal(SIGCHLD, handle_sigchld);
	(void) signal(SIGPIPE, SIG_IGN);

	got_hup=0;

	init_mime();
    	if(hostname==(char*) 0) {
		//if(do_syslog) syslog(LOG_NOTICE, "%.80s starting on port %d", SERVER_SOFTWARE,(int) port);
		if(debug) (void) fprintf(stderr, "%s: %.80s starting on port %d\n", argv0, SERVER_SOFTWARE,(int) port);
	} else {
		//if(do_syslog) syslog(LOG_NOTICE, "%.80s starting on %.80s, port %d", SERVER_SOFTWARE,hostname, (int) port );
		if(debug) (void) fprintf(stderr, "%s: %.80s starting on %.80s, port %d\n", argv0,SERVER_SOFTWARE,hostname, (int) port);
	}

	/* Main loop. */
	for (;;) {
		if(got_hup) {
			got_hup=0;
		}

		/* Do a select() on at least one and possibly two listen fds.
		** If there's only one listen fd then we could skip the select
		** and just do the (blocking) accept(), saving one system call;
		** that's what happened up through version 1.18.  However there
		** is one slight drawback to that method: the blocking accept()
		** is not interrupted by a signal call.  Since we definitely want
		** signals to interrupt a waiting server, we use select() even
		** if there's only one fd.
		*/
		FD_ZERO(&lfdset);
		maxfd=-1;
		if (listen4_fd != -1) {
			FD_SET(listen4_fd, &lfdset);
			if (listen4_fd > maxfd) maxfd=listen4_fd;
		}
		if (listen6_fd != -1) {
			FD_SET(listen6_fd, &lfdset);
			if (listen6_fd > maxfd) maxfd=listen6_fd;
		}
		if (select(maxfd + 1, &lfdset, (fd_set*) 0, (fd_set*) 0, (struct timeval*) 0) < 0) {
			if (errno == EINTR || errno == EAGAIN) continue;	/* try again */
			if(do_syslog) syslog(LOG_CRIT, "select - %m");
			perror("select");
			exit(1);
		}

		/* Accept the new connection. */
		sz=sizeof(usa);
		if (listen4_fd != -1 && FD_ISSET(listen4_fd, &lfdset)) {
			conn_fd=accept(listen4_fd, &usa.sa, &sz);
		} else if (listen6_fd != -1 && FD_ISSET(listen6_fd, &lfdset)) {
			conn_fd=accept(listen6_fd, &usa.sa, &sz);
		} else {
			if(do_syslog) syslog(LOG_CRIT, "select failure");
			if(debug) (void) fprintf(stderr, "%s: select failure\n", argv0);
			exit(1);
		}
		if (conn_fd < 0) {
			if (errno == EINTR || errno == EAGAIN || errno == ECONNABORTED) continue;	/* try again */

			if(do_syslog) syslog(LOG_CRIT, "accept - %m");
			perror("accept");
			exit(1);
		}

		/* If we've reach max child procs, then send back server busy error */
		if (currproc >= maxproc) {
			close(conn_fd) ;
			continue ;
		}

		sigprocmask(SIG_BLOCK, &sigchildset, NULL);

		/* If maxperip is enabled, count the number of existing connections
		from this client and close the connection if the max is exceeded. */
		if(maxperip != 0) {
			int i;
			int nconns = 0;	
			for (i = 0; i < maxproc; i++) {
				if (clients[i].cpid == 0) {
					continue;
				} else {
					if (clients[i].caddr == usa.sa_in.sin_addr.s_addr) nconns++;
				}
			}

			if (nconns >= maxperip) {
				close(conn_fd);
				sigprocmask(SIG_UNBLOCK, &sigchildset, NULL);
				continue;
			}
		}

		/* Fork a sub-process to handle the connection. */
		r=fork();

		if (r < 0) {
			if(do_syslog) syslog(LOG_CRIT, "fork - %m");
			perror("fork");
			exit(1);
		}
		if (r == 0) {
			/* Child process. */
			client_addr=usa;
			if (listen4_fd != -1) (void) close(listen4_fd);
			if (listen6_fd != -1) (void) close(listen6_fd);
			handle_request();
			exit(0);
		}
		currproc++;
		if (maxperip != 0) {
			int i;	
			/* record in table of clients */
			for (i = 0; i < maxproc; i++) {
				if (clients[i].cpid == 0) {
					clients[i].cpid = r;
					clients[i].caddr = usa.sa_in.sin_addr.s_addr;
					break;
				}
			}
			if(i == maxproc) {
				if(do_syslog) syslog(LOG_CRIT, "client connection table full!");
				if(debug) (void) fprintf(stderr, "%s: client connection table full!\n", argv0);
			}
		}
		sigprocmask(SIG_UNBLOCK, &sigchildset, NULL);
		(void) close(conn_fd);
	}
}



static void usage(void) {
#ifdef USE_SSL
	(void) fprintf(stderr, "usage:  %s [-C configfile] [-D] [-S] [-E certfile] [-Y cipher] [-p port] [-d dir] [-c cgipat] [-u user] [-h hostname] [-i pidfile] [-T charset] [-M maxage] [-maxproc max_concurrent_procs] [-maxperip max_concurrent_procs_per_ip] [nosyslog]\n", argv0);
#else /* USE_SSL */
	(void) fprintf(stderr, "usage:  %s [-C configfile] [-D] [-p port] [-d dir] [-c cgipat] [-u user] [-h hostname] [-i pidfile] [-T charset] [-M maxage] [-maxproc max_concurrent_procs] [-maxperip max_concurrent_procs_per_ip] [nosyslog]\n", argv0);
#endif /* USE_SSL */
	exit(1);
}


static void read_config(char *filename) {
	FILE* fp;
	char line[10000];
	char *cp;
	char *cp2;
	char *name;
	char *value;

	fp=fopen(filename, "r");
	if (fp == (FILE*) 0) {
		if(do_syslog) syslog(LOG_CRIT, "%s - %m", filename);
		perror(filename);
		exit(1);
	}

	while (fgets(line, sizeof(line), fp) != (char*) 0) {
		/* Trim comments. */
		if ((cp=strchr(line, '#')) != (char*) 0) *cp='\0';
		if ((cp=strchr(line, ';')) != (char*) 0) *cp='\0';
		/* Skip leading whitespace. */
		cp=line;
		cp += strspn(cp, " \t\012\015");

		/* Split line into words. */
		while (*cp != '\0') {
			/* Find next whitespace. */
			cp2=cp + strcspn(cp, " \t\012\015");
			/* Insert EOS and advance next-word pointer. */
			while (*cp2 == ' ' || *cp2 == '\t' || *cp2 == '\012' || *cp2 == '\015') *cp2++='\0';
			/* Split into name and value. */
			name=cp;
			value=strchr(name, '=');
			if (value != (char*) 0) *value++='\0';
			/* Interpret. */
			if (strcasecmp(name, "debug") == 0) {
				no_value_required(name, value);
				debug=1;
			} else if (strcasecmp(name, "port") == 0) {
				value_required(name, value);
				port=(unsigned short) atoi(value);
			} else if (strcasecmp(name, "dir") == 0) {
				value_required(name, value);
				dir=e_strdup(value);
			} else if (strcasecmp(name, "user") == 0) {
				value_required(name, value);
				user=e_strdup(value);
			} else if (strcasecmp(name, "cgipat") == 0) {
				value_required(name, value);
				cgi_pattern=e_strdup(value);
			} else if (strcasecmp(name, "host") == 0) {
				value_required(name, value);
				hostname=e_strdup(value);
			} else if (strcasecmp(name, "pidfile") == 0) {
				value_required(name, value);
				pidfile=e_strdup(value);
			} else if (strcasecmp(name, "charset") == 0) {
				value_required(name, value);
				charset=e_strdup(value);
			} else if (strcasecmp(name, "max_age") == 0) {
				value_required(name, value);
				max_age=atoi(value);
			} else if (strcasecmp(name, "max_proc") == 0) {
				value_required(name, value);
				maxproc=atoi(value);
			} else if (strcasecmp(name, "max_perip") == 0) {
				value_required(name, value);
				maxperip=atoi(value);
			} else if (strcasecmp(name, "nosyslog") == 0) {
				no_value_required(name, value);
				do_syslog=0;
			}
#ifdef USE_SSL
			else if (strcasecmp(name, "ssl") == 0) {
				no_value_required(name, value);
				do_ssl=1;
			} else if (strcasecmp(name, "certfile") == 0) {
				value_required(name, value);
				certfile=e_strdup(value);
			} else if (strcasecmp(name, "cipher") == 0) {
				value_required(name, value);
				cipher=e_strdup(value);
			}
#endif /* USE_SSL */
			else {
				if(debug) (void) fprintf(stderr, "%s: unknown config option '%s'\n", argv0, name);
				exit(1);
			}

			/* Advance to next word. */
			cp=cp2;
			cp += strspn(cp, " \t\012\015");
		}
	}

	(void) fclose(fp);
}


static void value_required(char *name,char *value) {
	if (value == (char*) 0) {
		if(debug) (void) fprintf(stderr, "%s: value required for %s option\n", argv0, name);
		exit(1);
	}
}


static void no_value_required(char *name,char *value) {
	if (value != (char*) 0) {
		if(debug) (void) fprintf(stderr, "%s: no value required for %s option\n",argv0, name);
		exit(1);
	}
}


static int initialize_listen_socket(usockaddr *usaP) {
	int listen_fd;
	int i;

	/* Check sockaddr. */
	if (! sockaddr_check(usaP)) {
		if(do_syslog) syslog(LOG_ERR, "unknown sockaddr family on listen socket - %d",usaP->sa.sa_family);
		if(debug) (void) fprintf(stderr, "%s: unknown sockaddr family on listen socket - %d\n",argv0, usaP->sa.sa_family);
		return -1;
	}

	listen_fd=socket(usaP->sa.sa_family, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		if(do_syslog) syslog(LOG_CRIT, "socket %.80s - %m", ntoa(usaP));
		perror("socket");
		return -1;
	}

	(void) fcntl(listen_fd, F_SETFD, 1);

	i=1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*) &i, sizeof(i)) < 0) {
		if(do_syslog) syslog(LOG_CRIT, "setsockopt SO_REUSEADDR - %m");
		perror("setsockopt SO_REUSEADDR");
		return -1;
	}

	if (bind(listen_fd, &usaP->sa, sockaddr_len(usaP)) < 0) return -1;
	if (listen(listen_fd, 1024) < 0) {
		if(do_syslog) syslog(LOG_CRIT, "listen - %m");
		perror("listen");
		return -1;
	}

	return listen_fd;
}


/* This runs in a child process, and exits when done, so cleanup is
** not needed.
*/
static void handle_request(void) {
	char *method_str;
	char *line;
	char *cp;
	int r, file_len, i;
	const char *index_names[]={"index.exh", "index.exc", "index.exl"};

	/* Set up the timeout for reading. */
	(void) signal(SIGALRM, handle_read_timeout);
	(void) alarm(READ_TIMEOUT);

	/* Initialize the request variables. */
	remoteuser=(char*) 0;
	method=METHOD_UNKNOWN;
	path=(char*) 0;
	file=(char*) 0;
	pathinfo=(char*) 0;
	query="";
	protocol=(char*) 0;
	status=0;
	bytes=-1;
	req_hostname=(char*) 0;

	authorization=(char*) 0;
	content_type=(char*) 0;
	content_length=-1;
	cookie=(char*) 0;
	host=(char*) 0;
	if_modified_since=(time_t) -1;
	referer="";
	useragent="";

#ifdef USE_SSL
	if(do_ssl) {
		ssl=SSL_new(ssl_ctx);
		SSL_set_fd(ssl, conn_fd);
		if (SSL_accept(ssl) == 0) {
			ERR_print_errors_fp(stderr);
			exit(1);
		}
	}
#endif /* USE_SSL */

	/* Read in the request. */
	start_request();
	for (;;) {
		char buf[10000];
		int r=my_read(buf, sizeof(buf));
		if (r < 0 && (errno == EINTR || errno == EAGAIN)) continue;
		if (r <= 0) break;
		(void) alarm(READ_TIMEOUT);
		add_to_request(buf, r);
		if (strstr(request, "\015\012\015\012") != (char*) 0 ||
			strstr(request, "\012\012") != (char*) 0)
			break;
	}

	/* Parse the first line of the request. */
	method_str=get_request_line();
	if (method_str == (char*) 0) send_error(400, "Bad Request", "", "Can't parse request.");
	path=strpbrk(method_str, " \t\012\015");
	if (path == (char*) 0) send_error(400, "Bad Request", "", "Can't parse request.");
	*path++='\0';
	path += strspn(path, " \t\012\015");
	protocol=strpbrk(path, " \t\012\015");
	if (protocol == (char*) 0) send_error(400, "Bad Request", "", "Can't parse request.");
	*protocol++='\0';
	protocol += strspn(protocol, " \t\012\015");

	query=strchr(path, '?');
	if (query == (char*) 0) {
		query="";
	} else {
		*query++='\0';
	}

	/* Parse the rest of the request headers. */
	while ((line=get_request_line()) != (char*) 0) {
		if (line[0] == '\0') {
			break;
		} else if (strncasecmp(line, "Authorization:", 14) == 0) {
			cp=&line[14];
			cp += strspn(cp, " \t");
			authorization=cp;
		} else if (strncasecmp(line, "Content-Length:", 15) == 0) {
			cp=&line[15];
			cp += strspn(cp, " \t");
			content_length=atol(cp);
		} else if (strncasecmp(line, "Content-Type:", 13) == 0) {
			cp=&line[13];
			cp += strspn(cp, " \t");
			content_type=cp;
		} else if (strncasecmp(line, "Cookie:", 7) == 0) {
			cp=&line[7];
			cp += strspn(cp, " \t");
			cookie=cp;
		} else if (strncasecmp(line, "Host:", 5) == 0) {
			cp=&line[5];
			cp += strspn(cp, " \t");
			host=cp;
			if (strchr(host, '/') != (char*) 0 || host[0] == '.') send_error(400, "Bad Request", "", "Can't parse request.");
		} else if (strncasecmp(line, "If-Modified-Since:", 18) == 0) {
			cp=&line[18];
			cp += strspn(cp, " \t");
			if_modified_since=tdate_parse(cp);
		} else if (strncasecmp(line, "Referer:", 8) == 0) {
			cp=&line[8];
			cp += strspn(cp, " \t");
			referer=cp;
		} else if (strncasecmp(line, "User-Agent:", 11) == 0) {
			cp=&line[11];
			cp += strspn(cp, " \t");
			useragent=cp;
		}
	}

	if (strcasecmp(method_str, get_method_str(METHOD_GET)) == 0) {
		method=METHOD_GET;
	} else if (strcasecmp(method_str, get_method_str(METHOD_HEAD)) == 0) {
		method=METHOD_HEAD;
	} else if (strcasecmp(method_str, get_method_str(METHOD_POST)) == 0) {
		method=METHOD_POST;
	} else {
		send_error(501, "Not Implemented", "", "That method is not implemented.");
	}
	
	strdecode(path, path);
	if (path[0] != '/') send_error(400, "Bad Request", "", "Bad filename.");
	file=&(path[1]);
	de_dotdot(file);
	if (file[0] == '\0') file="./";
	if (file[0] == '/' || (file[0] == '.' && file[1] == '.' && (file[2] == '\0' || file[2] == '/'))) send_error(400, "Bad Request", "", "Illegal filename.");
	/* Set up the timeout for writing. */
	(void) signal(SIGALRM, handle_write_timeout);
	(void) alarm(WRITE_TIMEOUT);

	r=stat(file, &sb);
	if (r < 0) r=get_pathinfo();
	if (r < 0) send_error(404, "Not Found", "", "File not found.");
	file_len=strlen(file);
	if (! S_ISDIR(sb.st_mode)) {
		/* Not a directory. */
		while (file[file_len - 1] == '/') {
			file[file_len - 1]='\0';
			--file_len;
		}
		do_file();
	} else {
		char idx[10000];

		/* The filename is a directory.  Is it missing the trailing slash? */
		if (file[file_len - 1] != '/' && pathinfo == (char*) 0) {
			char location[10000];
			if (query[0] != '\0') {
				(void) snprintf(location, sizeof(location), "Location: %s/?%s", path, query);
			} else {
				(void) snprintf(location, sizeof(location), "Location: %s/", path);
			}
			send_error(302, "Found", location, "Directories must end with a slash.");
		}

		/* Check for an index file. */
		for (i=0; i < sizeof(index_names) / sizeof(char*); ++i) {
			(void) snprintf(idx, sizeof(idx), "%s%s", file, index_names[i]);
			if (stat(idx, &sb) >= 0) {
				file=idx;
				do_file();
				goto got_one;
			}
		}

		/* Nope, no index file */
		send_error(400, "Bad Request", "", "Invalid Request.");
		got_one: ;
	}

#ifdef USE_SSL
	SSL_free(ssl);
#endif /* USE_SSL */
}


static void de_dotdot(char *file) {
	char *cp;
	char *cp2;
	int l;

	/* Collapse any multiple / sequences. */
	while ((cp=strstr(file, "//")) != (char*) 0) {
		for (cp2=cp + 2; *cp2 == '/'; ++cp2) continue;
		(void) strcpy(cp + 1, cp2);
	}

	/* Remove leading ./ and any /./ sequences. */
	while (strncmp(file, "./", 2) == 0) (void) strcpy(file, file + 2);
		while ((cp=strstr(file, "/./")) != (char*) 0) (void) strcpy(cp, cp + 2);

	/* Alternate between removing leading ../ and removing xxx/../ */
	for (;;) {
		while (strncmp(file, "../", 3) == 0) (void) strcpy(file, file + 3);
		cp=strstr(file, "/../");
		if (cp == (char*) 0) break;
		for (cp2=cp - 1; cp2 >= file && *cp2 != '/'; --cp2) continue;
		(void) strcpy(cp2 + 1, cp + 4);
	}

	/* Also elide any xxx/.. at the end. */
	while ((l=strlen(file)) > 3 && strcmp((cp=file + l - 3), "/..") == 0) {
		for (cp2=cp - 1; cp2 >= file && *cp2 != '/'; --cp2) continue;
		if (cp2 < file) break;
		*cp2='\0';
	}
}


static int get_pathinfo(void) {
	int r;
	pathinfo=&file[strlen(file)];
	for (;;) {
		do {
			--pathinfo;
			if (pathinfo <= file) {
				pathinfo=(char*) 0;
				return -1;
			}
		} while (*pathinfo != '/');
		*pathinfo='\0';
		r=stat(file, &sb);
		if (r >= 0) {
			++pathinfo;
			return r;
		} else {
			*pathinfo='/';
		}
	}
}


static void do_file(void) {
	char mime_encodings[500];
	const char *mime_type;
	char fixed_mime_type[500];
	int fd;

	/* Is it CGI? */
	if(cgi_pattern != (char*) 0 && match(cgi_pattern, file)) {
		do_cgi();
		return;
	} else {
		if (pathinfo != (char*) 0) send_error(404, "Not Found", "", "File not found.");
	}

	fd=open(file, O_RDONLY);
	if (fd < 0) {
		if(do_syslog) syslog(LOG_INFO, "%.80s File \"%.80s\" is protected",ntoa(&client_addr), path);
		send_error(403, "Forbidden", "", "File is protected.");
	}
	mime_type=figure_mime(file, mime_encodings, sizeof(mime_encodings));
	(void) snprintf(fixed_mime_type, sizeof(fixed_mime_type), mime_type,charset);
	if (if_modified_since != (time_t) -1 && if_modified_since >= sb.st_mtime) {
		add_headers(304, "Not Modified", "", mime_encodings, fixed_mime_type,(off_t) -1, sb.st_mtime);
		send_response();
		return;
	}
	add_headers(200, "Ok", "", mime_encodings, fixed_mime_type, sb.st_size,sb.st_mtime);
	send_response();
	if (method == METHOD_HEAD) return;

	/* ignore zero-length files */
	if (sb.st_size > 0) {
#ifndef USE_SSL
		(void) my_sendfile(fd, conn_fd, 0, sb.st_size);
#else /* USE_SSL */
		if(do_ssl) { 
			send_via_write(fd, sb.st_size);
		} else {
			(void) my_sendfile(fd, conn_fd, 0, sb.st_size);
		}
#endif /* USE_SSL */
	}
	(void) close(fd);
}

static void do_cgi(void) {
	char** argp;
	char** envp;
	int parse_headers;
	char *binary;
	char *directory;

	if (method != METHOD_GET && method != METHOD_POST) send_error(501, "Not Implemented", "", "That method is not implemented for CGI.");

	/* If the socket happens to be using one of the stdin/stdout/stderr
	** descriptors, move it to another descriptor so that the dup2 calls
	** below don't screw things up.  We arbitrarily pick fd 3 - if there
	** was already something on it, we clobber it, but that doesn't matter
	** since at this point the only fd of interest is the connection.
	** All others will be closed on exec.
	*/
	if (conn_fd == STDIN_FILENO || conn_fd == STDOUT_FILENO || conn_fd == STDERR_FILENO) {
		int newfd=dup2(conn_fd, STDERR_FILENO + 1);
		if (newfd >= 0) conn_fd=newfd;
		/* If the dup2 fails, shrug.  We'll just take our chances.
		** Shouldn't happen though.
		*/
	}

	/* Make the environment vector. */
	envp=make_envp();

	/* Make the argument vector. */
	argp=make_argp();

	/* Set up stdin.  For POSTs we may have to set up a pipe from an
	** interposer process, depending on if we've read some of the data
	** into our buffer.  We also have to do this for all SSL CGIs.
	*/
#ifdef USE_SSL
	if ((method == METHOD_POST && request_len > request_idx) || do_ssl) {
#else /* USE_SSL */
	if ((method == METHOD_POST && request_len > request_idx)) {
#endif /* USE_SSL */
		int p[2];
		int r;

		if (pipe(p) < 0) send_error(500, "Internal Error", "", "Something unexpected went wrong making a pipe.");
		r=fork();
		if (r < 0) send_error(500, "Internal Error", "", "Something unexpected went wrong forking an interposer.");
		if (r == 0) {
		/* Interposer process. */
			(void) close(p[0]);
			cgi_interpose_input(p[1]);
			exit(0);
		}
		(void) close(p[1]);
		if (p[0] != STDIN_FILENO) {
			(void) dup2(p[0], STDIN_FILENO);
			(void) close(p[0]);
		}
	} else {
		/* Otherwise, the request socket is stdin. */
		if (conn_fd != STDIN_FILENO) (void) dup2(conn_fd, STDIN_FILENO);
	}

	/* Set up stdout/stderr.  For SSL, or if we're doing CGI header parsing,
	** we need an output interposer too.
	*/

	if (strncmp(argp[0], "nph-", 4) == 0) {
		parse_headers=0;
	} else {
		parse_headers=1;
	}
#ifdef USE_SSL
	if (parse_headers || do_ssl) {
#else /* USE_SSL */
	if (parse_headers) {
#endif /* USE_SSL */
		int p[2];
		int r;

		if (pipe(p) < 0) send_error(500, "Internal Error", "", "Something unexpected went wrong making a pipe.");
		r=fork();
		if (r < 0) send_error(500, "Internal Error", "", "Something unexpected went wrong forking an interposer.");
		if (r == 0) {
			/* Interposer process. */
			(void) close(p[1]);
			cgi_interpose_output(p[0], parse_headers);
			exit(0);
		}
		(void) close(p[0]);
		if (p[1] != STDOUT_FILENO) (void) dup2(p[1], STDOUT_FILENO);
		if (p[1] != STDERR_FILENO) (void) dup2(p[1], STDERR_FILENO);
		if (p[1] != STDOUT_FILENO && p[1] != STDERR_FILENO) (void) close(p[1]);
	} else {
		/* Otherwise, the request socket is stdout/stderr. */
		if (conn_fd != STDOUT_FILENO) (void) dup2(conn_fd, STDOUT_FILENO);
		if (conn_fd != STDERR_FILENO) (void) dup2(conn_fd, STDERR_FILENO);
	}

	/* At this point we would like to set conn_fd to be close-on-exec.
	** Unfortunately there seems to be a Linux problem here - if we
	** do this close-on-exec in Linux, the socket stays open but stderr
	** gets closed - the last fd duped from the socket.  What a mess.
	** So we'll just leave the socket as is, which under other OSs means
	** an extra file descriptor gets passed to the child process.  Since
	** the child probably already has that file open via stdin stdout
	** and/or stderr, this is not a problem.
	*/
	/* (void) fcntl(conn_fd, F_SETFD, 1); */

	/* Close the log file. */
	if (logfp != (FILE*) 0) (void) fclose(logfp);

	/* Close syslog. */
	if(do_syslog) closelog();

	/* Set priority. */
	(void) nice(CGI_NICE);

	/* Split the program into directory and binary, so we can chdir()
	** to the program's own directory.  This isn't in the CGI 1.1
	** spec, but it's what other HTTP servers do.
	*/
	directory=e_strdup(file);
	binary=strrchr(directory, '/');
	if (binary == (char*) 0) {
		binary=file;
	} else {
		*binary++='\0';
		(void) chdir(directory);	/* ignore errors */
	}

	/* Default behavior for SIGPIPE. */
	(void) signal(SIGPIPE, SIG_DFL);
	
	/* Run the program. */
	(void) execve(binary, argp, envp);

	/* Something went wrong. */
	send_error(500, "Internal Error", "", "Something unexpected went wrong running a CGI program.");
}


/* This routine is used only for POST requests.  It reads the data
** from the request and sends it to the child process.  The only reason
** we need to do it this way instead of just letting the child read
** directly is that we have already read part of the data into our
** buffer.
**
** Oh, and it's also used for all SSL CGIs.
*/
static void cgi_interpose_input(int wfd) {
	size_t c;
	ssize_t r, r2;
	char buf[1024];

	c=request_len - request_idx;
	if (c > 0) {
		if (write(wfd, &(request[request_idx]), c) != c) return;
	}
	while (c < content_length) {
		r=my_read(buf, MIN(sizeof(buf), content_length - c));
		if (r < 0 && (errno == EINTR || errno == EAGAIN)) {
			sleep(1);
			continue;
		}
		if (r <= 0) return;
		for(;;) {
			r2=write(wfd, buf, r);
			if (r2 < 0 && (errno == EINTR || errno == EAGAIN)) {
				sleep(1);
				continue;
			}
			if (r2 != r) return;
			break;
		}
		c += r;
	}
	post_post_garbage_hack();
}


/* Special hack to deal with broken browsers that send a LF or CRLF
** after POST data, causing TCP resets - we just read and discard up
** to 2 bytes.  Unfortunately this doesn't fix the problem for CGIs
** which avoid the interposer process due to their POST data being
** short.  Creating an interposer process for all POST CGIs is
** unacceptably expensive.
*/
static void post_post_garbage_hack(void) {
	char buf[2];

#ifdef USE_SSL
	/* We don't need to do this for SSL, since the garbage has
	** already been read.  Probably.
	*/
	if(do_ssl) return;
#endif /* USE_SSL */

	set_ndelay(conn_fd);
	(void) read(conn_fd, buf, sizeof(buf));
	clear_ndelay(conn_fd);
}

/* This routine is used for parsed-header CGIs and for all SSL CGIs. */
static void cgi_interpose_output(int rfd, int parse_headers) {
	ssize_t r, r2;
	char buf[1024];

	if (! parse_headers) {
		/* If we're not parsing headers, write out the default status line
		** and proceed to the echo phase.
		*/
		char http_head[]="HTTP/1.0 200 OK\015\012";
		(void) my_write(http_head, sizeof(http_head));
	} else {
		/* Header parsing.  The idea here is that the CGI can return special
		** headers such as "Status:" and "Location:" which change the return
		** status of the response.  Since the return status has to be the very
		** first line written out, we have to accumulate all the headers
		** and check for the special ones before writing the status.  Then
		** we write out the saved headers and proceed to echo the rest of
		** the response.
		*/
		size_t headers_size, headers_len;
		char *headers;
		char *br;
		int status;
		char *title;
		char *cp;

		/* Slurp in all headers. */
		headers_size=0;
		add_to_buf(&headers, &headers_size, &headers_len, (char*) 0, 0);
		for (;;) {
			r=read(rfd, buf, sizeof(buf));
			if (r < 0 && (errno == EINTR || errno == EAGAIN)) {
				sleep(1);
				continue;
			}
			if (r <= 0) {
				br=&(headers[headers_len]);
				break;
			}

			add_to_buf(&headers, &headers_size, &headers_len, buf, r);
			if ((br=strstr(headers, "\015\012\015\012")) != (char*) 0 || (br=strstr(headers, "\012\012")) != (char*) 0) break;
		}

		/* If there were no headers, bail. */
		if (headers[0] == '\0') return;

		/* Figure out the status. */
		status=200;
		if ((cp=strstr(headers, "Status:")) != (char*) 0 && cp < br && (cp == headers || *(cp-1) == '\012')) {
			cp += 7;
			cp += strspn(cp, " \t");
			status=atoi(cp);
		}
		if ((cp=strstr(headers, "Location:")) != (char*) 0 && cp < br && (cp == headers || *(cp-1) == '\012')) status=302;
	
		/* Write the status line. */
		switch (status) {
			case 200: title="OK"; break;
			case 302: title="Found"; break;
			case 304: title="Not Modified"; break;
			case 400: title="Bad Request"; break;
			case 401: title="Unauthorized"; break;
			case 403: title="Forbidden"; break;
			case 404: title="Not Found"; break;
			case 408: title="Request Timeout"; break;
			case 500: title="Internal Error"; break;
			case 501: title="Not Implemented"; break;
			default: title="Opps"; break;
		}
		syslog_log_entry(status,title);
		(void) snprintf(buf, sizeof(buf), "HTTP/1.0 %d %s\015\012", status, title);
		(void) my_write(buf, strlen(buf));

		/* Write the saved headers. */
		(void) my_write(headers, headers_len);
	}

	/* Echo the rest of the output. */
	for (;;) {
		r=read(rfd, buf, sizeof(buf));
		if (r < 0 && (errno == EINTR || errno == EAGAIN)) {
			sleep(1);
			continue;
		}
		if (r <= 0) goto done;
		for (;;) {
			r2=my_write(buf, r);
			if (r2 < 0 && (errno == EINTR || errno == EAGAIN)) {
				sleep(1);
				continue;
			}
			if (r2 != r) goto done;
			break;
		}
	}
	done:
	shutdown(conn_fd, SHUT_WR);
}


/* Set up CGI argument vector.  We don't have to worry about freeing
** stuff since we're a sub-process.  This gets done after make_envp() because
** we scribble on query.
*/
static char **make_argp(void) {
	char** argp;
	int argn;
	char *cp1;
	char *cp2;

	/* By allocating an arg slot for every character in the query, plus
	** one for the filename and one for the NULL, we are guaranteed to
	** have enough.  We could actually use strlen/2.
	*/
	argp=(char**) malloc((strlen(query) + 2) * sizeof(char*));
	if (argp == (char**) 0) return (char**) 0;

	argp[0]=strrchr(file, '/');
	if (argp[0] != (char*) 0) {
		++argp[0];
	} else {
		argp[0]=file;
	}
    	argn=1;
	/* According to the CGI spec at http://hoohoo.ncsa.uiuc.edu/cgi/cl.html,
	** "The server should search the query information for a non-encoded =
	** character to determine if the command line is to be used, if it finds
	** one, the command line is not to be used."
	*/
	if(strchr(query, '=') == (char*) 0) {
		for (cp1=cp2=query; *cp2 != '\0'; ++cp2) {
			if (*cp2 == '+') {
				*cp2='\0';
				strdecode(cp1, cp1);
				argp[argn++]=cp1;
				cp1=cp2 + 1;
			}
		}
		if (cp2 != cp1) {
			strdecode(cp1, cp1);
			argp[argn++]=cp1;
		}
	}

	argp[argn]=(char*) 0;
	return argp;
}


/* Set up CGI environment variables. Be real careful here to avoid
** letting malicious clients overrun a buffer.  We don't have
** to worry about freeing stuff since we're a sub-process.
*/
static char **make_envp(void) {
	static char *envp[50];
	int envn;
	char *cp;
	char buf[256];
	char rp[MAXPATHLEN];

	envn=0;
	envp[envn++]=build_env("PATH=%s", CGI_PATH);
	envp[envn++]=build_env("LD_LIBRARY_PATH=%s", CGI_LD_LIBRARY_PATH);
	envp[envn++]=build_env("SERVER_SOFTWARE=%s", SERVER_SOFTWARE);
	cp=hostname;
	if (cp != (char*) 0) envp[envn++]=build_env("SERVER_NAME=%s", cp);
	envp[envn++]="GATEWAY_INTERFACE=CGI/1.1";
	envp[envn++]="SERVER_PROTOCOL=HTTP/1.0";
	(void) snprintf(buf, sizeof(buf), "%d", (int) port);
	envp[envn++]=build_env("SERVER_PORT=%s", buf);
	envp[envn++]=build_env("REQUEST_METHOD=%s", get_method_str(method));
	envp[envn++]=build_env("SCRIPT_NAME=%s", path);
	envp[envn++]=build_env("SCRIPT_FILENAME=%s", realpath(file, rp));
	if (pathinfo != (char*) 0) {
		envp[envn++]=build_env("PATH_INFO=/%s", pathinfo);
		(void) snprintf(buf, sizeof(buf), "%s%s", cwd, pathinfo);
		envp[envn++]=build_env("PATH_TRANSLATED=%s", buf);
	}
	if (query[0] != '\0') envp[envn++]=build_env("QUERY_STRING=%s", query);
	envp[envn++]=build_env("REMOTE_ADDR=%s", ntoa(&client_addr));
	if (referer[0] != '\0') envp[envn++]=build_env("HTTP_REFERER=%s", referer);
	if (useragent[0] != '\0') envp[envn++]=build_env("HTTP_USER_AGENT=%s", useragent);
	if (cookie != (char*) 0) envp[envn++]=build_env("HTTP_COOKIE=%s", cookie);
	if (host != (char*) 0) envp[envn++]=build_env("HTTP_HOST=%s", host);
	if (content_type != (char*) 0) envp[envn++]=build_env("CONTENT_TYPE=%s", content_type);
	if (content_length != -1) {
		(void) snprintf(buf, sizeof(buf), "%lu", (unsigned long) content_length);
		envp[envn++]=build_env("CONTENT_LENGTH=%s", buf);
	}
	if (remoteuser != (char*) 0) envp[envn++]=build_env("REMOTE_USER=%s", remoteuser);
	if (authorization != (char*) 0) envp[envn++]=build_env("AUTH_TYPE=%s", "Basic");
	if (getenv("TZ") != (char*) 0) envp[envn++]=build_env("TZ=%s", getenv("TZ"));
	envp[envn]=(char*) 0;
	return envp;
}


static char *build_env(char *fmt,char *arg) {
	char *cp;
	int size;
	static char *buf;
	static int maxbuf=0;

	size=strlen(fmt) + strlen(arg);
	if (size > maxbuf) {
		if (maxbuf == 0) {
			maxbuf=MAX(200, size + 100);
			buf=(char*) e_malloc(maxbuf);
		} else {
			maxbuf=MAX(maxbuf * 2, size * 5 / 4);
			buf=(char*) e_realloc((void*) buf, maxbuf);
		}
	}
	(void) snprintf(buf, maxbuf, fmt, arg);
	cp=e_strdup(buf);
	return cp;
}

static void send_error(int s,char *title,char *extra_header,char *text) {
	add_headers(s, title, extra_header, "", "text/html; charset=%s", (off_t) -1, (time_t) -1);
	send_error_body(s, title, text);
	send_response();
	syslog_log_entry(s,text);
#ifdef USE_SSL
	SSL_free(ssl);
#endif /* USE_SSL */
	exit(1);
}


static void send_error_body(int s,char *title,char *text) {
	char filename[1000];
	char buf[10000];
	int buflen;
	buflen=snprintf(buf, sizeof(buf), "<html>\n<head><title>%d %s</title></head>\n<body>\n<h3>%d %s</h3>\n",s, title, s, title);
	add_to_response(buf, buflen);
	buflen=snprintf(buf, sizeof(buf), "%s\n", text);
	add_to_response(buf, buflen);
	buflen=snprintf(buf, sizeof(buf), "</body>\n</html>\n");
	add_to_response(buf, buflen);
}

static void add_headers(int s,char *title,char *extra_header,char *me,char *mt, off_t b, time_t mod) {
	time_t now, expires;
	char timebuf[100];
	char buf[10000];
	int buflen;
	int s100;
	const char *rfc1123_fmt="%a, %d %b %Y %H:%M:%S GMT";

	status=s;
	bytes=b;

	start_response();
	buflen=snprintf(buf, sizeof(buf), "%s %d %s\015\012", protocol, status, title);
	add_to_response(buf, buflen);
	
	buflen=snprintf(buf, sizeof(buf), "Server: %s\015\012", SERVER_SOFTWARE);
	add_to_response(buf, buflen);

	now=time((time_t*) 0);
	(void) strftime(timebuf, sizeof(timebuf), rfc1123_fmt, gmtime(&now));
	buflen=snprintf(buf, sizeof(buf), "Date: %s\015\012", timebuf);
	add_to_response(buf, buflen);
	s100=status / 100;
	if (s100 != 2 && s100 != 3) {
		buflen=snprintf(buf, sizeof(buf), "Cache-Control: no-cache,no-store\015\012");
		add_to_response(buf, buflen);
	}
	if (extra_header != (char*) 0 && extra_header[0] != '\0') {
		buflen=snprintf(buf, sizeof(buf), "%s\015\012", extra_header);
		add_to_response(buf, buflen);
	}
	if (me != (char*) 0 && me[0] != '\0') {
		buflen=snprintf(buf, sizeof(buf), "Content-Encoding: %s\015\012", me);
		add_to_response(buf, buflen);
	}
	if (mt != (char*) 0 && mt[0] != '\0') {
		buflen=snprintf(buf, sizeof(buf), "Content-Type: %s\015\012", mt);
		add_to_response(buf, buflen);
	}
	if (bytes >= 0) {
		buflen=snprintf(buf, sizeof(buf), "Content-Length: %lld\015\012", (int64_t) bytes);
		add_to_response(buf, buflen);
	}

	if (max_age >= 0) {
		expires=now + max_age;
		(void) strftime(timebuf, sizeof(timebuf), rfc1123_fmt, gmtime(&expires));
		buflen=snprintf(buf, sizeof(buf),"Cache-Control: max-age=%d\015\012Expires: %s\015\012", max_age, timebuf);
		add_to_response(buf, buflen);
	}
	if (mod != (time_t) -1) {
		(void) strftime(timebuf, sizeof(timebuf), rfc1123_fmt, gmtime(&mod));
		buflen=snprintf(buf, sizeof(buf), "Last-Modified: %s\015\012", timebuf);
		add_to_response(buf, buflen);
	}
	buflen=snprintf(buf, sizeof(buf), "Connection: close\015\012\015\012");
	add_to_response(buf, buflen);
}

static void start_request(void) {
	request_size=0;
	request_idx=0;
}

static void add_to_request(char *str, size_t len) {
	add_to_buf(&request, &request_size, &request_len, str, len);
}

static char *get_request_line(void) {
	int i;
	char c;

	for (i=request_idx; request_idx < request_len; ++request_idx) {
		c=request[request_idx];
		if (c == '\012' || c == '\015') {
			request[request_idx]='\0';
			++request_idx;
			if (c == '\015' && request_idx < request_len && request[request_idx] == '\012') {
				request[request_idx]='\0';
				++request_idx;
			}
			return &(request[i]);
		}
	}
	return (char*) 0;
}


static char *response;
static size_t response_size, response_len;

static void start_response(void) {
	response_size=0;
}

static void add_to_response(char *str, size_t len) {
	add_to_buf(&response, &response_size, &response_len, str, len);
}

static void send_response(void) {
	(void) my_write(response, response_len);
}


static void send_via_write(int fd, off_t size) {
	if (size <= SIZE_T_MAX) {
		size_t size_size=(size_t) size;
		void *ptr=mmap(0, size_size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (ptr != (void*) -1) {
			(void) my_write(ptr, size_size);
			(void) munmap(ptr, size_size);
		}
	} else {
		/* mmap can't deal with files larger than 2GB. */
		char buf[30000];
		ssize_t r, r2;

		for(;;) {
			r=read(fd, buf, sizeof(buf));
			if (r < 0 && (errno == EINTR || errno == EAGAIN)) {
				sleep(1);
				continue;
			}
			if (r <= 0) return;
			for (;;) {
				r2=my_write(buf, r);
				if (r2 < 0 && (errno == EINTR || errno == EAGAIN)) {
					sleep(1);
					continue;
				}
				if (r2 != r) return;
				break;
			}
		}
	}
}

static ssize_t my_read(char *buf, size_t size) {
#ifdef USE_SSL
	if(do_ssl) {
		return SSL_read(ssl, buf, size);
	} else {
		return read(conn_fd, buf, size);
	}
#else /* USE_SSL */
	return read(conn_fd, buf, size);
#endif /* USE_SSL */
}

static ssize_t my_write(char *buf, size_t size) {
#ifdef USE_SSL
	if(do_ssl) {
		return SSL_write(ssl, buf, size);
	} else {
		return write(conn_fd, buf, size);
	}
#else /* USE_SSL */
		return write(conn_fd, buf, size);
#endif /* USE_SSL */
}


static int my_sendfile(int fd, int socket, off_t offset, size_t nbytes) {
	off_t lo=offset;
	return sendfile(socket, fd, &lo, nbytes);
}


static void add_to_buf(char** bufP, size_t* bufsizeP, size_t* buflenP,char *str, size_t len) {
	if (*bufsizeP == 0) {
		*bufsizeP=len + 500;
		*buflenP=0;
		*bufP=(char*) e_malloc(*bufsizeP);
	} else if (*buflenP + len >= *bufsizeP) {
		*bufsizeP=*buflenP + len + 500;
		*bufP=(char*) e_realloc((void*) *bufP, *bufsizeP);
	}
	(void) memmove(&((*bufP)[*buflenP]), str, len);
	*buflenP += len;
	(*bufP)[*buflenP]='\0';
}


static char *get_method_str(int m) {
	switch(m) {
		case METHOD_GET: return "GET";
		case METHOD_HEAD: return "HEAD";
		case METHOD_POST: return "POST";
		default: return "UNKNOWN";
	}
}


struct mime_entry {
	char *ext;
	size_t ext_len;
	char *val;
	size_t val_len;
};
static struct mime_entry enc_tab[]={
#include "mime_encodings.h"
};
static const int n_enc_tab=sizeof(enc_tab) / sizeof(*enc_tab);
static struct mime_entry typ_tab[]={
#include "mime_types.h"
};
static const int n_typ_tab=sizeof(typ_tab) / sizeof(*typ_tab);

/* qsort comparison routine - declared old-style on purpose, for portability. */
static int ext_compare(struct mime_entry *a,struct mime_entry *b) {
	return strcmp(a->ext, b->ext);
}


static void init_mime(void) {
	int i;

	/* Sort the tables so we can do binary search. */
	qsort(enc_tab, n_enc_tab, sizeof(*enc_tab), ext_compare);
	qsort(typ_tab, n_typ_tab, sizeof(*typ_tab), ext_compare);

	/* Fill in the lengths. */
	for(i=0; i < n_enc_tab; ++i) {
		enc_tab[i].ext_len=strlen(enc_tab[i].ext);
		enc_tab[i].val_len=strlen(enc_tab[i].val);
	}
	for(i=0; i < n_typ_tab; ++i) {
		typ_tab[i].ext_len=strlen(typ_tab[i].ext);
		typ_tab[i].val_len=strlen(typ_tab[i].val);
	}
}


/* Figure out MIME encodings and type based on the filename.  Multiple
** encodings are separated by commas, and are listed in the order in
** which they were applied to the file.
*/
static const char *figure_mime(char *name,char *me, size_t me_size) {
	char *prev_dot;
	char *dot;
	char *ext;
	int me_indexes[100], n_me_indexes;
	size_t ext_len, me_len;
	int i, top, bot, mid;
	int r;
	const char *default_type="text/plain; charset=%s";
	const char *type;

	/* Peel off encoding extensions until there aren't any more. */
	n_me_indexes=0;
	for(prev_dot=&name[strlen(name)]; ; prev_dot=dot) {
		for(dot=prev_dot - 1; dot >= name && *dot != '.'; --dot);
		if (dot < name) {
			/* No dot found.  No more encoding extensions, and no type
			** extension either.
			*/
			type=default_type;
			goto done;
		}
		ext=dot + 1;
		ext_len=prev_dot - ext;
		/* Search the encodings table.  Linear search is fine here, there
		** are only a few entries.
		*/
		for(i=0; i < n_enc_tab; ++i) {
			if(ext_len == enc_tab[i].ext_len && strncasecmp(ext, enc_tab[i].ext, ext_len) == 0) {
				if (n_me_indexes < sizeof(me_indexes)/sizeof(*me_indexes)) {
					me_indexes[n_me_indexes]=i;
					++n_me_indexes;
				}
				goto next;
			}
		}
		/* No encoding extension found.  Break and look for a type extension. */
		break;

		next: ;
	}

	/* Binary search for a matching type extension. */
	top=n_typ_tab - 1;
	bot=0;
	while(top >= bot) {
		mid=(top + bot) / 2;
		r=strncasecmp(ext, typ_tab[mid].ext, ext_len);
		if (r < 0) {
			top=mid - 1;
		} else if (r > 0) {
			bot=mid + 1;
		} else {
			if (ext_len < typ_tab[mid].ext_len) {
				top=mid - 1;
			} else if (ext_len > typ_tab[mid].ext_len) {
				bot=mid + 1;
			} else {
				type=typ_tab[mid].val;
				goto done;
			}
		}
	}
	type=default_type;

	done:

	/* The last thing we do is actually generate the mime-encoding header. */
	me[0]='\0';
	me_len=0;
	for (i=n_me_indexes - 1; i >= 0; --i) {
		if (me_len + enc_tab[me_indexes[i]].val_len + 1 < me_size) {
			if (me[0] != '\0') {
				(void) strcpy(&me[me_len], ",");
				++me_len;
			}
			(void) strcpy(&me[me_len], enc_tab[me_indexes[i]].val);
			me_len += enc_tab[me_indexes[i]].val_len;
		}
	}

	return type;
}


static void handle_sigterm(int sig) {
	closelog();
	exit(1);
}


/* SIGHUP says to re-open the log file. */
static void handle_sighup(int sig) {
	const int oerrno=errno;
	/* Set up handler again. */
	(void) signal(SIGHUP, handle_sighup);


	/* Just set a flag that we got the signal. */
	got_hup=1;
	
	/* Restore previous errno. */
	errno=oerrno;
}


static void handle_sigchld(int sig) {
	const int oerrno=errno;
	pid_t pid;
	int status;

	(void) signal(SIGCHLD, handle_sigchld);

	/* Reap defunct children until there aren't any more. */
	for(;;) {
		pid=waitpid((pid_t) -1, &status, WNOHANG);
		if ((int) pid == 0) break;
		if ((int) pid < 0) {
			if (errno == EINTR || errno == EAGAIN) continue;
			/* ECHILD shouldn't happen with the WNOHANG option,
			** but with some kernels it does anyway.  Ignore it.
			*/
			if (errno != ECHILD) {
				if(do_syslog) syslog(LOG_ERR, "child wait - %m");
				perror("child wait");
			}
			break;
		}
		currproc--;
		if (maxperip != 0) {
			int i;
			/* remove from list of clients */
			for (i = 0; i < maxproc; i++) {
				if (clients[i].cpid == pid) {
					clients[i].cpid = 0;
					break;
				}
			}
			if (i == maxproc) {
				syslog(LOG_CRIT, "reaped child %d not found in table!", pid);
			}
		}
	}

	/* Restore previous errno. */
	errno=oerrno;
}

static void handle_read_timeout(int sig) {
	send_error(408, "Request Timeout", "","No request appeared within a reasonable time period.");
}

static void handle_write_timeout(int sig) {
    	if(do_syslog) syslog(LOG_INFO, "%.80s connection timed out writing", ntoa(&client_addr));
    	exit(1);
}



static void lookup_hostname(usockaddr *usa4P,size_t sa4_len,int *gotv4P,usockaddr *usa6P,size_t sa6_len,int *gotv6P) {
#ifdef USE_IPV6
	struct addrinfo hints;
	char portstr[10];
	int gaierr;
	struct addrinfo* ai;
	struct addrinfo* ai2;
	struct addrinfo* aiv6;
	struct addrinfo* aiv4;

	(void) memset(&hints, 0, sizeof(hints));
	hints.ai_family=PF_UNSPEC;
	hints.ai_flags=AI_PASSIVE;
	hints.ai_socktype=SOCK_STREAM;
	(void) snprintf(portstr, sizeof(portstr), "%d", (int) port);
    	if((gaierr=getaddrinfo(hostname, portstr, &hints, &ai)) != 0) {
		if(do_syslog) syslog(LOG_CRIT, "getaddrinfo %.80s - %s", hostname,gai_strerror(gaierr));
		if(debug) (void) fprintf(stderr, "%s: getaddrinfo %.80s - %s\n", argv0, hostname,gai_strerror(gaierr));
		exit(1);
	}

    	/* Find the first IPv6 and IPv4 entries. */
	aiv6=(struct addrinfo*) 0;
	aiv4=(struct addrinfo*) 0;
	for(ai2=ai; ai2 != (struct addrinfo*) 0; ai2=ai2->ai_next) {
		switch(ai2->ai_family) {
			case AF_INET6:
	    			if(aiv6 == (struct addrinfo*)0) aiv6=ai2;
	    		break;
	    		case AF_INET:
	    			if(aiv4 == (struct addrinfo*) 0) aiv4=ai2;
	    		break;
		}
	}

    	if(aiv6 == (struct addrinfo*) 0) {
		*gotv6P=0;
    	} else {
		if(sa6_len < aiv6->ai_addrlen) {
	    		if(do_syslog) syslog(LOG_CRIT, "%.80s - sockaddr too small (%lu < %lu)",hostname, (unsigned long) sa6_len,(unsigned long) aiv6->ai_addrlen);
	    		if(debug) (void) fprintf(stderr, "%s: %.80s - sockaddr too small (%lu < %lu)\n",argv0, hostname, (unsigned long) sa6_len,(unsigned long) aiv6->ai_addrlen);
	    		exit(1);
	    	}
		(void) memset(usa6P, 0, sa6_len);
		(void) memmove(usa6P, aiv6->ai_addr, aiv6->ai_addrlen);
		*gotv6P=1;
	}

    	if(aiv4 == (struct addrinfo*) 0) {
		*gotv4P=0;
    	} else {
		if(sa4_len < aiv4->ai_addrlen) {
	    		if(do_syslog) syslog(LOG_CRIT, "%.80s - sockaddr too small (%lu < %lu)",hostname, (unsigned long) sa4_len,(unsigned long) aiv4->ai_addrlen);
	    		if(debug) (void) fprintf(stderr, "%s: %.80s - sockaddr too small (%lu < %lu)\n",argv0, hostname, (unsigned long) sa4_len,(unsigned long) aiv4->ai_addrlen);
	    		exit(1);
	    	}
		(void) memset(usa4P, 0, sa4_len);
		(void) memmove(usa4P, aiv4->ai_addr, aiv4->ai_addrlen);
		*gotv4P=1;
	}

	freeaddrinfo(ai);

#else /* USE_IPV6 */

	struct hostent* he;

	*gotv6P=0;

	(void) memset(usa4P, 0, sa4_len);
	usa4P->sa.sa_family=AF_INET;
    	if(hostname == (char*) 0) {
		usa4P->sa_in.sin_addr.s_addr=htonl(INADDR_ANY);
    	} else {
		usa4P->sa_in.sin_addr.s_addr=inet_addr(hostname);
		if((int) usa4P->sa_in.sin_addr.s_addr == -1) {
	    		he=gethostbyname(hostname);
	    		if(he==(struct hostent*) 0) {
				if(do_syslog) syslog(LOG_CRIT, "gethostbyname %.80s failed", hostname);
				if(debug) (void) fprintf(stderr, "%s: gethostbyname %.80s failed\n", argv0,hostname);
				exit(1);
			}
	    		if (he->h_addrtype != AF_INET) {
				if(do_syslog) syslog(LOG_CRIT, "%.80s - non-IP network address", hostname);
				if(debug) (void) fprintf(stderr, "%s: %.80s - non-IP network address\n", argv0,hostname);
				exit(1);
			}
	    		(void) memmove(&usa4P->sa_in.sin_addr.s_addr, he->h_addr, he->h_length);
	    	}
	}
	usa4P->sa_in.sin_port=htons(port);
	*gotv4P=1;
#endif /* USE_IPV6 */
}


static char *ntoa(usockaddr *usaP) {
#ifdef USE_IPV6
	static char str[200];
    	if(getnameinfo(&usaP->sa, sockaddr_len(usaP), str, sizeof(str), 0, 0, NI_NUMERICHOST)!=0) {
		str[0]='?';
		str[1]='\0';
	} else if(IN6_IS_ADDR_V4MAPPED(&usaP->sa_in6.sin6_addr) && strncmp(str,"::ffff:",7)==0) {
		/* Elide IPv6ish prefix for IPv4 addresses. */
		(void) strcpy(str,&str[7]);
	}
    	return str;
#else /* USE_IPV6 */
    	return inet_ntoa(usaP->sa_in.sin_addr);
#endif /* USE_IPV6 */
}

static int sockaddr_check(usockaddr *usaP) {
	switch(usaP->sa.sa_family) {
		case AF_INET: return 1;
#ifdef USE_IPV6
		case AF_INET6: return 1;
#endif /* USE_IPV6 */
		default:
		return 0;
	}
}

static size_t sockaddr_len(usockaddr *usaP) {
	switch(usaP->sa.sa_family) {
		case AF_INET: return sizeof(struct sockaddr_in);
#ifdef USE_IPV6
		case AF_INET6: return sizeof(struct sockaddr_in6);
#endif /* USE_IPV6 */
		default:
		return 0;	/* shouldn't happen */
	}
}

/* Copies and decodes a string.  It's ok for from and to to be the
** same string.
*/

static void strdecode(char *to,char *from) {
	for(;*from != '\0'; ++to, ++from) {
		if(from[0]== '%' && isxdigit(from[1]) && isxdigit(from[2])) {
	    		*to=hexit(from[1]) * 16 + hexit(from[2]);
	    		from += 2;
		} else {
			*to=*from;
		}
    	}
	*to='\0';
}

static int hexit(char c) {
	if(c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return 0;           /* shouldn't happen, we're guarded by isxdigit() */
}


/* Base-64 decoding.  This represents binary data as printable ASCII
** characters.  Three 8-bit binary bytes are turned into four 6-bit
** values, like so:
**
**   [11111111]  [22222222]  [33333333]
**
**   [111111] [112222] [222233] [333333]
**
** Then the 6-bit values are represented using the characters "A-Za-z0-9+/".
*/

static int b64_decode_table[256]={
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
	52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
	15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
	-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
	41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
};

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
** Return the actual number of bytes generated.  The decoded size will
** be at most 3/4 the size of the encoded, and may be smaller if there
** are padding characters (blanks, newlines).
*/
static int b64_decode(const char *str,unsigned char *space,int size) {
	const char *cp;
	int space_idx, phase;
	int d, prev_d=0;
	unsigned char c;

	space_idx=0;
	phase=0;
    	for(cp=str; *cp != '\0'; ++cp) {
		d=b64_decode_table[(int) *cp];
		if(d!= -1) {
			switch(phase) {
				case 0:
					++phase;
				break;
				case 1:
					c=((prev_d<<2) | ((d & 0x30)>>4));
					if (space_idx<size) space[space_idx++]=c;
					++phase;
				break;
				case 2:
					c=(((prev_d & 0xf)<<4) | ((d & 0x3c)>>2));
					if(space_idx<size) space[space_idx++]=c;
					++phase;
				break;
				case 3:
					c=(((prev_d & 0x03)<<6) | d);
					if(space_idx<size) space[space_idx++]=c;
					phase=0;
				break;
			}
			prev_d=d;
		}
	}
	return space_idx;
}


/* Set NDELAY mode on a socket. */
static void set_ndelay(int fd) {
	int flags, newflags;
	flags=fcntl(fd,F_GETFL,0);
	if(flags!=-1) {
		newflags=flags | (int) O_NDELAY;
		if(newflags!=flags) (void) fcntl(fd,F_SETFL,newflags);
	}
}


/* Clear NDELAY mode on a socket. */
static void clear_ndelay(int fd) {
	int flags, newflags;
	flags=fcntl(fd,F_GETFL,0);
    	if(flags!=-1) {
		newflags=flags & ~ (int) O_NDELAY;
		if(newflags!=flags) (void) fcntl(fd,F_SETFL,newflags);
	}
}


static void *e_malloc(size_t size) {
	void *ptr;
	ptr=malloc(size);
    	if(ptr==(void*)0) {
		if(do_syslog) syslog(LOG_CRIT, "out of memory");
		if(debug) (void) fprintf(stderr, "%s: out of memory\n", argv0);
		exit(1);
	}
	return ptr;
}

static void *e_realloc(void *optr,size_t size) {
	void *ptr;
	ptr=realloc(optr,size);
    	if(ptr==(void*)0) {
		if(do_syslog) syslog(LOG_CRIT, "out of memory");
		if(debug) (void) fprintf(stderr, "%s: out of memory\n", argv0);
		exit(1);
	}
    	return ptr;
}


static char *e_strdup(char *ostr) {
	char *str;
    	str=strdup(ostr);
    	if(str==(char*)0) {
		if(do_syslog) syslog(LOG_CRIT, "out of memory copying a string");
		if(debug) (void) fprintf(stderr, "%s: out of memory copying a string\n", argv0);
		exit(1);
	}
    	return str;
}

static int file_exist(char *s) {
        struct stat ss;
        int i = stat(s, &ss);
        if (i < 0) return 0;
        if ((ss.st_mode & S_IFREG) || (ss.st_mode & S_IFLNK)) return(1);
        return(0);
}

static void syslog_log_entry(int status,char *text) {
	char url[500];
	if (protocol == (char*) 0) protocol="UNKNOWN";
	if (referer == "") referer="-";
	if (path == (char*) 0) path="";
	(void) snprintf(url, sizeof(url), "%s", path);
	if(do_syslog) syslog(LOG_INFO,"\"%s\",\"%s %s %s\",\"%d %s\",\"%.200s\",\"%.200s\"",ntoa(&client_addr), get_method_str(method), url,protocol, status, text, referer, useragent);
}
