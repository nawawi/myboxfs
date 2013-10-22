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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sysexits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "p3scan.h"
#include "getlinep3.h"

#define DEFAULT_SERVER  "127.0.0.1"
#define DEFAULT_PORT    "3310"
#define COMMAND         "STREAM\r\n"
#undef DEBUG_MESSAGE
struct configuration_t * config;

extern char *substr(char *string, int start, size_t count);
extern int checktimeout(struct proxycontext *p);
extern char *trim(char *s);

int check(struct proxycontext *p, char * filetoscan, char ** virname){
  int svr_socket=0,sec_socket=0;
  int ret=0, sendret=0;
  int buf_len=0;
  int mailfd=0;
  unsigned long len=0;
  char buf[256]="";
  char *buf_c;
  char *port=NULL;
#define VISIZE 1000
  char *vi=malloc(VISIZE);
  struct sockaddr_in clamd_pri, clamd_sec;
  struct linebuf *filebuf;

  if (vi==NULL) return SCANNER_RET_ERR;

  /* Create socket */
  svr_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

  if (svr_socket == -1){
     do_log(LOG_CRIT, "Could not create socket!");
     return SCANNER_RET_ERR;
  }
  bzero(&clamd_pri, sizeof(clamd_pri));
  clamd_pri.sin_family = AF_INET;
  clamd_pri.sin_addr.s_addr = inet_addr(config->clamdserver);
  clamd_pri.sin_port = htons(strtoul(config->clamdport, NULL, 10));
  /* Connect to server */
  ret = connect(svr_socket,(struct sockaddr *) &clamd_pri, sizeof(clamd_pri));
  if (ret != 0){
     do_log(LOG_CRIT, "Could not connect!");
     close(svr_socket);
     return SCANNER_RET_ERR;
  }
  /* Send "STREAM" command */
  if (write(svr_socket,COMMAND,strlen(COMMAND))==-1){
     do_log(LOG_DEBUG, "write(): %s", strerror(errno));
     do_log(LOG_CRIT,"write(): %s", strerror(errno));
     close(svr_socket);
     return EX_IOERR;
  }
  /* Read server response to get new port number */
  if ((buf_len=read(svr_socket,buf,sizeof(buf)-1))==-1){
     do_log(LOG_DEBUG, "read(): %s", strerror(errno));
     do_log(LOG_CRIT,"read(): %s", strerror(errno));
     return EX_IOERR;
  }
  /* Parse port number from "PORT XX..." */
  buf[buf_len] = 0;
  if (strncasecmp(buf, "PORT ", sizeof("PORT ") -1) != 0) {
     do_log(LOG_DEBUG, "could not get port!");
     close(svr_socket);
     return EX_PROTOCOL;
  }
  port = buf + sizeof("PORT ") -1;
  while(*port == ' ') port ++;
  /* Create new socket */
  bzero(&clamd_sec, sizeof(clamd_sec));
  clamd_sec.sin_family = AF_INET;
  clamd_sec.sin_addr.s_addr = inet_addr(config->clamdserver);
  clamd_sec.sin_port = htons(strtoul(port, NULL, 10));

  sec_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

  if (sec_socket == -1){
     do_log(LOG_CRIT, "Could not create secondary socket!");
     close(svr_socket);
     return SCANNER_RET_ERR;
  }

   /* Connect to new port */
  ret = connect(sec_socket,(struct sockaddr *) &clamd_sec, sizeof(clamd_sec));

  if (ret != 0){
     do_log(LOG_CRIT, "Could not connect secondary!");
     close(svr_socket);
     close(sec_socket);
     return SCANNER_RET_ERR;
  }

  // Send file:
  filebuf=linebuf_init(16384);
  if ((mailfd=open(filetoscan, O_RDONLY ))<0){
     close(svr_socket);
     close(sec_socket);
     linebuf_uninit(filebuf);
     do_log(LOG_EMERG, "Can't open mailfile (%s)!\n", filetoscan);
     return SCANNER_RET_ERR;
  }
  while (1){
     ret=checktimeout(p);
     if (ret < 0){
        close(svr_socket);
        close(sec_socket);
        linebuf_uninit(filebuf);
        close(mailfd);
        return SCANNER_RET_CRIT;
     }
     if ((ret=getlinep3(mailfd, filebuf))<0){
        if (ret==GETLINE_TOO_LONG){
           /* Buffer contains part of line, take care of later */
        } else {
           /* Other error, take care of later */
           break;
        }
     }
     if (filebuf->linelen >=0 ){
        len += filebuf->linelen;
        if (config->debug_scanning) do_log(LOG_DEBUG, ">%s", filebuf->line);
        /* Take care of buffer here */
        if (ret==GETLINE_TOO_LONG){
           sendret=writeline(sec_socket, WRITELINE_LEADING_NONE, filebuf->line);
        } else {
           sendret=writeline(sec_socket, WRITELINE_LEADING_RN, filebuf->line);
        }
        if (sendret==GETLINE_PIPE){
           do_log(LOG_CRIT, "Clam Server disappeared!");
           close(svr_socket);
           close(sec_socket);
           linebuf_uninit(filebuf);
           close(mailfd);
           return EPIPE;
        } else if (sendret){
           linebuf_uninit(filebuf);
           close(svr_socket);
           close(sec_socket);
           close(mailfd);
           do_log(LOG_EMERG, "Error sending mail to clamd!");
           /* we are dead now. Should not reach here. But allow it
           to fall through in case LOG_EMERG is changed in the future. */
           return SCANNER_RET_ERR;
        }
     }
  }
  linebuf_uninit(filebuf);
  close(mailfd);
  /* Close secondary socket to force primary socket output */
  close(sec_socket);
  if (ret!=GETLINE_EOF){
     do_log(LOG_CRIT, "error reading from mailfile %s, error code: %d", filetoscan, ret);
     close(svr_socket);
     return SCANNER_RET_ERR;
  }
  /* Get response from primary socket */
  if ((buf_len=read(svr_socket,buf,sizeof(buf)-1))==-1){
     do_log(LOG_DEBUG, "read(): %s", strerror(errno));
     do_log(LOG_CRIT,"read(): %s", strerror(errno));
     close(svr_socket);
     return EX_IOERR;
  }
  /* If response contains "FOUND" then there is a virus */
  // "stream: Eicar-Test-Signature FOUND"
  /* If response contains "OK" then there is no virus */
  // "stream: OK"
  /* Parse Virusname */

  close(svr_socket);
  buf[buf_len] = 0;
  buf_c = buf + buf_len;
  while(*buf_c == '\r' || *buf_c == ' ') buf_c --;
  if (buf_c - buf >= sizeof("FOUND") && strncasecmp(buf_c - sizeof("FOUND"), "FOUND", sizeof("FOUND")-1) == 0) {
    char *buf_s = buf;
    buf_c -= sizeof("FOUND");
    if (strncasecmp(buf_s, "stream:", sizeof("stream:")-1) == 0) {
      buf_s += sizeof("stream:")-1;
      while(*buf_s == ' ') buf_s ++;
      snprintf(vi,(int)(buf_c - buf_s),"%s",buf_s);
    }
  }
  *virname=vi;
  trim(buf_c);
  if (strlen(buf_c) >3) return SCANNER_RET_VIRUS; // contains a virus
  else return SCANNER_RET_OK; // all ok, no virus
}

static int init1(void){
   int   len,loc,loc2;
   char  *tmp=NULL;

   if (strlen(NONULL(config->virusscanner))<1){
      tmp=strndup(DEFAULT_SERVER,strlen(DEFAULT_SERVER));
      strncat(tmp,":",1);
      strncat(tmp,DEFAULT_PORT,4);
      config->virusscanner=tmp;
      do_log(LOG_CRIT, "Clamd init: No scanner was defined. we're using %s",config->virusscanner);
   }
   tmp=strchr(config->virusscanner,':');
   if (tmp){
      loc = tmp-config->virusscanner;
      loc2 = tmp-config->virusscanner+1;
      len=strlen(config->virusscanner);
      config->clamdport=substr(config->virusscanner,loc2,len);
      config->clamdserver=strndup(config->virusscanner,loc); //TODO: 6 bytes in 1 blocks are definitely lost in loss record 1 of 2
      do_log(LOG_DEBUG, "Clamd init. Server: %s Port: %s",config->clamdserver, config->clamdport);
   }else{
      do_log(LOG_CRIT, "Clamd init unable to locate separator: %s",config->virusscanner);
      return SCANNER_RET_ERR;
   }
   if (strlen(NONULL(config->virusscanner))<1){
      do_log(LOG_CRIT, "no scanner was defined. scanning completely disabled");
      return SCANNER_RET_ERR;
   }
   return 0;
}

static int scan(struct proxycontext *p, char **virname){
   int ret;

   do_log(LOG_DEBUG, "Clamd TCP scanner says hello");
   ret = check(p, p->scanthis, virname);
   do_log(LOG_DEBUG, "Clamd TCP scanner says goodbye: %i",ret);
   return ret;
}

static void uninit1(void){
   if(!config->virusscanner) free(config->virusscanner);
}

scanner_t scanner_clamd = {
   "clamd",                /* name */
   "ClamAV TCP Daemon",    /* description */
   &init1,                 /* init1 (once, afer startup) */
   NULL,                   /* init2 (every connection before first mail) */
   &scan,                  /* scan */
   NULL,                   /* uninit2 */
   &uninit1,               /* uninit1 */
   0                       /* dirscan */
};
