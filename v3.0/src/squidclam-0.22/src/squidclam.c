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

/** return codes
 * 0 - Success 
 * 1 - Could not unbuffer stdout
 * 2 - problem with clamav / clamd
 * 3 - problem with temp files
 * 4 - problem with curl
 */

/* includes  */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#ifndef S_SPLINT_S
	#include <syslog.h>
	#include <curl/curl.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

#include "version.h"
#include "squidclam.h"

/** if defined hand url over to squid and don't do anything in case of error */
/* else give the admin an oportunity to realize problems earlier */
//#define ERRIGNORE

/* if defined display debug output */
// #define DEBUG

/** init all the fake useragents. but now wie may have other problems
 * fake useragents are used to fool scripts which try to get arround squidclam
 * by serving bad content only to real useragents 
 * Its not a good solution to just route the used useragent through. Because
 * doing this gives an attacker a chance by just looking for calls from the same
 * useragent to the same file. Now its only possible for him to have a look at
 * the same IP looking after the same file. Still not good but I don't know a
 * better solution for now. 
 *
 * As of this squidclam seems to be broken by design. There are just too many
 * possible holes. To name a few,
 * - ssl sites
 * - dynamik sites
 * - big files > fsize
 * 
 * of cause you can (and have to) limit access to such files using squid.conf.
 * But I think it's not a good choice to trade freedom for security. Neither
 * in real life nor in cyberspace.
 *
 * */

char *useragents[] = {
	"Mozilla/4.0 (compatible; MSIE 4.0;) Dillo/0.8.5-i18n-misc", /* 1 */
	"Mozilla/5.0 (compatible; MSIE 5.5;) Firefox/1.5",           /* 2 */
	"Mozilla/4.61 [en] (X11; U; ) - BrowseX (2.0.0 Windows)",    /* 3 */
	"Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.4.1) Gecko/20031114 Epiphany/1.0.4", /* 4 */
	"Mozilla/5.0 (X11; U; Linux i686) Gecko/20030422 Galeon/1.3.4", /* 5 */
	"Lynx/2.8 (compatible; iCab 2.9.8; Macintosh; U; 68K)", /* 6 */
	"Mozilla/5.0 (compatible; Konqueror/3.4; Linux) KHTML/3.4.3 (like Gecko) (Kubuntu package 4:3.4.3-0ubuntu1)", /* 7 */
	"Mozilla/4.01 (Compatible; Acorn Phoenix 2.08 [intermediate]; RISC OS 4.39) Acorn-HTTP/0.84", /* 8 */
	"Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.6) Gecko/20040206 Firefox/0.8 Mnenhy/0.6.0.103", /* 9 */
	"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; FREE; .NET CLR 1.1.4322)", /* 10 */
	"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0; .NET CLR 1.0.3705; .NET CLR 1.1.4322)", /* 11 */
	SQUIDCLAMVERSION,
};


/** Memory funktion to store the download  */
size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
      register int realsize = size * nmemb;
      struct MemoryStruct *mem = (struct MemoryStruct *)data;
     
      mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
      
      if (mem->memory) {
          memcpy(&(mem->memory[mem->size]), ptr, realsize);
          mem->size += realsize;
          mem->memory[mem->size] = 0;
      }
	  if (cfg.debug > 0) {
		  printf("%i\n", realsize);
	  }

      return realsize;
}

/** get random number  */
unsigned char 
get_random_number (void)
{
    int rf;
    unsigned char ret[1];

    /* open random source */
    if ((rf = open("/dev/urandom", O_RDONLY)) == -1) {
        usleep(DTIME);
        return -1;
    }
    
    /* get one random byte */
    if (read(rf,ret,1) != 1) {
        return -1;
    }
    
    /* close random source */
    close(rf);
    
    return ret[0];
}

/** one bloated main for the moment */
int main (void)
{
    unsigned char virname[256]="default-you-shouldn't-get-this-check-your-setup";

    struct request {
        char url[10000]; /* don't forget to also change scanf if you tweak this */
        char src[255];   /* don't forget to also change scanf if you tweak this */
        char ident[255]; /* don't forget to also change scanf if you tweak this */
        char method[31]; /* don't forget to also change scanf if you tweak this */
    } rq;

    CURL *eh;
    
    struct MemoryStruct mem;

    int  fd=0;
	unsigned short uagent=0;
	unsigned char tt[255], slimit[255];
    unsigned char erbuf[CURL_ERROR_SIZE+1];
	unsigned char ret=0;
    
    unsigned int rand=0;

	FILE *cf; /* config file handle */

    /* init struct */
    mem.memory=NULL; /* we expect realloc(NULL, size) to work */
    mem.size = 0;    /* no data at this point */
    memset(rq.url, 0, sizeof(rq.url));
    memset(rq.src, 0, sizeof(rq.src));
    memset(rq.ident, 0, sizeof(rq.ident));
    memset(rq.method, 0, sizeof(rq.method));

    /* get random number */
    if ((rand = get_random_number()) == -1) {
        syslog(LOG_ERR, "random number failed");
        rand=10; /* default value of rand */
    }

    /* start syslogging */
    openlog("squidclam", LOG_PID, LOG_DAEMON);
    
    /* now sleep some time to distribute the load at startup 
	 * 0.01 < t < 2.55 seconds */ 
    usleep(rand*10000); 

	/* open config */
	cf = opencfg();
	
	/* read config and fill the struct with values */
	cfg = read_cfg(cf);

	/* get random number for the useragent to use */
	if (cfg.hideme > 0) {
		uagent=(rand%11);
	} 
	else {
		uagent=12;
	}

	if (cfg.debug > 0) {
		fprintf(stderr, "\nproxy=%.600s\nurl=%.600s\ntmp=%.600s\nfsize=%i\nclamdserver=%.100s\nnumber=%i\ndebug=%i\nhideme=%i\nerrignore=%i\n",
			cfg.proxy, cfg.url, cfg.tmp, cfg.fsize, cfg.cip, cfg.cp, cfg.debug, cfg.hideme, cfg.errignore);
	}	

	/* close config */
	if (cf != NULL) {
		closecfg(cf);
	}

    /* info that we go up */
    syslog(LOG_INFO,"squidclam starting up");

    /* make stdout line buffered */
    if (setvbuf(stdout, NULL, _IOLBF, 0) != 0) {
        syslog(LOG_ERR, "stdout not line buffered exiting");
        usleep(DTIME); /* don't do DoS */
        return 1;
    }

    /* curl init */
    curl_global_init(CURL_GLOBAL_ALL);
    /* get an easy handle */
    if ((eh = curl_easy_init()) == NULL) {
        syslog(LOG_ERR,"curl_easy_init failed\n");
        usleep(DTIME); /* don't do DoS */
        return 4;
    }

	/* get a object for mkstemp (bad programming) */ 
	strncpy(tt,cfg.tmp,254);
	tt[255] = 0x00;
   
    /* get a filehandle preferable in a ram disk */
    if ((fd = mkstemp(tt)) == -1) {
		syslog(LOG_ERR,"Could not get tempfile handle for template %.255s. Read `man mkstemp`", cfg.tmp);
		if (cfg.debug > 0) {
			fprintf(stderr,"Could not get tempfile handle for template %.255s. Read `man mkstemp`", cfg.tmp);
		}
        usleep(DTIME); /* don't do DoS */
        return 3;
    }

    /* if clamd is running as a different user this is needed */
    /* could be better with files only readable by one group  */
    if (chmod(tt, 0644) != 0) {
        syslog(LOG_ERR,"Could not set tempfile permissions to 0644 (%.255s)", tt);
        usleep(DTIME);
        return 3;
    }

    /* set the proxy */
    curl_easy_setopt(eh, CURLOPT_PROXY, cfg.proxy);
    /* set http version to fool buggy servers */
    curl_easy_setopt(eh, CURL_HTTP_VERSION_1_0, TRUE);
	/* fail on errors to avoid pebcak problems */
	curl_easy_setopt(eh, CURLOPT_FAILONERROR, TRUE);
	/* set useragent */
	curl_easy_setopt(eh, CURLOPT_USERAGENT, useragents[uagent]);
    /* follow location (303 MOVED) */
    curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, TRUE);
    /* only follow 5 redirects */
    curl_easy_setopt(eh, CURLOPT_MAXREDIRS, 5);
    /* use curl to save the data in a ramfile */
    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(eh, CURLOPT_WRITEDATA, (void *)&mem);
    /* get usefull error messages */
    curl_easy_setopt(eh, CURLOPT_ERRORBUFFER, erbuf);
	/* Limit bytes to be downloaded */
	sprintf(slimit,"0-%i",cfg.fsize);
	curl_easy_setopt(eh, CURLOPT_RANGE, slimit);

	
    /* loop to parse the given URLs */
    while (scanf("%10000s %255s %255s %31s", rq.url, rq.src, rq.ident, rq.method) != EOF) {
        /* show which url we're handling */
		if (cfg.debug > 0) {
			syslog(LOG_INFO,"handle url (%.10000s) src=(%.255s) ident=(%.255s) method=(%.31s)\n", rq.url, rq.src, rq.ident, rq.method);
		}
        /* quick "fix" for the POST Problem */
        if (rq.method[0] != 'G') {
			
			if (cfg.debug > 0) {
				syslog(LOG_INFO,"requests other than GET are ignored for now (%10000s)\n", rq.url);
			}

			putchar('\n');
            continue;
        }

        /* clean struct */
        free(mem.memory);
        mem.memory = NULL;
        mem.size = 0;    /* no data at this point */
    
        /* set the url to retrive */
        curl_easy_setopt(eh, CURLOPT_URL, rq.url);

        /* get the file */
        if (curl_easy_perform(eh) != 0) {
            /* if error, give back to squid */
			if (cfg.errignore > 0) {
				putchar('\n');
			}
			else {
				fprintf(stdout,"%.1000s?url=%.10000s&virus=%.255s %.255s %.255s %.31s\n", cfg.url, rq.url, 
                    "squidclam_get_file_failed", rq.src, rq.ident, rq.method);
			}
			if (cfg.debug > 0) {
				syslog(LOG_INFO,"could not get (%.10000s) error was (%.255s)\n", rq.url, erbuf);
            }
			continue;
        }
        
        /* reset file handle to the beginning */
        lseek(fd,SEEK_SET,SEEK_SET);

        /* get a file out of the memory chunk */
        if (write(fd,mem.memory, mem.size) != mem.size) {
			if (cfg.errignore > 0) {
				putchar('\n');
		} else {
            fprintf(stdout,"%.1000s?url=%.10000s&virus=%.255s %.255s %.255s %.31s\n", cfg.url, rq.url, 
                    "squidclam_writing_tempfile_failed", rq.src, rq.ident, rq.method);
		}
            syslog(LOG_INFO,"error writing to tempfile");
            continue;
        }

        /* make sure, there is no extra data in tempfile */
        if (ftruncate(fd, mem.size) != 0) {
			if (cfg.errignore > 0) {
				putchar('\n');
			} 
			else {
				fprintf(stdout,"%.1000s?url=%.10000s&virus=%.255s %.255s %.255s %.31s\n", cfg.url, rq.url, 
						"squidclam_truncate_tempfile_failed", rq.src, rq.ident, rq.method);
			}
            syslog(LOG_INFO,"error truncating tempfile");
            continue;
        }

        /* scan the file with clamd */
        /* we sadly have to use files for clamav to work properly */
        /* but we can put them into a ramdisk or tmpfs ;) */
        /* then we only have to deal with insecure tmpfile creation */
		ret = clamdscan(tt, virname);
		if (ret == 0) {
            /* file seems to be clean */
				putchar('\n');
				if (cfg.debug > 0) {
					syslog(LOG_INFO,"Scanned but no Virus found at: (%.10000s)\n", rq.url);
					syslog(LOG_INFO,"Size of file: %i\n", mem.size);
					}
		}
		else {
			    fprintf(stdout,"%.1000s?url=%.10000s&virus=%.255s %.255s %.255s %.31s\n", cfg.url, rq.url, virname, rq.src, rq.ident, rq.method);
				syslog(LOG_WARNING, "INFECTED url=%.10000s virus=%.255s src=%.255s ident=%.255s method=%.255s",
						rq.url, virname, rq.src, rq.ident, rq.method);
				if (cfg.debug > 0) {
					syslog(LOG_WARNING, "Size of file: %i\n", mem.size);
				}
		}
    }
    
    /* clean up  */
    closelog();
    free(mem.memory);
    curl_global_cleanup();
    unlink(tt);
    return 0;
}
