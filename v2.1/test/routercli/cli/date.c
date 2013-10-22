#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "util.c"


/* This 'date' command supports only 2 time setting formats,
   all the GNU strftime stuff (its in libc, lets use it),
   setting time using UTC and displaying int, as well as
   an RFC 822 complient date output for shell scripting
   mail commands */

/* Input parsing code is always bulky - used heavy duty libc stuff as
   much as possible, missed out a lot of bounds checking */

/* Default input handling to save suprising some people */
const char *bb_opt_complementaly;

typedef struct
{
    unsigned char opt;
    char list_flg;
    unsigned long switch_on;
    unsigned long switch_off;
    unsigned long incongruously;
    void **optarg;              /* char **optarg or llist_t **optarg */
} t_complementaly;
						
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


/* The new stuff for LRP */

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

int main(int argc, char **argv)
{
	char *date_str = NULL;
	char *date_fmt = NULL;
	char *t_buff;
	int set_time;
	int utc;
	int use_arg = 0;
	time_t tm;
	unsigned long opt;
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
			date_fmt = (opt & DATE_OPT_RFC2822 ?
					(utc ? "%a, %d %b %Y %H:%M:%S GMT" :
					"%a, %d %b %Y %H:%M:%S %z") :
					"%a %b %e %H:%M:%S %Z %Y");

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
	puts(t_buff);

	return EXIT_SUCCESS;
}
