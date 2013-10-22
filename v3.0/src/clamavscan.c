/* mod_clamavscan - send all traffic to anti virus scanner clamav
 *
 *
 *
 * Author: Rene Mayrhofer <rene@gibraltar.at>
 * based on code from Kurt Huwig <kurt@openantivirus.org>
 */

// to get strnlen
#define __USE_GNU

#include "squid.h"
#include "module.h"
#include "vscan-common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <clamav.h>

#define MODNAME "mod_clamavscan"

#define TEMPDIR "/var/tmp/squid/"
#define MAX_SCAN_SIZE_DEFAULT 500000
// disable this if every small packet should be scanned (worse performance...)
// you can also set the scanBlockSize parameter at run time to 1 to achieve the
// same effect as disabling this define
// SHIT - this is not secure right now! scanning the whole file and aborting then does not work as expected!
//#define USE_SCANNING_BLOCKS
//#define SCAN_BLOCK_SIZE_DEFAULT 50000

// disable the maximum file size limit for scanning by default
#define MAX_FILE_SIZE_TO_SCAN_DEFAULT -1

#define CLAM_SCAN_OPTIONS CL_SCAN_STDOPT | CL_SCAN_ARCHIVE | CL_SCAN_OLE2 | CL_SCAN_PE | CL_SCAN_HTML

/* This is the Module object - it contains "static" data that should be kept
 over the lifetime of the scanner module, e.g. the clamav root node for its
 internal data. */
classdef(private clamavscanObject, moduleObject, ({
	char* databasePath;
        char* actionPath;
	struct cl_node* root;
	struct cl_stat dbstat;
	struct cl_limits limits;
	hash_table* hash;
	int maxScanSize;
#ifdef USE_SCANNING_BLOCKS
	int scanBlockSize;
#endif
        int maxFileSizeToScan;
}))

/* This is the filter object - it is created and destroyed for each call to
 the filter and should therefore be as lean as possible. */
classdef(private clamavscanFilterObject, filterObject, ({
	char* databasePath;
        char* actionPath;
	struct cl_node** root;
	struct cl_stat* dbstat;
        struct cl_limits* limits;
        hash_table* hash;
	int maxScanSize;
#ifdef USE_SCANNING_BLOCKS
	int scanBlockSize;
#endif
        int maxFileSizeToScan;
}))

#define CLAMAVSCAN_C
#include <classes.dh>

#define DAEMON_MESSAGE "CLAMAV FILTER\n"
#define MSG_BUFFER_LENGTH  256
#define DELIMITER      '\''
#define SEPARATOR      '/'

Ddef_none(clamavscanObject)

Ddef_none(clamavscanFilterObject)

CBDATA_TYPE(clamavscanObject);
CBDATA_TYPE(clamavscanFilterObject);

/* This is the main method of the filter: It is called for every data block
 that passes through the proxy and that matches the filter criteria (which is
 set to * / * MIME types below, i.e. all data.
 The content from buf with length len should be appended to target whenever
 the filter says this is OK.
 Beware: Only return a data block of length "len" (and return this length as
 return value), return 0, or return -1. In all my experiments, it was not
 possible to successfully alter the length of the data block this way, i.e.
 it is not possible to shrink or enlarge the chunk that is appended to
 target. You can alter the content, but the length is either 0 or len.
 Returning -1 tells the proxy to abort this request and cancel the client
 transfer. */
static int cfFilter(filterObject *this, MemBuf *target,
		    const char *buf, int len,
		    const char* content_type, const char* client_addr, const char* auth_user)
{
    int ret, virnum, scan_fd;
    long int scanned_size;
#ifdef USE_SCANNING_BLOCKS
    long int *last_scanned_size;
#endif
    char* virname, *tmpfile;
    char msg[MSG_BUFFER_LENGTH];
    struct stat scan_stat;

    clamavscanFilterObject *cf = CC(this, clamavscanFilterObject);

    debug(93,9) (MODNAME ": called cfFilter with url='%s', client_addr='%s', %d bytes\n", moduleUrlClean(cf->uri), client_addr, len);

    // Only need to scan if we are called with data...
    if (len > 0) {
	// before checking, check if the database has been modified - this might be inefficient in here
	if (cl_statchkdir(cf->dbstat) == 1) {
		debug(93,3) (MODNAME ": database change detected, reloading now\n");
		cl_free(*cf->root);
		*cf->root = NULL;
                virnum = 0;
		ret = cl_loaddbdir(cf->databasePath, cf->root, &virnum);
		if (ret) {
      			debug(93, 1) (MODNAME ": unable to initialize clamav library: cl_loaddbdir error %s\n", cl_strerror(ret));
      			return -1;
		}
		debug(93,9) (MODNAME "1\n");
    		ret = cl_build(*cf->root);
		if (ret) {
		    debug(93, 1) (MODNAME ": unable to construct internal trie: cl_build error %s\n", cl_strerror(ret));
		    return -1;
		}
		else
		    debug(93,9) (MODNAME ": finished contructing internal trie\n");
		debug(93,9) (MODNAME "2\n");
                cl_statfree(cf->dbstat);
		debug(93,9) (MODNAME "3\n");
                cl_statinidir(cf->databasePath, cf->dbstat);
		debug(93,9) (MODNAME ": successfully re-loaded database with %d viruses\n", virnum);
        }

        /* Ok, another addition for better performance: only scan files up to
	 the given maximum file size and skip scanning if the file grows
	 larger. This works (TM) in terms of good performance for large files,
	 but of course lets through any larger files with virii. However, we
	 always scan the first maxFileSizeToScan bytes (but at most maxScanSize
	 bytes with each scanning, i.e. a sliding window of maxScanSize bytes
	 over the whole file). Thus, any virus that is contained in these first
         maxScanSize bytes will be caught. */
	if (cf->maxFileSizeToScan == -1 || scan_stat.st_size <= cf->maxFileSizeToScan) {

  	  // get the file name from the hash and update with the buffer
#ifdef USE_SCANNING_BLOCKS
	  if ((tmpfile = update_file(cf->hash, MODNAME, TEMPDIR, moduleUrlClean(cf->uri), client_addr, buf, len, &last_scanned_size)) == NULL) {
#else
	  if ((tmpfile = update_file(cf->hash, MODNAME, TEMPDIR, moduleUrlClean(cf->uri), client_addr, buf, len, NULL)) == NULL) {
#endif
	      debug(93,0) (MODNAME ": unable to obtain temporary file for scanning\n");
              return -1;
	  }
	  debug(93,3) (MODNAME ": using temporary file '%s' for this request\n", tmpfile);

	  /* This is a small hack for better performance: skip the beginning of
	   the file to scan only the specified amount of bytes. */
	  if ((scan_fd = open(tmpfile, O_RDONLY)) == -1) {
	      debug(93,0) (MODNAME ": unable to open temporary scan file '%s': %s (error %d)\n", tmpfile, strerror(errno), errno);
	      return -1;
	  }
	  if (fstat(scan_fd, &scan_stat)) {
	      debug(93,0) (MODNAME ": unable to stat temporary scan file '%s': %s (error %d)\n", tmpfile, strerror(errno), errno);
	      return -1;
	  }
	  if (scan_stat.st_size > cf->maxScanSize) {
	      if (lseek(scan_fd, scan_stat.st_size - cf->maxScanSize, SEEK_SET) !=
		  scan_stat.st_size - cf->maxScanSize) {
		  debug(93,1) (MODNAME ": unable to skip %ld bytes of temporary scan file '%s': %s (error %d)\n",
			       (scan_stat.st_size - cf->maxScanSize), tmpfile, strerror(errno), errno);
	      }
	  }

	  /* only scan in block sizes (also for better performance), but always
	   scan the complete file before returning it (see below) */
#ifdef USE_SCANNING_BLOCKS
	  if (scan_stat.st_size - *last_scanned_size > cf->scanBlockSize) {
	      *last_scanned_size = scan_stat.st_size;
#endif
	      // cl_scanfile only adds to this number...
	      scanned_size=0;
	      ret = cl_scandesc(scan_fd, &virname, &scanned_size, *cf->root,
				cf->limits, CLAM_SCAN_OPTIONS);
#ifdef USE_SCANNING_BLOCKS
	  }
	  else {
	      /* Hmm - can this be a security risk? Think a bit longer about it.
	       Right now it seems like the client might get the complete virus
	       code, but since we scan the whole file at the end, it shouldn't
	       get a completed download (it should be aborted even when the file
	       is complete). */
	      ret = CL_CLEAN;
	  }
#endif
	  if (close(scan_fd)) {
	      debug(93,0) (MODNAME ": unable to close temporary scan file '%s': %s (error %d)\n", tmpfile, strerror(errno), errno);
	      return -1;
	  }
        }
	else {
	    /* When we are no longer scanning the file because it has grown
	     larger than the set maximum size limit, then there's no point
	     in letting it grow and keeping it around - remove it already
             at this point. */
	    remove_file(cf->hash, MODNAME, moduleUrlClean(cf->uri), client_addr);
	    ret = CL_CLEAN;
	}

	/* Since version XXXX, libclamav even fails to return the number of
         bytes that were scanned successfully.... This just sucks. */
	if (ret == CL_CLEAN /*&& scanned_size == len*/) {
	    // clean - copy to target
	    memBufAppend(target, buf, len);
	    // pass the buffer unmodified
	    debug(93,9) (MODNAME ": %s regarded as clean (scanned %ld bytes until now), returning %d bytes\n", moduleUrlClean(cf->uri), scanned_size, len);
	    return len;
	}
	else {
	    if (ret == CL_VIRUS) {
		// virus detected, return an empty buffer
		debug(93,0) (MODNAME ": %s contains virus %s, blocking %d bytes !\n", moduleUrlClean(cf->uri), virname, len);
                if (cf->actionPath)
                        execute_action(cf->actionPath, MODNAME, moduleUrlClean(cf->uri), virname, content_type, client_addr, auth_user, 0);
		//return 0;
		// do we have a MIME type ?
		if (content_type == NULL || strncmp(content_type, "text/", 5) != 0) {
		    // this closes the connection to the client (browser displays a server error)
		    debug(93,3) (MODNAME ": content type is not text (%s), simply dropping connection\n", content_type ? content_type : "unknown");
		    return -1;
		}
		else {
		    int msglen, i;
		    // try: send an error message
		    debug(93,3) (MODNAME ": content type is text (%s), removing virus code\n", content_type);
		    snprintf(msg, MSG_BUFFER_LENGTH, "<p><b><i>VIRUS '%s' REMOVED BY GIBRALTAR VIRUS PROTECTION</i></b></p>", virname);
		    // retain the original len to not confuse the
		    msglen = strnlen(msg, MSG_BUFFER_LENGTH);
		    if (msglen > len)
			msglen = len;
		    memBufAppend(target, msg, msglen);
		    for (i=msglen; i<len; i++)
			memBufAppend(target, " ", 1);
		    return len;
		}
	    }
	    else {
		// some error
		/*if (scanned_size != len) {
		 debug(93,3) (MODNAME ": error: only scanned %ld out of %d bytes!\n", scanned_size, len);
		 return -1;
		 }
		 else*/ {
		     debug(93,0) (MODNAME "Warning: scanning of buffer failed: cl_scandesc error %s. Letting file pass although it could not be scanned.\n", cl_strerror(ret));
		     memBufAppend(target, buf, len);
                     return len;
		     /*return -1;*/
		 }
	    }
	}
    }
    else if (len == -1) {
	// URL transfer complete, therefore remove the temp file

#ifdef USE_SCANNING_BLOCKS
	/* but scan it completely and abort the transfer even now (when the
	 whole file has already been sent to the client) in case we get an
	 error - DOESN'T WORK AS EXPECTED - THE CLIENT COMPLETES IT'S FILE
         EVEN IF WE ABORT HERE ! */
	if ((tmpfile = query_file(cf->hash, MODNAME, moduleUrlClean(cf->uri), client_addr)) == NULL) {
	    debug(93,0) (MODNAME ": unable to obtain temporary file to finish scanning\n");
            return -1;
	}
	debug(93,3) (MODNAME ": finishing request: scanning temporary file '%s'\n", tmpfile);
	scanned_size=0;
	ret = cl_scanfile(tmpfile, &virname, &scanned_size, *cf->root,
			  cf->limits, CLAM_SCAN_OPTIONS);
	// remove the file anyways...
#endif
	remove_file(cf->hash, MODNAME, moduleUrlClean(cf->uri), client_addr);
#ifdef USE_SCANNING_BLOCKS
	if (ret == CL_CLEAN)
	    // but only return 0 if the complete file has been deemed clean
#endif
	    return 0;
#ifdef USE_SCANNING_BLOCKS
	else if (ret == CL_VIRUS) {
	    debug(93,0) (MODNAME ": %s contains virus %s, blocking file !\n", moduleUrlClean(cf->uri), virname);
	    if (cf->actionPath)
		execute_action(cf->actionPath, MODNAME, moduleUrlClean(cf->uri), virname, content_type, client_addr, auth_user, 0);
            return -1;
	}
	else {
	    debug(93,3) (MODNAME ": scanning of buffer failed: cl_scanfile error %s\n", cl_strerror(ret));
	    return -1;
	}
#endif
    } else
	return 0;
}

static void cfDestroy(void *this)
{
    clamavscanFilterObject *cf = CC(this, clamavscanFilterObject);
    debug(93,9) (MODNAME ": destroy filter\n");
    // scanner dependent
    (void) cf;
}

/* moduleObject->filter for FIL_CONTFILTER:
   filterObject constructor.
*/
static void *clamavscanFilter(clamavscanObject *this, const void *arg)
{
    clamavscanFilterObject *f;
    //const repParam *rp = arg;

    CBDATA_INIT_TYPE(clamavscanFilterObject);
    f = new(clamavscanFilterObject);
    f->filter = cfFilter;
    // not necessary for this filter
    f->destroy = cfDestroy;
    //f->destroy = NULL;

    // scanner dependent
    f->root = &this->root;
    f->dbstat = &this->dbstat;
//    f->limits = &this->limits;
    f->databasePath = this->databasePath;
    f->actionPath = this->actionPath;
    f->hash = this->hash;
    f->maxScanSize = this->maxScanSize;
#ifdef USE_SCANNING_BLOCKS
    f->scanBlockSize = this->scanBlockSize;
#endif
    f->maxFileSizeToScan = this->maxFileSizeToScan;

    return f;
}

static void moduleDestroy(void *this)
{
    clamavscanObject *m = CC(this, clamavscanObject);
    debug(93,9) (MODNAME ": destroy module\n");
    // scanner dependent

    cl_free(m->root);
}

/* This method is called once upon initializing the whole module. */
void moduleInit(const wordlist *args)
{
    clamavscanObject *m;
    int virnum, ret;

    CBDATA_INIT_TYPE(clamavscanObject);
    m = new(clamavscanObject);
    m->chain.typ = FIL_CONTFILTER;
    m->info.version = MODULE_API_VERSION;
    m->description = "clamav virus scanner";
    m->trigger = "*/*";
    m->destroy = moduleDestroy;
    m->filter = clamavscanFilter;
    if (args) {
        m->databasePath = args->key;
        args = args->next;
    } else {
        m->databasePath = cl_retdbdir();
    }
    if (args) {
	if (sscanf(args->key, "%d", &m->maxScanSize) != 1) {
	    debug(93, 0) (MODNAME ": Unable to parse maximum scan size (second parameter), setting to default value %d\n", MAX_SCAN_SIZE_DEFAULT);
            m->maxScanSize = MAX_SCAN_SIZE_DEFAULT;
	}
        args = args->next;
    } else {
        m->maxScanSize = MAX_SCAN_SIZE_DEFAULT;
    }
#ifdef USE_SCANNING_BLOCKS
    if (args) {
	if (sscanf(args->key, "%d", &m->scanBlockSize) != 1) {
	    debug(93, 0) (MODNAME ": Unable to parse scan block size (third parameter), setting to default value %d\n", SCAN_BLOCK_SIZE_DEFAULT);
            m->scanBlockSize = SCAN_BLOCK_SIZE_DEFAULT;
	}
        args = args->next;
    } else {
        m->scanBlockSize = SCAN_BLOCK_SIZE_DEFAULT;
    }
#endif
    if (args) {
	if (sscanf(args->key, "%d", &m->maxFileSizeToScan) != 1) {
	    debug(93, 0) (MODNAME ": Unable to parse maximum file size to scan (third parameter), setting to default value %d\n", MAX_FILE_SIZE_TO_SCAN_DEFAULT);
            m->maxFileSizeToScan = MAX_FILE_SIZE_TO_SCAN_DEFAULT;
	}
        args = args->next;
    } else {
        m->maxFileSizeToScan = MAX_FILE_SIZE_TO_SCAN_DEFAULT;
    }
    if (args) {
        m->actionPath = args->key;
        args = args->next;
    } else {
        m->actionPath = NULL;
    }

    /* Starting from here it is dependent on the virus scanner */
#ifdef USE_SCANNING_BLOCKS
    debug(93,0) (MODNAME ": Initializing clamav virus scanning module with libclamav version %s, using a scan buffer size of %d bytes with blocks of %d bytes and scanning files up to %d bytes size\n", cl_retver(), m->maxScanSize, m->scanBlockSize, m->maxFileSizeToScan);
#else
    debug(93,0) (MODNAME ": Initializing clamav virus scanning module with libclamav version %s, using a scan buffer size of %d bytes and scanning files up to %d bytes size\n", cl_retver(), m->maxScanSize, m->maxFileSizeToScan);
#endif
    virnum=0;
    ret = cl_loaddbdir(m->databasePath, &m->root, &virnum);
    if (ret) {
      debug(93, 1) (MODNAME ": unable to initialize clamav library: cl_loaddbdir error %s\n", cl_strerror(ret));
      debug(91, 1) (MODNAME "database path is %s\n", m->databasePath);
      return;
    }
    debug(93,0) (MODNAME ": successfully loaded database from %s with %d viruses\n", m->databasePath, virnum);
    ret = cl_build(m->root);
    if (ret) {
      debug(93, 1) (MODNAME ": unable to contruct internal trie: cl_build error %s\n", cl_strerror(ret));
      return;
    }
    else
      debug(93,9) (MODNAME ": finished contructing internal trie\n");

    // also initialize the dbstat struct to monitor the database for changes
    memset(&m->dbstat, 0, sizeof(struct cl_stat));
    cl_statinidir(m->databasePath, &m->dbstat);
    debug(93,3) (MODNAME ": initialized database for monitoring\n");

    memset(&m->limits, 0, sizeof(struct cl_limits));
    m->limits.maxfiles = 1000; /* max files */
    m->limits.maxfilesize = 1 * 1048576; /* maximal archived file size = 1 Mb */
    m->limits.maxreclevel = 5; /* maximal recursion level */
    m->limits.maxratio = 200; /* maximal compression ratio */
    m->limits.archivememlim =  1; /* enable memory limit for bzip2 scanner */

    if (!init_hash(&m->hash, MODNAME)) {
	debug(93,1) (MODNAME ": unable to create hash for temporary files\n");
        return;
    }
    debug(93,9) (MODNAME ": created temporary file hash: %p\n", m->hash);

    moduleRegister(CC(m, moduleObject));
    debug(93,3) (MODNAME ": registered filter module\n");
}
