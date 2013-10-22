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
 ***************************************************************************
 *
 * Sophie routine by Salvatore Toribio, based on the Trophie routine (20050514)
 *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <malloc.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <ctype.h>

#include "p3scan.h"

#define DEFAULT_SOCKET_PATH "/var/spool/sophos"

struct configuration_t * config;

static int  sophie_fd;    // fd for log
static int  connected;    // have done connect

static struct sockaddr_un sophie_socket; // AF_UNIX address of local logger

static int sophie_socket_connect(struct proxycontext *p) {
	if(sophie_fd == -1) {
		bzero((char *)&sophie_socket, sizeof(sophie_socket));
		sophie_socket.sun_family=AF_UNIX;
		strcpy(sophie_socket.sun_path, config->virusscanner);
		if((sophie_fd=socket(AF_UNIX,SOCK_STREAM,0)) < 0 ) {
			do_log(LOG_CRIT, "create socket error: socket() not created %s",
			config->virusscanner);
			return -1;
			}
		}
		if(sophie_fd!=-1 && connected==-1) {
			do_log(LOG_DEBUG, "Trying to connect to socket");
			if(connect(sophie_fd, (struct sockaddr *)(&sophie_socket),sizeof(sophie_socket.sun_family) + strlen(config->virusscanner)) >= 0) {
				connected=1;
				do_log(LOG_DEBUG, "sophie_socket_connect connected");
				return 0;
			}
		} else {
			do_log(LOG_DEBUG, "Already connected");
			return 0;
		}
		do_log(LOG_CRIT, "can't connect to socket %s", config->virusscanner);
		return -1;
}

static void sophie_socket_close(void) {
	close(sophie_fd);
	sophie_fd=-1;
	connected=0;
	do_log(LOG_DEBUG, "sophie_socket_close");
}


static int sophie_scanfile(struct proxycontext * p, char * filetoscan, char ** virname) {
	char *sendbuf;
	char recvbuf[512];
	int len;

	*virname=NULL;
	if(sophie_fd<0 || !connected) {
		if(sophie_socket_connect(p)!=0) return SCANNER_RET_ERR;
	}
	len=strlen(filetoscan);
	sendbuf=malloc(len+2);
	(void)snprintf(sendbuf, len+2, "%s\n", filetoscan);
	/* send filename */
	do_log(LOG_DEBUG, "Sending to socket -> %s",filetoscan);
	//do_log(LOG_DEBUG, "Sending to socket");
	if(write(sophie_fd, sendbuf, len+1) <0) {
		do_log(LOG_ALERT, "Can't write to sophie socket");
		free(sendbuf);
		return SCANNER_RET_ERR;
	}
	free(sendbuf);
	do_log(LOG_DEBUG, "OK");
	/* retrieve message */
	memset(recvbuf, 0, sizeof(recvbuf));
	if((len = read(sophie_fd, recvbuf, sizeof(recvbuf))) > 0) {
		do_log(LOG_DEBUG, "%i bytes read", len);
		if(strchr(recvbuf, '\n')) *strchr(recvbuf, '\n') = '\0';
		if(recvbuf[0] == '1') {
			/* virus */
			do_log(LOG_DEBUG, "it's a virus");
			*virname=strdup(recvbuf+2);
			return SCANNER_RET_VIRUS;
		} else if(!strncmp(recvbuf, "-1", 2)) {
			do_log(LOG_CRIT, "Error scanning %s (error or file not found)", filetoscan);
			return SCANNER_RET_ERR;
		}
	} else {
		do_log(LOG_ALERT, "Can't read message to sophie socket");
		return SCANNER_RET_ERR;
	}
	return SCANNER_RET_OK;
}

static int init1(void) {
	do_log(LOG_DEBUG, "Sophie Init1");
	if(strlen(NONULL(config->virusscanner))<1) {
		//do_log(LOG_CRIT, "no scanner was defined. we're using " DEFAULT_SOCKET_PATH);
		config->virusscanner=strdup(DEFAULT_SOCKET_PATH);
	}
	connected=-1;
	sophie_fd=-1;
	do_log(LOG_DEBUG, "Sophie Init1 Done");
	return 0;
}

static int init2(struct proxycontext *p) {
	do_log(LOG_DEBUG, "Sophie Init2");
	/* Connect to socket */
	if(sophie_socket_connect(p)!=0) return -1;
	do_log(LOG_DEBUG, "Sophie Init2 Done");
	return 0;
}

static void uninit2(struct proxycontext *p) {
	sophie_socket_close();
}

static int scan(struct proxycontext *p, char ** virname) {
	int ret;
	do_log(LOG_DEBUG, "Sophie scanner says hello");
	ret=sophie_scanfile(p, p->scanthis, virname);
	do_log(LOG_DEBUG, "Sophie scanner says goodbye");
	//close(sophie_fd);
	//sophie_fd=-1;
	//connected=0;
	return ret;
}

scanner_t scanner_sophie = {
	"sophie",                    /* name */
	"Sophos antivirus daemon",  /* description */
	&init1,                       /* init1 (once, afer startup) */
	&init2,                       /* init2 (every connection before first mail) */
	&scan,                        /* scan */
	&uninit2,                     /* uninit2 */
	NULL,                         /* uninit1 */
	0                             /* dirscan */
};
