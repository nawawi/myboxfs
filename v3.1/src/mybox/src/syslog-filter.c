#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define SYSLOG_EXE "/service/tools/syslog.exc"

int main(void) {
	char buffer[2048], *pt;
	FILE *p;
	while((pt=fgets(buffer,sizeof(buffer), stdin))!=NULL) {
		if(file_exists(SYSLOG_EXE)) {
        		if((p=popen(SYSLOG_EXE,"w"))!=NULL) {
                		fputs(pt,p);
                		pclose(p);
        		}
		}
	}
	exit(0);
}

