
#include "squid.h"
#include "module.h"
#include "patfile.h"

#define MODNAME "cfmod_redirect"
#define CFMOD_REDIRECT_C
#include <classes.dh>

/* moduleObject->filter for FIL_REDIRECT:
   arg is the requested URI.
   Returns the redirected URI. Returns NULL if no redirection.
*/
static void *redirFilter(moduleObject *this, const void *arg)
{
    const char *uri = arg;
    char *r;
    patfileCheckReload(this->patFile);
    r = patfileMatch(this->patFile, uri);
    if (r)
        debug(93, 3) (MODNAME ": '%s' => '%s'\n", moduleUrlClean(uri), r);
    return r;
}

void moduleInit(const wordlist *args)
{
    moduleObject *m;
    patFileObject *pf;
    if (!args) {
        debug(93, 1) (MODNAME ": needs an argument\n");
        return;
    }
    m = new(moduleObject);
    m->chain.typ = FIL_REDIRECT;
    m->info.version = MODULE_API_VERSION;
    m->description = "URI Redirector";
    m->filter = redirFilter;
    pf = patfileNew(args->key, 1);
    m->patFile = REF(pf);

    moduleRegister(m);
}
