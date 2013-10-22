#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define TASKQ_EXE "/service/tools/taskq_exec.exc"
#define TASKQ_PATH "/var/sys/taskq"

static int check_request(void) {
        DIR *dir;
        struct dirent *entry;
	int check=0;
        if((dir = opendir(TASKQ_PATH)) == NULL) return(check);
        for(;;) {
                if((entry = readdir(dir)) == NULL) {
                        closedir(dir);
                        return(check);
                }
                if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")) continue;
                check++;
		return(check);
        }
        return(check);
}

int main(void) {
	chdir("/");
        for(;;) {
		if(check_request() > 0) {
			if(file_exists(TASKQ_EXE)) {
				system(TASKQ_EXE);
			}
		}
		sleep(1);
	}
}
