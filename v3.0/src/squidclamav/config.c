
#include "paths.h"
#include "cfav.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern int bridge_mode;		/* from main.c */
extern int check_content_type;	/* from main.c */
extern int has_regex;		/* from main.c */
extern char *redirect_url;	/* from main.c */
extern int force;		/* from main.c */
extern char *log_file;		/* from main.c */
extern char *proxy;		/* from main.c */
extern char *clamd_ip;		/* from main.c */
extern char *clamd_port;	/* from main.c */
extern char *clamd_local;	/* from main.c */
extern int timeout;		/* from main.c */
extern char *max_size;		/* from main.c */

void load_patterns(void) {
	char buff[MAX_BUFF];
	FILE *fp;
	if((fp=fopen(CONFIG_FILE, "rt"))==NULL) {
		logit(log_file, "unable to open configuration file\n");
		bridge_mode=1;
		return;
	}
	logit(log_file, "Reading configuration from %s\n", CONFIG_FILE);
	while(!bridge_mode &&(fgets(buff, MAX_BUFF, fp)!=NULL)) {
		/* skip blank lines and comments */
		if((strncmp(buff, "#", 1)==0)||(strncmp(buff, "\n", 1)==0))continue;
    
		if(strlen(buff)!=1) {
			/* chop newline */
			buff[strlen(buff)- 1]='\0';
			add_to_patterns(buff);
		}
	}
  	if(redirect_url==NULL) {
    		logit(log_file, "No redirection URL set, going to BRIDGE mode\n");
    		bridge_mode=1;
  	}
  	fclose(fp);
}

void add_to_patterns(char *pattern) {
	char first[MAX_BUFF];
	char second[MAX_BUFF];
	char type[MAX_BUFF];
	char accel[MAX_BUFF];
	regex_t compiled;
	struct REGEX_pattern rpattern;
	int stored;
  
  	/*  The regex_flags that we use are:
      		REG_EXTENDED 
      		REG_NOSUB 
      		REG_ICASE; 
	*/

	int regex_flags=REG_NOSUB; 
	rpattern.type=NORMAL;
	rpattern.case_sensitive=1;
  
	stored=sscanf(pattern, "%s %s %s %s", type, first, second, accel);

 	if((stored < 2)||(stored > 4)) {
		logit(log_file, "unable to get a pair of patterns in add_to_patterns() for [%s]\n", pattern);
		bridge_mode=1;
		return;
	}
  
	if(stored==2) strcpy(second, "");
   
	if(strcmp(type, "redirect")==0) {
		redirect_url=(char *)malloc(sizeof(char)*(strlen(first)+1));
		strcpy(redirect_url, first);
		return;
 	}
   
	if(strcmp(type, "timeout")==0) {
		timeout=atoi(first);
		return;
	}
    
	if(strcmp(type, "force")==0) {
		force=atoi(first);
		return;
	}
    
	if(strcmp(type, "max_size")==0) {
		max_size=atoi(first);
		return;
	}

	if(strcmp(type, "logfile")==0) {
		log_file=(char *)malloc(sizeof(char)*(strlen(first)+1));
		strcpy(log_file, first);
		return;
	}
     
	if(strcmp(type, "proxy")==0) {
		proxy=(char *)malloc(sizeof(char)*(strlen(first)+1));
 		strcpy(proxy, first);
		return;
	}
      
	if(strcmp(type, "clamd_ip")==0) {
		clamd_ip=(char *)malloc(sizeof(char)*(strlen(first)+1));
		strcpy(clamd_ip, first);
		return;
	}
  
	if(strcmp(type, "clamd_port")==0) {
		clamd_port=(char *)malloc(sizeof(char)*(strlen(first)+1));
		strcpy(clamd_port, first);
		return;
	}
    
	if(strcmp(type, "clamd_local")==0) {
		clamd_local=(char *)malloc(sizeof(char)*(strlen(first)+1));
		strcpy(clamd_local, first);
		return;
 	}
  

	if((strcmp(type, "abort")==0)||(strcmp(type, "aborti")==0)) {
		rpattern.type=ABORT;
 	}
  
	if((strcmp(type, "content")==0)||(strcmp(type, "contenti")==0)) {
		rpattern.type=CONTENT;
		check_content_type=1;
	}

	if((strcmp(type, "regexi")==0)||(strcmp(type, "aborti")==0)||(strcmp(type, "contenti")==0)) {
		regex_flags |= REG_ICASE;
		rpattern.case_sensitive=0;
	}

	if((strcmp(type, "regex")==0)||(strcmp(type, "regexi")==0)) {
		has_regex=1;
	}
  
	if(regcomp(&compiled, first, regex_flags)) {
		logit(log_file, "Invalid regex [%s] in pattern file\n", first);
		bridge_mode=1;
		return;
	}
  
	rpattern.cpattern=compiled;
  
	rpattern.pattern=(char *)malloc(sizeof(char)*(strlen(first)+1));
	if(rpattern.pattern==NULL) {
		logit(log_file, "unable to allocate memory in add_to_patterns()\n");
		bridge_mode=1;
		return;
	}
	strcpy(rpattern.pattern, first);

	rpattern.replacement=(char *)malloc(sizeof(char)*(strlen(second)+1));
	if(rpattern.replacement==NULL) {
		logit(log_file, "unable to allocate memory in add_to_patterns()\n");
		bridge_mode=1;
		return;
	}
	strcpy(rpattern.replacement, second);

	/* use accelerator string if it exists */
	if(stored==4) {
		rpattern.has_accel=1;
		rpattern.accel=get_accel(accel, &rpattern.accel_type,rpattern.case_sensitive);
		if(rpattern.accel==NULL) {
			logit(log_file, "unable to allocate memory from get_accel()\n");
			bridge_mode=1;
			return;
		}
	} else {
		rpattern.has_accel=0;
		rpattern.accel=NULL;
	}
	add_to_plist(rpattern);
}

char *get_accel(char *accel, int *accel_type, int case_sensitive) {
	/* returns the stripped accelerator string or NULL 
	if memory can't be allocated 
	converts the accel string to lower case 
	if(case_sensitive)
	
	accel_type is assigned one of the values:
     	#define ACCEL_NORMAL 1
     	#define ACCEL_START  2
     	#define ACCEL_END    3     
	*/
  
	int len, i;
	char *new_accel=NULL;
	*accel_type=0;
  
	len=strlen(accel);
	if(accel[0]=='^') *accel_type=ACCEL_START;
	if(accel[len - 1]=='$') *accel_type=ACCEL_END;
	if(!*accel_type) *accel_type=ACCEL_NORMAL;
	if(*accel_type==ACCEL_START || *accel_type==ACCEL_END) {
    
		/* copy the strings */
		new_accel=(char *)malloc(sizeof(char)* strlen(accel));
		if(new_accel==NULL) return NULL;
		if(*accel_type==ACCEL_START) {
			if(case_sensitive) {
				for(i=0; i < len; i++) new_accel[i]=accel[i+1];
			} else {
				for(i=0; i < len; i++) new_accel[i]=tolower(accel[i+1]);
			}
		}

		if(*accel_type==ACCEL_END) {
			if(case_sensitive) {
				for(i=0; i < len - 1; i++) new_accel[i]=accel[i];
			} else {
				for(i=0; i < len - 1; i++) new_accel[i]=tolower(accel[i]);
			}
			new_accel[i]='\0';
		}
    
	} else {
		new_accel=strdup(accel);
 		if(!case_sensitive) {
			for(i=0; i < len; i++) new_accel[i]=tolower(accel[i]);
			new_accel[i]='\0';
		}
	}
	return new_accel;
}

