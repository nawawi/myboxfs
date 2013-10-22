/*
 * Utility routines.
 *
 * Copyright (C) tons of folks.  Tracking down who wrote what
 * isn't something I'm going to worry about...  If you wrote something
 * here, please feel free to acknowledge your work.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Based in part on code from sash, Copyright (c) 1999 by David I. Bell 
 * Permission has been granted to redistribute this code under the GPL.
 *
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <utime.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdlib.h>
#include <crypt.h>
#include <linux/kernel.h>
#include <linux/sys.h>
#include <sys/time.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <wait.h>
#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../lib/ext.h"

int showversion(void)
{
    const long minute = 60;
    const long hour = minute * 60;
    const long day = hour * 24;
    const double megabyte = 1024 * 1024;
    struct sysinfo si;
    struct utsname u;
    uname (&u);
    sysinfo(&si);
    printf("RouterCLI (tm) 2 Software (Version %s) (c) 2004 Alexander Eremin <xyzyx@rambler.ru>\n",VERSION);
    printf("%s release %s (version %s) on %s\n",u.sysname,
    u.release,u.version,u.machine);
    printf("Router uptime %ld days, %ld:%02ld:%02ld\n",
    si.uptime / day, (si.uptime % day) / hour,
    (si.uptime % hour) / minute, si.uptime % minute);
    printf("total RAM %5.1f MB\n",si.totalram / megabyte);
    printf("free  RAM %5.1f MB\n",si.freeram / megabyte);
    printf("process count %d\r\n",si.procs);
    return 0;
        
}
int showtime(void) {	
    struct timeval tv;
    struct tm* ptm;
    char time_string[40];
    long milliseconds;
    gettimeofday(&tv,NULL);
    ptm=localtime(&tv.tv_sec);
    strftime(time_string,sizeof(time_string),"%a %b %d  %H:%M:%S",ptm);
    milliseconds=tv.tv_usec / 1000;
    printf(" %s.%03ld\r\n",time_string,milliseconds);
    return 0;
}

#define SIZE 256

int tm(void) {
    char buffer[SIZE];
    time_t curtime;
    struct tm *loctime;
    curtime = time (NULL);
    loctime = localtime (&curtime);
    strftime (buffer, SIZE, "%I:%M:%S", loctime);
    fputs (buffer, stdout);
    return 0;
}

extern HIST_ENTRY **history_list ();
int showhist (void) {
    HIST_ENTRY **list;
    register int i;
    list = history_list ();
    if (list) {
	for (i = 0; list[i]; i++)
	   printf ("  %s\r\n",list[i]->line);
    }
    return 0;
}

char enpass[50];


char *cli_chomp(char *str)
{
    int t=0;
    char*c;
    for (c=str;c[0]!='\0';c++)
    if (isspace(c[0])) t++;
    else c[0-t] = c[0];
    c[-t] = '\0'; 
    return c;
}

char *dupstr (s)
char *s;
{
    char *r;
    r = xmalloc (strlen (s) + 1);
    strcpy (r, s);
    return (r);
}
const char *memory_exhausted = "memory exhausted\n";

FILE *xfopen(const char *path, const char *mode)
{
	FILE *fp;
	if ((fp = fopen(path, mode)) == NULL)
	{
	    printf("%s", path);
	    exit (0);
	}
	return fp;
}

/* this should really be farmed out to libbusybox.a */
extern void *xmalloc(size_t size)
{
	void *ptr = malloc(size);
	if (!ptr)
	{
	    printf(memory_exhausted);
	    exit (0);
	}
	return ptr;
}

/* Like strncpy but make sure the resulting string is always 0 terminated. */  
char * safe_strncpy(char *dst, const char *src, size_t size)
{   
	dst[size-1] = '\0';
	return strncpy(dst, src, size-1);   
}

char dns[40];
int resolve(void)
{
    char *token;
    char line[MAX_LINE_LEN + 1] ;
    FILE *fp = fopen( "/etc/resolv.conf", "r" ) ;
    if (fp==NULL)
    {
	fprintf(stderr,"%% Error getting nameserver\n");
	return (-1);
    } else {
	while( fgets( line, MAX_LINE_LEN, fp ) != NULL )
	{
	    token = strtok( line, " " ) ;
	    if( token != NULL && strcmp(token,"nameserver") ==0 )
	    {
    		token = strtok( NULL, "\t\n\r" ) ;
		strcpy(dns,token ) ;
	    }
	}
    }
    return (0);
}

int intverify(char *ifname)
{
    struct ifreq ifr;
    int fd;
    fd = socket(AF_INET,SOCK_DGRAM, 0);
       if (fd >= 0) 
       {
    	    strcpy(ifr.ifr_name, ifname);
	    if (ioctl(fd, SIOCGIFFLAGS, &ifr) != 0) 
	    {
		//printf("%% Invalid interface\n");    
		return (1);
	    }	    
      }
      close(fd );
    return (0);
}

int secret(int sec)
{
    unsigned long seed[2];
    char salt[] = "$1$........";
    const char *const seedchars =
		    "./0123456789ABCDEFGHIJKLMNOPQRST"
	    	    "UVWXYZabcdefghijklmnopqrstuvwxyz";
    char *password;
    int i;
    seed[0] = time(NULL);
    seed[1] = getpid() ^ (seed[0] >> 14 & 0x30000);
    for (i = 0; i < 8; i++)
    salt[3+i] = seedchars[(seed[i/5] >> (i%5)*6) & 0x3f];
    password = crypt(getpass("Password:"), salt);
    strcpy(enpass,password);
    return 0;
}
int 
check_secret(void)
{
    char *result;
    int ok;
    result = crypt(getpass("Password:"), enpass);
    ok = strcmp (result, enpass) == 0;
    if (!ok) printf ("%% Bad secrets\r\n");
    return ok ? 0 : 1;
    
}
			 
// date						
static struct tm *date_conv_time(struct tm *tm_time, const char *t_string)
{
	int nr;

	nr = sscanf(t_string, "%2d%2d%2d%2d%d", &(tm_time->tm_mon),
				&(tm_time->tm_mday), &(tm_time->tm_hour), &(tm_time->tm_min),
				&(tm_time->tm_year));

	if (nr < 4 || nr > 5) {
		printf("%% Invalid date\r\n");
	}

	/* correct for century  - minor Y2K problem here? */
	if (tm_time->tm_year >= 1900) {
		tm_time->tm_year -= 1900;
	}
	/* adjust date */
	tm_time->tm_mon -= 1;

	return (tm_time);

}


static struct tm *date_conv_ftime(struct tm *tm_time, const char *t_string)
{
	struct tm t;

	/* Parse input and assign appropriately to tm_time */

	if (t =
		*tm_time, sscanf(t_string, "%d:%d:%d", &t.tm_hour, &t.tm_min,
						 &t.tm_sec) == 3) {
		/* no adjustments needed */
	} else if (t =
			   *tm_time, sscanf(t_string, "%d:%d", &t.tm_hour,
								&t.tm_min) == 2) {
		/* no adjustments needed */
	} else if (t =
			   *tm_time, sscanf(t_string, "%d.%d-%d:%d:%d", &t.tm_mon,
								&t.tm_mday, &t.tm_hour, &t.tm_min,
								&t.tm_sec) == 5) {
		/* Adjust dates from 1-12 to 0-11 */
		t.tm_mon -= 1;
	} else if (t =
			   *tm_time, sscanf(t_string, "%d.%d-%d:%d", &t.tm_mon,
								&t.tm_mday, &t.tm_hour, &t.tm_min) == 4) {
		/* Adjust dates from 1-12 to 0-11 */
		t.tm_mon -= 1;
	} else if (t =
			   *tm_time, sscanf(t_string, "%d.%d.%d-%d:%d:%d", &t.tm_year,
								&t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min,
								&t.tm_sec) == 6) {
		t.tm_year -= 1900;	/* Adjust years */
		t.tm_mon -= 1;	/* Adjust dates from 1-12 to 0-11 */
	} else if (t =
			   *tm_time, sscanf(t_string, "%d.%d.%d-%d:%d", &t.tm_year,
								&t.tm_mon, &t.tm_mday, &t.tm_hour,
								&t.tm_min) == 5) {
		t.tm_year -= 1900;	/* Adjust years */
		t.tm_mon -= 1;	/* Adjust dates from 1-12 to 0-11 */
	} else {
		printf("%% Invalid date\r\n");
	}
	*tm_time = t;
	return (tm_time);
}

#define DATE_OPT_RFC2822	0x01
#define DATE_OPT_SET    	0x02
#define DATE_OPT_UTC    	0x04
#define DATE_OPT_DATE   	0x08
#define DATE_OPT_REFERENCE	0x10
#ifdef CONFIG_FEATURE_DATE_ISOFMT
# define DATE_OPT_TIMESPEC	0x20
#endif

int setclock(int argc, char **argv)
{
	char *date_str = NULL;
	char *date_fmt = NULL;
	char *t_buff;
	int set_time;
	int utc;
	int use_arg = 0;
	time_t tm;
	unsigned long opt = 0;
	struct tm tm_time;
	char *filename = NULL;


	set_time = opt & DATE_OPT_SET;
	utc = opt & DATE_OPT_UTC;
	if ((utc) && (putenv("TZ=UTC0") != 0)) {
		printf("%% Memory error\r\n");
	}
	use_arg = opt & DATE_OPT_DATE;
	if(opt & 0x80000000UL)
		printf("%% Incomplete command\r\n");

	if ((date_fmt == NULL) && (optind < argc) && (argv[optind][0] == '+')) {
		date_fmt = &argv[optind][1];	/* Skip over the '+' */
	} else if (date_str == NULL) {
		set_time = 1;
		date_str = argv[optind];
	}

	/* Now we have parsed all the information except the date format
	   which depends on whether the clock is being set or read */

	if(filename) {
	//	struct stat statbuf;
	//	if(stat(filename,&statbuf))
	//		printf("File '%s' not found.\n",filename);
	//	tm=statbuf.st_mtime;
	} else time(&tm);
	memcpy(&tm_time, localtime(&tm), sizeof(tm_time));
	/* Zero out fields - take her back to midnight! */
	if (date_str != NULL) {
		tm_time.tm_sec = 0;
		tm_time.tm_min = 0;
		tm_time.tm_hour = 0;

		/* Process any date input to UNIX time since 1 Jan 1970 */
		if (strchr(date_str, ':') != NULL) {
			date_conv_ftime(&tm_time, date_str);
		} else {
			date_conv_time(&tm_time, date_str);
		}

		/* Correct any day of week and day of year etc. fields */
		tm_time.tm_isdst = -1;	/* Be sure to recheck dst. */
		tm = mktime(&tm_time);
		if (tm < 0) {
			 printf("%% Invalid date\r\n");
			 
		}
		if (utc && (putenv("TZ=UTC0") != 0)) {
			 printf("%% Memory error\r\n");
			 
		}

		/* if setting time, set it */
		if (set_time && (stime(&tm) < 0)) {
			printf("%% Cannot set date\r\n");
		}
	}

	/* Display output */

	/* Deal with format string */
	if (date_fmt == NULL) {
			date_fmt = (opt & DATE_OPT_RFC2822 ? (utc ? "%a, %d %b %Y %H:%M:%S GMT" : "%a, %d %b %Y %H:%M:%S %z") : "%a %b %e %H:%M:%S %Z %Y");

	} else if (*date_fmt == '\0') {
		/* Imitate what GNU 'date' does with NO format string! */
		printf("\n");
		return EXIT_SUCCESS;
	}

	/* Handle special conversions */

	if (strncmp(date_fmt, "%f", 2) == 0) {
		date_fmt = "%Y.%m.%d-%H:%M:%S";
	}

	/* Print OUTPUT (after ALL that!) */
	t_buff = xmalloc(201);
	strftime(t_buff, 200, date_fmt, &tm_time);
	printf("Clock set to %s\n",t_buff);

	return EXIT_SUCCESS;
}
