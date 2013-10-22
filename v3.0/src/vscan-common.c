/* Helper for all virus scan filters.
 *
 *
 *
 * Author: Rene Mayrhofer <rene@gibraltar.at>
 */


#include "squid.h"
#include "module.h"
#include "vscan-common.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int execute_action(const char* actionCmd, const char* moduleName,
		   const char* url, const char* virname,
                   const char* contentType,
		   const char* clientAddr, const char* userName,
		   int wait) {
    pid_t child;
    int ret;
    char* cmdArgs[7];

    cmdArgs[0] = actionCmd;
    cmdArgs[1] = url; //url
    cmdArgs[2] = virname; //virus name
    cmdArgs[3] = contentType; // HTTP reply content type
    cmdArgs[4] = clientAddr; //client IP
    cmdArgs[5] = userName; //authenticated user
    cmdArgs[6] = NULL;

    child = fork();
    if (child == -1) {
	debug(93,3) ("%s: error: could not fork action\n", moduleName);
	return -1;
    }
    else if (child == 0) {
	// child
	fclose(stdout);
	fclose(stdin);
	fclose(stderr);
	execv(actionCmd, cmdArgs);
	debug(93,3) ("%s: error: could not exec action: %d\n", moduleName, errno);
        return 0;
    }
    else {
	// parent
	if (wait) {
	    waitpid(child, &ret, 0);

            if (WIFEXITED(ret)) {
                return WEXITSTATUS(ret);
            }
            else {
                debug(93,3) ("%s: error: child exited abnormally\n", moduleName);
                return -1;
	    }
	}
	else
            return 0;
    }
}

typedef struct _tmpfile_hash_link tmpfile_hash_link;

struct _tmpfile_hash_link {
    hash_link hash;
    char* filename;
    long int lastScannedSize;
};

int init_hash(hash_table** hash, const char* moduleName)
{
    if ((*hash = hash_create((HASHCMP *) strcmp, 229, hash4)) < 0) {
	debug(93, 1) ("%s: critical error, can't create hash!\n", moduleName);
        return 0;
    }
    debug (93,9) ("%s: created temp file hash\n", moduleName);
    return 1;
}

static tmpfile_hash_link* query_hash(hash_table* hash, const char* moduleName,
				     const char * const url,
				     const char * const client_addr)
{
    tmpfile_hash_link* lp;
    char *key;

    int keylen;
    // first create the key
    keylen = strnlen(url, 1024) + strnlen(client_addr, 1024);
    if ((key = (char*) xmalloc (sizeof(char) * (keylen + 1))) == NULL) {
	debug(93, 1) ("%s: unable to acquire temporary memory (tried to get %d chars)\n", moduleName, keylen+1);
        return NULL;
    }
    strncpy(key, url, keylen);
    strncat(key, client_addr, keylen);
    debug(93, 9) ("%s: using key '%s' for lookup\n", moduleName, key);

    lp = hash_lookup(hash, key);
    xfree(key);
    return lp;
}

const char * const query_file(hash_table* hash, const char* moduleName,
			      const char * const url,
			      const char * const client_addr)
{
    return query_hash(hash, moduleName, url, client_addr)->filename;
}

/* Updates a temp file with a new data block (or creates it if it does not
 exist yet. Returns the temporary file name. */
/* We need to use both the URL and the client_addr for checking, if two
 clients retrieve the same file, it would clash. However, this is pretty
 inefficent - FIXME (i.e. make me deal with multiple downloads of the same
 URL). */
const char * const update_file(hash_table* hash, const char* moduleName,
                               const char * const tempdir,
			       const char * const url,
			       const char * const client_addr,
			       const char * const buf,
			       int len,
			       long int ** lastScannedSize)
{
    FILE* f;
    char* tmpfile, *key;
    tmpfile_hash_link* lp;
    int keylen;

    debug(93, 9) ("%s: called update_file with url='%s', client_addr='%s' and %d bytes\n", moduleName, url, client_addr, len);

    // first create the key
    keylen = strnlen(url, 1024) + strnlen(client_addr, 1024);
    if ((key = (char*) xmalloc (sizeof(char) * (keylen + 1))) == NULL) {
	debug(93, 1) ("%s: unable to acquire temporary memory (tried to get %d chars)\n", moduleName, keylen+1);
        return NULL;
    }
    strncpy(key, url, keylen);
    strncat(key, client_addr, keylen);
    debug(93, 9) ("%s: using key '%s' for lookup\n", moduleName, key);

    // do we already know that combination?
    if ((lp = hash_lookup(hash, key)) == NULL) {
        // nope, create a new temp file
	tmpfile = tempnam(tempdir, "vscan");
	if (tmpfile == NULL) {
	    debug(93,3) ("%s: error: could not create temporary file name\n", moduleName);
	    xfree(key);
            return NULL;
	}
	// insert into the hash
        lp = xmalloc(sizeof(tmpfile_hash_link));
	lp->hash.key = xstrdup(key);
	lp->filename = xstrdup(tmpfile);
        lp->lastScannedSize = 0;
        xfree(tmpfile);
        // yuck, C style superclass casting
        hash_join(hash, &lp->hash);
    }
    xfree(key);

    // here we are sure that the file name has been set (in lp->filename)
    // care about file permissions!
    umask(0077);
    f = fopen(lp->filename, "ab");
    if (f == NULL) {
	debug(93,3) ("%s: error: could not create temporary file %s\n", moduleName, lp->filename);
	return NULL;
    }
    fwrite(buf, 1, len, f);
    fclose(f);
    if (lastScannedSize != NULL)
	*lastScannedSize = &lp->lastScannedSize;
    return lp->filename;
}

void remove_file(hash_table* hash, const char* moduleName,
		 const char * const url,
		 const char * const client_addr)
{
    tmpfile_hash_link* lp;

    if ((lp = query_hash(hash, moduleName, url, client_addr)) != NULL) {
	hash_remove_link(hash, (hash_link*) lp);
	// also remove the file and free the filename
	debug(93, 9) ("%s: removing file %s\n", moduleName, lp->filename);
	unlink(lp->filename);
	xfree((void*) lp->hash.key);
	xfree((void*) lp->filename);
        xfree((void*) lp);
    }
    else
	debug(93, 9) ("%s: tried to remove file for key that is not in the hash, ignoring request\n");
}
