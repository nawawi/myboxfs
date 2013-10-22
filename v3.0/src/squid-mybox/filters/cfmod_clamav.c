/* 
	cfmod_clamavscan - send all traffic to anti virus scanner clamav
	Mohd Nawawi Mohamad Jamili <nawawi@tracenetwork.com.my>

	based on idea:	Kurt Huwig <kurt@openantivirus.org>
 	   		Rene Mayrhofer <rene@gibraltar.at>
*/

#define _GNU_SOURCE
#include "squid.h"
#include "module.h"
#include "patfile.h"
#include "lib.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <clamav.h>

#define MODNAME "cfmod_clamav"
#define ALLOWFILE "/etc/cf/av.allow"
#define DBDIR "/strg/mybox/patterns/av"

#define CFMOD_CLAMAV_C
#include <classes.dh>

#define TEMPDIR "/tmp/scan/"
#define MIN_FILE_SIZE 67
#define MAX_FILE_SIZE 1024
#define CLAM_SCAN_OPTIONS CL_SCAN_STDOPT | CL_SCAN_ARCHIVE | CL_SCAN_OLE2 | CL_SCAN_PE | CL_SCAN_PDF | CL_SCAN_ALGORITHMIC | CL_SCAN_HTML
#define MSG_BUFFER_LENGTH 1024

Ddef_none(clamavscanObject)
Ddef_none(clamavscanFilterObject)

classdef(private clamavscanObject, moduleObject, ({
	char *databasePath;
        char *actionPath;
	struct cl_node* root;
	struct cl_stat dbstat;
	struct cl_limits limits;
	hash_table* hash;
	int min_file_size;
        int max_file_size;
}))

classdef(private clamavscanFilterObject, filterObject, ({
	char *databasePath;
        char *actionPath;
	struct cl_node** root;
	struct cl_stat* dbstat;
        struct cl_limits* limits;
        hash_table* hash;
	int min_file_size;
        int max_file_size;
}))

static int cfFilter(filterObject *this, MemBuf *target,const char *buf, int len,const char *content_type, const char *client_addr, const char *auth_user) {
	int ret, scan_fd, virnum;
	long int scanned_size;
	unsigned const char *virname;
	char *tmpfile;
	char msg[MSG_BUFFER_LENGTH];
	struct stat scan_stat;

 	clamavscanFilterObject *cf=CC(this, clamavscanFilterObject);
	debug(93,9) (MODNAME ": called cfFilter with url='%s', client_addr='%s', %d bytes\n", moduleUrlClean(cf->uri), client_addr, len);

	if(len > 0) {
		if(cl_statchkdir(cf->dbstat)==1) {
			debug(93, 3) (MODNAME ": database change detected, reloading now\n");
			cl_free(*cf->root);
			*cf->root=NULL;
			virnum=0;
			ret=cl_loaddbdir(cf->databasePath, cf->root, &virnum);
			if(ret) {
				debug(93, 9) (MODNAME ": unable to initialize clamav library: cl_loaddbdir error %s\n", cl_strerror(ret));
				return -1;
			}
			debug(93,3) (MODNAME "1\n");
			ret=cl_build(*cf->root);
			if(ret) {
				debug(93, 9) (MODNAME ": unable to construct internal trie: cl_build error %s\n", cl_strerror(ret));
				return -1;
			} else {
				debug(93,9) (MODNAME ": finished contructing internal trie\n");
			}
			debug(93,3) (MODNAME "2\n");
			cl_statfree(cf->dbstat);
			debug(93,3) (MODNAME "3\n");
			cl_statinidir(cf->databasePath, cf->dbstat);
			debug(93,3) (MODNAME ": successfully re-loaded database with %d viruses\n", virnum);
		}

		if(len >= cf->min_file_size && len <= cf->max_file_size) {
			if((tmpfile=update_file(cf->hash, MODNAME, TEMPDIR, moduleUrlClean(cf->uri), client_addr, buf, len, NULL))==NULL) {
				debug(93, 1) (MODNAME ": unable to obtain temporary file for scanning\n");
 				return -1;
			}
			debug(93, 9) (MODNAME ": using temporary file '%s' for this request\n", tmpfile);

			if((scan_fd=open(tmpfile, O_RDONLY))==-1) {
				debug(93, 1) (MODNAME ": unable to open temporary scan file '%s': %s (error %d)\n", tmpfile, strerror(errno), errno);
				return -1;
			}
			if(fstat(scan_fd, &scan_stat)) {
				debug(93, 1) (MODNAME ": unable to stat temporary scan file '%s': %s (error %d)\n", tmpfile, strerror(errno), errno);
				return -1;
			}
			if(scan_stat.st_size > cf->max_file_size) {
				if(lseek(scan_fd, scan_stat.st_size - cf->max_file_size, SEEK_SET)!=scan_stat.st_size - cf->max_file_size) {
					debug(93,1) (MODNAME ": unable to skip %llu bytes of temporary scan file '%s': %s (error %d)\n",(scan_stat.st_size - cf->min_file_size), tmpfile, strerror(errno), errno);
	  			}
			}
			if(scan_stat.st_size == 0) {
				ret=CL_CLEAN;
			} else {
				scanned_size=0;
				ret=cl_scandesc(scan_fd, &virname, &scanned_size, *cf->root,cf->limits, CLAM_SCAN_OPTIONS);
			}
			if(close(scan_fd)) {
				debug(93, 1) (MODNAME ": unable to close temporary scan file '%s': %s (error %d)\n", tmpfile, strerror(errno), errno);
				return -1;
			}
		} else {
			remove_file(cf->hash, MODNAME, moduleUrlClean(cf->uri), client_addr);
			ret=CL_CLEAN;
		}

		if(ret==CL_CLEAN) {
			memBufAppend(target, buf, len);
			remove_file(cf->hash, MODNAME, moduleUrlClean(cf->uri), client_addr);
			//debug(93,3) (MODNAME ": %s regarded as clean (scanned %ld bytes until now), returning %d bytes\n", moduleUrlClean(cf->uri), scanned_size, len);
			debug(93,3) (MODNAME ": %s : CLEAN : Returning %d bytes\n", moduleUrlClean(cf->uri), len);
			return len;
		} else {
			if(ret==CL_VIRUS) {
				remove_file(cf->hash, MODNAME, moduleUrlClean(cf->uri), client_addr);
				//debug(93, 3) (MODNAME ": %s contains virus %s, blocking %d bytes !\n", moduleUrlClean(cf->uri), virname, len);
				debug(93, 3) (MODNAME ": %s : VIRUS %s : Removing %d bytes\n", moduleUrlClean(cf->uri), virname, len);
				if(cf->actionPath) {
					execute_action(cf->actionPath, MODNAME, moduleUrlClean(cf->uri), virname, content_type, client_addr, auth_user, 0);
				}
				int msglen, i;
				snprintf(msg, MSG_BUFFER_LENGTH, "VIRUS '%s' REMOVED BY MYBOX FIREWALL SYSTEM",virname);
				msglen=strnlen(msg, MSG_BUFFER_LENGTH);
				if(msglen > len) msglen=len;
				memBufAppend(target, msg, msglen);
				for(i=msglen; i<len; i++) memBufAppend(target, " ", 1);
				return len;
			} else {
				debug(93,1) (MODNAME "Warning: scanning of buffer failed: cl_scandesc error %s. Letting file pass although it could not be scanned.\n", cl_strerror(ret));
				memBufAppend(target, buf, len);
				return len;
			}
		}
	} else if(len==-1) {
		remove_file(cf->hash, MODNAME, moduleUrlClean(cf->uri), client_addr);
		return 0;
	}
	return 0;
}

static void cfDestroy(void *this) {
	clamavscanFilterObject *cf=CC(this, clamavscanFilterObject);
	debug(93,9) (MODNAME ": destroy filter\n");
	(void) cf;
}

static void *clamavscanFilter(clamavscanObject *this, const void *arg) {
	clamavscanFilterObject *f;
	const repParam *rp = arg;

	if(strstr(rp->uri,myboxuri)) return NULL;
	if(this->patFile && (patfileCheckReload(this->patFile),patfileMatch(this->patFile, rp->uri))) return NULL;
	if(this->patFile && rp->client_addr && (patfileCheckReload(this->patFile),patfileMatch(this->patFile, rp->client_addr))) return NULL;
	if(this->patFile && rp->auth_user && (patfileCheckReload(this->patFile),patfileMatch(this->patFile, rp->auth_user))) return NULL;

 	f=new(clamavscanFilterObject);
	f->filter=cfFilter;
 	f->destroy=cfDestroy;
	f->root=&this->root;
	f->dbstat=&this->dbstat;
	f->databasePath=this->databasePath;
	f->actionPath=this->actionPath;
	f->hash=this->hash;
	f->min_file_size=this->min_file_size;
	f->max_file_size=this->max_file_size;
	return f;
}

static void moduleDestroy(void *this) {
	clamavscanObject *m=CC(this, clamavscanObject);
	debug(93,9) (MODNAME ": destroy module\n");
	cl_free(m->root);
}

void moduleInit(const wordlist *args) {
	clamavscanObject *m;
	patFileObject *pf;
	int virnum, ret;
	m=new(clamavscanObject);
	m->chain.typ=FIL_CONTFILTER;
	m->info.version=MODULE_API_VERSION;
	m->description="clamav virus scanner";
	m->trigger="*/*";
	m->destroy=moduleDestroy;
	m->filter=clamavscanFilter;

	if(__file_exists(ALLOWFILE)) {
		pf=patfileNew(ALLOWFILE,0);
		
	} else {
		pf=NULL;
	}
	m->patFile=REF(pf);

	if(__is_dir(DBDIR)) {
		m->databasePath=DBDIR;
	} else {
		m->databasePath=cl_retdbdir();
	}

	if(args) {
		if(sscanf(args->key, "%d", &m->min_file_size)!=1) {
			m->min_file_size=MIN_FILE_SIZE;
		}
		args=args->next;
	} else {
		m->min_file_size=MIN_FILE_SIZE;
	}

	if(args) {
		if(sscanf(args->key, "%d", &m->max_file_size)!=1) {
			m->max_file_size=MAX_FILE_SIZE;
		}
		args=args->next;
	} else {
		m->max_file_size=MAX_FILE_SIZE;
	}

	if(args) {
		m->actionPath=args->key;
 		args=args->next;
	} else {
		m->actionPath=NULL;
	}


	/* Starting from here it is dependent on the virus scanner */
	debug(93,0) (MODNAME ": Initializing clamav virus scanning module:\n");
	debug(93,0) (MODNAME ": Libclamav version : %s\n",cl_retver());
	debug(93,0) (MODNAME ": Minimum file size : %d byte\n",m->min_file_size);
	debug(93,0) (MODNAME ": Maximum file size : %d byte\n",m->max_file_size);

	virnum=0;
	ret=cl_loaddbdir(m->databasePath, &m->root, &virnum);
	if(ret) {
		debug(93, 1) (MODNAME ": unable to initialize clamav library: cl_loaddbdir error %s\n", cl_strerror(ret));
		debug(93, 3) (MODNAME "database path is %s\n", m->databasePath);
		return;
	}
	debug(93, 9) (MODNAME ": successfully loaded database from %s with %d viruses\n", m->databasePath, virnum);
	ret=cl_build(m->root);
	if(ret) {
		debug(93, 1) (MODNAME ": unable to contruct internal trie: cl_build error %s\n", cl_strerror(ret));
		return;
	} else {
      		debug(93, 3) (MODNAME ": finished contructing internal trie\n");
	}
	memset(&m->dbstat, 0, sizeof(struct cl_stat));
	cl_statinidir(m->databasePath, &m->dbstat);
	debug(93, 3) (MODNAME ": initialized database for monitoring\n");

	memset(&m->limits, 0, sizeof(struct cl_limits));
	m->limits.maxfiles=1000; /* max files */
	m->limits.maxfilesize=1 * 1048576; /* maximal archived file size=1 Mb */
	m->limits.maxreclevel=5; /* maximal recursion level */
	m->limits.maxratio=200; /* maximal compression ratio */
	m->limits.archivememlim= 1; /* enable memory limit for bzip2 scanner */

	if(!init_hash(&m->hash, MODNAME)) {
		debug(93,1) (MODNAME ": unable to create hash for temporary files\n");
		return;
	}
	debug(93,3) (MODNAME ": created temporary file hash: %p\n", m->hash);
	moduleRegister(CC(m, moduleObject));
	debug(93,3) (MODNAME ": registered filter module\n");
}
