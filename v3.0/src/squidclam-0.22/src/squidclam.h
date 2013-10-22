/*********************************************************************
 * Copyright (C) 2005 Daniel Lord (squidclam At users DoT sf Dot net)*
 *                                                                   *
 * This is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published *
 * by the Free Software Foundation; either version 2 of the License, *
 * or (at your option) any later version.                            *
 *
 *
 * This software is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 * GNU General Public License for more details.                      *
 * 	                                                                 *
 * You should have received a copy of the GNU General Public         *
 * License along with this software; if not, write to the Free       *
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,   *
 * MA  02111-1307, USA.                                              *
 **********************************************************************/

#ifndef _SQUIDCLAM_H
#define _SQUIDCLAM_H

/** Page to redirect requests to if malware is found. */
#ifndef ERROR
    #define ERROR    "http://127.0.0.1/antivir.php"    /* url with the antivir.php file */
#endif
/** Proxy to use for the downloads */
#ifndef MY_PROXY
    #define MY_PROXY "127.0.0.1"
#endif
/** Template for the tempfiles */
#ifndef TMPF
    #define TMPF     "/dev/shm/squidclamXXXXXXXX"      /* tempfile preferable on ramdisk */
#endif
/** Maximum size of files to scan */
#ifndef FSIZE
    #define FSIZE (204800)         /* scan/get max 100kb of data */
#endif
/** Config filename */
#ifndef CONFIG
    #define CONFIG "/etc/squidclam.conf"
#endif
/** Clamav server tcp address */
#ifndef CLAMDSERVER
    #define CLAMDSERVER "127.0.0.1"
#endif
/** Port used by clamd */
#ifndef CLAMDPORT
    #define CLAMDPORT 3310
#endif
/** */
#ifndef SECTIME
    #define SECTIME 240
#endif

#ifndef UNIX 
    #define TCP
#endif

#define TRUE  1
#define FALSE 0

#define BUFM (255)
#define SBUFM (255)-1

/** Time to wait after error */
#define DTIME (100000)

/** Memory struct prototype, used by curl to store the downloaded files */
struct MemoryStruct {
	/** memory chunk */
	char *memory;
	/** size of saved data */
	size_t size;
};

/** config settings prototype. All config options are stored in here */
struct cfg {
	/** proxy adress */
    char *proxy;
	/** url to redirect to if malware is found */
    char *url;
	/** tempfile template */
    char *tmp;
	/** clamd server ip */
    char *cip;
	/** clamd port */
	unsigned int cp;
	/** max size of files to scan */
    unsigned int fsize;
	/** debug or no debug thats the question */
	unsigned int debug;
	/** show errors to the user or not? */
	unsigned int errignore;
	/** show squidclam-$Version in http requests */
	unsigned int hideme;
} cfg;

/* function prototyps config.c */
FILE * opencfg (void);
int closecfg (FILE *fd);
char * stripblank (char *s);
char * stripctrl (char *s);
int is_cfg_line (char *p);
char * getvalue (char *s);
int partial_cont (char *s);
struct cfg read_cfg (FILE *f);

/** function prototyps clamdscan.c */
char clamdscan (char *file, char *virname);
#endif
