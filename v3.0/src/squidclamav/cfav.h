
#ifndef CFAV_H
#ifndef LISTS_H
#include "lists.h"
#endif

#define CFAV_H 1
#include <stdarg.h>
#define MAX_BUFF 8193

struct IN_BUFF {
	char url[8193];
	char src_address[255];
	char ident[255];
	char method[31];
};

int load_in_buff(char *, struct IN_BUFF *);

char *replace_string (struct pattern_item *, char *);
int match_accel(char *url, char *accel, int accel_type, int case_sensitive);
int pattern_compare(char *);
int count_parenthesis (char *);
int get_ip(char *buff, struct IP *src_address);
void load_patterns(void);
char *get_accel(char *accel, int *accel_type, int case_sensitive);

#endif








