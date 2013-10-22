/*

 (C) Copyright 2002,2003 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.

 This program is not free software; you cannot redistribute it and/or
 modify without permission copyright owner.

 This code is protected by copyright law and international treaties. 
 Unauthorized reproduction or distribution of this program, or any portion of it, 
 may result in severe civil and criminal penalties, and will be prosecuted to the 
 maximum extent possible under the law.

 $Id: strings.c,v 1.00 2003/07/28 1:07 AM nawawi Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

char rbuf[100];
char nbr[8192];
char version[250];
char space[]="  													";

void rmspace(char *x) {
	char *t;
   	for(t=x+strlen(x)-1; (((*t=='\x20')||(*t=='\x0D')||(*t=='\x0A')||(*t=='\x09'))&&(t >= x)); t--);
   	if(t!=x+strlen(x)-1) *(t+1)=0;
   	for(t=x; (((*t=='\x20')||(*t=='\x0D')||(*t=='\x0A')||(*t=='\x09'))&&(*t)); t++);
   	if(t!=x) strcpy(x,t);
}

const char *randstring(int length) {
	char *po;
    	int i;
    	po=rbuf;
    	if(length >100) length = 100;
    	for(i=0;i<length;i++) { *po=(char)(0x61+(rand()&15)); po++; }
    	*po=0x0;
    	po=rbuf;
    	return po;
}

int replace(char *rps,char whatc, char toc) {
	char *p1;    
   	p1=strchr(rps,whatc);
   	while(p1) {
      		*p1=toc;
      		p1++;
      		p1=strchr(p1,whatc);
   	}
}

int ucase (char *inc) {
   	char *po;
   	for (po = inc;*po;po++) *po = toupper( *po );
   	return 0x0;
}

char *strmncpy(char *dest, char *source, unsigned int len) {
	char bf[strlen(source)+2];
    	char *pt;
    	if(dest==NULL | source==NULL) return NULL;
    	memcpy(&bf[0],source,strlen(source)+1);
    	pt=strncpy(dest,bf,len-1);
    	if(strlen(source)+1>=len)
	dest[len-1]=0;
    	return pt;
}

char *rtrim(char *totrim) {
    	int smlen;
    	char *pnt;
    	pnt = totrim;
    	smlen = strlen(totrim);
    	if(smlen>4095) smlen = 4095;
    	pnt=pnt+smlen;
    	while (*pnt == ' ' || *pnt== 0x0) { 
		*pnt=0x0;
		if(pnt==totrim) {
	    		break;
		} else {
	    		pnt--;
		} 
    }
    return totrim;
}

char *nobreak(char *tobreak) {
	int smlen;
    	char *pnt;
    	pnt=strchr(tobreak,'\r');
    	if(pnt==NULL) pnt=strchr(tobreak,'\n');
    	if(pnt != NULL) {
       		smlen = pnt-tobreak;
       		smlen++;
       		if(smlen > 8191) smlen=8191;
       		strmncpy(nbr,tobreak,smlen);
       		pnt=nbr;
    	} else {
      		pnt=tobreak;
    	}
    	return rtrim(pnt);
}

void splitc(char *first, char *rest, char divider) {
	char *p;
   	p=strchr(rest, divider);
   	if(p==NULL) {
      		if((first != rest) && (first != NULL))
	 	first[0] = 0;
      		return;
   	}
   	*p=0;
   	if(first != NULL) strcpy(first, rest);
   	if(first != rest) strcpy(rest, p + 1);
}

int file_exist(char *s) {
        struct stat ss;
        int i = stat(s, &ss);
        if (i < 0) return 0;
        if ((ss.st_mode & S_IFREG) || (ss.st_mode & S_IFLNK)) return(1);
        return(0);
}


// busybox - reset / clear
void reset_screen(void) {
	if(isatty(0) || isatty(0) || isatty(0)) {
        	/* See 'man 4 console_codes' for details:
                 * "ESC c"                      -- Reset
                 * "ESC ( K"            -- Select user mapping
                 * "ESC [ J"            -- Erase display
                 * "ESC [ 0 m"          -- Reset all Graphics Rendition display attributes
                 * "ESC [ ? 25 h"       -- Make cursor visible.
                 */
                printf("\033c\033(K\033[J\033[0m\033[?25h");
        }
}

void clear_screen(void) {
        printf("\033[H\033[J");
}

void get_network(char *ip, char *mask, char *ret) {
        FILE *f;
        char buf[150], pp[150], cmd[150];
	snprintf(cmd,sizeof(cmd),"/bin/ipcalc -n %s %s |cut -d '=' -f 2",ip,mask);
        if((f=popen(cmd,"r"))!=NULL) {
		if(fgets(buf,sizeof(buf),f)!=NULL) {
			sprintf(ret,"%s",buf);
		}
		 pclose(f);
        }
	rmspace(ret);
}

void get_bcast(char *ip, char *mask, char *ret) {
        FILE *f;
        char buf[150], pp[150], cmd[150];
	snprintf(cmd,sizeof(cmd),"/bin/ipcalc -b %s %s |cut -d '=' -f 2",ip,mask);
        if((f=popen(cmd,"r"))!=NULL) {
		if(fgets(buf,sizeof(buf),f)!=NULL) {
			sprintf(ret,"%s",buf);
		}
		 pclose(f);
        }
	rmspace(ret);
}

void get_prefix(char *ip, char *mask, char *ret) {
        FILE *f;
        char buf[150], pp[150], cmd[150];
	snprintf(cmd,sizeof(cmd),"/bin/ipcalc -p %s %s |cut -d '=' -f 2",ip,mask);
        if((f=popen(cmd,"r"))!=NULL) {
		if(fgets(buf,sizeof(buf),f)!=NULL) {
			sprintf(ret,"%s",buf);
		}
		 pclose(f);
        }
	rmspace(ret);
}
