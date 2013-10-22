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
 ***************************************************************************/

#ifndef _P3SCAN_H
#define _P3SCAN_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_SYSLOG_H
#include <syslog.h>
#endif

#include "getlinep3.h"

#ifdef CLAMAV
#include <clamav.h>
#endif
#include <ctype.h>

#define PROGNAM "P3Scan"
#ifndef PROGRAM
#define PROGRAM PACKAGE
#endif
#define MAX_PSEUDO_ARGV 52
#define MINSPACE 10000
#define MAXFILENAME 4096
#define MESSAGE_NOVIRINFO "[no virusinfo could be examined]"
#define FILEDEL "The infected message has been deleted."
#define FOUNDIBL "Not a virus! Found in BlackList!"
#define SUBJBL "[ Blacklisted ] "
#define SVRCMD "NOOP"
#define PERIOD "."
#define BOGUSX "X-P3Scan: Due to an extremely large attachment you see this message line."
#define NUL '\0'
#define DEFLOGOPT LOG_PID|LOG_CONS
#define DEFLOGFAC  LOG_DAEMON

#define CONFIG_STATE_CMDPRE 1
#define CONFIG_STATE_FILE 2
#define CONFIG_STATE_CMD 3

#define SCANNER_INIT_NO 0
#define SCANNER_INIT_OK 1
#define SCANNER_INIT_NULL 2 /* scanner needs no init */
#define SCANNER_INIT_ERR -1

#define SCANNER_RET_OK 0
#define SCANNER_RET_ERR -1
#define SCANNER_RET_VIRUS 2
#define SCANNER_RET_CRIT 3

#define MAX_VIRUS_CODES 16

#if defined(__USE_FILE_OFFSET64) || defined(__USE_LARGEFILE64)
#define FSTAT(a, b) fstat64(a,b)
#define LSTAT(a, b) lstat64(a,b)
#define STAT(a, b) stat64(a,b)
#define stat_t stat64
#else
#define FSTAT(a, b) fstat(a,b)
#define LSTAT(a, b) lstat(a,b)
#define STAT(a, b) stat(a,b)
#define stat_t stat
#endif
#ifndef TMP_MAX
#define TMP_MAX 238328
#endif

#define SETIFNULL(a,b) if (!a) a=b  /* wow, that's magic */
#define NONULL(x) ( x==NULL ? "" : x) /* this is nice, found in the mutt code */

/* default configuration, anything can be changed at runtime */
#define ACT_RETR                 1  /* Action = RETR msg from server */
#define ACT_TOP                  2  /* Action = TOP or part of msg from server */
#define ACT_SMTP                 3  /* Action = Sending message to server */
#define ACT_IMAP                 4  /* Action = */
#define LETTERS                  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
#define PORT_NUMBER              8110
#define SMTP_PORT                25
#define POP3_PORT                110
#define IMAP_PORT                143
#define MAX_CHILDS               10
#define TIMEOUT                  30
#define VIRUS_SCANNER            NULL
#define VIRUS_SCANNER_VIRUSCODE  1
#define DEBUG                    0
#define QUIET                    0
#define OVERWRITE                NULL
#define CHECKSPAM                0
#define SPAMCHECK                "/bin/spamc"
#define DELIT                    0
#define NEWLINE                  '\n'
#define SUBJECT                  "[ VIRUS ]"
#define SPAMSUBJECT              "[ SPAM ]"
#define NOTIFY                   "The email has been removed"
#define SMTPRSET                 "Virus detected"
/* Defaut maximum mail size for scanning. ZERO for no limit! */
#define MAX_SIZE_SCAN            0
#ifdef HAVE_SETSOCKOPT
/* TOS:  do not set, or use IPTOS_[LOWDELAY|THROUGHPUT|RELIABILITY|LOWCOST] */
#define SET_TOS                  IPTOS_THROUGHPUT
#endif
#define POK                      "+OK"
#undef DEBUG_MEM                 /* print meminfo every log message when debugging */
#undef DEBUG_MESSAGE             /* print message lines */
#undef DEBUG_SCANNING            /* print message lines while scanning */
#undef DEBUG_SMTP                /* print smtp messages lines */
#define oops(s) { perror((s)); exit(EXIT_FAILURE); }
#define MALLOC(s,t) if(((s) = malloc(t)) == NULL) { oops("error: malloc() "); }

#ifndef LOGOPT
#define LOGOPT LOG_PID|LOG_CONS
#endif

#ifndef LOGFAC
#define LOGFAC LOG_DAEMON
/* To log to file:
   In the Makefile, change LOG_DAEMON to:
   LOGFAC="LOG_LOCAL0"
   or if not defined there, comment above and uncommment below.

   Add to /etc/syslog.conf (and restart it):
   local0.*                                                -/var/log/p3scan
*/
//#define LOGFAC LOCAL0
#endif
/* default configuration ends here */

pthread_mutex_t engine_lock;
pthread_attr_t ta;

/*
 * Thread that reloads virus definitions as needed
 */
void av_refresh();

typedef struct proxycontext {
	struct linebuf *serverbuf;
	struct linebuf *clientbuf;
	struct linebuf *hdrbuf;
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;
	struct paramlist *params;
	struct hostent *server_host;
	socklen_t socksize;
	off_t hdroffset;
	time_t now;
	unsigned int msgnum;
	unsigned long bytecount;
  	int client_fd;
  	int server_fd;
  	int mailhdr_fd;
  	int blacklist;
  	int whitelist;
  	int action;
  	int actport;
  	int boguspos;
  	int checksmtp; /* used to bypass checking for smtp */
  	int cksmtp; /* if scanning an smtp submission */
  	int errmsg;
  	int extra;
  	int fakehdrdone;
  	int gobogus;
  	int hdrdate;
  	int hdrfrom;
  	int hdrto;
  	int header_exists;
  	int imap;
  	int ismail;
  	int mailcount;
  	int newsock;
  	int noop;
  	int noscan;
  	int nosend;
  	int notified;
  	int nowrite;
  	int posttop;
  	int scannerinit; /* see SCANNER_INIT_* */
  	int topping;
  	int viret;
  	int spamfound;
	char vnmsg[MAXFILENAME];
	char mailfile[MAXFILENAME];
	char maildir[MAXFILENAME]; /* mailfile.content */
	char mailhdr[MAXFILENAME];
	char cbuff[1];
	char listfileb[512];
	char listfilew[512];
	char *actsvr;
	char *mailuser;
	char *extrasubj; /* extra notification subject line */
	const char *filename;
	char *filestatus; /* infected mail kept or deleted */
	char *scanthis; /* depending on demime linked to mailfile / maildir */
	char *virinfo; /* has to be filled from the scanner */
} proxycontext;

typedef struct scanner_t {
	char *name;
	char *descr;
	int (*init1)(void);
	int (*init2)(struct proxycontext *);
	int (*scan)(struct proxycontext *, char ** virname);
	void (*uninit2)(struct proxycontext *);
	void (*uninit1)(void);
	int dirscan;
} scanner_t;

typedef struct configuration_t {
	struct        sockaddr_in addr;
	struct        sockaddr_in targetaddr;
	scanner_t *   scanner;
	char *blacksubj;
	char *clamdport;
	char *clamdserver;
	char *checkspam;
	char *configfile;
	char *mailuser;
	char *extra;
	char *ispspam;
	char *mail;
	char *notify;
	char *notifydir;
	char *overwrite;
	char *pidfile;
	char *subject;
	char *spamsubject;
	char *smtprset;
	char *syslogname;
	char *virusdir;
	char *virusdirbase;
	char *virusregexp;
	char *virusscanner;
	char *virustemplate;
	unsigned long freespace;
  	int gvcode[MAX_VIRUS_CODES + 1];
  	int viruscode[MAX_VIRUS_CODES + 1];
  	int altvnmsg;
  	int blackshort;
  	int broken;
  	int child;
  	int clamav;
  	int cleankill;
  	int debug;
	int debug_imap;
  	int debug_memory;
  	int debug_message;
  	int debug_scanning;
  	int debug_smtp;
  	int delit;
  	int spamdelete;
  	int imaport;
  	int ispam;
  	int maxchilds;
  	int noeom;
  	int nohup;
  	int nospampipe;
  	int password;
  	int p3logopt;
  	int p3logfac;
  	int quiet;
  	int scannerenabled;
  	int skipsize;
  	int smtpport;
  	int smtpsize;
  	int status;
  	int target;
  	int timeout;
	int virusregexpsub;
} configuration_t;

extern void do_log(int level, const char *fmt,...);
void context_uninit(struct proxycontext * p);
typedef void Sigfunc(int);

#endif
