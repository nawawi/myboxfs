/* mod_gifanim - reset all GIF animations to one cycle
 */

#include "squid.h"
#include "module.h"
#include "patfile.h"

#define MODNAME "cfmod_gifanim"

static const char magic[] = "\x21\xff\x0bNETSCAPE2.0\x03\x01";
static const char nomagic[] = "\x21\xfe\x0bXXXXXXXX1.0\x03\x01";
#define TRIGGERLEN 16
#define KILLLEN 3

classdef(private gifanimObject, moduleObject, ({
    int ncycles;
}))

classdef(private gifanimFilterObject, filterObject, ({
    int posin;
    int inHold;
}))

#define CFMOD_GIFANIM_C
#include <classes.dh>

Ddef_none(gifanimObject)

Ddef_none(gifanimFilterObject)

static int cfFilter(filterObject *this, MemBuf *target,
                    const char *buf, int len)
{
    gifanimFilterObject *cf = CC(this, gifanimFilterObject);
    int i;
    debug(93,9) (MODNAME ": %s: pos %d len %d\n", moduleUrlClean(cf->uri),
		 cf->posin, len);
    for (i=0; i<len; ++i) {
        if (cf->inHold >= TRIGGERLEN+KILLLEN) {
            char x[3] = { 0 };
            int nc = CC(this->owner,gifanimObject)->ncycles;
            debug(93,3) (MODNAME ": %s: breaking animation to %d\n",
                         moduleUrlClean(cf->uri), nc);
            if (nc < 0)
                return -1;
            x[0] = nc & 255;
            x[1] = nc >> 8;
            memBufAppend(target, nc ? magic : nomagic, TRIGGERLEN);
            memBufAppend(target, x, 3);
            cf->inHold = 0;
            memBufAppend(target, buf+i, 1);
        } else if (cf->inHold >= TRIGGERLEN) {
            ++cf->inHold;
        } else if (cf->inHold > 0) {
            if (buf[i] == magic[cf->inHold]) {
                ++cf->inHold;
            } else {
                memBufAppend(target, magic, cf->inHold);
                cf->inHold = 0;
                memBufAppend(target, buf+i, 1);
            }
        } else {
            if (buf[i] == magic[0])
                cf->inHold = 1;
            else
                memBufAppend(target, buf+i, 1);
        }
    }
    cf->posin += len;
    return len;
}

/* moduleObject->filter for FIL_CONTFILTER:
   filterObject constructor.
*/
static void *gifanimFilter(gifanimObject *this, const void *arg)
{
    gifanimFilterObject *f;
    const repParam *rp = arg;
    if (this->patFile && (patfileCheckReload(this->patFile),
                          patfileMatch(this->patFile, rp->uri)))
        return NULL;

    f = new(gifanimFilterObject);
    f->filter = cfFilter;
    f->posin = f->inHold = 0;

    return f;
}


void moduleInit(const wordlist *args)
{
    gifanimObject *m;
    patFileObject *pf;

    m = new(gifanimObject);
    m->chain.typ = FIL_CONTFILTER;
    m->info.version = MODULE_API_VERSION;
    m->description = "animation breaker";
    m->trigger = "image/gif";
    m->filter = gifanimFilter;
    if (args) {
        m->ncycles = atoi(args->key);
        args = args->next;
    } else {
        m->ncycles = 1;
    }

    pf = args ? patfileNew(args->key, 0) : NULL;
    m->patFile = REF(pf);

    moduleRegister(CC(m, moduleObject));
}

