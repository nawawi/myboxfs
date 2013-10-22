/***************************************************************************
 *   Copyright (C) 2003-2007 by Jack S. Lai                                *
 *   laitcg at gmail dot com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 *   This program is released under the GPL with the additional exemption  *
 *   that compiling, linking, and/or using OpenSSL is allowed."            *
 *   (http://www.openssl.org/support/faq.html#LEGAL2)                      *
 ***************************************************************************/

/**
 * It's intent is to provide a follow on program to POP3-Virusscan-Proxy 0.4
 * http://pop3vscan.sourceforge.net by Folke Ashberg <folke at ashberg dot de>.
 *
 * It is based upon his program but provides numerous changes to include
 * scanning pop3 mail for spam, hardening the program, addaption to todays
 * email environment, and many, many other changes.
 *
 * The initial release of p3scan includes patches made and submitted to the
 * original project but were never incorporated. Please see the README for
 * further information.
 */


#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_DIRENT_H
#include <dirent.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if HAVE_MALLOC
#if HAVE_MALLOC_H
#include <malloc.h>
#endif
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h> /* linux/netfilter_ipv4.h needs defined first */
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#if HAVE_SYSLOG_H
#include <syslog.h>
#endif
#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_CLAMAV
#include <clamav.h>
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <grp.h>
#include <libgen.h>

#include <linux/netfilter_ipv4.h>

#include <netinet/ip.h>
#include <pthread.h>
#include <pwd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ioctl.h>

#ifdef PCRE
#ifdef PCRE_HEADER
#include <pcre.h>
#endif
#ifdef PCRE_DIR_HEADER
#include <pcre/pcre.h>
#endif
#endif

#include "p3scan.h"
#include "parsefile.h"
#include "scanner.h"
#include "restart.h"

/* p->ismail legend:
   ismail=0 not processing a message - parsing client commands
   ismail=1 enable message parsing (got RETR/TOP/DATA) - start trapping email
   ismail=2 got "+ok", read the mail into file
   ismail=3 closed header buffer
   ismail=4 have entire message, start processing - email complete, start server noop's
   ismail=5 scanning done, send mail to client
*/

/* globals / protos */
int numprocs;
static volatile sig_atomic_t clean=0, emptied=0, reload=0, loop=0;
struct configuration_t *config;
const char BLACKLIST[10]="blacklist";
const char WHITELIST[10]="whitelist";

/* ClamAV stuff */
#ifdef CLAMAV
static struct cl_stat *dbstat = NULL;
const struct cfgstruct *copt;
struct cl_engine *engine = NULL;
struct cl_limits limits;
struct cl_cvd *daily;
const char *dbdir;
unsigned int sigs = 0;
const char *clamvirname;
char  *version;

pthread_mutex_t reload_mutex;
pthread_mutex_t doingload_mutex;
#endif
static int sockfd, connfd; /* has to be global, do_sigterm_main() want's to close them */

/* the proxycontext is global for do_sigterm_proxy().
 * no other function should use it! */
static struct proxycontext *global_p;

static int stralloc_ptr;
static char *strings[8];
static int str_tag[8];

// xmkdir -- make directory
int xmkdir(char *p) {
	long mode=0700;
        mode_t mask;
        char *s=malloc(strlen(p) * sizeof(char)); 
        char *path=malloc(strlen(p) * sizeof(char)); 
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

char *trim(char *s) {
        size_t len=strlen(s);
        size_t lws;
        while(len && isspace(s[len-1])) --len;
        if(len) {
                lws=strspn(s, " \n\r\t\v\f");
                memmove(s, s + lws, len -= lws);
        }
        s[len]=0;
        return s;
}


void do_sigterm_main(int signr) {
	int ret;
  	if(signr != -1 ) do_log(LOG_NOTICE, "signalled, doing cleanup");
  	r_close(sockfd);
  	if(config->scannerenabled && config->scanner->uninit1) {
    		do_log(LOG_DEBUG, "calling uninit1");
    		config->scanner->uninit1();
    		do_log(LOG_DEBUG, "uninit1 done");
  	}
  	if((ret=unlink(config->pidfile)!=0)) do_log(LOG_NOTICE, "ERR: Unable to remove %s", config->pidfile);
#ifdef CLAMAV
  	if(engine) cl_free(engine);
#endif
  	do_log(LOG_NOTICE, PROGRAM " terminates now");
  	exit(EXIT_SUCCESS);
}

void do_sigusr1(int sig_num) {
	do_log(LOG_NOTICE, "Child signalled! Performing clean exit...");
	clean=1;
}

#ifdef CLAMAV
void do_sigusr2(int sig_num) {
	do_log(LOG_NOTICE, "Child signalled! Checking ClamAV Database...");
  	if(clamcheck()) {
      		do_log(LOG_CRIT, "ERR: ClamAV reload failed! config and restart " PROGRAM);
      		do_sigterm_main(-1);
  	}
}
#endif

void do_log(int level, const char *fmt,...) {
#define __PUFFER_LEN__ 4096
	char puffer[__PUFFER_LEN__];
	char timenow[28];
	time_t now = time(NULL);
	va_list az;
	struct mallinfo m=mallinfo();

	if(!config->debug && !config->status && level==LOG_DEBUG) return;
	if(!config->debug && config->quiet && level==LOG_NOTICE) return;
  
	va_start(az,fmt);
	vsnprintf(puffer, __PUFFER_LEN__, fmt, az);
	ctime_r(&now, timenow);
	if(config->status) {
		fprintf(stderr,"%s\n", puffer);
  	} else if(config->debug) {
		fflush(stdout);
		if(config->debug_memory) {
			fprintf(stderr,"%.8s %s[%i]: Mem: %i %s\n",timenow+11, config->syslogname, getpid(), m.uordblks, puffer);
    		} else {
			fprintf(stderr,"%.8s %s[%i]: %s\n",timenow+11, config->syslogname, getpid(), puffer);
		}
    		fflush(NULL);
	} else {
    		openlog(config->syslogname, config->p3logopt, config->p3logfac);
    		syslog(LOG_NOTICE, "%s\n", puffer);
		closelog();
	}
	if(level==LOG_EMERG) {
		fprintf(stderr, "%s\n", puffer);
		if(config->cleankill) {
			syslog(LOG_NOTICE, "ERR: Exiting now...\n");
			/* Tell main p3scan to abort */
			if(config->child) kill(getppid(),SIGUSR1);
			va_end(az);
			exit(EXIT_FAILURE);
		}
	}
	va_end(az);
	return;
#undef __PUFFER_LEN__
}

int simple_match_one(const char *pattern, int patternlen, const char *string) {
        const char *p;
	int i, pl;
        for(p=pattern;p - pattern < patternlen; ++p, ++string ) {
                if(*p=='?' && *string!='\0') continue;
                if(*p== '*') {
                        p++;
                        i = strlen( string );
                        pl = patternlen - ( p - pattern );
                        for ( ; i >= 0; i-- ) {
                                if(simple_match_one(p,pl,&(string[i]))) return 1;
                        }
                        return 0;
                }
                if(*p!=*string) return 0;
        }
        if(*string=='\0') return 1;
        return 0;
}

int simple_match(const char *pattern, const char *string) {
	const char *or;
	for(;;) {
		or=strchr( pattern, '|' );
		if(or==(char*) 0 ) {
			return simple_match_one( pattern, strlen( pattern ),string);
		}
		if(simple_match_one(pattern, or - pattern, string)) return 1;
		pattern = or + 1;
	}
	return 0;
}

int preg_match(char *pattern,char *line) {
	pcre *rx;
	const char *pcre_error;
	int pcre_erroffset, ret;
	int offsets[50];
	rx = pcre_compile(pattern, PCRE_UNGREEDY | PCRE_CASELESS,&pcre_error, &pcre_erroffset, NULL);
        if(rx) {
                ret=pcre_exec(rx, NULL, line, strlen(line), 0, 0, offsets, 50);
                if(ret > -1 ) return 1;
	}
	return 0;
}

int str_match(char *pattern,char *line) {
	if(simple_match(pattern,line) ==1) return 1;
	if(preg_match(pattern,line) ==1) return 1;
	return 0;
}

int stristr(const char *string, const char *pattern) {
	const char *pptr, *sptr, *start;
	for(start = string; *start != NUL; start++) {
		for( ; ((*start!=NUL) && (toupper(*start) != toupper(*pattern))); start++);
    		if(NUL == *start) return -1;
		pptr = pattern;
		sptr = start;
		while(toupper(*sptr) == toupper(*pptr)) {
			sptr++;
			pptr++;
			if(NUL == *pptr) return (start-string);
		}
	}
	return -1;
}

char *substr(const char *string, int start, size_t count) {
	size_t len = strlen(string);
	if(start < 0) start = len + start;
	if(start >= 0 && start < len) {
		if(count == 0 || count > len - start) count = len - start;
		char *smtpstr=malloc(count+1);
		memcpy(smtpstr, string + start, count);
		smtpstr[count] = 0;
		return smtpstr;
	}
	char *smtpstr=malloc(sizeof(char));
	smtpstr[0] = 0;
	return smtpstr;
}

char *stralloc(size_t length) {
	register int i;
	if(UINT_MAX == length) return NULL;
	i = stralloc_ptr++;
	++length;
	if((!strings[i]) || (length > strlen(strings[i]))) {
		strings[i] = realloc(strings[i], length);
		assert(NULL != strings[i]);
		str_tag[i] = -1;
	} else {
		str_tag[i] = 0;
	}
	stralloc_ptr &= 7;
	return(strings[i]);
	// Maintains 8 strings in a circular buffer
}

char *right(char *string, size_t i) {
	char *buf;
	size_t strlength = strlen(string);
	if(i > strlength) i = strlength;
	buf=stralloc(i);
	strcpy(buf, &string[strlength-i]);
	return buf;
}

char *strreplace(char *haystack,char *needle,char *rstr) {
	size_t size=strlen(haystack)-strlen(needle)+strlen(rstr)+1;
	char *p=malloc(size);
	char *ptrp=p;
	char *newstr=haystack;
	char *match;
	char *replace;
	int i,j;

	if(!p) return NULL;
	while(*newstr){
	match=needle;
 	replace=rstr;
	i=0;
	while(*newstr && *match) {
		if(*newstr != *match){
			*ptrp++=*newstr++;
			match=needle;
			i=0;
		} else if(*newstr==*match) {
			*ptrp++=*newstr++;
			match++;
			i++;
			}
		}
		if(i==(int)strlen(needle)) {
			j=0;
			while(j<i) {
				ptrp--;
				j++;
			}
			while(*replace) *ptrp++=*replace++;
		}
	}
	*ptrp='\0';
	return(p);
}

char *make_message(const char *fmt, ...) {
	/* Guess we need no more than 100 bytes. */
	int n, size = 100;
	char *msg;
	va_list ap;
	if((msg = malloc (size)) == NULL) return NULL;
	while(1) {
		/* Try to print in the allocated space. */
		va_start(ap, fmt);
		n = vsnprintf(msg, size, fmt, ap);
		va_end(ap);
		/* If that worked, return the string. */
		if(n > -1 && n < size) return msg;
		/* Else try again with more space. */
		if(n > -1) {
			size = n+1;   /* glibc 2.1 - precisely what is needed*/
		} else {
			size *= 2; /* glibc 2.0 - twice the old size*/
		}
		if((msg = realloc (msg, size)) == NULL) return NULL;
	}
}

#ifdef CLAMAV

int init_clamav(void) {
	int ret, len;
	char *path;
	char mvit[100];
	struct stat foo;
	if(config->clamav) {
		if(engine) {
			cl_free(engine);
			engine = NULL;
		}
		dbdir=cl_retdbdir();
		if(dbstat == NULL) {
			dbstat = (struct cl_stat *) malloc(sizeof(struct cl_stat));
			if(!dbstat) {
				cl_free(engine);
				do_log(LOG_EMERG, "ClamAV can't allocate memory for dbstat ERR: %s",cl_strerror(ret));
				return SCANNER_RET_ERR;
			}
		} else {
			if((ret = cl_statfree(dbstat))) {
				if(engine) cl_free(engine);
				do_log(LOG_EMERG, "ClamAV Database statfree ERR: %s",cl_strerror(ret));
				return SCANNER_RET_ERR;
			}
		}
		if((ret = cl_load(dbdir, &engine, &sigs, CL_DB_STDOPT))) {
			if(engine) cl_free(engine);
			if(dbstat) cl_statfree(dbstat);
			do_log(LOG_EMERG, "ERR: %s",cl_strerror(ret));
			return SCANNER_RET_ERR;
		}
		// build engine
		if((ret = cl_build(engine))) {
			if(engine) cl_free(engine);
			if(dbstat) cl_statfree(dbstat);
			// Emergency: init_clamav. could not init.
			do_log(LOG_EMERG, "ClamAV Database build ERR: %s",cl_strerror(ret));
			return SCANNER_RET_ERR;
		}
		memset(dbstat, 0, sizeof(struct cl_stat));
		cl_statinidir(dbdir, dbstat);
		// set up archive limits (ClamAV Defaults)
		memset(&limits, 0, sizeof(struct cl_limits));
		limits.maxfiles = 1000; // max files
		limits.maxfilesize = 10 * 1024 * 1024; // 10 MB maximum size of archived/compressed file (files exceeding this limit will be ignored)
		limits.maxreclevel = 10; // maximum recursion level for archives
		limits.maxmailrec = 64; // maximum recursion level for mail files
		limits.maxratio = 200; // maximum compression ratio
	}
	if(reload) {
		do_log(LOG_NOTICE, "ClamAV Database correctly reloaded (%d signatures)", sigs);
	} else {
		do_log(LOG_NOTICE, "ClamAV Database correctly loaded (%d signatures)", sigs);
	}
	return SCANNER_RET_OK;
}

int clamcheck(void) {
	int ret = 0;
	char buf[256];
	char *path;
	struct stat foo;

	if(!reload) {
		if(dbstat == NULL) {
			do_log(LOG_DEBUG,"No stats for Database check - forcing reload");
			ret=1;
		}
		if(!ret) ret = cl_statchkdir(dbstat);
		if(ret==1) {
			if(!reload) {
				if(config->child) {
					do_log(LOG_DEBUG,"Telling parent[%i] database reload needed...",getppid());
					kill(getppid(),SIGUSR2);
					sleep(1);
					while(reload) {
						do_log(LOG_DEBUG,"Waiting for ClamAV database reload...");
						sleep(1);
					}
					// Done reloading, recheck status (to reset ret);
					ret = cl_statchkdir(dbstat);
				} else {
					pthread_mutex_lock(&doingload_mutex);
					pthread_mutex_lock(&reload_mutex);
					//Only one child should be here as $reload is atomic
					reload=1;
					pthread_mutex_unlock(&reload_mutex);
					do_log(LOG_NOTICE,"ClamAV Database modification detected. Forcing reload.");
					ret = init_clamav();
					if(ret) {
						pthread_mutex_lock(&reload_mutex);
						reload=0;
						pthread_mutex_unlock(&reload_mutex);
						pthread_mutex_unlock(&doingload_mutex);
						config->scannerenabled=0;
						// Emergency: scan_mailfile: Could not re-init database.
						do_log(LOG_EMERG, "ERR: %i Database re-init failed! check config and restart %s", ret, PROGRAM);
						/* we are dead now. Should not reach here. But allow it to fall through in case LOG_EMERG is changed 
						in the future. */
						return SCANNER_RET_CRIT;
					}
					pthread_mutex_lock(&reload_mutex);
					reload=0;
					pthread_mutex_unlock(&reload_mutex);
					pthread_mutex_unlock(&doingload_mutex);
				}
			} else {
				while(reload) {
					do_log(LOG_DEBUG,"Waiting for ClamAV database reload...");
					sleep(1);
				}
			}
		}
	} else { // reload==1
		while(reload) {
			do_log(LOG_DEBUG,"Waiting for ClamAV database reload...");
			sleep(1);
		}
	}
	if(ret) {
		do_log(LOG_EMERG, "ERR: ClamAV Database problem! %d", ret);
		return SCANNER_RET_CRIT;
	}
	do_log(LOG_DEBUG,"ClamAV Database status OK");
	return SCANNER_RET_OK;
}
#endif // clamav

int scan_directory(struct proxycontext *p) {
	int ret, ret_all, viret, fdc;
	DIR *dp;
	struct dirent *de;
	struct stat sbuf;
	char *file;
	int maildirlen;
	char *virname;
#define VISIZE 1000
	char *vi;
	int vipos = 0;
#ifdef CLAMAV
	unsigned long int blocks = 0;
	const char *virnamec;
#endif

	/* scan directory */
	maildirlen=strlen(p->maildir);
	if(stat (p->maildir, &sbuf) == -1) {
		context_uninit(p);
		// Emergency: scan_directory: directory does not exist.
		do_log(LOG_EMERG, "ERR: %s does not exist", p->maildir);
		return SCANNER_RET_ERR;
	}
	if(!S_ISDIR(sbuf.st_mode)) {
		context_uninit(p);
		// Emergency: scan_directory: x is not a directory.
		do_log(LOG_EMERG, "ERR: %s is not a directory", p->maildir);
		return SCANNER_RET_ERR;
	}
	if((dp = opendir (p->maildir)) == NULL) {
		context_uninit(p);
		// Emergency: scan_directory: can't open directory.
		do_log(LOG_EMERG, "ERR: Can't open directory %s", p->maildir);
		return SCANNER_RET_ERR;
	}
	if((vi=malloc(VISIZE))==NULL) return SCANNER_RET_ERR;
	ret_all=SCANNER_RET_OK;
	vi[0]='\0';
	while((de = readdir (dp)) != NULL) {
		if(strcmp (de->d_name, ".") == 0) continue;
		if(strcmp (de->d_name, "..") == 0) continue;
		if((file=malloc(maildirlen + strlen(de->d_name) +2))==NULL) return SCANNER_RET_ERR;
		sprintf(file, "%s/%s", p->maildir, de->d_name);
		if(lstat(file, &sbuf) == -1) {
			context_uninit(p);
			free(file);
			// Emergency: scan_directory: Can't get file info.
			do_log(LOG_EMERG, "ERR: Can't get file info for %s - I won't touch it.", file);
			continue;
		}
		if(!S_ISREG(sbuf.st_mode)) {
			context_uninit(p);
			// Emergency: scan_directory: x is not a file.
			do_log(LOG_EMERG, "ERR: %s is not a regular file. I won't touch it.", file);
			free(file);
			continue;
		}
		/* build filename */
		do_log(LOG_DEBUG, "Going to scan '%s'", file);
		p->scanthis=file;
#ifdef CLAMAV
		clamvirname=NULL;
		if(config->clamav) {
			// Check database
			if(clamcheck()) return SCANNER_RET_CRIT;
			if((fdc = open(p->scanthis, O_RDONLY)) == -1) {
				do_log(LOG_CRIT, "ERR: Can't open file %s for scanning!", p->scanthis);
				return SCANNER_RET_CRIT;
			}
			/* scan descriptor */
			if((viret = cl_scandesc(fdc, &virnamec, &blocks, engine, &limits, CL_SCAN_STDOPT)) == CL_VIRUS) {
				p->virinfo=strdup(virnamec);
				viret=SCANNER_RET_VIRUS;
			} else {
				if(viret != CL_CLEAN) do_log(LOG_CRIT,"ERR: %s", cl_strerror(ret));
			}
			close(fdc);
		} else {
#endif
			ret=config->scanner->scan(p, &virname);
			if(ret==SCANNER_RET_VIRUS) {
				ret_all=SCANNER_RET_VIRUS;
				if(virname && strlen(virname)<VISIZE - vipos - 4) {
					strcat(&vi[vipos], virname);
					vipos+=strlen(virname);
					strcat(vi, " & ");
					vipos+=3;
				}
			} else if(ret == SCANNER_RET_ERR && ret_all != SCANNER_RET_VIRUS) {
				ret_all = SCANNER_RET_ERR;
			}
			do_log(LOG_DEBUG, "Finished scanning '%s'(%s=%s)", file,(virname ? virname : "none"),
					(ret_all==SCANNER_RET_OK ? "OK" : (ret_all==SCANNER_RET_VIRUS ? "VIRUS" : "ERROR")));
			if(virname) free(virname);
			free(file);
#ifdef CLAMAV
		}
#endif
	}
	closedir (dp);
	if(vipos>4) {
		vi[vipos-3]='\0';
		p->virinfo=vi;
	} else {
		p->virinfo=NULL;
	}
	return ret_all;
}

/* clean_child_directory -- iterate through directory
 *                          removing all files and subdirectories
 *
 * args:
 * childpid -- used to construct directory path
 *             to erase. If called by child pass
 *             getpid()
 *
 * returns:
 * 0 -- OK
 * 1 -- mem allocation error
 *
 * note: do_log(LOG_EMERG, ...) currently doesn't return!!!
 */
int clean_child_directory(pid_t childpid) {
	DIR           *dp;
	struct dirent *de;
	sigset_t      blockset, oldset;
	char          *dir,*file;
	int           dirlen,filelen;

	// Block signals until done
	if((sigfillset(&blockset))<0) do_log(LOG_EMERG, "(blockset) could not init!");
	// Emergency: clean_child_directory: could not init
	if((sigprocmask(SIG_SETMASK,&blockset,&oldset))<0) do_log(LOG_EMERG, "(blockset) couldn't block!");
	dirlen=strlen(config->virusdirbase)+20;
	if((dir=malloc(dirlen))==NULL) {
		// Emergency: clean_child_directory: couldn't unblock-1
		if((sigprocmask(SIG_SETMASK,&oldset,NULL))<0) do_log(LOG_EMERG, "(blockset) couldn't unblock-1!");
		do_log(LOG_CRIT, "ERR: clean_child_dir - no memory(1)");
		return 1;
	}
	snprintf(dir, dirlen, "%s/children/%d/", config->virusdirbase, childpid);
	/* Erase directory if it exists */
	if((dp = opendir (dir)) != NULL) {
		do_log(LOG_DEBUG, "Erasing %s contents", dir);
		while((de = readdir (dp)) != NULL) {
			if(strcmp (de->d_name, ".") == 0) continue;
			if(strcmp (de->d_name, "..") == 0) continue;
			filelen=dirlen + strlen(de->d_name) + 2;
			if((file=malloc(filelen))==NULL) {
				do_log(LOG_CRIT, "ERR: clean_child_dir - no memory(2)");
				return 1;
			}
			snprintf(file, filelen, "%s%s", dir, de->d_name);
			do_log(LOG_DEBUG, "Unlinking (%s)", file);
			if((unlink(file)<0)) {
				// Emergency: clean_child_directory: couldn't unblock-2
				if((sigprocmask(SIG_SETMASK,&oldset,NULL))<0) do_log(LOG_EMERG, "(blockset) couldn't unblock-2!");
				do_log(LOG_CRIT, "ERR: File Error! Could not erase %s",file);
				return 1;
			}
			free(file);
		}
		closedir(dp);
		do_log(LOG_DEBUG, "Removing directory %s", dir);
		if((rmdir(dir)<0)) {
			// Emergency: clean_child_directory: couldn't unblock-3
			if((sigprocmask(SIG_SETMASK,&oldset,NULL))<0) do_log(LOG_EMERG, "(blockset) couldn't unblock-3!");
			do_log(LOG_CRIT, "ERR: Directory Error! Could not erase %s",dir);
			return 1;
		}
	}
	free(dir);
	// Emergency: clean_child_directory: couldn't unblock-4
	if((sigprocmask(SIG_SETMASK,&oldset,NULL))<0) do_log(LOG_EMERG, "(blockset) couldn't unblock-4!");
	return 0;
}

int checktimeout(struct proxycontext *p) {
	int readerr=0, senterr=0;
	char svrout[1];
	char *scbuf=NULL;

	if(p->cksmtp) return 0;
	if(p->action==ACT_TOP) return 0; // some clients just hang on!
	if(p->now+config->timeout<time(NULL)) {
		if(p->ismail != 5) { // Are we sending message to client?
			if(config->broken) {
        			/* Line parsing */
        			if(!p->gobogus) readerr=getlinep3(p->mailhdr_fd,p->hdrbuf);
        			if(readerr>=0 && !p->gobogus) {
          				senterr=writeline(p->client_fd, WRITELINE_LEADING_RN, p->hdrbuf->line);
          				if(senterr < 0) return senterr;
          				do_log(LOG_DEBUG, "Timeout: Sent client a line: %s", p->hdrbuf->line);
					p->hdroffset++;
				} else {
					/* End of hdrbuf! We are still parsing! */
					if(!p->gobogus) p->gobogus=1;
					senterr=writeline(p->client_fd, WRITELINE_LEADING_RN, BOGUSX);
					if(senterr < 0) return senterr;
					do_log(LOG_DEBUG, "Timeout: Sent client a bogus line.");
				}
			} else {
			/* Character parsing */
				if(!p->gobogus) readerr=r_read(p->mailhdr_fd,p->cbuff,1);
				if(readerr>=0 && !p->gobogus) {
					senterr=secure_write(p->client_fd,p->cbuff,1);
					if(senterr < 0) return senterr;
					scbuf=strndup(p->cbuff,1);
					do_log(LOG_DEBUG, "Timeout: Sent client a character: %s",scbuf);
					p->hdroffset++;
				} else {
          				/* End of hdrbuff! We are still parsing! */
					p->gobogus=1;
					if(p->boguspos < 74) {
						svrout[0]=BOGUSX[p->boguspos];
						senterr=secure_write(p->client_fd,svrout,1);
						if(senterr < 0) return senterr;
						do_log(LOG_DEBUG, "Timeout: Sent client a character: %s",svrout);
						p->boguspos++;
					} else {
						if(p->boguspos < 422) {
							senterr=secure_write(p->client_fd,PERIOD,1);
							if(senterr < 0) return senterr;
							p->boguspos++;
						} else {
							senterr=writeline(p->client_fd,WRITELINE_LEADING_N,PERIOD);
							if(senterr < 0) return senterr;
							p->boguspos=0;
						}
					}
				}
			}
		}
		/* Only send NOOP when we are processing or sending to client*/
		if(p->ismail > 3) {
			do_log(LOG_DEBUG, "Sending server a NOOP...");
			senterr=writeline(p->server_fd, WRITELINE_LEADING_RN, SVRCMD);
			if(senterr < 0) return senterr;
			p->noop++;
		}
		do_log(LOG_DEBUG, "Reseting time...");
		p->now = time(NULL);
	}
	return 0;
}

int checkbuff(int fdc) {
	fd_set rfds;
	struct timeval tv;
	int retval,fdc2;
	FD_ZERO(&rfds);
	FD_SET(fdc, &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = 10000;
	fdc2=fdc+1;
	retval = select(fdc2, &rfds, NULL, NULL, &tv);
	if(retval == -1) {
		return 2;
	} else {
		if(retval) return 1;
	}
	return 0;
}

int scan_mailfile(struct proxycontext *p) {
	int ret=0, ret2=0, viret=0, fdc=0;
	int spamfd=-1, htmlfd=-1;
	unsigned long len=0, kbfree=0;
	char spmcmd[512];
	char newmsg[512];
#define NEWMSG "newmsg "
#define ALTMSG "vnmsg "
	FILE *scanner;
	static char line[4096*16];
#ifdef CLAMAV
	unsigned long int blocks = 0;
	const char *virname;
#endif
	struct statvfs fs;
	ret=checktimeout(p);
	if(ret < 0) return SCANNER_RET_CRIT;
	/* See if we want to manipulate the virus notification message before it might be sent */
	if(config->altvnmsg) {
		len=strlen(config->virusdir)+strlen(ALTMSG);
		snprintf(p->vnmsg, len, "%s%s", config->virusdir,ALTMSG);
		if((spamfd=r_open3(p->vnmsg, O_WRONLY | O_CREAT | O_TRUNC,  S_IRUSR | S_IWUSR ))<0) {
			p->errmsg=1;
			do_log(LOG_ALERT, "ERR: Can't create alt virus msg!");
			return SCANNER_RET_CRIT;
		}
		if((htmlfd=r_open2(config->virustemplate, O_RDONLY ))<0) {
			p->errmsg=1;
			do_log(LOG_ALERT, "ERR: Can't open virsus template!");
			return SCANNER_RET_CRIT;
		}
		ret=copyfile(htmlfd,spamfd);
		ret=r_close(spamfd);
		ret=r_close(htmlfd);
	}
  
	/* See if we have enough room to process the message based upon
	   what the user determines is enough room in p3scan.conf
	   This was already done... but it is also dynamic so check again.
	*/
  
	if(statvfs( config->virusdir, &fs ) == SCANNER_RET_ERR) {
		p->errmsg=1;
		context_uninit(p);
		// Emergency: scan_mailfile: Unable to get available space.
		do_log(LOG_EMERG, "ERR: Unable to get available space!");
		return SCANNER_RET_CRIT; // Should never reach here, but keep it clean. :)
	}
  
	if(fs.f_bsize==1024) {
		kbfree=fs.f_bavail;
	} else {
		kbfree=fs.f_bsize * (fs.f_bavail / 1024) + fs.f_bavail%1024 * fs.f_bsize / 1024;
	}  	
	if(config->freespace != 0 && kbfree < config->freespace ) {
		p->errmsg=1;
		// Emergency: scan_mailfile: Not enough space.
		do_log(LOG_EMERG, "ERR: Not enough space! Available space: %lu", kbfree);
		return SCANNER_RET_CRIT;
	}

	 /* perform doctor work (scan for a virus) */
	if(!p->imap) {
		ret=checktimeout(p);
		if(ret < 0) return SCANNER_RET_CRIT;
	}
	p->virinfo=NULL;
	p->scanthis = p->mailfile;
	/* invoke configured scanner */
	do_log(LOG_DEBUG, "Invoking scanner");
	p->virinfo=NULL;
#ifdef CLAMAV
	if(config->clamav) {
		// Check database
		if(clamcheck()) return SCANNER_RET_CRIT;
		if((fdc = open(p->scanthis, O_RDONLY)) == -1) {
			do_log(LOG_CRIT, "ERR: Can't open file %s for scanning!", p->scanthis);
			return SCANNER_RET_CRIT;
		}
		/* scan descriptor */
		if((viret = cl_scandesc(fdc, &clamvirname, &blocks, engine, &limits, CL_SCAN_STDOPT)) == CL_VIRUS) {
			p->virinfo=strdup(clamvirname);
			viret=SCANNER_RET_VIRUS;
		} else {
			if(viret != CL_CLEAN) do_log(LOG_CRIT,"ERR: %s", cl_strerror(ret));
		}
		close(fdc);
	} else
#endif
	{
		viret=config->scanner->scan(p, &p->virinfo);
		do_log(LOG_DEBUG, "Scanner returned %i", viret);
		/* TODO: Fail on unexpected scanner return code. */
	}
	if(p->virinfo) trim(p->virinfo);
	if(!p->imap) {
		ret=checktimeout(p);
		if(ret < 0) return SCANNER_RET_CRIT;
	}
	/* Do we want to scan for spam? */
	if(strlen(NONULL(config->checkspam))>1 && !config->ispam && !viret && !p->cksmtp && !p->imap) {
		do_log(LOG_DEBUG, "Checking for spam");
		if(config->nospampipe) {
			len=strlen(config->checkspam)+strlen(p->mailfile)+2;  // 2 = one space plus NULL string terminator
			snprintf(spmcmd, len, "%s %s", config->checkspam, p->mailfile);
		} else {
			len=strlen(config->checkspam)+strlen(" < ")+strlen(p->mailfile)+strlen(" 2>&1 ");
			snprintf(spmcmd, len, "%s < %s 2>&1", config->checkspam, p->mailfile);
		}
		do_log(LOG_DEBUG, "spmcmd: '%s'", spmcmd);
		/* Ok, now, create a new message */
		len=strlen(config->virusdir)+strlen(NEWMSG);
		snprintf(newmsg, len, "%s%s", config->virusdir,NEWMSG);
		if((spamfd=r_open3(newmsg,O_WRONLY | O_CREAT | O_TRUNC,  S_IRUSR | S_IWUSR))<0) {
			do_log(LOG_ALERT, "ERR: Can't create newmsg!");
			p->errmsg=1;
			return SCANNER_RET_CRIT;
		}
		if((scanner=popen(spmcmd, "r"))==NULL) {
			p->errmsg=1;
			do_log(LOG_ALERT, "ERR: Can't start spammer '%s' !!!", spmcmd);
			return SCANNER_RET_ERR;
		}
		/* call made, check timeout until data returned */
		fdc=fileno(scanner);
		ret2=checkbuff(fdc);
		if(ret2 > 1) return SCANNER_RET_CRIT;
		while(!ret2) {
			ret2=checkbuff(fdc);
			if(ret2 > 1) return SCANNER_RET_CRIT;
			ret=checktimeout(p);
			if(ret < 0) return SCANNER_RET_CRIT;
		}
		int hackme=0, skipmsg=0;
		p->spamfound=0;
		while((fgets(line, 4095, scanner))!=NULL) {
			ret=checktimeout(p);
			if(ret < 0) return SCANNER_RET_CRIT;
			line[strlen(line)-1]='\0';
			/* write the buffer to our new message */
			if(config->debug_scanning) do_log(LOG_DEBUG, "SpammerLine: '%s'", line);
			ret=checktimeout(p);
			if(ret < 0) return SCANNER_RET_CRIT;
			
			// little hack - awie
			if(skipmsg==1) continue;
			if(strncmp(line,"Subject:",8)==0) {
				if(strstr(line,config->spamsubject)) {
					p->spamfound=1;
					do_log(LOG_NOTICE, "SPAM detected mailfrom=%s mailto=%s subject=%s host=%s",
						paramlist_get(p->params, "%MAILFROM%"),
						paramlist_get(p->params, "%MAILTO%"),
						paramlist_get(p->params, "%SUBJECT%"),						
						inet_ntoa(p->client_addr.sin_addr)
						);
				}
			}
			if(p->spamfound==1) {
				if(hackme==0 && strncmp(line,"X-Spam-Checker-Version",22)==0) {
					hackme=1;
				}
				if(config->spamdelete==1) {
					if(skipmsg==0 && strncmp(line,"Content-Type:",13)==0) {
						skipmsg=1;
						continue;
					}
				} else {
					if(strncmp(line,"This is a multi-part message in MIME format",43)==0) {
						writeline(spamfd, WRITELINE_LEADING_N,"");
						writeline(spamfd, WRITELINE_LEADING_N, line);
						writeline(spamfd, WRITELINE_LEADING_N,"");
						continue;
					}
					if(strncmp(line,"Content-Transfer-Encoding: 8bit",31)==0) {
						writeline(spamfd, WRITELINE_LEADING_N, line);
						writeline(spamfd, WRITELINE_LEADING_N,"");
						continue;
					}
					if(hackme==2) {
						if(strncmp(line,"X-POP3-SCANNER",14)==0) continue;
					}
				}
			}
			if((strncmp(line,".",1 ) == 0 && strlen(line) == 1)) {
				do_log(LOG_DEBUG, "Not writing '.'");
			} else if((strncmp(line,".\r",2) == 0 && strlen(line) == 2)) {
				do_log(LOG_DEBUG, "Not writing '.'");
			} else {
				if(p->spamfound==1 && hackme==1) {
					writeline(spamfd, WRITELINE_LEADING_N, "X-POP3-SCANNER: TraceNetwork/Mybox Internet Security");
					writeline(spamfd, WRITELINE_LEADING_N, "X-SPAM-SCANNER: TraceNetwork/Mybox Internet Security");
					hackme=2;
				}
				writeline(spamfd, WRITELINE_LEADING_N, line);
			}
		}
		do_log(LOG_DEBUG, "Writing new .");
		writeline(spamfd, WRITELINE_LEADING_RN, ".");
		if((spamfd=r_close(spamfd))<0) {
			context_uninit(p);
			// Emergency: scan_mailfile: Can't close client message: spamcheck.
			do_log(LOG_EMERG, "ERR: Can't close newmsg spamfd");
			return SCANNER_RET_CRIT;
		}
		ret=pclose(scanner);
		if(p->spamfound==1) {
			/* Spam report is now in $virusdir/newmsg
			so now replace original msg with newmsg */
		
			if((spamfd=r_open3(p->mailfile, O_WRONLY | O_CREAT | O_TRUNC,  S_IRUSR | S_IWUSR ))<0) {
				p->errmsg=1;
				do_log(LOG_ALERT, "ERR: Can't open mailfile!");
				return SCANNER_RET_CRIT;
			}
			if((htmlfd=r_open2(newmsg, O_RDONLY ))<0) {
				p->errmsg=1;
				do_log(LOG_ALERT, "ERR: Can't open mailfile!");
				return SCANNER_RET_CRIT;
			}
			ret=copyfile(htmlfd,spamfd);
			ret=r_close(spamfd);
			ret=r_close(htmlfd);
		}
	}
	/* End of spam checking */
	if(strlen(NONULL(p->virinfo))<1) {
		if(p->virinfo) free(p->virinfo);
		p->virinfo=strdup(MESSAGE_NOVIRINFO);
	}
	ret=viret;
	return ret;
}

unsigned long send_mailfile(char *mailfile, int fd, struct proxycontext *p) {
	struct linebuf *filebuf, *footbuf;
	int mailfd, footfd;
	int res=0, sendret=0, gotprd=0, gottxt=0, nogo=0;
	unsigned long len=0;
	char svrout[1];

	if((mailfd=r_open2(mailfile, O_RDONLY ))<0) {
		context_uninit(p);
		// Emergency: send_mailfile: Can't open client mail file.
		do_log(LOG_EMERG, "ERR: Can't open mailfile (%s)!", mailfile);
		return 0;
	}
	filebuf=linebuf_init(16384);
	footbuf=linebuf_init(512);
	if(!filebuf) {
		r_close(mailfd);
		r_close(fd);
		context_uninit(p);
		// Emergency: send_mailfile: Unable to get memory.
		do_log(LOG_EMERG, "ERR: Could not allocate memory for sending mail!");
	}
	gotprd=0;
	/* advance to mailfd pointer to past data already sent: */
	if(config->broken) {
		if(p->hdroffset && !p->gobogus) {
			while(p->hdroffset) {
				res=getlinep3(mailfd, filebuf);
				p->hdroffset--;
			}
		}
	} else {
		if(p->hdroffset) {
			lseek(mailfd, p->hdroffset, SEEK_SET);
		}
		/* See if bogus headerline sent */
		if(p->gobogus) {
			if(p->boguspos < 91) {
				svrout[0]=BOGUSX[p->boguspos];
				secure_write(p->client_fd,svrout,1);
				p->boguspos++;
			}
			/* now close it */
			writeline(p->client_fd,WRITELINE_LEADING_RN,PERIOD);
			p->gobogus=0;
		}
	}
	while(1) {
		if(!p->imap) {
			sendret=checktimeout(p);
			if(sendret==GETLINE_PIPE) {
				do_log(LOG_CRIT, "ERR: Client disappeared during mail send!");
				linebuf_uninit(filebuf);
				return EPIPE;
			} else if(sendret) {
				context_uninit(p);
				linebuf_uninit(filebuf);
				// Emergency: send_mailfile: Sending mail to client
				do_log(LOG_EMERG, "ERR: Sending mail to client");
				/* we are dead now. Should not reach here. But allow it to fall through in case LOG_EMERG is changed in the future. */
				return 1;
			}
		}
		if((res=getlinep3(mailfd, filebuf))<0) {
			if(res==GETLINE_TOO_LONG) {
				/* Buffer contains part of line,
				take care of later */
			} else {
				/* Other error, take care of later */
				break;
			}
		}
		if(filebuf->linelen >=0 ) {
			len += filebuf->linelen;
			if(config->debug_message) do_log(LOG_DEBUG, ">%s", filebuf->line);
			if(!p->imap) {
				if((strncmp(filebuf->line,".",1 ) == 0 && strlen(filebuf->line) == 1)) gotprd=1;
				if((strncmp(filebuf->line,".\r",2) == 0 && strlen(filebuf->line) == 2)) gotprd=1;
				if(strncmp(filebuf->line,"Content-Type: ",14)==0) {
					if((strncmp(filebuf->line,"Content-Type: text/plain;",25)==0 && strlen(filebuf->line)==25)) {
						gottxt=1;
					} else {
						nogo=1;
					}
				}
			}
			/* Take care of buffer here */
			if(res==GETLINE_TOO_LONG) {
				sendret=writeline(fd, WRITELINE_LEADING_NONE, filebuf->line);
			} else {
				if(!p->cksmtp) {
					sendret=writeline(fd, WRITELINE_LEADING_RN, filebuf->line);
				} else {
					if(!gotprd) sendret=writeline(fd, WRITELINE_LEADING_RN, filebuf->line);
				}
			}
			if(sendret==GETLINE_PIPE) {
				do_log(LOG_CRIT, "ERR: Client disappeared during mail send!");
				linebuf_uninit(filebuf);
				return EPIPE;
			} else if(sendret) {
				context_uninit(p);
				linebuf_uninit(filebuf);
				// Emergency: send_mailfile: Sending mail to client
				do_log(LOG_EMERG, "ERR: Sending mail to client");
				/* we are dead now. Should not reach here. But allow it
				to fall through in case LOG_EMERG is changed in the future. */
				return 1;
			}
		}
	}
	if(res!=GETLINE_EOF) {
		do_log(LOG_CRIT, "ERR: reading from mailfile %s, error code: %d", mailfile, res);
		linebuf_uninit(filebuf);
		return 1;
	}
	if(!gotprd && !p->imap) {
		do_log(LOG_DEBUG, "Wrote new EOM.");
		writeline(fd, WRITELINE_LEADING_RN, ".");
	}
	linebuf_uninit(filebuf);
	r_close(mailfd);
	return len;
}

void set_defaultparams(struct proxycontext * p) {
	char buf[256];
	FILE *scanner;
	static char line[512], virdef[512];
	int len=0, vlen=0;
	char vcmd[512];
#ifdef CLAMAV
	char *path;
	struct stat foo;
#endif

	gethostname(buf, 256);
	paramlist_set(p->params, "%HOSTNAME%", buf);
	if(getdomainname(buf, 256)) do_log(LOG_CRIT,"ERR: getdomainname");
	paramlist_set(p->params, "%DOMAINNAME%", buf);
	paramlist_set(p->params, "%PROGNAME%", PROGNAM);
	paramlist_set(p->params, "%VERSION%", VERSION);
	paramlist_set(p->params, "%CLIENTIP%", inet_ntoa(p->client_addr.sin_addr));
	snprintf(buf, sizeof(buf),"%i", ntohs(p->client_addr.sin_port));
	paramlist_set(p->params, "%CLIENTPORT%", buf);
	paramlist_set(p->params, "%SERVERIP%", inet_ntoa(p->server_addr.sin_addr));
	snprintf(buf, sizeof(buf), "%i", ntohs(p->server_addr.sin_port));
	paramlist_set(p->params, "%SERVERPORT%", buf);
	if(p->cksmtp) {
		paramlist_set(p->params, "%PROTOCOL%", "SMTP");
	} else if(p->imap) {
		paramlist_set(p->params, "%PROTOCOL%", "IMAP");
	} else {
		paramlist_set(p->params, "%PROTOCOL%", "POP3");
	}
#ifdef CLAMAV
	if(config->clamav) {
		version=strdup(cl_retver());
		dbdir = cl_retdbdir();
		if(!(path = malloc(strlen(dbdir) + 22))) do_log(LOG_CRIT,"ERR: Could not get database path.");
		// Check for updated info first... otherwise, we might get the originally installed database.
		sprintf(path, "%s/daily.inc/daily.info", dbdir);
		if(stat(path, &foo) == -1) {
			sprintf(path, "%s/daily.cvd", dbdir);
		}
		if((daily = cl_cvdhead(path))) {
			time_t t = (time_t) daily->stime;
			snprintf(buf,sizeof(buf),"ClamAV %s/%d/%s", version, daily->version, ctime(&t));
			paramlist_set(p->params, "%VDINFO%", buf);
		}
		free(path);
	} else {
#endif
		paramlist_set(p->params, "%VDINFO%", NULL);
#ifdef CLAMAV
	}
#endif
}

void set_maildateparam(struct paramlist * params) {
	char buf[256];
	int diff_hour, diff_min;
	time_t now = time(NULL);
	struct tm *tm = localtime(&now);
	struct tm local;
	struct tm *gmt;
	int len;
	memcpy(&local, tm, sizeof(struct tm));
 	gmt = gmtime(&now);

	diff_min = 60*(local.tm_hour - gmt->tm_hour) + local.tm_min - gmt->tm_min;
	if(local.tm_year != gmt->tm_year) {
		diff_min += (local.tm_year > gmt->tm_year)? 1440 : -1440;
	} else if(local.tm_yday != gmt->tm_yday) {
		diff_min += (local.tm_yday > gmt->tm_yday)? 1440 : -1440;
	}
	diff_hour = diff_min/60;
	diff_min  = abs(diff_min - diff_hour*60);
	len = strftime(buf, sizeof(buf), "%a, ", &local);
	sprintf(buf + len, "%02d ", local.tm_mday);
	len += strlen(buf + len);
	len += strftime(buf + len, sizeof(buf) - len, "%b %Y %H:%M:%S", &local);
	sprintf(buf + len, " %+03d%02d", diff_hour, diff_min);
	paramlist_set(params, "%MAILDATE%", buf);
}

void set_paramsfrommailheader(char *mailfile, struct paramlist * params, struct proxycontext *p) {
	struct linebuf *lb;
	int fd=0;
	char *c;
	if((fd=r_open2(mailfile, O_RDONLY ))<0) return;
	lb=linebuf_init(65535);
	while(getlinep3(fd, lb)>=0) {
		if(lb->linelen >0 ) {
			if(!strncasecmp(lb->line, "from: ", 6)) {
				c=lb->line+6;
				trim(c);
				paramlist_set(params, "%MAILFROM%", c);
			} else if(!strncasecmp(lb->line, "subject: ", 9)) {
				c=lb->line+9;
				trim(c);
				paramlist_set(params, "%SUBJECT%", c);
			} else if(!strncasecmp(lb->line, "To: ", 4)) {
				c=lb->line+4;
				trim(c);
				paramlist_set(params, "%MAILTO%", c);
			} else if(!strncasecmp(lb->line, "X-RCPT-TO: ", 11)) {
				if(!paramlist_get(params, "%USERNAME%")) {
					c=lb->line+11;
					trim(c);
					c = substr(c,1,(strlen(c)-2));
					paramlist_set(params, "%USERNAME%", c);
					free(c);
					if(!p->mailuser) p->mailuser=paramlist_get(params,"%USERNAME%");
				}
			}
		} else if(lb->linelen == 0) break; // only the header
	}
	linebuf_uninit(lb);
	r_close(fd);
}

void unset_paramsfrommailheader(struct paramlist * params) {
	paramlist_set(params, "%MAILFROM%", NULL);
	paramlist_set(params, "%SUBJECT%", NULL);
	paramlist_set(params, "%MAILTO%", NULL);
	paramlist_set(params, "%VIRUSNAME%", NULL);
	paramlist_set(params, "%MAILFILE%", NULL);
	paramlist_set(params, "%P3SCANID%", NULL);
	paramlist_set(params, "%VDINFO%", NULL);
}

int do_virusaction(struct proxycontext *p) {
	struct linebuf *ckbuf;
	struct stat  sbuf;
	char *mail=NULL, *mailx=NULL;
	char svrout[1];
	char comm[4096];
	unsigned long len;
	int readerr=0, bufferr=0, subjfd=-1, extrafd=-1;
	int ret, cuid, save_errno, status;

	if(p->cksmtp) {
		do_log(LOG_WARNING,"554 %s '%s'",config->smtprset,paramlist_get(p->params, "%VIRUSNAME%"));
		writeline_format(p->client_fd, WRITELINE_LEADING_RN, "554 %s '%s'",config->smtprset,paramlist_get(p->params, "%VIRUSNAME%"));
		writeline_format(p->server_fd, WRITELINE_LEADING_RN, "RSET");
		writeline_format(p->server_fd, WRITELINE_LEADING_RN, "QUIT");
		return 0;
	}
	/* complete the header of the original message... */
	if(config->broken) {
		if(p->hdroffset && !p->gobogus) {
			while((readerr=getlinep3(p->mailhdr_fd,p->hdrbuf)!=GETLINE_EOF)) {
				writeline(p->client_fd, WRITELINE_LEADING_RN, p->hdrbuf->line);
			}
		} else {
			while((readerr=r_read(p->mailhdr_fd,p->cbuff,1)>0)) {
				if(readerr < 0) {
					context_uninit(p);
					// Emergency: do_virusaction: Could not read header file (broken)
					do_log(LOG_EMERG, "ERR: Could not read header file (broken)!");
				}
				bufferr=secure_write(p->client_fd,p->cbuff,1);
				if(bufferr==GETLINE_ERR) {
					context_uninit(p);
					// Emergency: do_virusaction: Could not flush header file (broken)
					do_log(LOG_EMERG, "ERR: Could not flush header buffer to client (broken)!");
				}
			}
		}
	} else {
		if(p->gobogus) {
			if(p->boguspos < 74) {
				svrout[0]=BOGUSX[p->boguspos];
				secure_write(p->client_fd,svrout,1);
				p->boguspos++;
			}
			/* now close it */
			writeline(p->client_fd,WRITELINE_LEADING_RN,PERIOD);
			p->gobogus=0;
		} else {
			while((readerr=r_read(p->mailhdr_fd,p->cbuff,1)>0)) {
				if(readerr < 0) {
					context_uninit(p);
					// Emergency: do_virusaction: Could not read header file
					do_log(LOG_EMERG, "ERR: Could not read header file!");
				}
				if((strncmp(p->cbuff,"\r",1)==0) || (strncmp(p->cbuff,"\n",1)==0) ) {
					bufferr=secure_write(p->client_fd,"\r\n",2);
				} else {
					if((strncmp(p->cbuff,"\r",1)!=0) && (strncmp(p->cbuff,"\n",1)!=0)) {
						bufferr=secure_write(p->client_fd,p->cbuff,1);
					}
				}
				if(bufferr==GETLINE_ERR) {
					context_uninit(p);
					// Emergency: do_virusaction: Could not flush header file
					do_log(LOG_EMERG, "ERR: Could not flush header buffer to client!");
				}
			}
		}
	}
	if(!p->hdrdate) writeline_format(p->client_fd,WRITELINE_LEADING_RN,"Date: %s", paramlist_get(p->params, "%MAILDATE%"));
	if(!p->hdrfrom) writeline_format(p->client_fd,WRITELINE_LEADING_RN,"From: %s", paramlist_get(p->params, "%MAILFROM%"));
	if(!p->hdrto) writeline_format(p->client_fd,WRITELINE_LEADING_RN,"To: %s", paramlist_get(p->params, "%MAILTO%"));
	p->hdroffset=0;
	p->extra=0;
	if((mail = malloc(strlen(p->mailfile)+10))==NULL) do_log(LOG_CRIT,"ERR: malloc(mail)");
	if((extrafd=r_open2(EXTRA,O_RDONLY))>=0 && !p->nosend) {
		ckbuf=linebuf_init(128);
		if((readerr=getlinep3(extrafd,ckbuf)!=GETLINE_EOF)) {
			p->extra=1;
			if((mailx = malloc(strlen(p->mailfile)+10))==NULL) do_log(LOG_CRIT,"ERR: malloc(mailx)");
		} else {
			do_log(LOG_NOTICE, "ERR: p3scan.extra should not be blank");
		}
		linebuf_uninit(ckbuf);
		r_close(extrafd);
	}
	/* If mail is infected AND we just want to delete it, just don't move it */
	if(!config->delit) {
		snprintf(comm,4096,"%s/%s ",config->virusdirbase,p->filename);
		trim(comm);
		if((subjfd=r_open3(comm, O_WRONLY | O_CREAT | O_TRUNC,  S_IRUSR | S_IWUSR ))<0) {
			p->errmsg=1;
			do_log(LOG_ALERT, "ERR: Can't create %s!",comm);
			return SCANNER_RET_CRIT;
		}
		if((extrafd=r_open2(p->mailfile, O_RDONLY ))<0) {
			p->errmsg=1;
			do_log(LOG_ALERT, "ERR: Can't open mailfile: %s!",p->mailfile);
			return SCANNER_RET_CRIT;
		}
		ret=copyfile(extrafd,subjfd);
		ret=r_close(extrafd);
		ret=fchmod(subjfd, S_IRUSR | S_IWUSR);
		ret=r_close(subjfd);
		unlink(p->mailfile);
		sync();
	}
	sprintf(mail, "%s/%i.mailout", config->notifydir,getpid());
	if(p->extra && !p->nosend) sprintf(mailx, "%s/%i.extrout", config->notifydir,getpid());
	if(!config->altvnmsg) {
		subjfd = r_open2(config->virustemplate,O_RDONLY);
		if(subjfd<0) {
			save_errno=errno;
			free(mail);
			if(p->extra) free(mailx);
			context_uninit(p);
			// Emergency: do_virusaction: Could not open virus template
			do_log(LOG_EMERG,"ERR: Critical error opening file '%s', error: %s Program aborted.", config->virustemplate,strerror(save_errno));
			// should not reach here as we are dead
		}
		readerr=r_read(subjfd,comm,4096);
		// Check for Subject line...
		if(strncmp(comm,"Subject:",8)!=0) {
			if(p->nosend) {
				writeline_format(p->client_fd,WRITELINE_LEADING_RN,"Subject: %s %s",config->blacksubj,paramlist_get(p->params, "%MAILFROM%"));
			} else {
				writeline_format(p->client_fd,WRITELINE_LEADING_RN,"Subject: %s %s",config->subject,paramlist_get(p->params, "%VIRUSNAME%"));
			}
		}
		r_close(subjfd);
	}
	if(p->extra && !p->nosend) {
		if((ret = parsefile(EXTRA, mailx, p->params, WRITELINE_LEADING_RN))!=0) {
			context_uninit(p);
			unlink(mail);
			free(mail);
			if(p->extra) {
				unlink(mailx);
				free(mailx);
			}
			if(ret<0) {
				// Emergency: do_virusaction: Can't open/create warning message
				do_log(LOG_EMERG, "ERR: Can't open extra mail notification template %s",EXTRA);
			} else {
				do_log(LOG_EMERG, "ERR: Can't create extra virus warning mail message %s",p->mailfile);
			}
			return -1;
		}
	}
	if(config->altvnmsg && !p->nosend) {
		if((ret = parsefile(p->vnmsg, mail, p->params, WRITELINE_LEADING_RN))!=0) {
			context_uninit(p);
			unlink(mail);
			free(mail);
			if(p->extra) {
				unlink(mailx);
				free(mailx);
			}
			if(ret<0) {
				// Emergency: do_virusaction: Can't open/create alternate mail warning message
				do_log(LOG_EMERG, "ERR: Can't open alternate mail notification template %s",p->vnmsg);
			} else {
				do_log(LOG_EMERG, "ERR: Can't create virus warning mail message %s",p->mailfile);
			}
			return -1;
		}
		do_log(LOG_DEBUG,"Sending alternate virus notification: %s",p->vnmsg);
	} else {
		if((p->nosend && !config->blackshort) || (!p->nosend)) {
			if((ret = parsefile(config->virustemplate, mail, p->params, WRITELINE_LEADING_RN))!=0) {
				context_uninit(p);
				unlink(mail);
				free(mail);
				if(p->extra) {
					unlink(mailx);
					free(mailx);
				}
				if(ret<0) {
					// Emergency: do_virusaction: Can't open/create main notification message
					do_log(LOG_EMERG, "ERR: Can't open mail notification template %s",config->virustemplate);
				} else {
					do_log(LOG_EMERG, "ERR: Can't create virus warning mail message %s",p->mailfile);
				}
				return -1;
			}
		}
	}
	do_log(LOG_DEBUG, "sending new mail");
	if(p->nosend && config->blackshort) {
		writeline_format(p->client_fd, WRITELINE_LEADING_RN, "%s",PERIOD);
	} else {
		len=send_mailfile(mail, p->client_fd,p);
		p->bytecount+=len;
	}
	unlink(mail);
	free(mail);
	if(p->extra && !p->nosend) {
		unlink(mailx);
		free(mailx);
	}
	p->ismail=0;
	if(len>0 || (p->nosend && config->blackshort)) return 0;
	return -1;
}

struct proxycontext * context_init(void) {
	struct proxycontext * p;
	if((p=malloc(sizeof(struct proxycontext)))==NULL) return NULL;
	p->socksize=sizeof(struct sockaddr_in);
	p->serverbuf=linebuf_init(MAXFILENAME);
	p->clientbuf=linebuf_init(MAXFILENAME);
	if(config->broken) p->hdrbuf=linebuf_init(MAXFILENAME);
	p->msgnum=0;
	p->bytecount=0;
	p->client_fd=-1;
	p->server_fd=-1;
	p->mailhdr_fd=-1;
	p->blacklist=0;
	p->whitelist=0;
	p->action=0;
	p->actport=0;
	p->boguspos=0;
	p->checksmtp=0;
	p->cksmtp=0;
	p->errmsg=0;
	p->extra=0;
	p->fakehdrdone=0;
	p->gobogus=0;
	p->hdrdate=0;
	p->hdrfrom=0;
	p->hdrto=0;
	p->header_exists=0;
	p->imap=0;
	p->ismail=0;
	p->mailcount=0;
	p->newsock=0;
	p->noop=0;
	p->noscan=0;
	p->nosend=0;
	p->notified=0;
	p->nowrite=0;
	p->posttop=0;
	p->scannerinit=SCANNER_INIT_NO;
	p->topping=0;
	p->viret=0;
	p->spamfound=0;
	p->actsvr=NULL;
	p->mailuser=NULL;
	p->extrasubj=NULL;
	p->filename=NULL;
	p->filestatus=NULL;
	p->scanthis=NULL;
	p->virinfo=NULL;
	return p;
}

void context_uninit(struct proxycontext * p) {
	if(p->client_fd > 0 ) r_close(p->client_fd);
	if(p->mailhdr_fd > 0 ) r_close(p->mailhdr_fd);
	if(p->server_fd > 0 ) {
		r_close(p->server_fd);
	}
	if(!loop) paramlist_uninit(&p->params);
	linebuf_uninit(p->clientbuf);
	linebuf_uninit(p->serverbuf);
	if(config->broken) linebuf_uninit(p->hdrbuf);
	free(p);
}

void closehdrfile(struct proxycontext * p) {
	p->fakehdrdone=1;
	r_close(p->mailhdr_fd);
	p->hdroffset=0;
	p->mailhdr_fd = r_open2(p->mailhdr, O_RDONLY);
	if(p->mailhdr_fd<0) {
		context_uninit(p);
		// Emergency: closehdrfile: Could not open header file.
		do_log(LOG_EMERG,"ERR: Critical error opening file '%s', Program aborted.", p->mailhdr);
		/* should not reach here as we are dead */
	}
	p->now = time(NULL);
	if(!p->notified && !p->cksmtp) {
		do_log(LOG_DEBUG, "Informing email client to wait...");
		writeline_format(p->client_fd, WRITELINE_LEADING_RN,"+OK P3Scan'ing...");
		p->notified=1;
	}
	p->ismail=3;
}

void do_sigterm_proxy(int signr) {
	sigset_t blockset, oldset;

	if((sigfillset(&blockset))<0) do_log(LOG_EMERG, "(blockset) could not init!");
	if((sigprocmask(SIG_SETMASK,&blockset,&oldset))<0) do_log(LOG_EMERG, "(blockset) couldn't block-5!");
	do_log(LOG_DEBUG, "do_sigterm_proxy, signal %i", signr);
	if(global_p == NULL) {
		do_log(LOG_DEBUG, "already uninitialized");
		return;
	}
	if(signr != -1) do_log(LOG_CRIT, "ERR: We caught SIGTERM!"); /* sig -1 is ok */
	if(global_p->client_fd != -1) r_close(global_p->client_fd);
	if(global_p->server_fd != -1) r_close(global_p->server_fd);
	if(global_p->scannerinit==SCANNER_INIT_OK && config->scanner->uninit2) {
		do_log(LOG_DEBUG, "calling uninit2");
		config->scanner->uninit2(global_p);
		do_log(LOG_DEBUG, "uninit2 done");
	}
	do_log(LOG_DEBUG, "Uninit context");
	context_uninit(global_p);
	global_p=NULL;
	if((sigprocmask(SIG_SETMASK,&oldset,NULL))<0) do_log(LOG_EMERG, "(blockset) couldn't unblock-5!");
	do_log(LOG_DEBUG,"context_uninit done, exiting now (sig_proxy)");
	if(signr != -1) exit(EXIT_FAILURE);
}

void checklist(struct proxycontext *p) { // see if black/whitelist are valid
	struct linebuf *listbuf;
	unsigned long len;
	int blacklist_fd, whitelist_fd, ret;
	len=strlen(CONFIGBASE)+1+strlen(BLACKLIST)+1;
	snprintf(p->listfileb, len, "%s/%s", CONFIGBASE,BLACKLIST);
	len=strlen(CONFIGBASE)+1+strlen(WHITELIST)+1;
	snprintf(p->listfilew, len, "%s/%s", CONFIGBASE,WHITELIST);
	p->blacklist=0;
	p->whitelist=0;
	listbuf=linebuf_init(256);
	if((blacklist_fd=r_open2(p->listfileb, O_RDONLY))>=0) {
		while((ret=getlinep3(blacklist_fd, listbuf))>=0) {
			if(strlen(NONULL(listbuf->line))) {
				if(listbuf->linelen > 2 && !p->blacklist) {
					p->blacklist=1;
					do_log(LOG_DEBUG,"Found %s",p->listfileb);
				}
			}
		}
		if(!p->blacklist) do_log(LOG_DEBUG,"Found %s but it's blank... ignoring", p->listfileb);
		r_close(blacklist_fd);
	}
	if((whitelist_fd=r_open2(p->listfilew, O_RDONLY))>=0) {
		while((ret=getlinep3(whitelist_fd, listbuf))>=0) {
			if(strlen(NONULL(listbuf->line))) {
				if(listbuf->linelen > 2 && !p->whitelist) {
					p->whitelist=1;
					do_log(LOG_DEBUG,"Found %s",p->listfilew);
				}
			}
		}
		if(!p->whitelist) do_log(LOG_DEBUG,"Found %s but it's blank... ignoring", p->listfilew);
		r_close(whitelist_fd);
	}
	linebuf_uninit(listbuf);
}

int proxy(struct proxycontext *p) {
	fd_set fds_read;
	struct timeval timeout;
	struct linebuf *listbuf;
	int scanfd=-1;
	int error;
	int maybe_a_space; // signals a space in the keyword for setting USERNAME var
	int clientret, serverret;
	unsigned long len, len1, msgsize;
	char *rest, *rest2;
	char buf[64]; // used to set username
#ifdef SET_TOS
	int tos;
#endif
	int scannerret, fauth_fd, ret;
	int trapdata=0, trapcapa1=0, postdata=0;
	int trapcapa2=0, trapcapa3=0;
	int smtpcmd=0;
	int smtprstlb=0;
	char *msgptr=NULL;
	char fcmd[512];
	FILE *scanner;
	static char  line[512];
	int first=1,didit=0,again=0, uidl=0;
	int getmsgsize=0;
	size_t  i,ii,user_len,len_t;
	char *tmp=NULL;
	char *tmp2=NULL;
	char *tmp3=NULL;
	char *sname,*sport;
	int blacklist_fd, whitelist_fd;
#define STARTWITH 1
	int *msgnr = NULL, *tmpmsgnr;
	char *itag;
	long int inr;
	const char spacer[] = " ";
	int imapinit;

	p->now = time(NULL);
	p->mailhdr_fd=-1;
	p->noop=0;
	p->cksmtp=0;
	p->server_addr.sin_family = AF_INET;
#ifdef CLAMAV
	char *path;
	struct stat foo;
#endif

	if(htonl(INADDR_ANY) == config->targetaddr.sin_addr.s_addr) {
		if(getsockopt(p->client_fd, SOL_IP, SO_ORIGINAL_DST, &p->server_addr, &p->socksize)) {
			do_log(LOG_CRIT, "ERR: No IP-Conntrack-data (getsockopt failed)");
			return 1;
		}
		/* try to avoid loop */
		if(((ntohl(p->server_addr.sin_addr.s_addr) == INADDR_LOOPBACK) && p->server_addr.sin_port == config->addr.sin_port )
			/* src.ip == dst.ip */
			|| (p->server_addr.sin_addr.s_addr == p->client_addr.sin_addr.s_addr)) {
			loop=1;
			do_log(LOG_CRIT, "ERR: Oops, that would loop!");
			return 1;
		}
	} else {
		/* non-proxy mode */
		p->server_addr.sin_addr.s_addr = config->targetaddr.sin_addr.s_addr;
		p->server_addr.sin_port = config->targetaddr.sin_port;
	}
	/* open socket to 'real-server' */
	if((p->server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		do_log(LOG_CRIT, "ERR: Cannot open socket to real-server");
		return 1;
	}
#ifdef SET_TOS
	tos=SET_TOS;
	if(setsockopt(p->client_fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos))) do_log(LOG_WARNING, "Can't set TOS (incoming connection)");
	if(setsockopt(p->server_fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos))) do_log(LOG_WARNING, "Can't set TOS (outgoing connection)");
#endif
	p->serverbuf=linebuf_init(4096);
	p->params=paramlist_init();
	p->clientbuf=linebuf_init(4096);
	/* Check connection type */
	if(ntohs(p->server_addr.sin_port)==config->smtpport) {
		p->cksmtp=1;   /* Processing an SMTP message */
		p->checksmtp=1;/* We should scan it */
		do_log(LOG_NOTICE, "SMTP Connection from %s:%i", inet_ntoa(p->client_addr.sin_addr), ntohs(p->client_addr.sin_port));
	} else if(ntohs(p->server_addr.sin_port)==config->imaport) {
		p->imap=1;
		do_log(LOG_NOTICE, "IMAP Connection from %s:%i", inet_ntoa(p->client_addr.sin_addr), ntohs(p->client_addr.sin_port));
	} else {
		do_log(LOG_NOTICE, "POP3 Connection from %s:%i", inet_ntoa(p->client_addr.sin_addr), ntohs(p->client_addr.sin_port));
	}
	do_log(LOG_NOTICE, "Real-server address is %s:%i", inet_ntoa(p->server_addr.sin_addr), ntohs(p->server_addr.sin_port));
	ret=connect(p->server_fd, (struct sockaddr *) &p->server_addr, p->socksize);
	if(ret) {
		do_log(LOG_CRIT, "ERR: Cannot connect to real-server: %s",inet_ntoa(p->server_addr.sin_addr));
		return 1;
	}
	set_defaultparams(p);
	do_log(LOG_DEBUG, "starting mainloop");
	while(1) {
		/* read from client */
		clientret=getlinep3(p->client_fd, p->clientbuf);
		if(clientret<0) {
			if(clientret==GETLINE_TOO_LONG) {
				do_log(LOG_DEBUG, "Line too long: Getting rest of line.");
			} else {
				do_log(LOG_DEBUG, "Closing connection (no more input from client)");
				return 0;
			}
		}
		if(clientret==GETLINE_OK) {
			if(p->cksmtp) {
				if(p->ismail < 2 || p->ismail >4) {
					do_log(LOG_DEBUG, "--> %s", p->clientbuf->line);
				} else {
					if(config->debug_smtp) {
						do_log(LOG_DEBUG, "--> %s", p->clientbuf->line);
					}
				}
			} else if(p->imap) {
				if(config->debug_imap) {
					if(!strncasecmp(p->clientbuf->line,"login", 6)) {
						if(config->password) {
							do_log(LOG_DEBUG, "--> %s", p->clientbuf->line);
						} else {
							do_log(LOG_DEBUG, "--> <login/password sent>");
						}
					} else {
						do_log(LOG_DEBUG, "--> %s", p->clientbuf->line);
					}
				}
			} else {
				if(!strncasecmp(p->clientbuf->line,"pass", 4)) {
					if(config->password) {
						do_log(LOG_DEBUG, "--> %s", p->clientbuf->line);
					} else {
						do_log(LOG_DEBUG, "--> <password sent>");
					}
				} else {
					do_log(LOG_DEBUG, "--> %s", p->clientbuf->line);
				}
			}
 		}

		/* read from server */
		serverret=getlinep3(p->server_fd, p->serverbuf);
		if(serverret<0) {
			if(serverret==GETLINE_TOO_LONG) {
				do_log(LOG_DEBUG, "Line too long: Getting rest of line.");
			} else {
				do_log(LOG_DEBUG, "Closing connection (no more input from server)");
				if(config->skipsize && (sizeof msgnr) && !p->cksmtp) {
					if(msgnr) free(msgnr);
				}
				return 0;
			}
		} else {
			if(p->noop) {
				if(!strncasecmp(p->serverbuf->line,POK, 3)) {
					do_log(LOG_DEBUG, "%s: NOOP response. Flushed %i NOOP's", p->serverbuf->line, p->noop);
					linebuf_zero(p->serverbuf);
					p->noop=0;
				} else {
					do_log(LOG_DEBUG, "Oops, %s doesn't looks like a server's NOOP response. Waiting next...", p->serverbuf->line);
				}
			}
		}
		if(serverret==GETLINE_OK && p->serverbuf->line != NULL) {
			if(((p->ismail < 2 || p->ismail > 3) || (config->debug_message)) && !p->imap) {
				do_log(LOG_DEBUG, "<-- %s", p->serverbuf->line);
			} else {
				if(p->imap && config->debug_imap) {
					do_log(LOG_DEBUG, "<-- %s", p->serverbuf->line);
				}
			}
		}
		/* Check for UIDL server side command */
		if(!p->ismail && p->serverbuf->linelen > 0 && !strncasecmp(p->serverbuf->line,"uidl", 4)) uidl = 1;
		if(getmsgsize && !p->ismail) {
			// in LIST, save message sizes to an array.
			if((!strncasecmp(p->serverbuf->line,"+ok", 3) || (uidl && !strncasecmp(p->serverbuf->line,".", 1))) && !didit) {
				didit=1;
				ii=0;
				if(uidl) uidl = 0;
				if((msgnr=malloc(STARTWITH)) == NULL) {
					do_log(LOG_EMERG,"LIST malloc failed!");
				}
			} else {
				if((p->serverbuf->line[0]=='.')) {
					getmsgsize=0;
				} else {
					i=strtol(p->serverbuf->line, &rest, 10);
					if(i != ii) {
						if((tmpmsgnr=realloc(msgnr,sizeof(int) * i)) == NULL) {
							if(msgnr) free(msgnr);
							do_log(LOG_EMERG,"LIST realloc failed!");
						} else {
							msgnr=tmpmsgnr;
						}
						msgnr[i-1]=strtol(rest,&rest2,10) / 1024;
						do_log(LOG_DEBUG,"Set msgsize to %d for msgnr %i",msgnr[i-1],i);
						ii=i;
					}
				}
			}
		}
		if(p->cksmtp && trapdata && !smtpcmd && !postdata) {
			do_log(LOG_DEBUG, "not reading input buffers...");
		} else {
			if(clientret == GETLINE_NEED_READ && serverret == GETLINE_NEED_READ) {
				FD_ZERO(&fds_read);
				FD_SET(p->client_fd, &fds_read);
				FD_SET(p->server_fd, &fds_read);
				timeout.tv_sec = 300;
				timeout.tv_usec = 0;
				if((ret=select(p->server_fd + 1, &fds_read, NULL, NULL, &timeout))<1) {
					/* timeout */
					do_log(LOG_DEBUG, "select returned %i", ret);
					break;
				} else { 
					continue;
				}
			}
		}
		if(p->ismail>3) p->serverbuf->line=NULL;
		if(p->clientbuf->linelen>=0 && p->ismail<2 ) { // Not processing message
			/* scan command the client sent */
			if(p->imap) {
				// Lets do all imap stuff here
				if((stristr(p->clientbuf->line,"fetch")>=0 
					&& (stristr(p->clientbuf->line,"RFC822")>=0 
					|| stristr(p->clientbuf->line,"body")>=0 
					|| stristr(p->clientbuf->line,"text")>=0)) 
					&& ((stristr(p->clientbuf->line,"BODYSTRUCTURE")>=0) 
					|| (stristr(p->clientbuf->line,"MIME"))>=0)) {
						// we have something that may be infected or we have header info...
						tmp=strndup(p->clientbuf->line,p->clientbuf->linelen);
						// get IMAP tag number... being we can only work on one command at a time, save it locally.
						itag=strsep(&tmp,spacer);
						// now get message UID number
						tmp2=strsep(&tmp,spacer);
						tmp2=strsep(&tmp,spacer);
						if((stristr(tmp2,"fetch")>=0)) {
							while(stristr(tmp2,"fetch")>=0) tmp2=strsep(&tmp,spacer);
						}
						if(scanfd > 0) {
							do_log(LOG_DEBUG,"%d was not processed.",inr);
							r_close(scanfd);
						}
						inr=atoi(tmp2);
						do_log(LOG_DEBUG,"getting imap tag: %s msgnr: %d for scanning...",itag,inr);
						p->ismail=1;
						p->action=ACT_IMAP;
				} else if((stristr(p->clientbuf->line,"login")>=0)) {
					//itag LOGIN "user" "password"
					tmp=strndup(p->clientbuf->line,p->clientbuf->linelen);
					tmp2=strsep(&tmp,spacer); // itag
					tmp2=strsep(&tmp,spacer); // LOGIN
					tmp3=strsep(&tmp,spacer); // "user"
					tmp2=substr(tmp3,1,strlen(tmp3)-2);
					paramlist_set(p->params, "%USERNAME%",tmp2);
					free(tmp2);
					p->mailuser=paramlist_get(p->params,"%USERNAME%");
					do_log(LOG_DEBUG,"IMAP USER %s",p->mailuser);
					checklist(p);
				}
			} else if(!strncasecmp(p->clientbuf->line,"list", 4)) {
				if(config->skipsize) getmsgsize=1;
				didit=0;
			} else if(!strncasecmp(p->clientbuf->line,"retr", 4)) {
				p->msgnum=atoi(&p->clientbuf->line[5]);
				if(p->msgnum<1) {
					/* that's not ok */
					do_log(LOG_WARNING,"RETR msg %i (<1) !!!! ", p->msgnum);
					p->ismail=0;
				} else {
					if(config->scannerenabled) {
						do_log(LOG_DEBUG,"RETR %i (Scanner Enabled)", p->msgnum);
						/* enable message parsing (only if scanner enabled) */
						p->ismail=1;
						p->action=ACT_RETR;
						if(config->skipsize) {
							if((msgnr[p->msgnum-1] >= config->skipsize)) {
								p->ismail=0;
								p->action=0;
								do_log(LOG_NOTICE,"Skipped scanning: %s message: %d size: %dk", p->mailuser, p->msgnum, msgnr[p->msgnum-1]);
							}
						}
					} else {
						do_log(LOG_DEBUG,"RETR %i (Scanner Disabled)", p->msgnum);
					}
					p->mailcount++;
				}
			} else if(!strncasecmp(p->clientbuf->line,"capa",4)) {
				trapcapa1=1;
				trapcapa2=1;
				trapcapa3=1;
				do_log(LOG_DEBUG,"Client checking server CAPABILITIES...");
			} else if(!strncasecmp(p->clientbuf->line,"top", 3)) {
				p->msgnum=atoi(&p->clientbuf->line[4]);
				if(p->msgnum<1) {
					// that's not ok
					do_log(LOG_WARNING,"TOP msg %i (<1) !!!! ", p->msgnum);
					p->ismail=0;
				} else {
					if(config->scannerenabled) {
						do_log(LOG_DEBUG,"TOP %i (Scanner Enabled)", p->msgnum);
						/* enable message parsing (only if scanner enabled) */
						p->ismail=1;
						p->action=ACT_TOP;
						if(config->skipsize) {
							if((msgnr[p->msgnum-1] >= config->skipsize)) {
								p->ismail=0;
								p->action=0;
								do_log(LOG_NOTICE,"Skipped scanning: %s message: %d size: %dk", p->mailuser, p->msgnum, msgnr[p->msgnum-1]);
							}
						}
					} else {
						do_log(LOG_DEBUG,"TOP %i (Scanner Disabled)", p->msgnum);
					}
					p->mailcount++;
				}
			} else if(!strncasecmp(p->clientbuf->line,"mail from:", 10) && p->cksmtp) {
				msgptr=p->clientbuf->line + stristr(p->clientbuf->line,"size=");
				if(config->smtpsize && msgptr >= p->clientbuf->line) {
					/* we have size of message */
					char *str = substr(msgptr,5,strlen(msgptr)-5);
					msgsize=atoi(str) / 1024;
					free(str);
					if(msgsize > config->smtpsize) {
						do_log(LOG_CRIT,"SMTP Message too large for scanning: %i",msgsize);
						p->checksmtp=0;
						p->cksmtp=0;
						p->ismail=0;
						r_close(p->mailhdr_fd);
						unlink(p->mailhdr);
					} else do_log(LOG_DEBUG,"smtpsize=%i",msgsize);
				}
				len=p->clientbuf->linelen-10;
				if(msgptr) len=(len-5)-strlen(msgptr+5);
				if(len >= sizeof(buf)) len=sizeof(buf)-1;
				if(len>0) {
					memcpy(buf, (char *)(p->clientbuf->line)+10, len );
					buf[len]='\0';
					if(maybe_a_space) {
						char *pbuf=strchr(buf,' ');
						if(NULL != pbuf) *pbuf='\0';
					}
					trim(buf);
					if(strlen(NONULL(buf))>1) {
						paramlist_set(p->params, "%USERNAME%",buf);
						p->mailuser=paramlist_get(p->params,"%USERNAME%");
						do_log(LOG_DEBUG,"SMTP USER set to: %s",p->mailuser);
					} else if(strlen(NONULL(p->mailuser))) {
						do_log(LOG_DEBUG,"SMTP USER was NULL. Currently set to: %s", p->mailuser);
					} else {
						do_log(LOG_DEBUG,"SMTP USER remains not set");
					}
				}
				if(p->checksmtp) smtpcmd++;
			} else if(!strncasecmp(p->clientbuf->line,"rcpt to:", 8) && p->cksmtp) {
				if(p->checksmtp) smtpcmd++;
			} else if(!strncasecmp(p->clientbuf->line,"uidl", 4)) {
				uidl = 1;
			} else if(!strncasecmp(p->clientbuf->line,"data", 4) && p->cksmtp && p->checksmtp) {
				/* SMTP message being submitted */
				if(config->scannerenabled) {
					p->ismail=1;
					trapdata=1; /* do not send "DATA" command to server */
					do_log(LOG_DEBUG,"intercepted DATA command. smtpcmd=%i",smtpcmd);
					p->clientbuf->linelen=GETLINE_LINE_NULL;
					// been through once? If so, we need to clean up!
					if(!smtprstlb) {
						smtprstlb=1;
					} else {
						postdata=0;
					}
				}
				p->mailcount++;
			} else {
				p->ismail=0;
			}
			if(((maybe_a_space = !strncasecmp(p->clientbuf->line,"apop", 4)) || !strncasecmp(p->clientbuf->line,"user", 4)) && !p->cksmtp) {
				len=p->clientbuf->linelen -5;
				if( len >= sizeof(buf)) len=sizeof(buf)-1;
				if(len>0) {
					memcpy(buf, (char *)(p->clientbuf->line)+5, len );
					buf[len]='\0';
					/* we don't want a space (surely with "apop") strtok_r is another choice. */
					if(maybe_a_space) {
						char *pbuf=strchr(buf,' ');
						if(NULL != pbuf) *pbuf='\0';
					}
					trim(buf);
					if(strlen(NONULL(buf))>1) {
						paramlist_set(p->params, "%USERNAME%",buf);
						p->mailuser=paramlist_get(p->params,"%USERNAME%");
						do_log(LOG_DEBUG,"USER set to: %s",p->mailuser);
					} else if(strlen(NONULL(p->mailuser))) {
						do_log(LOG_DEBUG,"USER was NULL. Currently set to: %s", p->mailuser);
					} else {
						do_log(LOG_DEBUG,"USER remains not set");
					}
				} else {
					if(strlen(NONULL(paramlist_get(p->params, "%USERNAME%")))) paramlist_set(p->params, "%USERNAME%", "unknown");
				}
				checklist(p);
			}
			/* write clientbuf to server_fd */
			if(p->clientbuf->linelen>=0) {
				ret=writeline(p->server_fd, WRITELINE_LEADING_RN, p->clientbuf->line);
				if(ret) {
					do_log(LOG_CRIT, "ERR: Can't send to server!");
					return 1;
				}
			}
			p->clientbuf->linelen=GETLINE_LINE_NULL;
		}// Not processing message
		if(p->cksmtp && trapdata && !smtpcmd && !postdata) {
			/* tell the client we will accept their smtp traffic. */
			/**
				It seems RFC2821 says we can reset (abort) a submission
				by sending RSET after a DATA, EOM event like such:
				...
				DATA
				...
				.
				RSET
				but in the real world, this does not seem to work.
				So, we are going to let the server hang while we are
				processing this submission. Otherwise, the smtp server will
				send a partial message to the recipient in the event we want
				to abort the sending of the message. If we find that we do not
				wish to send the messge, we can then cleanly abort it.

				This assumes the actual server would have accepted the data
				and in my eyes, is not a clean solution.
			*/
			if(writeline(p->client_fd, WRITELINE_LEADING_RN,"354 "PROGRAM" "VERSION" accepting data.")) {
				do_log(LOG_CRIT, "ERR: Can't send 354 to client!");
				return 1;
			} else {
				do_log(LOG_DEBUG, "Sent 354 "PROGRAM" "VERSION" accepting data.");
				postdata=1;
			}
		}
		if(p->serverbuf->linelen>=0 || trapdata && !smtpcmd) {
			if((p->ismail==1 && !p->cksmtp) || (p->ismail==1 && p->cksmtp && !smtpcmd)) {
				/* scan for answer */
				if(!strncasecmp(p->serverbuf->line,"+ok", 3) || trapdata || p->action==ACT_IMAP) {
					imapinit=0;
					if(p->action==ACT_IMAP) {
						tmp=strndup(p->serverbuf->line,p->serverbuf->linelen);
						tmp2=strsep(&tmp,spacer);
						tmp2=strsep(&tmp,spacer);
						if((!strncasecmp(p->serverbuf->line,"*",1)) && (atoi(tmp2)==inr)) {
							// here comes the requested message(part)
							do_log(LOG_DEBUG, "Starting trap of IMAP message...");
							imapinit=1;
						}
					}
					/* Set timer now because we might have parsed alot of message numbers */
					if(!imapinit) p->now = time(NULL); // Can't see how we can use this for IMAP right now as IMAP hangs onto the socket.
					if(imapinit || !p->imap) {
						/* generate unique filename */
						do_log(LOG_DEBUG,"Generating mailfile...");
						len=strlen(config->virusdir)+14;
						snprintf(p->mailfile, len, "%sp3scan.XXXXXX", config->virusdir);
						snprintf(p->mailhdr, len, "%sp3shdr.XXXXXX", config->virusdir);
						if((scanfd=mkstemp(p->mailfile)) < 0 ) {
							p->ismail=0;
							context_uninit(p);
							// Emergency: proxy: Can't open mailfile
							do_log(LOG_EMERG,"ERR: Critical error opening file '%s', Program aborted.", p->mailfile);
							/* Should not reach here as we are dead */
						} else {
							do_log(LOG_DEBUG,"Created %s", p->mailfile);
							p->filename=right(p->mailfile,13);
							if(!p->imap) {
								if(( p->mailhdr_fd=mkstemp(p->mailhdr)) < 0 ) {
									p->ismail=0;
									context_uninit(p);
									// Emergency: proxy: Can't open header file
									do_log(LOG_EMERG,"ERR: Critical error opening file '%s', Program aborted.", p->mailhdr);
									/* Should not reach here as we are dead */
								}
							}
							p->ismail=2;
							p->header_exists=0;
							p->fakehdrdone=0;
							p->notified=0;
							p->gobogus=0;
							p->boguspos=0;
							p->hdrdate=0;
							p->hdrfrom=0;
							p->hdrto=0;
							p->errmsg=0;
							p->noscan=0;
							p->nosend=0;
							if(!p->cksmtp) {
								p->serverbuf->linelen=GETLINE_LINE_NULL; /* don't send response */
								if(config->broken && !p->imap) p->hdrbuf=linebuf_init(16384);
							}
						}
					}
				} else {
					do_log(LOG_DEBUG, "ismail=1, but we haven't got '+ok' so... setting p->ismail=0");
					p->ismail=0;
				}
			} else if(p->ismail>1) {
				/* that means we are reading mail into file */
				p->nowrite=0;
				if(p->action == ACT_IMAP) {
					// <itag> OK UID FETCH completed
					tmp=strndup(p->serverbuf->line,p->serverbuf->linelen);
					tmp2=strsep(&tmp,spacer);
					tmp3=strsep(&tmp,spacer);
					if((!strcasecmp(tmp2,itag)) && (!strcasecmp(tmp3,"OK"))) {
						p->nowrite=1;
						do_log(LOG_DEBUG,"Set nowrite... IMAP message complete.");
					}
				} else if(((p->cksmtp && p->clientbuf->linelen == 0 && !p->header_exists)) || ((!p->cksmtp && p->serverbuf->linelen == 0 && !p->header_exists))) {
					if(p->cksmtp) {
						writeline(scanfd, WRITELINE_LEADING_N,"X-SMTP-SCANNER: TraceNetwork/Mybox Internet Security");
						writeline(p->mailhdr_fd, WRITELINE_LEADING_N,"X-SMTP-SCANNER: TraceNetwork/Mybox Internet Security");
					} else if(p->imap) {
						writeline(scanfd, WRITELINE_LEADING_N,"X-IMAP-SCANNER: TraceNetwork/Mybox Internet Security");
						writeline(p->mailhdr_fd, WRITELINE_LEADING_N,"X-IMAP-SCANNER: TraceNetwork/Mybox Internet Security");
					} else {
						writeline(scanfd, WRITELINE_LEADING_N,"X-POP3-SCANNER: TraceNetwork/Mybox Internet Security");
						writeline(p->mailhdr_fd, WRITELINE_LEADING_N,"X-POP3-SCANNER: TraceNetwork/Mybox Internet Security");
					}
#ifdef CLAMAV
					if(config->clamav) {
						version=strdup(cl_retver());
						dbdir = cl_retdbdir();
						if(!(path = malloc(strlen(dbdir) + 22))) do_log(LOG_CRIT,"ERR: Could not get database path.");
						// Check for updated info first... otherwise, we might get the originally installed database.
						sprintf(path, "%s/daily.inc/daily.info", dbdir);
						if(stat(path, &foo) == -1) {
							sprintf(path, "%s/daily.cvd", dbdir);
						}
						if((daily = cl_cvdhead(path))) {
							time_t t = (time_t) daily->stime;
							sprintf(buf,"ClamAV %s/%d/%s", version, daily->version, ctime(&t));
							writeline_format(scanfd, WRITELINE_LEADING_N,"X-AV: %s",buf);
							writeline_format(p->mailhdr_fd, WRITELINE_LEADING_N,"X-AV: %s", buf);
						}
						free(path);
					}
#endif
					p->header_exists=1;
					/*  This is the first response the client gets after "RETR/TOP", so start
						countdown timer now...
					*/
					if(p->ismail < 3) {
						do_log(LOG_DEBUG,"Closing header buffer.");
						closehdrfile(p);
					} else if(!p->cksmtp && !p->imap) do_log(LOG_DEBUG,"notified=%i",p->notified);
				}
				if(p->cksmtp) {
					if(!strncasecmp(p->clientbuf->line,"Date:",5) && p->ismail < 3) p->hdrdate=1;
					if(!strncasecmp(p->clientbuf->line,"From:",5) && p->ismail < 3) p->hdrfrom=1;
					if(!strncasecmp(p->clientbuf->line,"To:",3) && p->ismail < 3) p->hdrto=1;
					if(strstr(p->clientbuf->line,"MIME") || strstr(p->clientbuf->line,"Content-Type") || !strncasecmp(p->clientbuf->line,"Subject:",8)) {
						if(p->ismail < 3) {
							do_log(LOG_DEBUG,"Caught MIME/Subj line, closing header buffer.");
							closehdrfile(p);
						}
					}
					if(clientret==GETLINE_TOO_LONG) {
						if(p->clientbuf->linelen >=0) {
							writeline(scanfd, WRITELINE_LEADING_NONE, p->clientbuf->line);
							if(p->ismail < 3) writeline(p->mailhdr_fd, WRITELINE_LEADING_NONE, p->clientbuf->line);
						}
					} else {
						if(p->clientbuf->linelen >=0) {
							writeline(scanfd, WRITELINE_LEADING_N, p->clientbuf->line);
							if(p->ismail < 3) writeline(p->mailhdr_fd, WRITELINE_LEADING_N, p->clientbuf->line);
						}
					}
				} else {
					if(!strncasecmp(p->serverbuf->line,"Date:",5) && p->ismail < 3) p->hdrdate=1;
					if(!strncasecmp(p->serverbuf->line,"From:",5) && p->ismail < 3) {
						p->hdrfrom=1;
						if(p->blacklist) {
							if(p->action==ACT_IMAP) do_log(LOG_DEBUG,"Checking blacklist");
							blacklist_fd=r_open2(p->listfileb, O_RDONLY);
							listbuf=linebuf_init(256);
							while((ret=getlinep3(blacklist_fd, listbuf))>=0) {
								if(strlen(NONULL(listbuf->line))) {
									trim(listbuf->line);
									if(strstr(p->serverbuf->line,listbuf->line)) {
										p->nosend=1;
										do_log(LOG_DEBUG,"Found %s in %s!", listbuf->line, p->listfileb);
									} else {
										if(str_match(listbuf->line,p->serverbuf->line)) {
											p->nosend=1;
											do_log(LOG_DEBUG,"Found %s in %s!", listbuf->line, p->listfileb);
										}
									} // Not in list
								} // Blank line
							}
							linebuf_uninit(listbuf);
							r_close(blacklist_fd);
						}
						if(p->whitelist && !p->nosend) {
							if(p->action==ACT_IMAP) do_log(LOG_DEBUG,"Checking whitelist");
							whitelist_fd=r_open2(p->listfilew, O_RDONLY);
							listbuf=linebuf_init(256);
							while((ret=getlinep3(whitelist_fd, listbuf))>=0) {
								if(strlen(NONULL(listbuf->line))) {
									trim(listbuf->line);
									if(strstr(p->serverbuf->line,listbuf->line)) {
										p->noscan=1;
										do_log(LOG_DEBUG,"Found %s in %s.", listbuf->line, p->listfilew);
									} else {
										if(str_match(listbuf->line,p->serverbuf->line)) {
											p->noscan=1;
											do_log(LOG_DEBUG,"Found %s in %s.", listbuf->line, p->listfilew);
										}
									}
								}
							}
							linebuf_uninit(listbuf);
							r_close(whitelist_fd);
						}
					}
					if(!strncasecmp(p->serverbuf->line,"To:",3) && p->ismail < 3) p->hdrto=1;
					if(strstr(p->serverbuf->line,"MIME") || strstr(p->serverbuf->line,"Content-Type") || !strncasecmp(p->serverbuf->line,"Subject:",8) && (!p->imap)) {
						if(((config->scanner->name="bash")) && !strncasecmp(p->serverbuf->line,"Subject:",8)) {
							p->serverbuf->line=strreplace(p->serverbuf->line,"'"," ");
							if(p->serverbuf->line==NULL) do_log(LOG_NOTICE,"ERR: No memory (strrplace)");
						}
						if(p->ismail < 3 && !p->imap) {
							do_log(LOG_DEBUG,"Caught MIME/Subj line, closing header buffer.");
							closehdrfile(p);
						}
					}
					if(!p->nowrite) {
						if(serverret==GETLINE_TOO_LONG) {
							writeline(scanfd, WRITELINE_LEADING_NONE, p->serverbuf->line);
							if(p->ismail < 3 && !p->imap) writeline(p->mailhdr_fd, WRITELINE_LEADING_NONE, p->serverbuf->line);
						} else {
							if(config->noeom) {
								if((strncmp(p->serverbuf->line,".",1 ) == 0 && p->serverbuf->linelen == 1)) {
									do_log(LOG_DEBUG, "Not writing '.'");
								} else if((strncmp(p->serverbuf->line,".\r",2) == 0 && p->serverbuf->linelen == 2)) {
									do_log(LOG_DEBUG, "Not writing '.<r>'");
								} else {
									writeline(scanfd, WRITELINE_LEADING_N, p->serverbuf->line);
								}
							} else {
								writeline(scanfd, WRITELINE_LEADING_N, p->serverbuf->line);
							}
							if(p->ismail < 3 && !p->imap) {
								writeline(p->mailhdr_fd, WRITELINE_LEADING_N, p->serverbuf->line);
							}
						}
						if(p->imap) do_log(LOG_DEBUG,"Wrote %s", p->serverbuf->line);
					}
					/* check if isp already marked as spam so we don't tie up anti-spam resources (Read as "SLOW Perl SpamAssassin!" :)
					For example cox.net marks spam as "-- Spam --".
					*/
					if(config->ispspam != NULL && 
						(strstr(p->serverbuf->line,config->ispspam)!=NULL 
						|| strstr(p->serverbuf->line,"[SPAM]")!=NULL
						|| strstr(p->serverbuf->line,"[ SPAM ]")!=NULL
						|| strstr(p->serverbuf->line,"*** SPAM ***")!=NULL)) config->ispam=1;
						/* Here is where we start feeding the client part of our header buffer until the message has been processed */
					if(!p->imap) {
						error=checktimeout(p);
						if(error < 0) do_log(LOG_CRIT,"ERR: Writing to client!");
					}
				}
				if(p->action==ACT_IMAP) {
					do_log(LOG_DEBUG,"Inside p->action==ACT_IMAP");
					if(p->nowrite) {
						// imap message complete. close the file buffer and scan...
						error=0;
						r_close(scanfd);
						do_log(LOG_DEBUG, "got '%s %s ...', mail is complete.",tmp2,tmp3);
						p->ismail=4;
						/* initialize the scanner before scanning the first mail but only if scanning is enabled */
						if(config->scannerenabled && p->scannerinit == SCANNER_INIT_NO) {
							if(config->scanner->init2) {
								if(config->scanner->init2(p)!=0) {
									context_uninit(p);
									// Emergency: proxy: Can't initialize scanner
									do_log(LOG_EMERG, "ERR: Can't initialize scanner!");
									/* Dead now. Configuration error! */
									p->scannerinit=SCANNER_INIT_ERR;
								} else p->scannerinit=SCANNER_INIT_OK;
							} else p->scannerinit=SCANNER_INIT_NULL;
						}
						set_maildateparam(p->params);
						set_paramsfrommailheader(p->mailfile, p->params,p);
						scannerret=SCANNER_RET_OK;
						snprintf(p->maildir, 4090, "%s.dir", p->mailfile);
						if(p->scannerinit > 0) scannerret=scan_mailfile(p);
						if(scannerret==SCANNER_RET_VIRUS) {
							/* virus */
							if(p->virinfo) trim(p->virinfo);
							paramlist_set(p->params, "%VIRUSNAME%", NONULL(p->virinfo));
							paramlist_set(p->params, "%MAILFILE%", p->mailfile);
							if(config->delit) {
								paramlist_set(p->params, "%P3SCANID%", config->notify);
							} else {
								paramlist_set(p->params, "%P3SCANID%", p->filename);
							}
							if(paramlist_get(p->params,"%MAILTO%")==NULL) {
								do_log(LOG_WARNING, "%s from %s:%s to %s:%s from %s user: %s virus: %s file: %s",
								paramlist_get(p->params,"%PROTOCOL%"),
								paramlist_get(p->params,"%CLIENTIP%"), paramlist_get(p->params,"%CLIENTPORT%"),
								paramlist_get(p->params,"%SERVERIP%"), paramlist_get(p->params,"%SERVERPORT%"),
								paramlist_get(p->params,"%MAILFROM%"),
								paramlist_get(p->params,"%USERNAME%"), paramlist_get(p->params,"%VIRUSNAME%"),
								paramlist_get(p->params,"%P3SCANID%"));
							} else {
								do_log(LOG_WARNING, "%s from %s:%s to %s:%s from %s to %s user: %s virus: %s file: %s",
								paramlist_get(p->params,"%PROTOCOL%"),
								paramlist_get(p->params,"%CLIENTIP%"), paramlist_get(p->params,"%CLIENTPORT%"),
								paramlist_get(p->params,"%SERVERIP%"), paramlist_get(p->params,"%SERVERPORT%"),
								paramlist_get(p->params,"%MAILFROM%"), paramlist_get(p->params,"%MAILTO%"),
								paramlist_get(p->params,"%USERNAME%"), paramlist_get(p->params,"%VIRUSNAME%"),
								paramlist_get(p->params,"%P3SCANID%"));
							}
							/*
							Example:
							C: A222 COPY 1:2 owatagusiam
							S: * NO Disk is 98% full, please delete unnecessary data
							S: A222 OK COPY completed
							*/
							do_log(LOG_DEBUG," * NO Message %i contains a virus (%s).", inr, paramlist_get(p->params, "%VIRUSNAME%"));
							writeline_format(p->client_fd, WRITELINE_LEADING_RN," * NO Message %i contains a virus (%s).", inr, paramlist_get(p->params, "%VIRUSNAME%"));
							do_log(LOG_DEBUG," * NO Message %i contains a virus (%s).", inr, paramlist_get(p->params, "%VIRUSNAME%"));
							writeline_format(p->client_fd, WRITELINE_LEADING_RN," * NO Message %i contains a virus (%s).", inr, paramlist_get(p->params, "%VIRUSNAME%"));
							do_log(LOG_DEBUG,"%s NO FETCH failed. Message %i contains a virus (%s).", itag, inr, paramlist_get(p->params, "%VIRUSNAME%"));
							writeline_format(p->client_fd, WRITELINE_LEADING_RN,"%s NO FETCH failed. Message %i contains a virus (%s).", itag, inr, paramlist_get(p->params, "%VIRUSNAME%"));
							unset_paramsfrommailheader(p->params);
							if(p->clientbuf->linelen) do_log(LOG_DEBUG,"IMAP: clientbuf: %s",p->clientbuf->line);
							p->clientbuf->linelen=GETLINE_LINE_NULL;
							if(p->serverbuf->linelen) do_log(LOG_DEBUG,"IMAP: serverbuf: %s",p->serverbuf->line);
							p->serverbuf->linelen=GETLINE_LINE_NULL;
						} /* virus */
						/* see if there was a critical error */
						if(scannerret==SCANNER_RET_CRIT) {
							if(!p->errmsg) do_log(LOG_CRIT,"ERR: Writing to client!");
							/* exact error already reported so kill the child. This should get the sysadmins attention. */
							p->ismail=0;
							return 1;
						}
						if(scannerret!=SCANNER_RET_VIRUS) { /* send mail if no virus */
							if(scannerret==SCANNER_RET_ERR) {
								//do_log(LOG_ALERT, "ERR: We can't say if it is a virus! So we have to give the client the mail! You should check your configuration/system");
								//context_uninit(p);
								// Emergency: proxy: Scanner returned unexpected error code.
								do_log(LOG_EMERG, "ERR: Scanner returned unexpected error code.");
								do_log(LOG_DEBUG, "Scanning aborted, sending mail now.");
								/* We are dead now. Don't let virus mails pass */
							} else {
								/* no virus  / error / scanning disabled */
								do_log(LOG_DEBUG, "Scanning done, sending mail now.");
							}
							p->ismail=5;
							if((len=send_mailfile(p->mailfile, p->client_fd, p))<0) {
								do_log(LOG_CRIT, "ERR: Can't send mail! We have to quit now!");
								p->ismail=0;
								return 1;
							}
							do_log(LOG_DEBUG, "Sending done.");
							p->ismail=0;
							p->bytecount+=len;
							p->serverbuf->linelen=GETLINE_LINE_NULL;
							linebuf_zero(p->serverbuf);
							trapdata=0;
							unlink(p->mailfile); /* we do not unlink virusmails, so only here */
							do_log(LOG_DEBUG, "Mail action complete");
						} /* send mail */
					}
					// End IMAP
				} else if((p->clientbuf->linelen==1 && p->clientbuf->line[0]=='.') || (p->serverbuf->linelen==1 && p->serverbuf->line[0]=='.')) {
					/* mail is complete */
					error=0;
					r_close(scanfd);
					do_log(LOG_DEBUG, "got '.\\r\\n', mail is complete.");
					if(p->ismail==2) closehdrfile(p);
					p->ismail=4;
					/* initialize the scanner before scanning the first mail but only if scanning is enabled */
					if(config->scannerenabled && p->scannerinit == SCANNER_INIT_NO) {
						if(config->scanner->init2) {
							if(config->scanner->init2(p)!=0) {
								context_uninit(p);
								// Emergency: proxy: Can't initialize scanner
								do_log(LOG_EMERG, "ERR: Can't initialize scanner!");
								/* Dead now. Configuration error! */
								p->scannerinit=SCANNER_INIT_ERR;
							} else p->scannerinit=SCANNER_INIT_OK;
						} else p->scannerinit=SCANNER_INIT_NULL;
					}
					set_maildateparam(p->params);
					set_paramsfrommailheader(p->mailfile, p->params,p);
					/* Scan the file now */
					scannerret=SCANNER_RET_OK;
					snprintf(p->maildir, 4090, "%s.dir", p->mailfile);
					if(p->scannerinit > 0) {
						if(!p->noscan && !p->nosend) {
							scannerret=scan_mailfile(p);
						} else {
							scannerret=SCANNER_RET_OK;
						}
						if(!p->noscan) {
							if(scannerret==SCANNER_RET_VIRUS || p->nosend) {
								/* virus or blacklist*/
								scannerret=SCANNER_RET_VIRUS;
								if(p->nosend) {
									p->virinfo=FOUNDIBL;
								} else {
									if(p->virinfo) trim(p->virinfo);
								}
								paramlist_set(p->params, "%VIRUSNAME%", NONULL(p->virinfo));
								paramlist_set(p->params, "%MAILFILE%", p->mailfile);
								if(config->delit) {
									paramlist_set(p->params, "%P3SCANID%", config->notify);
								} else {
									paramlist_set(p->params, "%P3SCANID%", p->filename);
								}
								if(!p->nosend) do_log(LOG_WARNING, "%s from %s:%s to %s:%s from %s to %s user: %s virus: %s file: %s",
													paramlist_get(p->params,"%PROTOCOL%"),
													paramlist_get(p->params,"%CLIENTIP%"), paramlist_get(p->params,"%CLIENTPORT%"),
													paramlist_get(p->params,"%SERVERIP%"), paramlist_get(p->params,"%SERVERPORT%"),
													paramlist_get(p->params,"%MAILFROM%"), paramlist_get(p->params,"%MAILTO%"),
													paramlist_get(p->params,"%USERNAME%"), paramlist_get(p->params,"%VIRUSNAME%"),
													paramlist_get(p->params,"%P3SCANID%"));
								if(do_virusaction(p)!=0) {
									if(p->cksmtp) {
										/* Try cleaning it up again */
										do_log(LOG_CRIT,"ERR: Virusaction failed. Sending 554 and reseting smtp data sent.");
										writeline_format(p->client_fd, WRITELINE_LEADING_RN, "554 %s '%s'",config->smtprset,paramlist_get(p->params, "%VIRUSNAME%"));
										do_log(LOG_DEBUG,"Sending RSET to real smtp server.");
										writeline_format(p->server_fd, WRITELINE_LEADING_RN, "RSET");
										writeline_format(p->server_fd, WRITELINE_LEADING_RN, "QUIT");
									} else {
										do_log(LOG_CRIT,"ERR: Virusaction failed. Sending -ERR and closing pop3 connection.");
										writeline_format(p->client_fd, WRITELINE_LEADING_RN,"-ERR Message %i contains a virus (%s).", p->msgnum, paramlist_get(p->params, "%VIRUSNAME%"));
									}
									p->ismail=0;
									return 1;
								}
								unset_paramsfrommailheader(p->params);
								p->clientbuf->linelen=GETLINE_LINE_NULL;
								p->serverbuf->linelen=GETLINE_LINE_NULL;
								if(config->delit) unlink(p->mailfile);
							} /* virus */
							/* see if there was a critical error */
							if(scannerret==SCANNER_RET_CRIT) {
								if(!p->errmsg) do_log(LOG_CRIT,"ERR: Writing to client!");
									/* exact error already reported so kill the child. This should get the sysadmins attention. */
									p->ismail=0;
									return 1;
							}
						} /* !p->noscan */
					} else {
						scannerret=SCANNER_RET_ERR; /* ! error */
					}

					if(scannerret!=SCANNER_RET_VIRUS) { /* send mail if no virus */
						if(scannerret==SCANNER_RET_ERR) {
							//do_log(LOG_ALERT, "ERR: We can't say if it is a virus! So we have to give the client the mail! You should check your configuration/system");
							//context_uninit(p);
							// Emergency: proxy: Scanner returned unexpected error code.
							do_log(LOG_EMERG, "ERR: Scanner returned unexpected error code.");
							do_log(LOG_DEBUG, "Scanning aborted, sending mail now.");
							/* We are dead now. Don't let virus mails pass */
						} else {
							/* no virus  / error / scanning disabled */
							do_log(LOG_DEBUG, "Scanning done, sending mail now.");
						}
						p->ismail=5;
						if(p->cksmtp) {
							do_log(LOG_DEBUG, "Sending DATA to server.");
							if(writeline(p->server_fd, WRITELINE_LEADING_RN, "DATA")) {
								do_log(LOG_CRIT, "ERR: Can't send DATA to server!");
								return 1;
							}
							p->serverbuf->linelen=GETLINE_LINE_NULL; /* assume 354 from server */
							do_log(LOG_DEBUG, "Sending smtp message to server.");
							if((len=send_mailfile(p->mailfile, p->server_fd, p))<0) {
								do_log(LOG_CRIT, "ERR: Can't submit mail! We have to quit now!");
								p->ismail=0;
								return 1;
							}
						} else {
							if((len=send_mailfile(p->mailfile, p->client_fd, p))<0) {
								do_log(LOG_CRIT, "ERR: Can't send mail! We have to quit now!");
								p->ismail=0;
								return 1;
							}
						}
						do_log(LOG_DEBUG, "Sending done.");
						p->ismail=0;
						p->bytecount+=len;
						p->serverbuf->linelen=GETLINE_LINE_NULL;
						linebuf_zero(p->serverbuf);
						trapdata=0;
						unlink(p->mailfile); /* we do not unlink virusmails, so only here */
					} /* send mail */
					r_close(p->mailhdr_fd);
					unlink(p->mailhdr);
					do_log(LOG_DEBUG, "Mail action complete");
				} /* mail complete */
				p->serverbuf->linelen=GETLINE_LINE_NULL; /* don't send to client */
			/* p->ismail>1 */
			} else if(trapcapa1 && !strncasecmp(p->serverbuf->line,"PIPELINING",10)) {
				p->serverbuf->linelen=GETLINE_LINE_NULL; /* don't send to client */
				trapcapa1=0;
				do_log(LOG_DEBUG, "Ignoring servers PIPELINING capability...");
			} else if(trapcapa3 && !strncasecmp(p->serverbuf->line,"STLS",4)) {
				p->serverbuf->linelen=GETLINE_LINE_NULL; /* don't send to client */
				trapcapa3=0;
				do_log(LOG_DEBUG, "Ignoring servers STLS capability...");
			} else if(p->cksmtp && !strncasecmp(p->serverbuf->line,"250-PIPELINING",14)) {
				p->serverbuf->linelen=GETLINE_LINE_NULL; /* don't send to client */
				do_log(LOG_DEBUG, "Ignoring SMTP servers PIPELINING capability...");
			}
		} /* server_buf_len >0 */
		/* we are not in mail-reading mode (ismail==0) */
		if( p->serverbuf->linelen>=0 ) {
			/* write server_buf to fd */
			if(smtpcmd && !strncasecmp(p->serverbuf->line,"250", 3)) smtpcmd--;
			if(!p->cksmtp || (p->cksmtp && strncasecmp(p->serverbuf->line,"354", 3))) {
				if(writeline(p->client_fd, WRITELINE_LEADING_RN, p->serverbuf->line)) {
					do_log(LOG_CRIT, "ERR: Can't send to client: %s", p->serverbuf->line);
					return 1;
				}
			} else {
				if(p->cksmtp) do_log(LOG_DEBUG,"Caught servers 354");
			}
			p->serverbuf->linelen=GETLINE_LINE_NULL;
			p->clientbuf->linelen=GETLINE_LINE_NULL;
		}
	} //end of while
	do_log(LOG_WARNING, "Connection timeout");
	do_log(LOG_DEBUG, "Child finished");
	return 0;
}

int do_sigchld_check(pid_t pid, int stat) {
	int termsig = WTERMSIG(stat);
	int signaled = WIFSIGNALED(stat);
	int stopped = WIFSTOPPED(stat);

	if(numprocs>0) numprocs--;
	if(termsig) {
		do_log(LOG_CRIT, "ERR: Attention: child with pid %i died with abnormal termsignal (%i)! This is probably a bug. Please report to the author (see 'Support' at http://p3scan.sourceforge.net). numprocs is now %i", pid, termsig, numprocs);
	} else if(signaled) {
		do_log(LOG_DEBUG, "waitpid: child %i died with a signal! %i, numprocs is now %i", pid, WTERMSIG(stat), numprocs);
	} else if(stopped) {
		do_log(LOG_DEBUG, "waitpid: child %i stopped! %i, numprocs is now %i", pid, WSTOPSIG(stat), numprocs);
	} else {
		do_log(LOG_DEBUG, "waitpid: child %i died with status %i, numprocs is now %i", pid, WEXITSTATUS(stat), numprocs);
	}
	return 1;
}

void do_sigchld(int signr) {
	pid_t pid;
	int stat;

	while((pid=r_waitpid(-1, &stat, WNOHANG)) > 0) do_sigchld_check(pid, stat);
 	if(clean && !numprocs) {
		do_log(LOG_NOTICE,"terminating due to child error.");
		raise(SIGTERM);
	}
}

void printversion(void) {
	printf("\n%s %s\n",PROGRAM,VERSION);
	printf("(C) 2003-2007 by Jack S. Lai, <laitcg at gmail.com> 12/05/2003\n");
	printf("     and by Folke Ashberg <folke at ashberg.de> pre-12/05/2003\n\n");
	printf("   Mybox Internet Security modification done by\n");
	printf("   Nawawi <nawawi@tracenetworkcorporation.com>\n");
	printf("   patch can be found at http://www.tracenetworkcorporation.com\n\n");
}

void usage() {
	printversion();
	printf(
		"Usage: " PROGRAM " [options]\n"
		"\n"
		"where options are:\n"
		"  -c, --configfile=FILE   Specify a configfile. Default is\n"
		"                          " CONFIGFILE "\n"
		"                          Can be configured with \n"
		"                          ./configure option '--sysconfdir='\n"
		"  -d, --debug             Turn on debugging.\n"
		"                          See " CONFIGFILE ":debug\n"
		"                          for recommended debug procedure.\n"
		"  -h, --help              Prints this text\n"
		"  -p, --password          Display passwords in debug mode.\n"
		"                          default: Do not display passwords.\n"
		"  -s, --settings          Display current settings from\n"
		"                          " CONFIGFILE "\n"
		"                          NOTE: You should send a\n"
		"                          `kill -HUP <p3scan pid>` to a running\n"
		"                          p3scan after making changes to\n"
		"                          " CONFIGFILE "\n"
		"  -v, --version           Prints version information\n\n"
		"Please see " CONFIGFILE " for numerous other options.\n"
	);
}

void parseargs(int c, char **v) {
	int option, option_index = 0, error=0;
	int pidfd;
	long i, ii;
	char *rest;
	struct servent *port;
	struct option long_options_cf[] = {
		{ "spamdelete",     no_argument,         NULL, 'a' },
		{ "altvnmsg",       no_argument,         NULL, 'b' },
		{ "freespace",      required_argument,   NULL, 'c' },
		{ "broken",         no_argument,         NULL, 'd' },
		{ "viruscode",      required_argument,   NULL, 'e' },
		{ "smtpsize",       required_argument,   NULL, 'f' },
		{ "debug",          no_argument,         NULL, 'g' },
		{ "extra",          required_argument,   NULL, 'h' },
		{ "smtpport",       required_argument,   NULL, 'i' },
		{ "virusregexp",    required_argument,   NULL, 'k' },
		{ "goodcode",       required_argument,   NULL, 'l' },
		{ "ip",             required_argument,   NULL, 'm' },
		{ "targetip",       required_argument,   NULL, 'n' },
		{ "delete",         no_argument,         NULL, 'o' },
		{ "skipsize",       required_argument,   NULL, 'p' },
		{ "checkspam",      required_argument,   NULL, 'q' },
		{ "pidfile",        required_argument,   NULL, 'r' },
		{ "maxchilds",      required_argument,   NULL, 't' },
		{ "ispspam",        required_argument,   NULL, 'u' },
		{ "notifydir",      required_argument,   NULL, 'v' },
		{ "delnotify",      required_argument,   NULL, 'w' },
		{ "timeout",        required_argument,   NULL, 'y' },
		{ "port",           required_argument,   NULL, 'z' },
		{ "targetport",     required_argument,   NULL, 'A' },
		{ "quiet",          no_argument,         NULL, 'B' },
		{ "virusdir",       required_argument,   NULL, 'C' },
		{ "smtprset",       required_argument,   NULL, 'D' },
		{ "scanner",        required_argument,   NULL, 'E' },
		{ "subject",        required_argument,   NULL, 'F' },
		{ "template",       required_argument,   NULL, 'G' },
		{ "scannertype",    required_argument,   NULL, 'H' },
		{ "user",           required_argument,   NULL, 'I' },
		{ "spamsubject",    required_argument,   NULL, 'J' },
		{ "extrapgm",       required_argument,   NULL, 'K' },
		{ "debug-memory",   no_argument,         NULL, 'L' },
		{ "debug-message",  no_argument,         NULL, 'M' },
		{ "debug-scanning", no_argument,         NULL, 'N' },
		{ "debug-smtp",     no_argument,         NULL, 'O' },
		{ "logopt",         required_argument,   NULL, 'P' },
		{ "logfac",         required_argument,   NULL, 'Q' },
		{ "cleankill",      no_argument,         NULL, 'T' },
		{ "imapport",       required_argument,   NULL, 'V' },
		{ "blackshort",     no_argument,         NULL, 'W' },
		{ "blacksubj",      required_argument,   NULL, 'X' },
		{ "debug-imap",     no_argument,         NULL, 'Y' },
		{ "nospampipe",     no_argument,         NULL, 'Z' },
		{ "noeom",          no_argument,         NULL, '2' },
		{ NULL,             no_argument,         NULL, 0 }
	};
	char getoptparam[] = "a:bc:de:f:gh:i:k:l:m:n:op:q:r:t:u:v:w:y:z:A:BC:D:E:F:G:H:I:J:K:L:MNOPQ:T:V:WX:YZ2";
	void switchoption(char opt, char *arg, char *optstr) {
		char *next_tok;
		char *where="p3scan.conf";
		switch(opt) {
			case 'a': // spamdelete
				config->spamdelete=1;
			break;
			case 'b': // altvnmsg
				config->altvnmsg=1;
			break;
			case 'c': // freespace
				i=strtol(arg, &rest, 10);
				config->freespace=(int)i;
			break;
			case 'd': // broken
				config->broken=1;
			break;
			case 'e': // viruscode
				ii = 0;
				next_tok = strtok(arg, " \t,");
				if(next_tok) {
					do {
						if(ii < MAX_VIRUS_CODES) {
							i=strtol(next_tok, &rest, 10);
							if((rest && strlen(rest)>0) || i<1 || i>256) {
								fprintf(stderr, "%s --viruscode has be a list of numbers (%s)\n", where, rest);
								error=1;
							} else {
								config->viruscode[ii]=(int)i;
							}
							ii++;
						}
					} while((next_tok = strtok(NULL, " \t,")) || (ii >= MAX_VIRUS_CODES));
				}
				config->viruscode[ii] = -1;
				if(ii == 0) {
					fprintf(stderr, "%s --viruscode has be a list of numbers (%s)\n", where, rest);
					error=1;
				}
			break;
			case 'f': // smtpsize
				i=strtol(arg, &rest, 10);
				config->smtpsize=(int)i;
			break;
			case 'g': // debug
				config->debug=1;
			break;
			case 'h': // extra
				config->extra=arg;
			break;
			case 'i': // smtpport
				i=strtol(arg, &rest, 10);
				if(rest && strlen(rest)>0) {
					if(i>0) {
						fprintf(stderr, "%s %s isn't a valid port\n", where, arg);
						error=1;
					} else {
						if((port=getservbyname(arg, "tcp"))!=NULL) {
							config->smtpport=ntohs(port->s_port);
						} else {
							fprintf(stderr, "Port lookup for '%s/tcp' failed! Check /etc/services\n", arg);
							error=1;
						}
					}
				} else {
					if(i>0) {
						config->smtpport=i;
					} else {
						fprintf(stderr, "%s Incorrect smtpport portnumber\n", where);
						error=1;
					}
				}
			break;
			case 'k': // virusregexp
 				config->virusregexp = arg;
				i=strlen(arg);
				if(arg[i-2]=='/' && isdigit(arg[i-1])) {
					arg[i-2]='\0';
					config->virusregexpsub=arg[i-1]-'0';
				}
			break;
			case 'l': // goodcode
				ii = 0;
				next_tok = strtok(arg, " \t,");
				if(next_tok) {
					do {
						if(ii < MAX_VIRUS_CODES) {
              						i=strtol(next_tok, &rest, 10);
              						if( (rest && strlen(rest)>0) || i<1 || i>256) {
                						fprintf(stderr, "%s --good viruscode has be a list of numbers (%s)\n", where, rest);
                						error=1;
              						} else config->gvcode[ii]=(int)i;
             						ii++;
            					}
          				} while((next_tok = strtok(NULL, " \t,")) || (ii >= MAX_VIRUS_CODES));
        			}
        			config->gvcode[ii] = -1;
        			if(ii == 0) {
          				fprintf(stderr, "%s --good viruscode has be a list of numbers (%s)\n", where, rest);
          				error=1;
        			}
        		break;
      			case 'm': // ip
				// Emergency: config: invalid ip
        			if(!strcmp(arg, "0.0.0.0")) {
					config->addr.sin_addr.s_addr=htonl(INADDR_ANY);
        			} else if(!inet_aton(arg, &config->addr.sin_addr)) {
					do_log(LOG_EMERG,"ERR: %s isn't a valid IP Adress",arg);
				}
        		break;
      			case 'n': // targetip
				// Emergency: config: invalid ip (target ip)
        			if(!strcmp(arg, "0.0.0.0")) {
          				config->addr.sin_addr.s_addr=htonl(INADDR_ANY);
          				config->targetaddr.sin_addr.s_addr=htonl(INADDR_ANY);
          				config->target=1;
        			} else if(!inet_aton(arg, &config->targetaddr.sin_addr)) {
					do_log(LOG_EMERG,"ERR: %s isn't a valid IP Adress.",arg);
				}
        		break;
      			case 'o': // delete
        			config->delit=1;
        			break;
      			case 'p': // skipsize
        			i=strtol(arg, &rest, 10);
        			config->skipsize=(int)i;
        		break;
      			case 'q': // checkspam
        			config->checkspam=arg;
        		break;
      			case 'r': // PID File
				// Emergency: config: pid exists
        			config->pidfile=arg;
        			if((pidfd=r_open2(config->pidfile,O_RDONLY ))>=0) {
          				pidfd=r_close(pidfd);
          				do_log(LOG_EMERG, "ERR: %s exists! Aborting!",config->pidfile);
          				// Should not reach here. We are dead.
          				exit(EXIT_FAILURE);
        			}
        		break;
      			case 't': // maxchilds
        			i=strtol(arg, &rest, 10);
        			if((rest && strlen(rest)>0) || i<1 || i>9999) {
          				fprintf(stderr, "%s --maxchilds has to be > 1 < 10000: %s\n", where, arg);
          				error=1;
        			} else {
					config->maxchilds=(int)i;
				}
        		break;
      			case 'u': // ispspam
        			config->ispspam = arg;
        		break;
      			case 'v': // notifydir
        			config->notifydir=arg;
        		break;
      			case 'w': // delnotify
        			config->notify=arg;
        		break;
      			case 'y': // timeout
        			i=strtol(arg, &rest, 10);
        			if((rest && strlen(rest)>0) || i<1 || i>9999) {
          				fprintf(stderr, "%s --timeout has to be > 1  < 10000: %s\n", where, arg);
          				error=1;
        			} else {
					config->timeout=(int)i;
				}
        		break;
      			case 'z': // port
 				// Emergency: config: invalid port
        			i=strtol(arg, &rest, 10);
        			if(rest && strlen(rest)>0) {
          				if(i>0) {
						do_log(LOG_EMERG,"ERR: %s isn't a valid port",arg);
          				} else {
            					if((port=getservbyname(arg, "tcp"))!=NULL) {
							config->addr.sin_port=port->s_port;
            					} else {
							do_log(LOG_EMERG,"ERR: Port lookup for '%s/tcp' failed! Check /etc/services",arg);
						}
          				}
        			} else {
          				if(i>0) {
						config->addr.sin_port=htons((int)i);
          				} else {
						do_log(LOG_EMERG,"ERR: Incorrect POP3 portnumber");
					}
        			}
        		break;
      			case 'A': // targetport
				// Emergency: config: invalid port (target)
        			i=strtol(arg, &rest, 10);
        			if(rest && strlen(rest)>0) {
          				if(i>0) {
						do_log(LOG_EMERG,"ERR: %s isn't a valid port",arg);
          				} else {
            					if((port=getservbyname(arg, "tcp"))!=NULL) {
							config->targetaddr.sin_port=port->s_port;
            					} else {
							do_log(LOG_EMERG,"ERR: Port lookup for '%s/tcp' failed! Check /etc/services",arg);
						}
          				}
        			} else {
          				if(i>0) {
						config->targetaddr.sin_port=htons((int)i);
          				} else {
						do_log(LOG_EMERG,"ERR: Incorrect target portnumber");
					}
        			}
        			config->target=1;
        		break;
      			case 'B': //quiet
        			config->quiet=1;
        		break;
      			case 'C': // virusdir
        			config->virusdirbase=arg;
        			config->virusdir=config->virusdirbase;
        		break;
      			case 'D': // smtprset
       				config->smtprset=arg;
       			break;
      			case 'E': // scanner
        			config->virusscanner=arg;
        		break;
      			case 'F': // subject
        			config->subject = arg;
        		break;
      			case 'G': // template
        			config->virustemplate=arg;
        		break;
      			case 'H': // scannertype
        			// Patch by A.J. Weber
        			if(config->clamav) {
          				if(!strncasecmp(arg, "clamav", 6)) {
            					//use the internal clamav libs.
            					break;
          				}
          			} else {
            				fprintf(stderr, "Attempting to override internal clamav libraries with scannertype: %s\n", arg);
            				config->clamav = 0;
        			}
        			i=0;
        			do_log(LOG_DEBUG,"Checking scannerlist...");
        			//config->clamav=0;
       	 			while(scannerlist[i]) {
          				if(!strcasecmp(arg, scannerlist[i]->name)) {
            					config->scanner=scannerlist[i];
            					i=-1;
            					break;
          				}
          				i++;
        			}
				// Emergency: config: invalid scannertype
        			if(i!=-1 && !config->clamav) do_log(LOG_EMERG,"ERR: scannertype '%s' is not supported", arg);
        		break;
      			case 'J': // spamsubject
        			config->spamsubject = arg;
        		break;
      			case 'L': // debug-memory
        			config->debug_memory=1;
        		break;
      			case 'M': // debug-message
        			config->debug_message=1;
        		break;
      			case 'N': // debug-scanning
        			config->debug_scanning=1;
        		break;
      			case 'O': // debug-smtp
        			config->debug_smtp=1;
        		break;
      			case 'P': // logopt
				// Emergency: config: invalid logopt
        			i=strtol(arg, &rest, 10);
        			if(rest && strlen(rest)>0) {
          				if(i>0) do_log(LOG_EMERG,"ERR: %s isn't a valid log option",arg);
        			} else {
          				if(i>0) {
						config->p3logopt=(int)i;
          				} else {
						do_log(LOG_EMERG,"ERR: Incorrect log option: %s", arg);
					}
        			}
        		break;
      			case 'Q': // logfac
 				// Emergency: config: invalid logfac
        			i=strtol(arg, &rest, 10);
        			if(rest && strlen(rest)>0) {
          				if(i>0) do_log(LOG_EMERG,"ERR: %s isn't a valid log facility",arg);
        			} else {
          				if(i>0) {
						config->p3logopt=(int)i;
          				} else { 
						do_log(LOG_EMERG,"ERR: Incorrect log facility: %s", arg);
					}
        			}
        		break;
      			case 'T': // cleankill
        			config->cleankill=1;
        		break;
      			case 'V': // imapport
        			i=strtol(arg, &rest, 10);
        			if(rest && strlen(rest)>0) {
          				if(i>0) { /* 123abc */
            					fprintf(stderr, "%s %s isn't a valid port\n", where, arg);
            					error=1;
          				} else {
            					if((port=getservbyname(arg, "tcp"))!=NULL) {
							config->imaport=ntohs(port->s_port);
            					} else {
              						fprintf(stderr, "Port lookup for '%s/tcp' failed! Check /etc/services\n", arg);
              						error=1;
            					}
          				}
        			} else {
          				if(i>0) {
						config->imaport=i;
          				} else {
            					fprintf(stderr, "%s Incorrect IMAP portnumber\n", where);
            					error=1;
          				}
        			}
        		break;
      			case 'W': // blackshort
        			config->blackshort=1;
        		break;
      			case 'X': // blacksubj
        			config->blacksubj=arg;
        		break;
      			case 'Y': // debug-imap
        			config->debug_imap=1;
        		break;
      			case 'Z': // nospampipe
        			config->nospampipe=1;
        		break;
       			case '2': // noeom
        			config->noeom=1;
        		break;
     			default:
      				fprintf(stderr, "Option '%s' isn't known\n", optstr);
      				error=1;
    		}
	}// sub function switchoption
	opterr=0;
	optind=1;
	while(1) {
		option = getopt_long(c, v, getoptparam, long_options_cf, &option_index);
		if(option == EOF) break;
		switchoption(option, optarg, v[optind-1]);
	}
	if(optind < c) {
		error=1;
		while(optind < c) fprintf(stderr, "Unknown option '%s'\n", v[optind++]);
	}
}

Sigfunc *p3sigaction(int signo, Sigfunc *func) {
	struct sigaction act, oact;
	act.sa_handler = func;
	act.sa_flags = 0;
	if(signo != SIGALRM) act.sa_flags |= SA_RESTART;
	if((sigemptyset(&act.sa_mask) == -1) || (sigaction(signo, &act, &oact) == -1)) return(SIG_ERR);
	return(oact.sa_handler);
}

void display_settings() {
	do_log(LOG_DEBUG,"p3scan.conf: %s",config->configfile);
	do_log(LOG_DEBUG,"logopt: %i",config->p3logopt);
	do_log(LOG_DEBUG,"logfac: %i",config->p3logfac);
	if(config->debug || config->status) {
		if(config->debug) do_log(LOG_DEBUG,"debug: enabled");
		if(config->debug_imap) do_log(LOG_DEBUG,"debug-imap: enabled");
		if(config->debug_memory) do_log(LOG_DEBUG,"debug-memory: enabled");
		if(config->debug_message) do_log(LOG_DEBUG,"debug-message: enabled");
		if(config->debug_scanning) do_log(LOG_DEBUG,"debug-scanning: enabled");
		if(config->debug_smtp) do_log(LOG_DEBUG,"debug-smtp: enabled");
	} else {
		do_log(LOG_DEBUG,"debug: disabled");
	}
#ifdef CLAMAV
	if(!config->clamav) do_log(LOG_DEBUG,"Internal ClamAV: disabled");
#endif
	if(ntohs(config->addr.sin_addr.s_addr)) do_log(LOG_DEBUG,"ip: %i",ntohs(config->addr.sin_addr.s_addr));
	do_log(LOG_DEBUG,"maxchilds: %i",config->maxchilds);
	do_log(LOG_DEBUG,"port: %d",htons(config->addr.sin_port));
	if(config->quiet) do_log(LOG_DEBUG,"quiet: enabled");
	if(config->target) {
		if(ntohs(config->targetaddr.sin_addr.s_addr)) do_log(LOG_DEBUG,"targetip: %i",ntohs(config->targetaddr.sin_addr.s_addr));
		if(htons(config->targetaddr.sin_port)) do_log(LOG_DEBUG,"targetport: %d",htons(config->targetaddr.sin_port));
	}
#ifdef PCRE
	if(strlen(NONULL(config->virusregexp))) do_log(LOG_DEBUG,"virusregexp: %s",config->virusregexp);
#endif
	if(strlen(NONULL(config->pidfile))) do_log(LOG_DEBUG,"pidfile: %s",config->pidfile);
	if(strlen(NONULL(config->notifydir))) do_log(LOG_DEBUG,"notifydir: %s",config->notifydir);
	if(strlen(NONULL(config->virusdir))) do_log(LOG_DEBUG,"virusdir: %s",config->virusdir);
	if(config->delit) do_log(LOG_DEBUG,"delete: enabled");
	if(config->freespace) do_log(LOG_DEBUG,"freespace: %i",config->freespace);
	if(strlen(NONULL(config->virusscanner))) do_log(LOG_DEBUG,"scanner: %s",config->virusscanner);
	if(config->broken) do_log(LOG_DEBUG,"broken: enabled");
	if(strlen(NONULL(config->checkspam))) {
		do_log(LOG_DEBUG,"checkspam: %s",config->checkspam);
		if(config->spamdelete==1) {
			do_log(LOG_DEBUG,"Delete SPAM: enable");
		}
	}
	if(strlen(NONULL(config->virustemplate))) do_log(LOG_DEBUG,"template: %s",config->virustemplate);
	if(strlen(NONULL(config->subject))) do_log(LOG_DEBUG,"subject: %s",config->subject);
	if(strlen(NONULL(config->spamsubject))) do_log(LOG_DEBUG,"spamsubject: %s",config->spamsubject);
	if(strlen(NONULL(config->blacksubj))) do_log(LOG_DEBUG,"blacklist subject: %s",config->blacksubj);
	if(config->blackshort) do_log(LOG_DEBUG,"blackshort: enabled");
	if(strlen(NONULL(config->notify))) do_log(LOG_DEBUG,"notify: %s",config->notify);
	if(strlen(NONULL(config->extra))) do_log(LOG_DEBUG,"extra: %s",config->extra);
	do_log(LOG_DEBUG,"emailport: %d",config->smtpport);
	if(strlen(NONULL(config->smtprset))) do_log(LOG_DEBUG,"smtprset: %s",config->smtprset);
	if(config->smtpsize) do_log(LOG_DEBUG,"smtpsize: %d",config->smtpsize);
	if(config->password) do_log(LOG_DEBUG,"password: enabled");
	do_log(LOG_DEBUG,"imapport: %d",config->imaport);
	do_log(LOG_DEBUG,"timeout: %i",config->timeout);
	if(config->altvnmsg) do_log(LOG_DEBUG,"altvnmsg: enabled");
	if(config->skipsize) do_log(LOG_DEBUG,"skipsize: %dk",config->skipsize);
	if(config->cleankill) do_log(LOG_DEBUG,"cleankill: enabled");
	if(config->noeom) do_log(LOG_DEBUG,"noeom: enabled");
	if(config->nospampipe) do_log(LOG_DEBUG,"nospampipe: enabled");
}

void set_defaults() {
	config->addr.sin_addr.s_addr=INADDR_ANY;
	config->addr.sin_port=htons((int)PORT_NUMBER);
	config->addr.sin_family = AF_INET;
	config->targetaddr.sin_addr.s_addr=INADDR_ANY;
	config->targetaddr.sin_port=htons((int)PORT_NUMBER);
	config->targetaddr.sin_family = AF_INET;
#ifdef CLAMAV
	config->clamav=1;
#else
	config->clamav=0;
#endif
	config->scanner=scannerlist[0];
	config->checkspam=NULL;
	config->clamdport=NULL;
	config->clamdserver=NULL;
	config->blacksubj=SUBJBL;
	config->configfile=NULL;
	config->mailuser=NULL;
	config->extra=NULL;
	config->ispspam=NULL;
	config->notify=NOTIFY;
	config->notifydir=NOTIFY_MAIL_DIR;
	config->pidfile=PID_FILE;
	config->subject=SUBJECT;
	config->spamsubject=SPAMSUBJECT;
	config->smtprset=SMTPRSET;
	config->syslogname=PROGRAM;
	config->virusdir=VIRUS_DIR;
	config->virusdirbase=config->virusdir;
	config->virusregexp=NULL;
	config->virusscanner=NULL;
	config->virustemplate=VIRUS_TEMPLATE;
	config->freespace=MINSPACE;
	config->gvcode[0]=0;
	config->viruscode[0]=VIRUS_SCANNER_VIRUSCODE;
	config->viruscode[1]=-1;
	config->altvnmsg=0;
	config->blackshort=0;
	config->broken=0;
	config->debug=0;
	config->debug_imap=0;
	config->debug_memory=0;
	config->debug_message=0;
	config->debug_scanning=0;
	config->debug_smtp=0;
	config->delit=0;
	config->spamdelete=0;
	config->imaport=IMAP_PORT;
	config->ispam=0;
	config->maxchilds=MAX_CHILDS;
	config->noeom=0;
	config->nohup=0;
	config->password=0;
	config->p3logopt=DEFLOGOPT;
	config->p3logfac=DEFLOGFAC;
	config->quiet=0;
	config->scannerenabled=0;
	config->smtpport=SMTP_PORT;
	config->smtpsize=0;
	config->status=0;
	config->target=0;
	config->timeout=TIMEOUT;
	config->virusregexpsub=1;
	config->skipsize=0;
	config->cleankill=0;
	config->nospampipe=0;
}

void parse_config() {
	struct linebuf *cf;
	int fp, pargc, res;
	long i, ii;
	char *pargv[MAX_PSEUDO_ARGV];
	char *line = NULL;
	char *rest;

	/* Parse configuration file */
	if(!config->nohup) do_log(LOG_NOTICE,"Caught HUP, re-reading config...");
	if(!strlen(NONULL(config->configfile))) {
		config->configfile=CONFIGFILE;
	}
	if((fp=r_open2(config->configfile, O_RDONLY))>=0) {
		cf=linebuf_init(4096);
		pargc=1;
		pargv[0]="";
		while((res=getlinep3(fp, cf))>=0 && pargc<MAX_PSEUDO_ARGV-1) {
			trim(cf->line);
			if(cf->linelen > 2) {
				// Emergency: parse_config: memory error
				if(cf->line[0]!='#' && cf->line[0]!=';' && !(cf->line[0]=='/' && cf->line[1]=='/') && cf->line[0]!='=') {
					if((line=malloc(strlen(cf->line)+3))==NULL) do_log(LOG_EMERG, "ERR: Memory error parsing config file.");
					line[0]='-'; line[1]='-'; line[2]='\0';
					strcat(line, cf->line);
					pargv[pargc]=line;
					if((i=strcspn(line, " =\t"))>1) {
						if(i<strlen(line)) {
							line[i]='\0';
							if((ii=strspn(line+i+1," =\t"))>=0) {
								rest=line+i+ii+1;
								if(strlen(rest)>0 ) {
									pargv[pargc][i]='=';
									memmove(pargv[pargc]+i+1, rest, strlen(rest)+1);
								}
							}
						}
					}
					pargc++;
				}
			}
		}
		r_close(fp);
		linebuf_uninit(cf);
		pargv[pargc]=NULL;
 		parseargs(pargc, pargv);
	}
	if(!config->nohup) do_log(LOG_NOTICE,"Caught HUP, ...done.");
}

int main(int argc, char ** argv) {
	struct servent *port;
	struct passwd *pws;
	struct group *grp;
	struct proxycontext * p=NULL;
	struct sockaddr_in  addr;
	struct statvfs fs;

	socklen_t socksize = sizeof(struct sockaddr_in);
	size_t len = 0;
	ssize_t read;
	char *pargv[MAX_PSEUDO_ARGV];
	char chownit[100];
	char *newline = NULL;;
	char *responsemsg;
	int cnr, res;
	int connfd=0, cuid, guid;
	int digit_optind = 0;
	int dofree=0;
	int error = 0;
	int virusdirlen=0;
	int stat=0;
	int pargc;
	int pidfd;
	int sockfd, result, x, val, sock;
	int ires;
	unsigned long kbfree;
	FILE *pd;
	FILE *chowncmd;

	int newsock;
	char buffer[25];
	int nread;

	pid_t pid, sid;
	long n_desc;
	int i;

#ifdef MTRACE
	mtrace();
#endif
	clean=0;
	if((config=malloc(sizeof(struct configuration_t))) == NULL) {
		printf("This is bad... no memory already?");
		return 1;
	}
	/* set defaults */
	set_defaults();
	/* Parse command line options */
	while(1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options_cl[] = {
			{"configfile", required_argument, NULL, 'c'},
			{"debug",      no_argument,       NULL, 'd'},
			{"help",       no_argument,       NULL, 'h'},
			{"password",   no_argument,       NULL, 'p'},
			{"settings",   no_argument,       NULL, 's'},
			{"version",    no_argument,       NULL, 'v'},
			{NULL,         no_argument,       NULL, 0}
		};

		cnr = getopt_long (argc, argv, "c:dhpsv", long_options_cl, &option_index);
		if(cnr == -1) break;
		if(cnr == '?') {
			fprintf(stderr,"..aborting");
			free(config);
			exit(EXIT_FAILURE);
		}
		switch(cnr) {
			case 'c': /* config (file) */
				if(optarg) config->configfile = optarg;
			break;
			case 'd': /* debug */
				config->debug=1;
			break;
			case 'h': /* help */
				usage();
				exit(EXIT_SUCCESS);
			break;
			case 'p': /* password display */
				config->password=1;
				break;
			case 's': /* settings */
				config->nohup=1;
				parse_config();
				config->nohup=0;
				printversion();
				config->status=1;
				display_settings();
				config->status=0;
				exit(EXIT_SUCCESS);
			break;
			case 'v': /* version */
				printversion();
				exit(EXIT_SUCCESS);
			break;
		}
	}
	if(optind < argc) {
		printf("non-option ARGV-elements: ");
		while(optind < argc) printf("%s ", argv[optind++]);
		printf("\n");
	}
	config->nohup=1;
	parse_config();
	config->nohup=0;
	if(config->quiet && !config->debug) {
		config->quiet=0;
		do_log(LOG_NOTICE, "%s Version %s (Quiet Mode)",PROGNAM,VERSION);
		config->quiet=1;
	} else {
		do_log(LOG_NOTICE, "%s Version %s",PROGNAM,VERSION);
		if(config->clamav) {
			do_log(LOG_NOTICE, "Selected scannertype: clamav (Internal ClamAV)");
		} else {
			do_log(LOG_NOTICE, "Selected scannertype: %s (%s)", config->scanner->name,config->scanner->descr);
		}
	}

	// Emergency: main: Can't open socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0) do_log(LOG_EMERG,"ERR: Can't open socket!");
	val = 1;
	// Emergency: main: Can't create socket
	result = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if(result < 0) do_log(LOG_EMERG,"ERR: Can't create socket!");
	result = bind(sockfd, (struct sockaddr *) &config->addr, sizeof(config->addr));
	// Emergency: main: Can't bind socket
	if(result < 0) do_log(LOG_EMERG,"ERR: Can't bind to socket!");
	// Emergency: main: Can't listen socket
	if(listen(sockfd, 5)) do_log(LOG_EMERG, "ERR: Can't listen on socket");
	do_log(LOG_NOTICE, "Listening on %s:%i", inet_ntoa(config->addr.sin_addr), ntohs(config->addr.sin_port));
	if(!config->debug) {
		/* daemonize */
		if((pid = fork())<0) {
			exit(EXIT_FAILURE);
		} else if(pid != 0) {
			exit(EXIT_SUCCESS);
		}
		if((sid = setsid())<0) exit(EXIT_FAILURE);
		if(chdir("/")) exit(EXIT_FAILURE);
		umask(0);
		r_close(STDIN_FILENO);
		r_close(STDOUT_FILENO);
		r_close(STDERR_FILENO);
	}
	cuid=getuid();
	guid=getgid();
	pws = getpwuid(cuid);
	grp = getgrgid(guid);
	//do_log(LOG_DEBUG, "Running as user: %s group: %s",pws->pw_name,grp->gr_name);
	if((pd=fopen(config->pidfile, "w+"))!=NULL) {
		fprintf(pd, "%i\n", getpid());
		fclose(pd);
	} else {
		do_log(LOG_CRIT, "Can't write PID to %s", PID_FILE);
	}
	if(p3sigaction(SIGUSR1, do_sigusr1)<0) do_log(LOG_EMERG, "Could not set signal handler SIGUSR1");
#ifdef CLAMAV
	if(p3sigaction(SIGUSR2, do_sigusr2)<0) do_log(LOG_EMERG, "Could not set signal handler SIGUSR2");
#endif
	if(p3sigaction(SIGHUP, parse_config)<0) do_log(LOG_EMERG, "Could not set signal handler SIGHUP");
	if(p3sigaction(SIGCHLD, do_sigchld)<0) do_log(LOG_EMERG, "Could not set signal handler SIGCHLD");
	if(p3sigaction(SIGTERM, do_sigterm_main)<0) do_log(LOG_EMERG, "Could not set signal handler SIGTERM main");
	if(p3sigaction(SIGINT, do_sigterm_main)<0) do_log(LOG_EMERG, "Could not set signal handler SIGINT main");
#ifdef CLAMAV
	pthread_mutex_init(&reload_mutex, NULL);
	pthread_mutex_init(&doingload_mutex, NULL);
	if(config->clamav) {
		if(init_clamav()) {
			do_log(LOG_CRIT, "ERR: ClamAV init failed! config and restart " PROGRAM);
		} else {
			config->scannerenabled=1;
		}
	} else
#endif
	{
		if(config->scanner->init1) {
			if(config->scanner->init1()!=0) {
				do_log(LOG_CRIT, "ERR: Scanner init failed! config and restart " PROGRAM);
				config->scannerenabled = 0;
			} else {
				config->scannerenabled = 1;
			}
		} else {
			config->scannerenabled = 1;
		}
	}
	if(config->debug) display_settings();
	numprocs=0;
	do_log(LOG_DEBUG, "Waiting for connections.....");
	while((connfd = accept(sockfd, (struct sockaddr *)&addr,&socksize)) >= 0) {
		if(clean) {
			if(!numprocs) {
				exit(EXIT_FAILURE);
			} else {
				do_log(LOG_WARNING,"Shutting down and cannot fork. %i children to go... ",numprocs);
			}
			break;
		}
#ifdef CLAMAV
		// Check ClamAV Database before launching child:
		if(config->clamav) {
			if(clamcheck()) {
				do_log(LOG_CRIT, "ERR: ClamAV reload failed! config and restart " PROGRAM);
				do_sigterm_main(-1);
			}
		}
#endif
		if((pid=fork())>0) {
			/* parent */
			numprocs++;
			do_log(LOG_DEBUG, "Forked, pid=%i, numprocs=%i", pid, numprocs);
			close (connfd);
			if(numprocs>=config->maxchilds) {
				do_log(LOG_WARNING, "MAX_CHILDS (%i) reached!", config->maxchilds);
				while(1) {
					do_log(LOG_DEBUG,"r_waitpid");
					pid=r_waitpid(-1, &stat, 0); /* blocking */
					do_log(LOG_DEBUG,"do_sigchld_check");
					if(do_sigchld_check(pid, stat)) break;
				}
			}
		} else {
			/* child */
			config->child=1;
			pid=getpid();
			if( statvfs( config->virusdir, &fs ) == SCANNER_RET_ERR) {
				// Emergency: main: Can't get file space
				do_log(LOG_EMERG, "ERR: Unable to get available space!");
				return SCANNER_RET_CRIT; // Should never reach here, but keep it clean. :)
			}
			if(fs.f_bsize==1024) {
				kbfree=fs.f_bavail;
			} else {
				kbfree=fs.f_bsize * (fs.f_bavail / 1024) + fs.f_bavail%1024 * fs.f_bsize / 1024;
			}
			if(config->freespace != 0 && kbfree < config->freespace) {
				// Emergency: main: Not enough file space
				do_log(LOG_EMERG, "ERR: Not enough space! Available space: %lu", kbfree);
				do_sigterm_proxy(1);
				exit(EXIT_SUCCESS);
			}
			virusdirlen=strlen(config->virusdirbase)+20;
			// Emergency: main: memory error creating virusdir
			if((config->virusdir=malloc(virusdirlen))==NULL) do_log(LOG_EMERG, "No memory for virusdir.");
			snprintf(config->virusdir, virusdirlen, "%s/children/%d/", config->virusdirbase,pid);
			do_log(LOG_DEBUG, "setting the virusdir to %s", config->virusdir);
			// Emergency: main: Could not clean child directory
			if(clean_child_directory(pid)) do_log(LOG_EMERG, "Error calling clean child directory!");
			// Emergency: main: Could not create virusdir
			//if((mkdir (config->virusdir, S_IRWXU)<0)) do_log(LOG_EMERG,"Could not create virusdir %s",config->virusdir);
			if(xmkdir(config->virusdir)<0) do_log(LOG_EMERG,"Could not create virusdir %s",config->virusdir);
			if(p3sigaction(SIGCHLD, SIG_DFL)<0) do_log(LOG_EMERG, "Could not set sigaction handler SIGCHLD"); /* unset sigaction handler for child */
			if(p3sigaction(SIGPIPE, SIG_IGN)<0) do_log(LOG_EMERG, "Could not set sigaction handler SIGPIPE"); /* don't die on SIGPIPE */
			if(p3sigaction(SIGTSTP, SIG_IGN)<0) do_log(LOG_EMERG, "Could not set sigaction handler SIGTSTP"); /* Various TTY signals */
			if(p3sigaction(SIGTTOU, SIG_IGN)<0) do_log(LOG_EMERG, "Could not set sigaction handler SIGTTOU");
			if(p3sigaction(SIGTTIN, SIG_IGN)<0) do_log(LOG_EMERG, "Could not set sigaction handler SIGTTIN");
			if(p3sigaction(SIGTERM, do_sigterm_proxy)<0) do_log(LOG_EMERG, "Could not set sigaction handler SIGTERM child");
			if(p3sigaction(SIGINT,  do_sigterm_proxy)<0) do_log(LOG_EMERG, "Could not set sigaction handler SIGINT child");
			do_log(LOG_DEBUG, "Initialize Context");
			// Emergency: main: could not init variable list
			if((p=context_init())==NULL) do_log(LOG_EMERG, "Could not init stuct(p)");
			p->client_fd=connfd;
			p->client_addr=addr;
			if(config->clamav) p->scannerinit=SCANNER_INIT_OK;
			global_p=p;
			do_log(LOG_DEBUG, "starting proxy");
			if(proxy(p)) {
				responsemsg=strdup("Critical abort");
			} else {
				responsemsg=strdup("Clean Exit");
			}
			if(config->scannerenabled) {
				do_log(LOG_NOTICE, "Session done (%s). Mails: %i Bytes: %lu", responsemsg, p->mailcount, p->bytecount);
			} else {
				do_log(LOG_NOTICE, "Session done (%s). Mails: %i", responsemsg, p->mailcount);
			}
			// Emergency: main: Could not clean child directory
			if(clean_child_directory(pid)) do_log(LOG_EMERG, "Pau: Error calling clean child directory!");
			/* responsemsg created w/malloc through strdup */
			free(responsemsg);
			if(p3sigaction(SIGINT, SIG_DFL)<0) do_log(LOG_EMERG, "Could not set sigaction handler SIGINT child");
			if(p3sigaction(SIGTERM, SIG_DFL)<0) do_log(LOG_EMERG, "Could not set sigaction handler SIGTERM child");
			do_sigterm_proxy(-1);
			exit(EXIT_SUCCESS);
		}
	}
	do_log(LOG_NOTICE, "accept error");
	do_sigterm_main(-1);
	return 0;
}

