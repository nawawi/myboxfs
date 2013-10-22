/* 	Helper for all cfmod_* filters.
	Author: Nawawi <nawawi@tracenetwork.com.my>
		Rene Mayrhofer <rene@gibraltar.at>
*/

#define _GNU_SOURCE
#include "squid.h"
#include "module.h"
#include "lib.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

int __file_exists(const char *s) {
        struct stat ss;
        int i=stat(s, &ss);
        if (i < 0) return 0;
        if ((ss.st_mode & S_IFREG) || (ss.st_mode & S_IFLNK) || (ss.st_mode & S_IFDIR)) return(1);
        return(0);
}

int __is_dir(const char *s) {
        struct stat ss;
        int i=stat(s, &ss);
        if(i < 0) return(0);
        if((ss.st_mode & S_IFREG) || (ss.st_mode & S_IFDIR)) return(1);
        return(0);
}

int __mkdir(const char *p) {
        long mode=0700;
        mode_t mask;
        char *s=xmalloc(strlen(p) * sizeof(char)); 
        char *path=xmalloc(strlen(p) * sizeof(char)); 
        char c;
        struct stat st;
        strcpy(path,p);
        s=path;
        mask=umask(0);
        umask(mask & ~0300);

        do {
                c=0;
                while(*s) {
                        if(*s=='/') {
                                do {
                                        ++s;
                                } while (*s=='/');
                                c=*s;
                                *s=0;
                                break;
                        }
                        ++s;
                }
                if(mkdir(path, 0700) < 0) {
                        if(errno !=EEXIST || (stat(path, &st) < 0 || !S_ISDIR(st.st_mode))) {
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

int execute_action(char *actcmd, const char *modname,char *url,const char *virname,const char *ctype,const char *clientip,const char *username,int wait) {
	pid_t child;
	int ret;
	char *cmdArgs[7];
	cmdArgs[0]=actcmd;
	cmdArgs[1]=url; //url
	cmdArgs[2]=virname; //virus name
	cmdArgs[3]=ctype; // HTTP reply content type
	cmdArgs[4]=clientip; //client IP
	cmdArgs[5]=username; //authenticated user
	cmdArgs[6]=NULL;

	child=fork();
	if(child==-1) {
		debug(93,3) ("%s: error: could not fork action\n", modname);
		return -1;
	} else if(child==0) {
		// child
		fclose(stdout);
		fclose(stdin);
		fclose(stderr);
		execv(actcmd, cmdArgs);
		debug(93,3) ("%s: error: could not exec action: %d\n", modname, errno);
		return 0;
	} else {
		// parent
		if(wait) {
			waitpid(child, &ret, 0);
			if(WIFEXITED(ret)) {
				return WEXITSTATUS(ret);
			} else {
 				debug(93,3) ("%s: error: child exited abnormally\n", modname);
				return -1;
			}
		}
		return 0;
 	}
}

typedef struct _tmpfile_hash_link tmpfile_hash_link;

struct _tmpfile_hash_link {
	hash_link hash;
	char *filename;
	long int lastscannedsize;
};

int init_hash(hash_table **hash, const char *modname) {
 	if((*hash=hash_create((HASHCMP *) strcmp, 229, hash4)) < 0) {
		debug(93, 1) ("%s: critical error, can't create hash!\n", modname);
		return 0;
	}
	return 1;
}

static tmpfile_hash_link* query_hash(hash_table* hash, const char *modname,const char * const url,const char * const client_addr) {
	tmpfile_hash_link* lp;
	char *key;
	int keylen;
	// first create the key
	keylen=strnlen(url, 1024) + strnlen(client_addr, 1024);
	if((key=(char*) xmalloc (sizeof(char) * (keylen + 1)))==NULL) return NULL;
	strncpy(key, url, keylen);
	strncat(key, client_addr, keylen);
	debug(93, 9) ("%s: using key '%s' for lookup\n", modname, key);

	lp=hash_lookup(hash, key);
	xfree(key);
 	return lp;
}

/* Updates a temp file with a new data block (or creates it if it does not
 exist yet. Returns the temporary file name. */
/* We need to use both the URL and the client_addr for checking, if two
 clients retrieve the same file, it would clash. However, this is pretty
 inefficent - FIXME (i.e. make me deal with multiple downloads of the same
 URL). */
const char * const update_file(hash_table *hash, const char *modname,const char * const tempdir,const char * const url,const char * const client_addr,const char * const buf,int len,long int **lastscannedsize) {
	FILE* f;
 	char *key, *tmpfile;
	tmpfile_hash_link* lp;
	int keylen;

	// first create the key
	keylen=strnlen(url, 1024) + strnlen(client_addr, 1024);
	if((key=(char*) xmalloc (sizeof(char) * (keylen + 1)))==NULL) return NULL;
	strncpy(key, url, keylen);
	strncat(key, client_addr, keylen);

	// do we already know that combination?
	if((lp=hash_lookup(hash, key))==NULL) {
		// nope, create a new temp file
		if(__is_dir(tempdir)==0) __mkdir(tempdir);
		tmpfile=tempnam(tempdir, "file");
		if(tmpfile==NULL) {
			debug(93,3) ("%s: error: could not create temporary file name\n", modname);
			xfree(key);
			return NULL;
		}
		// insert into the hash
		lp=xmalloc(sizeof(tmpfile_hash_link));
		lp->hash.key=xstrdup(key);
		lp->filename=xstrdup(tmpfile);
		lp->lastscannedsize=0;
		xfree(tmpfile);
		hash_join(hash, &lp->hash);
	}
	xfree(key);

 	// here we are sure that the file name has been set (in lp->filename)
	// care about file permissions!
 	umask(0077);
 	f=fopen(lp->filename, "ab");
	if(f==NULL) return NULL;
	if(buf[0]!=' ') fwrite(buf, 1, len, f);
	fclose(f);
 	if(lastscannedsize !=NULL) *lastscannedsize=&lp->lastscannedsize;
	return lp->filename;
}

void remove_file(hash_table *hash, const char *modname,const char * const url,const char * const client_addr) {
	tmpfile_hash_link* lp;
	if((lp=query_hash(hash, modname, url, client_addr)) !=NULL) {
		hash_remove_link(hash, (hash_link*) lp);
		unlink(lp->filename);
		xfree((void*) lp->hash.key);
		xfree((void*) lp->filename);
		xfree((void*) lp);
	}
}
