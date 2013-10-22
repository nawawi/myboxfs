#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
	int sock;
	int bread;
	struct sockaddr_un server;
	char buf[512];
	char sockname[512]="/var/spool/sophos";
	char filename[1024];
	int ret=0;
	if(argc!=2) {
		printf("Usage: %s  <filename>\n", argv[0]);
		exit(-1);
	}

	memset(filename, 0, sizeof(filename));
	strncpy(filename, argv[1], sizeof(filename)-1);
	if(argv[2]!=NULL) {
		strncpy(sockname, argv[1], sizeof(sockname)-1);
	}
	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sock < 0) {
		perror("socket");
		exit(-1);
	}
	
	server.sun_family = AF_UNIX;
	strncpy(server.sun_path, sockname, sizeof(server.sun_path)-1);

	if(connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
		perror("connect");
		close(sock);
		exit(-1);
	}

	strncat(filename, "\n", sizeof(filename)-1);
	if(write(sock, filename, strlen(filename)) < 0) {
		perror("write");
		close(sock);
		exit(-1);
	}
	
	memset(buf, 0, sizeof(buf));
	if((bread = read(sock, buf, sizeof(buf))) > 0) {
		if(strchr(buf, '\n')) *strchr(buf, '\n') = '\0';
		if(strchr(filename, '\n')) *strchr(filename, '\n') = '\0';
		if (buf[0] == '1') {
			char *vname = buf+2;
			printf("%s\n",vname);
			ret=1;
		} else if (!strncmp(buf, "-1", 2)) {
			char *errmsg = buf+3;
			printf("ERROR: %s\n", errmsg);
			ret=-1;
		} else {
			printf("OK: %s\n", filename);
			ret=0;
		}
	} else {
		printf("read() from the socket failed\n");
		ret=-1;
	}
	close(sock);
	exit(ret);
}
