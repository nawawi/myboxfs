/* mod_cookies.c - kill cookies
   The module takes a file name as optional argument.
   If available, this is a file containing an allow list of URL regexes.
   Else, no cookies are allowed.
*/

#include "squid.h"
#include "module.h"
#include "patfile.h"

#define MODNAME "cfmod_cookies"
#define CFMOD_COOKIES_C
#include <classes.dh>

/* moduleObject->filter for FIL_REQHEADER/FIL_REPHEADER:
   arg is a hdrParam structure containing request URI and header.
   Return value is ignored.
*/
static void *cookieReqFilter(moduleObject *mod, const void *arg)
{
    const hdrParam *hp = arg;
    if (!mod->patFile ||
        (patfileCheckReload(mod->patFile),
         !patfileMatch(mod->patFile, hp->uri))) {
	if (httpHeaderDelById(hp->hdr, HDR_COOKIE) > 0)
            debug(93, 3) (MODNAME ": %s: deleted client cookie\n",
                          moduleUrlClean(hp->uri));
    }
    return NULL;
}

static void *cookieRepFilter(moduleObject *mod, const void *arg)
{
    const hdrParam *hp = arg;
    if (!mod->patFile ||
        (patfileCheckReload(mod->patFile),
         !patfileMatch(mod->patFile, hp->uri))) {
	if (httpHeaderDelById(hp->hdr, HDR_SET_COOKIE) > 0)
            debug(93, 3) (MODNAME ": %s: deleted server cookie\n",
                          moduleUrlClean(hp->uri));
    }
    return NULL;
}

void moduleInit(const wordlist *args)
{
    moduleObject *m;
    patFileObject *pf = (args ? patfileNew(args->key, 0) : NULL);

    m = new(moduleObject);
    m->chain.typ = FIL_REQHEADER;
    m->info.version = MODULE_API_VERSION;
    m->description = "Cookie blocker";
    m->filter = cookieReqFilter;
    m->patFile = REF(pf);
    moduleRegister(m);

    m = new(moduleObject);
    m->chain.typ = FIL_REPHEADER;
    m->info.version = MODULE_API_VERSION;
    m->description = "Set-Cookie blocker";
    m->filter = cookieRepFilter;
    m->patFile = REF(pf);
    moduleRegister(m);
}
