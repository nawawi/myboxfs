#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
    pid_t pid;
    
    pid = fork();
    if (pid == 0) {
        int fd = open("/dev/watchdog", O_WRONLY);
        time_t t;

        if (fd == -1) {
	    perror("watchdog");
	    exit(1);
        }
        while (1) {
  	    write(fd, "\0", 1);
	    sleep(1);
//	    t = time(NULL);
//	    printf("%s", ctime(&t));
        }

    } 
    exit(0);
}
