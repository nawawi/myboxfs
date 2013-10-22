
#ifndef LISTS_H

#define LISTS_H
#include <sys/types.h>
#include "regex.h"

#define NORMAL   1
#define CONTENT  2
#define ABORT    3

#define ACCEL_NORMAL 1
#define ACCEL_START  2
#define ACCEL_END    3

struct IP {
	short first;
	short second;
	short third;
};

struct IP_item {
	struct IP address;
	struct IP_item *next;
};


struct REGEX_pattern {
	char *pattern;
	char *replacement;
	int case_sensitive;
	int type;
	int has_accel;
	int accel_type;
	char *accel;
	regex_t cpattern;
};

struct pattern_item {
	struct REGEX_pattern patterns;
	struct pattern_item *next;
};

void init_ip_list(void);
int add_to_ip_list(struct IP);
void free_ip_list(void);

void init_pattern_list(void);
void add_to_plist(struct REGEX_pattern);
void add_to_patterns(char *pattern);
void free_plist(void);

#endif
