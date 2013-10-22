#include "paths.h"
#include "cfav.h"
#include "log.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void logit(char *filename, char *format, ...) {
	FILE *log;
	char *date_str = NULL;
	char msg[MAX_BUFF];
	va_list args;

	// don't log if no file created
  	if((log = fopen(filename, "r")) == NULL) return;
  
  	va_start(args, format);
  	if(vsprintf(msg, format, args) > (MAX_BUFF - 1)) return;
	va_end(args);
    	if((log = fopen(filename, "at")) == NULL) return;
	date_str = get_date();
	fprintf(log, "%s [%d]:%s", date_str, (int)getpid(), msg);
    	fflush(log);
    	free(date_str);
    	fclose(log);
}

char *get_date(void) {
	time_t tp;
	char *ascitime;
	char *s;
	tp = time(NULL);
	ascitime = ctime(&tp);
	s = (char *)malloc(sizeof(char) * (strlen(ascitime)+1));
	/* no use writing an error message, because this function
                will keep getting called! */
	if(s == NULL) exit(3); 
	strcpy(s, ascitime);
  	s[strlen(ascitime) - 1] = '\0';
	return s;
}
