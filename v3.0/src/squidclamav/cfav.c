#include "paths.h"
#include "cfav.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

extern struct pattern_item *phead;        /* from lists.c */
extern struct IP_item *head;              /* from lists.c */
extern int sig_hup;                       /* from main.c  */
extern char *log_file;                    /* from main.c  */


/* load the stdin for the redirector into
   an IN_BUFF structure 
   
   Sets in_buff.url to "" if the fields can't
   be converted
*/

int load_in_buff(char *buff, struct IN_BUFF *in_buff) {
	int converted;
	struct IP address;
  
	strcpy(in_buff->url, "");
	strcpy(in_buff->src_address, "");
	strcpy(in_buff->ident, "");
	strcpy(in_buff->method, "");
  
	converted=sscanf(buff, "%8193s %255s %255s %31s\n", in_buff->url,in_buff->src_address, in_buff->ident, in_buff->method);
  
	if(converted!=4) {
		logit(log_file, "%d=incorrect number of scanned fields\n", converted);
		return(1);
	}
  

	/* check the source IP address */
	if(strcmp(in_buff->src_address, "")==0) {
		logit(log_file, "in_buff.src_address is NULL in main()\n");
		return(1);
	}

	/* all methods must be GET */
	if((strcmp(in_buff->method, "")==0)||(strcmp(in_buff->method, "GET"))) {
		return(1);
	}

	/* handle the error condition in which 4 arguments not parsed,
	in which we just print buff to stdout */
	if(strlen(in_buff->url)<=4) {
		logit(log_file, "strlen in_buff->url=[%d] in main()\n",strlen(in_buff->url));
		return(1);
	}

	/* check that the IP source address supplied is valid */
	if(get_ip(in_buff->src_address,&address)) {
		logit(log_file, "in_buff.src_address not valid\n");
		return(1);
	}

	return(0);
}

/* returns 1 for a match, 0 if no match found */
int pattern_compare(char *url) {
	struct pattern_item *curr;
	char *new_string;
	curr=NULL;
  

	for(curr=phead; curr!=NULL; curr=curr->next) {
    
		/* return -1 if string matches ABORT pattern */
		/* return(1) if string matches REGEX pattern */
		/* return(0) otherwise */
    
		/* Now we must test for abort|aborti matching */
		if(regexec(&curr->patterns.cpattern, url, 0, 0, 0)==0) {
			new_string=strdup(curr->patterns.replacement);
			if(new_string!=NULL) {
				free(new_string);
				if(curr->patterns.type==ABORT) {
					return -1;
				} else {
					return(1);
				}
			}
		}
 	}
	return(0);
}


int match_accel(char *url, char *accel, int accel_type, int case_sensitive) {
	/* return(1) if url contains accel */
	int i, offset;
	char l_accel[MAX_BUFF];
	int accel_len;
	int url_len;

	if(accel_type==ACCEL_NORMAL) {
		if(case_sensitive) {
			for(i=0; url[i]!='\0'; i++) l_accel[i]=url[i];
		} else {
			/* convert to lower case */
			for(i=0; url[i]!='\0'; i++) l_accel[i]=tolower(url[i]);
		}
		l_accel[i]='\0';
    
		if(strstr(url, l_accel)) {
			return(1);
		} else {
			return(0);
		}
	}
  
  
	if(accel_type==ACCEL_START) {
		accel_len=strlen(accel);
		url_len=strlen(url);
		if(url_len < accel_len) return(0);
    
		if(case_sensitive) {
			for(i=0; i < accel_len; i++) {
				if(accel[i]!=url[i]) return(0);
			}
		} else {
			for(i=0; i < accel_len; i++) {
				if(accel[i]!=tolower(url[i])) return(0);
 			}
		}
 		return(1);
	}
  
	if(accel_type==ACCEL_END) {
		accel_len=strlen(accel);
 		url_len=strlen(url);
		offset=url_len - accel_len;
    
		if(offset < 0) return(0);
    
		if(case_sensitive) {
			for(i=0; i < accel_len; i++) {
				if(accel[i]!=url[i+offset]) return(0);
			}
		} else {
			for(i=0; i < accel_len; i++) {
				if(accel[i]!=tolower(url[i+offset])) return(0);
			}
		}
		return(1);
	}
  
	/* we shouldn't reach this section! */
	return(0);
}


char *replace_string(struct pattern_item *curr, char *url) {
	char buffer[MAX_BUFF];
	char *replacement_string=NULL;
	regmatch_t match_data[10];
	int parenthesis;
	char *in_ptr;
	char *out_ptr;
	int replay_num;
	int count;
  
  
	/* Perform the regex call */
	if(regexec(&(curr->patterns.cpattern), url, 10, &match_data[0], 0)!=0) return(NULL);
  
	/* Ok, setup the traversal pointers */
	in_ptr=curr->patterns.replacement;
 	out_ptr=buffer;
  
	/* Count the number of replays in the pattern */
 	parenthesis=count_parenthesis(curr->patterns.pattern);
	if(parenthesis < 0) {
		/* Invalid return value - don't log because we already have done it */
 		return(NULL);
	}
  
	/* Traverse the url string now */
	while(*in_ptr!='\0') {
		if(isdigit(*in_ptr)) {
			/* We have a number, how many chars are there before us? */
			switch(in_ptr - curr->patterns.replacement) {
			case 0:
			/* This is the first char
			Since there is no backslash before hand, this is not
			a pattern match, so loop around */
			{
				*out_ptr=*in_ptr;
				out_ptr++;
				in_ptr++;
				continue;
			}
			break;
			case 1:
			/* Only one char back to check, so see if it is a backslash */
			if(*(in_ptr - 1)!='\\') {
				*out_ptr=*in_ptr;
				out_ptr++;
				in_ptr++;
				continue;
			}
			break;
			default:
				/* Two or more chars back to check, so see if the previous is
				a backslash, and also the one before. Two backslashes mean
				that we should not replace anything! */
				if((*(in_ptr - 1)!='\\')||((*(in_ptr - 1)=='\\')&&(*(in_ptr - 2)=='\\'))) {
					*out_ptr=*in_ptr;
					out_ptr++;
					in_ptr++;
					continue;
				}
			}
      
			/* Ok, if we reach this point, then we have found something to
			replace. It also means that the last time we went through here,
			we copied in a backslash char, so we should backtrack one on
			the output string before continuing */
 			out_ptr--;
      
			/* We need to convert the current in_ptr into a number for array
			lookups */
			replay_num=(*in_ptr)- '0';
      
			/* Now copy in the chars from the replay string */
			for(count=match_data[replay_num].rm_so; count < match_data[replay_num].rm_eo; count++) {
				/* Copy in the chars */
				*out_ptr=url[count];
				out_ptr++;
 			}
      
 			/* Increment the in pointer */
			in_ptr++;
		} else {
      			*out_ptr=*in_ptr;
      			out_ptr++;
     			in_ptr++;
		}
    
    
		/* Increment the in pointer and loop around */
		/* in_ptr++; */
	}
  
	/* Terminate the string */
 	*out_ptr='\0';
  
	/* The return string is done, return to the caller */
	replacement_string=strdup(buffer);
	return(replacement_string);
}

int count_parenthesis(char *pattern) {
	int lcount=0;
	int rcount=0;
	int i;
  
	/* Traverse string looking for( and)*/
	for(i=0; i < strlen(pattern); i++) {
		/* We have found a left( */
		if(pattern[i]=='\(') {
			/* Do not count if there is a backslash */
			if((i!=0)&&(pattern [i-1]=='\\')) {
				continue;
			} else {
				lcount++;
			}
      		}
 		if(pattern[i]==')') {
			if((i!=0)&&(pattern [i-1]=='\\')) {
				continue;
			} else {
				rcount++;
			}
		}
	}
  
	/* Check that left==right */
	if(lcount!=rcount) return(-1);
	return(lcount);
}


int get_ip(char *src_addr, struct IP *src_address) {
	char *ptr;
	int address;
	int i;
	char s[strlen(src_addr)+ 1];

	strcpy(s, src_addr);
  
	/* split up each number from string */
	ptr=strtok(s, ".");
	if(ptr==NULL) {
		return(1);
	}
  
	/* make sure we have numbers and dots only! */
	if(strspn(s, "0123456789.")!=strlen(s)) return(1);
  
	address=atoi(ptr);
 	if(address < 0 || address > 255) {
		return(1);
	}
	src_address->first=address;
  
	for(i=2; i < 4; i++) {
		ptr=strtok(NULL, ".");
		if(ptr==NULL) {
			return(1);
		}
    
		/* make sure we have numbers and dots only! */
		if(strspn(s, "0123456789.")!=strlen(s)) return(1);
    
		address=atoi(ptr);
		if(address < 0 || address > 255) return(1);
		if(i==2) src_address->second=address;
		if(i==3) src_address->third=address;
	}
 	return(0);
}

