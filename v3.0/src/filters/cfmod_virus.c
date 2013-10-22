#define _GNU_SOURCE
#include "squid.h"
#include "module.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define MODNAME "cfmod_virus"

classdef(private genvscanObject, moduleObject, ({
	char* cmdPath;
        char* cmdLine;
	char** cmdArgs;
        int numArgs;
	int retValClean;
        int retValVirus;
	char* actionPath;
}))

classdef(private genvscanFilterObject, filterObject, ({
	char* cmdPath;
        char* cmdLine;
	char** cmdArgs;
        int numArgs;
	int retValClean;
        int retValVirus;
	char* actionPath;
}))

#define CFMOD_VIRUS_C
#include <classes.dh>

#define BUFFER_LENGTH  256
#define MSG_BUFFER_LENGTH  256
#define DELIMITER      '\''
#define SEPARATOR      '/'
#define COPY_BUFFER_LENGTH 4096

Ddef_none(genvscanObject)
Ddef_none(genvscanFilterObject)

static int cfFilter(filterObject *this, MemBuf *target,
		    const char *buf, int len,
		    char* content_type, char* client_addr, char* auth_user)
{
    int ret;
    //char* virname;
    char *tmpfile=NULL;
    tmpfile = (char*) xmalloc(sizeof(char) * 102);
    FILE* f;
    pid_t child;
    char msg[MSG_BUFFER_LENGTH];

    genvscanFilterObject *cf = CC(this, genvscanFilterObject);
    debug(93,9) (MODNAME ": %s: len %d\n", moduleUrlClean(cf->uri), len);

    if (len > 0) {
	// scanner dependent
	// write a temporary file
	tmpfile = tempnam(NULL, "vscan");
	if (tmpfile == NULL) {
	    debug(93,3) (MODNAME ": error: could not create temporary file name\n");
            return -1;
	}
	f = fopen(tmpfile, "w+b");
	if (f == NULL) {
	    debug(93,3) (MODNAME ": error: could not create temporary file %s\n", tmpfile);
            return -1;
	}
	fwrite(buf, 1, len, f);
	fclose(f);
        cf->cmdArgs[cf->numArgs+1] = tmpfile;
	child = fork();
	if (child == -1) {
	    debug(93,3) (MODNAME ": error: could not fork scanner\n");
	    return -1;
	}
	else if (child == 0) {
	    // child
	    fclose(stdout);
	    fclose(stdin);
            fclose(stderr);
            execv(cf->cmdPath, cf->cmdArgs);
	    debug(93,3) (MODNAME ": error: could not exec scanner: %d\n", errno);
	}
	else {
	    // parent
            waitpid(child, &ret, 0);
	}

        unlink(tmpfile);
	if (WIFEXITED(ret)) {
	    int cmdret = WEXITSTATUS(ret);
	    if (cmdret == cf->retValClean) {
		// clean - copy to target
		memBufAppend(target, buf, len);
		// pass the buffer unmodified
		return len;
	    }
	    else if (cmdret == cf->retValVirus) {
	      	// virus detected, return an empty buffer
	       	debug(93,0) (MODNAME ": %s contains virus, blocking !\n", moduleUrlClean(cf->uri));
            int msglen, i;
                            debug(93,3) (MODNAME ": content type is text (%s), removing virus code\n", content_type);
                            snprintf(msg, MSG_BUFFER_LENGTH, "<p><b><i>VIRUS REMOVED BY MYBOX FIREWALL SYSTEM VIRUS PROTECTION</i></b></p>");
                            // retain the original len to not confuse the
                            msglen = strnlen(msg, MSG_BUFFER_LENGTH);
                            if (msglen > len)
                                msglen = len;
                            memBufAppend(target, msg, msglen);
                            for (i=msglen; i<len; i++)
                                memBufAppend(target, " ", 1);
                            return len;
	    }
	    else {
		debug(93,3) (MODNAME ": error: unkown return value\n");
		return -1;
	    }
	}
	else {
	    debug(93,3) (MODNAME ": error: child exited abnormally\n");
            return -1;
	}
    }
    else
	return 0;
}

static void cfDestroy(void *this)
{
    //genvscanFilterObject *cf = CC(this, genvscanFilterObject);
    //debug(93,9) (MODNAME ": filter destroy\n");
    // scanner dependent
}

/* moduleObject->filter for FIL_CONTFILTER:
   filterObject constructor.
*/
static void *genvscanFilter(genvscanObject *this, const void *arg)
{
    genvscanFilterObject *f;
//    const repParam *rp = arg;

    CBDATA_INIT_TYPE(genvscanFilterObject);
    f = new(genvscanFilterObject);
    f->filter = cfFilter;
    f->destroy = cfDestroy;
    // not necessary for this filter
    //f->destroy = NULL;
    /* Starting from here it is dependent on the virus scanner */

    f->cmdPath = this->cmdPath;
    f->cmdArgs = this->cmdArgs;
    f->cmdLine = this->cmdLine;
    f->numArgs = this->numArgs;
    f->retValClean = this->retValClean;
    f->retValVirus = this->retValVirus;
    f->actionPath = this->actionPath;

    return f;
}

static void moduleDestroy(void *this)
{
    genvscanObject *m = CC(this, genvscanObject);
    debug(93,9) (MODNAME ": destroy module\n");
    // scanner dependent
    xfree(m->cmdArgs);
    xfree(m->cmdLine);
}

void moduleInit(const wordlist *args)
{
    genvscanObject *m;

    CBDATA_INIT_TYPE(genvscanObject);
    m = new(genvscanObject);
    m->chain.typ = FIL_CONTFILTER;
    m->info.version = MODULE_API_VERSION;
    m->description = "general virus scanner";
    m->trigger = "*/*";
    m->filter = genvscanFilter;
    m->destroy = moduleDestroy;
    if (args) {
	// scanner dependent
	char *buf=NULL;
	int i;

	// now split the optional arguments from the command itself
	// but first check how many arguments there are
	m->cmdLine = (char*) xmalloc(sizeof(char) * (strlen(args->key)+1));
	strcpy(m->cmdLine, args->key);
	buf = (char*) xmalloc(sizeof(char) * (strlen(m->cmdLine)+1));
        strcpy(buf, m->cmdLine);
	strtok(buf, " ");
        m->numArgs = 0;
	while (strtok(NULL, " ") != NULL)
	    m->numArgs++;
        xfree(buf);

	m->cmdPath = strtok(m->cmdLine, " ");
	// we need three more fields (one for the command name, one for the
	// file name to check and the NULL terminator)
	m->cmdArgs = (char**) xmalloc(sizeof(char*) * (m->numArgs+3));
	m->cmdArgs[0] = m->cmdPath;
        i = 1;
	while ((m->cmdArgs[i] = strtok(NULL, " ")) != NULL) {
	    i++;
	}
	m->cmdArgs[m->numArgs+2] = NULL;

        args = args->next;
    } else {
	// scanner dependent
	debug(93,0) (MODNAME ": error - need executable as parameter, disabling scanning\n");
	m->cmdPath = NULL;
	m->cmdArgs = NULL;
        m->numArgs = 0;
    }
    if (args) {
        m->actionPath = args->key;
        args = args->next;
    } else {
        m->actionPath = NULL;
    }

    if (args) {
	// scanner dependent
        m->retValVirus = atoi(args->key);
        args = args->next;
    } else {
	// scanner dependent
        m->retValVirus = 1;
    }
    if (args) {
	// scanner dependent
        m->retValClean = atoi(args->key);
        args = args->next;
    } else {
	// scanner dependent
        m->retValClean = 0;
    }
    if (m->cmdPath != NULL)
	debug(93,0) (MODNAME ": using virus scanner '%s', return value %d for infected, %d for clean files\n", m->cmdPath, m->retValVirus, m->retValClean);
    else
	// disable the filter
        m->filter = NULL;

    moduleRegister(CC(m, moduleObject));
}
