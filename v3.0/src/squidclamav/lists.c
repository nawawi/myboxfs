
#include "cfav.h"
#include "paths.h"
#include "log.h"
#include "lists.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>

extern int bridge_mode; /* from main.c */
extern char *log_file; /* from main.c */

struct IP_item *head;
struct pattern_item *phead;


void init_ip_list(void) {
	head = NULL;
}

void init_pattern_list(void) {
 	phead = NULL;
}

int add_to_ip_list(struct IP src_address) {
	struct IP_item *curr;
	struct IP_item *new;
  
	curr = NULL;
	new = NULL;
  
	new = (struct IP_item *)malloc(sizeof(struct IP_item));
	if(new == NULL) {
		logit(log_file, "unable to allocate memory in add_to_ip_list()\n");
		bridge_mode = 1;
		return 1;
	}
  
  
	new->address.first  = src_address.first;
	new->address.second = src_address.second;
	new->address.third  = src_address.third;
	new->next = NULL;
  
	if(head == NULL) {
		head = new;
	} else {
		for(curr = head; curr->next != NULL; curr = curr->next);
		curr->next = new;
 	}
	return 0;
}

void add_to_plist(struct REGEX_pattern pattern) {
	struct pattern_item *curr;
	struct pattern_item *new;
  
	curr = NULL;
 	new = NULL;
  
	/* two strings are already allocated in the "pattern" struct
	argument to this function */
  
	new = (struct pattern_item *)malloc(sizeof(struct pattern_item));
	if(new == NULL) {
		logit(log_file, "unable to allocate memory in add_to_plist()\n");
		bridge_mode = 1;
		return;
	}
  
	new->patterns.pattern = pattern.pattern;
	new->patterns.replacement = pattern.replacement;
	new->patterns.type = pattern.type;
	new->patterns.has_accel = pattern.has_accel;
	new->patterns.accel = pattern.accel;
	new->patterns.accel_type = pattern.accel_type;
	new->patterns.case_sensitive = pattern.case_sensitive;
  
	/* not sure whether we need to copy each item in the struct */
	new->patterns.cpattern = pattern.cpattern;
	new->next = NULL;
  
	if(phead == NULL) {
 		phead = new;
	} else {
		for(curr = phead; curr->next != NULL; curr = curr->next);
		curr->next = new;
	}
}

void free_ip_list(void) {
	struct IP_item *prev;
	struct IP_item *next;
  
	prev = NULL;
	next = NULL;
  
  	if(head != NULL) {
		next = head->next;
		free(head);
	}
  
	for(prev = next; next != NULL; ) {
		next = prev->next;
		free(prev);
		prev = next;
	}
	head = NULL;
}

void free_plist(void) {
	struct pattern_item *prev;
	struct pattern_item *next;
  
	prev = NULL;
	next = NULL;
  
	if(phead != NULL) {
		next = phead->next;
		free(phead);
	}
  
	for(prev = next; next != NULL; ) {
		next = prev->next;
    
		free(prev->patterns.pattern);
		free(prev->patterns.replacement);
		if(prev->patterns.accel) free(prev->patterns.accel);
    
		free(prev);
		prev = next;
	}
	phead = NULL;
}









