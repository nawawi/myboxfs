#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	if(argc >= 2) {
		int fd;
		if((fd = open(argv[1], O_RDWR)) == -1) {
			printf("Couldn't open device\n");
			return 0;
		}
		dup2(fd, 0);
		dup2(fd, 1);
		dup2(fd, 2);
        }
	if(argc >= 3) {
		execve(argv[2], &argv[2], NULL);
        } else {
		printf("No command\n");
	}
	return 0;
}

