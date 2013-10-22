#define TRUE  1
#define FALSE 0
#include "paths.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ctype.h>
#include "lists.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sysexits.h>
#include <errno.h>
#include <curl/curl.h>
#include <signal.h>
#include "cfav.h"

int dconnect(void);
void replace(char string[], char *from, char *to);

void sighup();			/* routine to reload virus database on SIGHUP signal */
void sigterm();		/* routine to quit with cleaning on SIGTERM signal(kill -15) */

int bridge_mode=0;
int check_content_type=0;
int has_regex=0;

/* Configuration options */
char *redirect_url=NULL;
int force=1;
char *log_file=LOG_FILE;
char *proxy=MY_PROXY;
char *clamd_local=NULL;
char *clamd_ip=CLAMD_SERVER;
char *clamd_port=CLAMD_PORT;
int max_size=FSIZE;
int timeout=60;
int wsockd;

struct IN_BUFF in_buff;

const char *useragents="Mozilla/4.61 [en] (X11; U; ) - Mybox Firewall System)";

void sigalarm(int sig) {
	logit(log_file, "Timeout waiting for clamd response\n");
	exit(3);
}

/* Get download as stream  */
size_t write_data(void *ptr, size_t data_size, size_t nmemb, void *stream) {
	int ret;
	int msize=data_size * nmemb;
	ret=write(wsockd, ptr, msize);
  	if(ret <= 0) {
		close(wsockd);
    	}
	return(msize);
}

int main(int argc, char **argv) {
	char buff[MAX_BUFF];
	int buff_status=0;

	CURL *eh=NULL;
	double usize=0;
	char *p=NULL;
	char err[CURL_ERROR_SIZE];
	int found=0;
	unsigned slimit[255];
	char cbuff[4096];
	char urlredir[MAX_BUFF];
	int nbread=0;
	int sockd;
	int loopw=60, port;
	struct sockaddr_in server;
	struct sockaddr_in peer;
	int peer_size;
	char *pt;
	int force_scan=0;
	char ubuff[MAX_BUFF]; /* 'Stated' url */

	/* make stdout line buffered */
  	if(setvbuf(stdout, NULL, _IOLBF, 0)!=0) {
		fprintf(stderr, "stdout not line buffered exiting\n");
		usleep(1000000);		/* don't do DoS */
      		exit(2);
    	}

	bridge_mode=0;
	init_ip_list();
	init_pattern_list();
  	load_patterns();

	signal(SIGHUP, sighup);	/* set function calls on signal trap */
	signal(SIGTERM, sigterm);

	if(bridge_mode) logit(log_file, "Invalid condition - continuing in BRIDGE mode\n");
	logit(log_file, "Mybox cfav (PID %d) started\n",(int) getpid());

  	if(!bridge_mode) {
		/* curl init */
      		curl_global_init(CURL_GLOBAL_ALL);
		/* get an easy handle */
		if((eh=curl_easy_init())==NULL) {
	  		logit(log_file, "Curl easy initialization failed\n");
	  		curl_global_cleanup();
	  		bridge_mode=1;
	  		logit(log_file, "Invalid condition - continuing in BRIDGE mode\n");
		} else {
			/* set the curl output to temporary file */
			curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, &write_data);
			/* set the error buffer */
			curl_easy_setopt(eh, CURLOPT_ERRORBUFFER, err);
			/* set the libcurl transfer timeout */
			curl_easy_setopt(eh, CURLOPT_TIMEOUT, timeout);
			/* do not install signal  handlers in thread context */
			curl_easy_setopt(eh, CURLOPT_NOSIGNAL, 1);
			/* Force HTTP 1.0 version */
			curl_easy_setopt(eh, CURL_HTTP_VERSION_1_0, TRUE);
			curl_easy_setopt(eh, CURLOPT_FAILONERROR, FALSE);
			/* set useragent */
			curl_easy_setopt(eh, CURLOPT_USERAGENT, useragents);
			/* follow location(303 MOVED) */
			curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, TRUE);
			/* only follow 3 redirects */
	  		curl_easy_setopt(eh, CURLOPT_MAXREDIRS, 3);
			/* Limit bytes to be downloaded */
			sprintf(slimit,"0-%i",max_size);
			curl_easy_setopt(eh, CURLOPT_RANGE, slimit);		
		}
	}

  	memset(ubuff, 0, sizeof(ubuff));
  	memset(buff, 0, sizeof(buff));
  	while(fgets(buff, MAX_BUFF, stdin)!=NULL) {

      		/* if configs are completely invalid or some other
         	exception occurs where we want the redirector to
         	continue operation(so that Squid still works!),
         	we simply echo stdin to stdout - i.e. "bridge mode" :-) */
      
		if(bridge_mode) {
	  		fprintf(stdout, "\n");
	  		continue;
		}

      		/* separate the four fields
         	from the single input line 
         	of stdin */

	 	buff_status=load_in_buff(buff, &in_buff);
		memset(buff, 0, sizeof(buff));

      		/* if four fields couldn't be separated, or the
         	converted values aren't appropriate, then
         	just echo back the line from stdin */

		if(buff_status==1) {
			fprintf(stdout, "\n");
	  		continue;
		}

      		/* now we can compare the URL to virus scan rules */
      		found=pattern_compare(in_buff.url);
      		/* -1 mean abort match => No antivir check */
      		/* if there's a regex and no content type pattern search and the regex
        	pattern is not found => No antivir check */

		if((found==-1)||((check_content_type==0) &&(has_regex==1) &&(found==0))) {
			/* no replacement for the URL was found */
			fprintf(stdout, "\n");
			continue;
		} else {
			/* set no proxy */
			curl_easy_setopt(eh, CURLOPT_PROXY, "");
	  		/* set the url */
	  		curl_easy_setopt(eh, CURLOPT_URL, in_buff.url);
	  		/* only get the header to check size and content type */
	  		curl_easy_setopt(eh, CURLOPT_NOBODY, TRUE);

	  		/* download the header */
	  		if(curl_easy_perform(eh)!=0) {
	        		if(force==0) {
		      			fprintf(stdout, "\n");
		      			continue;
				} else {
		    			force_scan=1;
				}
	    		}
	  		if(force_scan==0) {
	  			/* get the Content Length from the header */
	  			if(curl_easy_getinfo(eh, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &usize) !=CURLE_OK) {
		      			/* not ok --> squid should handle this */
		      			logit(log_file, "No content length from url %s\n", in_buff.url);
		      			fprintf(stdout, "\n");
		      			continue;
	    			}

	  			/* get the CONTENT_TYPE out of the header */
	  			if(check_content_type==1) {
	      				if(curl_easy_getinfo(eh, CURLINFO_CONTENT_TYPE, &p) != CURLE_OK) {
		  				/* not ok --> squid should handle this */
		  				logit(log_file, "No content type from url %s\n",in_buff.url);
		  				fprintf(stdout, "\n");
		  				continue;
	      				}
	      				if((p==NULL) ||(pattern_compare(p)==0)) {
		  				/* no replacement for the URL was found */
		  				fprintf(stdout, "\n");
		  				continue;
					}
	    			}
			}

			/***** Open a socket to Clamd daemon *****/
			if((sockd=dconnect()) < 0) {
				logit(log_file, "Can't connect to Clamd daemon\n");
				fprintf(stdout, "\n");
				return(-1);
			}

			if(write(sockd, "STREAM", 6) <= 0) {
				logit(log_file, "Can't write to Clamd socket.\n");
				fprintf(stdout, "\n");
				return(-1);
			}
	
			memset(cbuff, 0, sizeof(cbuff));
			while(loopw) {
				read(sockd, cbuff, sizeof(cbuff));
	      			if((pt=strstr(cbuff, "PORT"))) {
					pt += 5;
					sscanf(pt, "%d", &port);
					break;
				}
				loopw--;
			}
			if(!loopw) {
				logit(log_file, "Clamd daemon not ready for stream scanning.\n");
				fprintf(stdout, "\n");
				return(-1);
			}
			/* connect to clamd given port */
			if((wsockd=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				logit(log_file, "Can't create the Clamd socket.\n");
				fprintf(stdout, "\n");
				return(-1);
			}

			server.sin_family=AF_INET;
			server.sin_port=htons(port);

			peer_size=sizeof(peer);
			if(getpeername(sockd,(struct sockaddr *) &peer, &peer_size) < 0) {
				logit(log_file, "Can't get socket peer name.\n");
				fprintf(stdout, "\n");
				return(-1);
			}
			switch(peer.sin_family) {
				case AF_UNIX:
					server.sin_addr.s_addr=inet_addr("127.0.0.1");
					break;
				case AF_INET:
					server.sin_addr.s_addr=peer.sin_addr.s_addr;
	      				break;
	    			default:
	      				logit(log_file, "Unexpected socket type: %d.\n",peer.sin_family);
	      			return(-1);
			}
			if(connect(wsockd,(struct sockaddr *) &server,sizeof(struct sockaddr_in)) < 0) {
				close(wsockd);
				logit(log_file, "Can't connect to clamd [port: %d].\n", port);
				fprintf(stdout, "\n");
				return(-1);
			}
	
			/***** now get the body content *****/

			/* set proxy */
			if(strcmp(proxy,"none")!=0) {
				curl_easy_setopt(eh, CURLOPT_PROXY, proxy);
			}
			/* download body */
			curl_easy_setopt(eh, CURLOPT_NOBODY, FALSE);
			curl_easy_setopt(eh, CURLOPT_HTTPGET, TRUE);
			if(curl_easy_perform(eh)!=0) {
				logit(log_file, "Error when downloading url %s\n", in_buff.url);
				logit(log_file, "CURLOPT_ERRORBUFFER: %s\n", err);
				fprintf(stdout, "\n");
				continue;
			}

			close(wsockd);
			while((nbread=read(sockd, cbuff, sizeof(cbuff))) > 0) {
				if(strstr(cbuff, "FOUND\n")) {
					sprintf(urlredir, "%s?url=%s&source=%s&user=%s&virus=%s",redirect_url,in_buff.url,in_buff.src_address,in_buff.ident,cbuff);
					logit(log_file, "Redirecting URL to: %s\n", urlredir);
					replace(urlredir, " ", "+");
					strcpy(in_buff.url, urlredir);
				}
				memset(cbuff, 0, sizeof(cbuff));
			}
			close(sockd);
			if(strlen(urlredir) > 0) {
				fprintf(stdout, "%s %s %s %s\n", in_buff.url,in_buff.src_address, in_buff.ident, in_buff.method);
			} else {
				fprintf(stdout, "\n");
			}
			memset(urlredir, 0, sizeof(urlredir));
		}
	}

	/* cleanup */
	curl_global_cleanup();
	free(redirect_url);
	free_ip_list();
	free_plist();
	return(0);
}

void sighup() {
	signal(SIGHUP, sighup);	/* reset signal */
	/* simply terminate the program, squid should reload it */
	/* cleanup */
	curl_global_cleanup();
	free(redirect_url);
	free_ip_list();
	free_plist();
	exit(0);
}

void sigterm() {
	signal(SIGTERM, sigterm);	/* reset signal */
	logit(log_file, "Sigterm received, cleaning all before quit...\n");
	/* cleanup */
	curl_global_cleanup();
	free(redirect_url);
	free_ip_list();
	free_plist();
	exit(0);
}

int dconnect() {
	struct sockaddr_un userver;
	struct sockaddr_in server;
	struct hostent *he;
	int asockd;

	memset((char *) &userver, 0, sizeof(userver));
	memset((char *) &server, 0, sizeof(server));

	if(clamd_local!=NULL) {
		userver.sun_family=AF_UNIX;
		strncpy(userver.sun_path, clamd_local, sizeof(userver.sun_path));
		if((asockd=socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			logit(log_file, "Can't bind local socket on %s.\n", clamd_local);
			return(-1);
		}
		if(connect(asockd,(struct sockaddr *) &userver,sizeof(struct sockaddr_un)) < 0) {
			close(asockd);
			logit(log_file, "Can't connect to clamd on local socket %s.\n", clamd_local);
			return(-1);
		}
	} else {
		server.sin_addr.s_addr=inet_addr(clamd_ip);
		if((asockd=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			logit(log_file, "Can't create the socket.\n");
			return(-1);
		}

		server.sin_family=AF_INET;
		server.sin_port=htons(atoi(clamd_port));

		if((he=gethostbyname(clamd_ip))==0) {
			close(asockd);
			logit(log_file, "Can't lookup clamd hostname.\n");
			return(-1);
		}

		server.sin_addr=*(struct in_addr *) he->h_addr_list[0];
	
		if(connect(asockd,(struct sockaddr *) &server,sizeof(struct sockaddr_in)) < 0) {
			close(asockd);
			logit(log_file, "Can't connect to clamd.\n");
			return(-1);
		}
	}
	return(asockd);
}

void replace(char string[], char *from, char *to) {
	char *start, *p1, *p2;
	for(start=string; *start!='\0'; start++) {
		p1=from;
		p2=start;
		while(*p1!='\0') {
			if(*p1!=*p2) break;
			p1++;
			p2++;
		}
		if(*p1=='\0') {
			for(p1=to; *p1!='\0'; p1++) *start++=*p1;
		}
	}
}
