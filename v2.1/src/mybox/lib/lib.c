#include <libmybox.h>

// xmalloc -- Exit unless we can allocate memory.
void *xmalloc(size_t size) {
        void *ptr=malloc(size);
        if(!ptr) {
            printf("malloc() Out of memory, exiting\n");
            exit(1);
        }
        return ptr;
}

// xstrndup -- Exit unless we can allocate a copy of this many bytes of string.
void *xstrndup(char *s, size_t n) {
	void *ret = xmalloc(++n);
	strncpy(ret, s, n);
	return ret;
}

// xstrdup -- Exit unless we can allocate a copy of this string.
void *xstrdup(char *s) {
	return xstrndup(s,strlen(s));
}

// flush stdout stream
void xflush_stdout(void) {
	fflush(stdout);
}

// xsystem -- Run command
int xsystem(char *format, ...) {
        va_list va;
        int len, x;
        char *ret, *tty;
        va_start(va, format);
        len = vsnprintf(0, 0, format, va);
        len++;
        va_end(va);
        ret = xmalloc(len);
        va_start(va, format);
        vsnprintf(ret, len, format, va);
        va_end(va);
	tty=ttyname(0);
	freopen("/dev/null","w",stderr);
        x=system(ret);
	freopen(tty,"w",stderr);
	return x;
}

// xstrncpy -- safe version of strncpy 
char *xstrncpy(char *dst, const char *src, size_t size) {
        dst[size-1] = '\0';
	return strncpy(dst, src, size-1);
}

// xwrite -- safe version of write
ssize_t xwrite(int fd, const void *buf, size_t count) {
        ssize_t n;
        do {
                n = write(fd, buf, count);
        } while (n < 0 && errno == EINTR);
        return n;
}

void *xrealloc(void *ptr, size_t size) {
        ptr = realloc(ptr, size);
        if (ptr == NULL && size != 0) {
		printf("malloc() Out of memory, exiting\n");
		exit(1);
	}
        return ptr;
}

// file_exists -- Checks whether a file or directory exists
int file_exists(const char *s) {
        struct stat ss;
        int i=stat(s, &ss);
        if (i < 0) return 0;
        if ((ss.st_mode & S_IFREG) || (ss.st_mode & S_IFLNK) || (ss.st_mode & S_IFDIR)) return(1);
        return(0);
}

// is_dir -- Tells whether the filename is a directory
int is_dir(const char *s) {
        struct stat ss;
        int i=stat(s, &ss);
        if (i < 0) return(0);
        if ((ss.st_mode & S_IFREG) || (ss.st_mode & S_IFDIR)) return(1);
        return(0);
}

// basename -- Returns filename component of path
char *base_name(char **argv) {
	char *s;
	int n, p;
	p=strlen(argv[0]);
	s=basename(argv[0]);
	if(argv[1]!=NULL) {
		n=strlen(argv[1]);
		p=strlen(s);
		if((p>n)&&((strcmp)(s+p-n,argv[1])== 0)) {
			s[p-n]='\0';
		}
        }
	return s;
}

// read_oneline -- Get first line from file
int read_oneline(const char *filename, void *buf) {
        int fd;
        ssize_t ret;
        fd=open(filename, O_RDONLY);
        if(fd < 0) return -1;
        ret=read(fd, buf, 1023);
        ((char *)buf)[ret > 0 ? ret : 0]=0;
        close(fd);
        return ret;
}

// print_file -- Read file content and print to stdout
void print_file(char *fname) {
        char buf[1024];
        FILE *f;
        if(file_exists(fname)) {
                if((f=fopen(fname,"r"))!=NULL) {
                        while(fgets(buf,sizeof(buf),f)!=NULL) {
                                fprintf_stdout("%s",buf);
                        }
                        fclose(f);
                }
        }
	memset(buf,0x0,sizeof(buf));
}

// save_to_file -- Truncate string to file
int save_to_file(const char *filename,char *format, ...) {
        va_list va;
        int len, fd;
        char *buf;
	ssize_t ret;
        va_start(va, format);
        len = vsnprintf(0, 0, format, va);
        len++;
        va_end(va);
        buf = xmalloc(len);
        va_start(va, format);
        vsnprintf(buf, len, format, va);
        va_end(va);
        fd=open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
        if(fd < 0) return -1;
        ret=xwrite(fd, buf, strlen(buf));
        close(fd);
        return ret;
}

// append_to_file -- Append string to file
int append_to_file(const char *filename,char *format, ...) {
        va_list va;
        int len, fd;
        char *buf;
	ssize_t ret;
        va_start(va, format);
        len = vsnprintf(0, 0, format, va);
        len++;
        va_end(va);
        buf = xmalloc(len);
        va_start(va, format);
        vsnprintf(buf, len, format, va);
        va_end(va);
	fd=open(filename, O_RDWR | O_CREAT | O_APPEND, S_IREAD | S_IWRITE);
        if(fd < 0) return -1;
        ret=xwrite(fd, buf, strlen(buf));
        close(fd);
        return ret;
}

// xtouch -- simple create file with current time
int xtouch(const char *file) {
	return save_to_file(file,"");
}

// xmkdir -- make directory
int xmkdir(char *p) {
	long mode=0700;
        mode_t mask;
        char *s=xmalloc(strlen(p) * sizeof(char)); 
        char *path=xmalloc(strlen(p) * sizeof(char)); 
        char c;
        struct stat st;
        path=strdup(p);
        s=path;
        mask=umask(0);
	umask(mask & ~0300);

        do {
		c=0;
		while(*s) {
 			if (*s=='/') {
 				do {
 					++s;
 				} while (*s == '/');
				c = *s;
				*s = 0;
				break;
			}
			++s;
		}
                if(mkdir(path, 0700) < 0) {
                        if(errno != EEXIST || (stat(path, &st) < 0 || !S_ISDIR(st.st_mode))) {
				umask(mask);
                                break;
                        }
                        if(!c) {
				umask(mask);
 				return 0;
 			}
                }

                if(!c) {
			umask(mask);
                        if(chmod(path, mode) < 0) {
				break;
			}
			return 0;
                }
                *s=c;
        } while (1);
        return -1;
}

// clear_screen -- Clear terminal screen
void clear_screen(void) {
        printf("\033[H\033[J");
}

// log_action -- write mybox action log
void log_action(char *type, char *msg) {
	xsystem("/bin/php -f /service/init/misc.init mybox_slog \"%s\" \"%s\"",type,msg);
}

// get_eth -- Get available ethernet device
int get_eth(void) {
        FILE *f;
	int i=0, t=0;
        char buf[1024], dev[10];
	for(t=0;t<MAX_ETH;t++) eth_list[t].name[0]='\0';
        if((f=fopen("/proc/net/dev", "r"))!= NULL) {
                fgets(buf, sizeof(buf), f);
                fgets(buf, sizeof(buf), f);
                while((fgets(buf, sizeof(buf), f) != NULL)) {
                        trim(buf);
			memset(dev,0x0,sizeof(dev));
                        splitc(dev,buf,':');
                        if(!strstr(dev,"eth")) continue;
			strcpy(eth_list[i].name,dev);
			i++;
                }
                fclose(f);
        }
	memset(buf,0x0,sizeof(buf));memset(dev,0x0,sizeof(dev));
	return i;
}

void print_date(void) {
	char buffer[256];
	time_t curtime;
  	struct tm *loctime;
	curtime=time(NULL);
  	loctime=localtime(&curtime);
	strftime(buffer,sizeof(buffer), "%a %b %e %H:%M:%S %Z %Y\n", loctime);
  	fputs(buffer, stdout);
	xflush_stdout();
}

// fprintf_stdout -- print buffer to stdout and flush
void fprintf_stdout(char *format, ...) {
        va_list va;
        int len;
        char *buf;
        va_start(va, format);
        len = vsnprintf(0, 0, format, va);
        len++;
        va_end(va);
        buf = xmalloc(len);
        va_start(va, format);
        vsnprintf(buf, len, format, va);
        va_end(va);
	fprintf(stdout,"%s",buf);
	xflush_stdout();
}

// trim -- Strip whitespace from the beginning and end of a string
char *trim(char *s) {
        size_t len=strlen(s);
        size_t lws;
        while (len && isspace(s[len-1])) --len;
        if(len) {
                lws=strspn(s, " \n\r\t\v\f");
                memmove(s, s + lws, len -= lws);
        }
        s[len]=0;
        return s;
}

// stripshellargv -- Strip shell argument in string
char *stripshellargv(char *string) {
        stripchar(string,'|');
        stripchar(string,',');
        stripchar(string,'`');
        stripchar(string,';');
        stripchar(string,'$');
        return string;
}

// stripchar -- Strip character in string
char *stripchar(char *string, char r) {
        register char *s;
	char *t=xmalloc(1+strlen(string));
        int i, x=0;
        s=string;  
        for(i=0;i<strlen(string);i++) {
                if(string[i]==r) continue;
                t[x]=string[i];
                x++;
        }
        t[x]=0;
        strcpy(s,t);
        return s;
}

// splitc -- Split string
void splitc(char *first, char *rest, char divider) {
	char *p;
   	p=strchr(rest, divider);
   	if(p==NULL) {
      		if((first != rest) && (first != NULL))
	 	first[0]=0;
      		return;
   	}
   	*p=0;
   	if(first != NULL) strcpy(first, rest);
   	if(first != rest) strcpy(rest, p + 1);
}



// split_array -- Split string into array
char split_array(const char *s, const char *delimiters, char ***argvp) {
	int error;
	int i;
	const char *snew;
	char *t;

	if((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
      		errno=EINVAL;
      		return -1;
	}
	*argvp=NULL;                           
	snew=s + strspn(s, delimiters);         
	if ((t=xmalloc(strlen(snew) + 1)) == NULL) return -1; 
	strcpy(t, snew);               
	numtokens=0;
	if(strtok(t, delimiters) != NULL) {  
      		for (numtokens=1; strtok(NULL, delimiters) != NULL; numtokens++) ; 
	}
                            
   	if((*argvp=xmalloc((numtokens + 1)*sizeof(char *))) == NULL) {
      		error=errno;
      		free(t);
      		errno=error;
      		return -1; 
	}

	if(numtokens == 0) {
		free(t);
	} else {
      		strcpy(t, snew);
		**argvp=strtok(t, delimiters);
		for (i=1; i < numtokens; i++) *((*argvp) + i)=strtok(NULL, delimiters);
	} 
	*((*argvp) + numtokens)=NULL;
	return numtokens;
}  

// str_replace,str_replace_add -- Replace all occurrences of the search string with the replacement string
int str_replace_add(char **start, const char *string_add, size_t s) {
        char *tmp_ptr=NULL;
        size_t add_size;
        size_t ss;
        ss=s;
        if(ss == 0) ss=strlen(string_add) + 1;
        if(*start == NULL) {
                *start=xmalloc(ss);
                if(!(*start)) return -1;
                strncpy(*start, string_add, ss);
                if(s != 0) (*start)[ss]='\0';
                return 0;
        }
        add_size=strlen(*start);
        tmp_ptr=realloc(*start, (add_size + ss) + 1);
        if(tmp_ptr){
                *start=tmp_ptr;
                strncpy((*start) + add_size, string_add, ss);
                (*start)[add_size + ss]='\0';
                return 0;
        }
        return -1;
}
char *str_replace(const char *string, const char *search_string, const char *replace_string) {
        char *str=NULL, *pos;
        size_t spos;
        int res=0;

        if((pos=strstr(string, search_string)) == NULL) return NULL;
        spos=pos - string;
        if (spos == 0) {
                res += str_replace_add(&str, replace_string, 0);
                res += str_replace_add(&str, string + strlen(search_string), 0);
        } else{

                res += str_replace_add(&str, string, spos);
                res += str_replace_add(&str, replace_string, 0);
                res += str_replace_add(&str, string + strlen(search_string) + spos, 0);
        }

        if (res == 0) {
                return str;
        }
        free(str);
        return 0;
}

// mk_dev -- create device
int mk_dev(char *file,int major, int minor) {
	if(file_exists(file)) return -1;
	dev_t dev;
	mode_t mode;
	mode= S_IFCHR;
        mode |= 0600;
	dev=(major | minor);
	return mknod(file,mode,dev);
}


