/* mod_allowtype - reject stuff based on its MIME type, only let specified
   types through.
   The arguments to the module are: type [type...] [file]
   type are the MIME types.
   file is a file containing an allow list of URL regexes.
   File name must start with a / to be recognized.
*/

#include "squid.h"
#include "module.h"
#include "patfile.h"

classdef(allowtypeObject, moduleObject, ({
    wordlist *types;
}))

#define MODNAME "cfmod_allowtype"
#define CFMOD_ALLOWTYPE_C
#include <classes.dh>

Ddef(allowtypeObject) {
    wordlist *p0, *p;
    for (p = this->types; p; p = p0) {
	p0 = p->next;
	xfree(p->key);
	xfree(p);
    }
}

/* moduleObject->filter for FIL_REJECTTYPE:
   arg is typParam struct.
   Returns NULL iff the object is to be rejected.
*/
static void *allowFilter(allowtypeObject *this, const void *arg)
{
    const typParam *tp = arg;
    char *r = NULL;
    wordlist *w;

    /* check for allowed type */
    if (tp->typ) {
        char *x = canonType(tp->typ);
	for (w = this->types; w; w = w->next)
	    if (mtmatch(x, w->key)) {
                xfree(x);
		return (char *)tp->uri; /* XX */
            }
        xfree(x);
    }

    /* check for URI on allow list */
    if (this->patFile) {
	patfileCheckReload(this->patFile);
	r = patfileMatch(this->patFile, tp->uri);
    }
    if (!r)
	debug(93, 3) (MODNAME ": %s: rejecting type %s\n", tp->uri, tp->typ);
    return r;
}


void moduleInit(const wordlist *args)
{
    allowtypeObject *m;
    patFileObject *pf = NULL;
    wordlist *w;

    if (!args) {
        debug(93, 1) (MODNAME ": needs at least one argument\n");
        return;
    }
    m = new(allowtypeObject);
    m->chain.typ = FIL_REJECTTYPE;
    m->info.version = MODULE_API_VERSION;
    m->description = "Rejection by MIME type";
    m->trigger = "";
    m->filter = allowFilter;

    while (args) {
	if (args->key[0] == '/') {
	    pf = patfileNew(args->key, 0);
	    break;
	}
	w = xmalloc(sizeof(*w));
	w->key = xstrdup(args->key);
	/* we may reverse the list here since it is always checked at once */
	w->next = m->types;
	m->types = w;
	args = args->next;
    }
    m->patFile = REF(pf);
    moduleRegister(CC(m, moduleObject));
}

