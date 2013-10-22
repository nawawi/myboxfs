/* mod_rejecttype - reject stuff based on its MIME type.
   The arguments to the module are: type [file]
   type is the MIME type.
   file is a file containing an allow list of URL regexes.
   If file is not given, everything with that type is rejected.
*/

#include "squid.h"
#include "module.h"
#include "patfile.h"

#define MODNAME "cfmod_rejecttype"
#define CFMOD_REJECTTYPE_C
#include <classes.dh>

/* moduleObject->filter for FIL_REJECTTYPE:
   arg is typParam struct.
   Returns NULL iff the object is to be rejected.
*/
static void *rejectFilter(moduleObject *this, const void *arg)
{
    const typParam *tp = arg;
    char *r = NULL;
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
    moduleObject *m;
    patFileObject *pf;
    if (!args) {
        debug(93, 1) (MODNAME ": needs one or two arguments\n");
        return;
    }
    m = new(moduleObject);
    m->chain.typ = FIL_REJECTTYPE;
    m->info.version = MODULE_API_VERSION;
    m->description = "Rejection by MIME type";
    m->trigger = args->key;
    m->filter = rejectFilter;

    args = args->next;
    pf = args ? patfileNew(args->key, 0) : NULL;
    m->patFile = REF(pf);

    moduleRegister(m);
}

