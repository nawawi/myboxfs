#ifndef LOG_H
#define LOG_H
#ifdef __GNUC__
void logit(char *, char *, ...) __attribute__ ((format (printf, 2, 3)));
#else
void logit(char *, char *, ...);
#endif
char *get_date(void);
#endif
