/*********************************************************************
 * Copyright (C) 2005 Daniel Lord (squidclam At users DoT sf Dot net)*
 *                                                                   *
 * This is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published *
 * by the Free Software Foundation; either version 2 of the License, *
 * or (at your option) any later version.                            *
 *                                                                   *
 * This software is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 * GNU General Public License for more details.                      *
 *                                                                   *
 * You should have received a copy of the GNU General Public License *
 * along with this software; if not, write to the Free Software      *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,            *
 * MA  02111-1307, USA.                                              *
 *********************************************************************/

/* includes  */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/un.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

#include "version.h"
#include "squidclam.h"

/* contribution from Steve-o fnjordy at users.sourceforge.net */
int
call_unix_socket (char *socket_name)
{
   struct sockaddr_un sa;
   int s;

   memset(&sa, 0, sizeof(sa));
   sa.sun_family=AF_UNIX;
   strcpy(sa.sun_path, socket_name);
   if ((s=socket(AF_UNIX, SOCK_STREAM, 0)) < 0 ) {
       return(-1);
   }

   if (connect(s, (struct sockaddr *) & sa, sizeof(sa)) < 0 ) {
       close(s);
       return(-1);
   }

   return(s);
}


int 
call_socket (char *hostname, unsigned short portnum)
{
	struct sockaddr_in sa;
	int s;
	
	memset(&sa, 0, sizeof(sa));
	inet_aton(hostname, &(sa.sin_addr));
	sa.sin_family=AF_INET;
	sa.sin_port=htons((u_short)portnum);
	memset(&(sa.sin_zero), '\0', 8);

  	if ((s=socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		return(-1);
    }
	
	if (connect(s, (struct sockaddr *) & sa, sizeof(sa)) < 0 ) {
		close(s);
        return(-1);
    }
	
    return(s);
}


static int 
read_data(int s, int tout, char* in, size_t len)
{
	fd_set rfds;
	struct timeval tv;
	
	if (tout == 0) {
		return recv(s,in,len,0);
	}
	
	FD_ZERO(&rfds);
	FD_SET(s,&rfds);
	
	tv.tv_sec = tout;
	tv.tv_usec = 0;
	
	switch(select(s+1,&rfds,NULL,NULL,&tv)){
		case -1:
			return(-1);
			break;
		case 0:
			return(-2);
			break;
		default:
			return recv(s,in,len,0);
			break;
	}
}

static int sendclam(int s, char *data, size_t len)
{
	if (s > 0) {
		if (send(s,data,len,0) < len) {
			close(s);
			return(-1);
		}
	}
	
	return(0);
}

/* Wrapper function for use with clamd. The virname argument must have
 * room for at least 255 chars
 * return values:
 * 0 - file clean
 * 1 - virus found
 * 2 - failed to scan file
 * 4 - no clamd found
 */

char clamdscan (char *file, char *virname)
{
	char buf[BUFM];
	char t1[BUFM];
	char t3[BUFM];
	int s=0;
	char in[BUFM];
	int rlen=0;
	char *ptr;

    /* just to be sure there are only zeros in there...*/
	memset(in,0,BUFM);

 	snprintf(buf,SBUFM, "SCAN %.255s\n", file);

	if (cfg.cip[0] == '/') {    /* guess at unix socket instead of tcp socket */
		 s=call_unix_socket(cfg.cip);
	} 
	else {
		s=call_socket(cfg.cip,cfg.cp);
	}

	if (s < 0) {
		syslog(LOG_ERR, "Failed to connect to clamd server");
		fprintf(stderr,"Failed to connect to clamd server\n");
		return 4;
	}
	
	if (sendclam(s, buf,strlen(buf)) < 0 )	{
		syslog(LOG_ERR, "Failed to hand file over to clamd server");
		fprintf(stderr,"Failed to hand file over to clamd\n");
		return 4;
		close(s);
	}

	rlen = read_data(s, SECTIME, in, BUFM);

    close(s);

    in[rlen] = '\0';
	
	if ((ptr = strchr(in,'\n')) != NULL) {
		*ptr = '\0';
	}
	
	if (strcmp(&in[rlen-6],"FOUND") == 0) {
		memset(virname,0,BUFM);
		sscanf(in,"%255s %255s %255s\n", t1, virname, t3);
		
		if (cfg.debug > 0) {
			fprintf(stderr, "%s (%i)", virname, rlen); 
		}
	
		return 1;
	}
	
	return 0;
}

