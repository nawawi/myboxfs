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

#include <netinet/in.h>
#include <arpa/inet.h>

#include "version.h"
#include "squidclam.h"

/* open config file */
FILE * 
opencfg (void)
{
    FILE *fd=NULL;
	
	openlog("squidclam", LOG_PID, LOG_DAEMON);
	
    if ((fd = fopen(CONFIG, "rb")) == NULL) {
        fprintf(stderr,
				"failed to open config (%.255s) using hardcoded values\n"
				, CONFIG);
		syslog(LOG_WARNING, "No config (%s) file found, using defaults", CONFIG);
    }

	closelog();
    return fd;
}

/* close config file */
int 
closecfg (FILE *fd)
{
    fclose(fd);
    return 0;
}

/* strip blanks */
char * 
stripblank (char *s) {
	char *n;
	unsigned int l=0, x=0, i=0;
	
	/* get space for the new string */
	l = strlen(s);
	n = malloc( l+1 );
	
	for (i=0; i<l; i++) {
		if ( ( s[i] != ' ' ) && (s[i] != '\t') ) {
			n[x] = s[i];
			x++;
		}
	}
	
	/* make sure string is terminated */
	n[x] = 0x00;

	return n;
}

/* strip \r\n and some more */
char * 
stripctrl (char *s) {
	char *p;
	
	for (p=s;*p;p++) {
		if (iscntrl(*p) != 0) {
			*p = 0x00;
		}
	}
	
	return s;
}

/* check if this is a valid cfg line (no comment eg.) */
int 
is_cfg_line (char *p) {
	
	/* if line starts with a comment mark or is empty 
	 * return FALSE */
	if ((*p == '#') || (*p == '\0')) {
		return FALSE;
	}
	/* TODO */
	return TRUE;
}

/* get the value of the config variable */
char * getvalue (char *s) 
{
	char *x;
	
	x = strstr(s,"=");
	
	return ++x;
}

int partial_cont (char *s) 
{
	
	/* check if we read the whole line */
	if (s[strlen(s)-1] != '\n') {
		/* if not complete we have partial content so return TRUE */
		return TRUE;
	}
	
	return FALSE;
}

/* read config and init vars */
struct cfg read_cfg (FILE *f)
{
    //struct cfg cfg;
	char ts[100];
	char *p;

    cfg.proxy = NULL;
    cfg.url = NULL;
    cfg.tmp = NULL;
    cfg.fsize = 0;
    cfg.cip = NULL;
    cfg.cp = 0;
	
	if (f != NULL) {
		while (((fgets(ts,sizeof(ts),f)) != NULL) && (partial_cont(ts) != TRUE)) {
			p = stripblank(ts);
			p = stripctrl(p);
			if (is_cfg_line(p) == TRUE) {
				switch(*p) {
					case 'p':
						p = getvalue(p);
						cfg.proxy = malloc(strlen(p)+1);
						strcpy(cfg.proxy, p);
						break;
					case 'u':
						p = getvalue(p);
						cfg.url = malloc(strlen(p)+1);
						strcpy(cfg.url, p);
						break;
					case 't':
						p = getvalue(p);
						cfg.tmp = malloc(strlen(p)+1);
						strcpy(cfg.tmp, p);
						break;
					case 'f':
						p = getvalue(p);
						cfg.fsize = atoi(p);
						break;
                    case 'n':
                        p = getvalue(p);
                        cfg.cp = atoi(p);
                        break;
                    case 'c':
                        p = getvalue(p);
                        cfg.cip = malloc(strlen(p)+1);
                        strcpy(cfg.cip, p);
                        break;
					case 'd':
						p = getvalue(p);
						cfg.debug = atoi(p);
						break;
					case 'e':
						p = getvalue(p);
						cfg.errignore = atoi(p);
						break;
					case 'h':
						p = getvalue(p);
						cfg.hideme = atoi(p);
						break;
					default:
						fprintf(stderr,
								"wrong config line (%.255s)\n",ts);
						break;
				}
			}
		}
	}

	if (cfg.proxy == NULL) {
		cfg.proxy = malloc(sizeof(MY_PROXY)+1);
		strncpy(cfg.proxy, MY_PROXY, sizeof(MY_PROXY));
	}
	if (cfg.url == NULL) {
		cfg.url = malloc(sizeof(ERROR)+1);
		strncpy(cfg.url, ERROR, sizeof(ERROR));
	}
	if (cfg.tmp == NULL) {
		cfg.tmp = malloc(sizeof(TMPF)+1);
		strncpy(cfg.tmp, TMPF,sizeof(TMPF));
	}
	if (cfg.cip == NULL) {
		cfg.cip = malloc(sizeof(CLAMDSERVER)+1);
		strncpy(cfg.cip, CLAMDSERVER,sizeof(CLAMDSERVER));
	}
	if (cfg.cp <= 0) {
		cfg.cp = CLAMDPORT;
	}
	if (cfg.fsize <= 0) {
		cfg.fsize = FSIZE;
	}
	if ((cfg.debug <= 0) || (cfg.debug >= 2)) {
		cfg.debug = 0;
	}
	if ((cfg.errignore <= 0) || (cfg.errignore >= 2)) {
		cfg.errignore = 0;
	}
	if ((cfg.hideme <= 0) || (cfg.hideme >= 2)) {
		cfg.hideme = 0;
	}

    return cfg;
}

