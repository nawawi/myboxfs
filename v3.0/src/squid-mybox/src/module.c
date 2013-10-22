/*
 * DEBUG: section 92     Module loader and hooks
 * AUTHOR: Olaf Titz
 *
 * SQUID Internet Object Cache  http://squid.nlanr.net/Squid/
 * ----------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from the
 *  Internet community.  Development is led by Duane Wessels of the
 *  National Laboratory for Applied Network Research and funded by the
 *  National Science Foundation.  Squid is Copyrighted (C) 1998 by
 *  Duane Wessels and the University of California San Diego.  Please
 *  see the COPYRIGHT file for full details.  Squid incorporates
 *  software developed and/or copyrighted by other sources.  Please see
 *  the CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */

#include "squid.h"
#include "module.h"

/* ---------- Module loader, definitions ---------- */

/* This defines a common API between the Sun libdl and HP libdld.
   We use shl_t for library handles and
   void xdlsym(void **result, shl_t handle, const char *name);
   instead of dlsym().
   The rest follows the Sun API.
*/

#ifdef HAVE_LIBDL
#include <dlfcn.h>

/* Solaris/Linux/etc. */
typedef void *shl_t;
#define xdlsym(r, h, s) (r)=dlsym((h), (s))

#else
#ifdef HAVE_LIBDLD
#include <dl.h>

/* HPUX */
#define RTLD_NOW        BIND_IMMEDIATE
#define RTLD_LAZY       BIND_DEFERRED
#define RTLD_GLOBAL     0
#define dlerror()       strerror(errno)
#define dlopen(m, f)    shl_load((m), (f), 0)
#define xdlsym(r, h, s) \
        if (shl_findsym(&(h), (s), TYPE_UNDEFINED, &(r))<0) (r)=NULL
#define dlclose(h)      shl_unload((h))

#endif
#endif

#define MODULE_C
#include <classes.dh>

/* ---------- Object framework ---------- */
void *void_O_CTOR_(Object *this, _DTOR dtor)
{
    this->destroy = dtor;
    return this;
}

void Object_O_DTOR(Object *this)
{
#if 0
    if (this->refCount <= 0)
        cbdataFree(this);
#else
    assert(this->refCount <= 0);
    cbdataFree(this);
#endif
}

Ddef(moduleObject)
{
    /*
      next is handled in moduleUnloadAll()
      owner is handled in moduleDestroyProxy()
      description and trigger are not alloced
    */
    UNREF(this->patFile);
}

/* ---------- Book-keeping of loaded modules ---------- */

/* The shared library object */

classdef(private shlibObject, Object, ({
    struct _shlibObject *next, *prev;   /* Chain (weak ref!) */
    char *name;                         /* Module name */
    shl_t hndl;                         /* dlopen handle */
}))

typedef struct _moduleOwner {
    shlibObject *owner;                 /* Containing shlib */
    void (*realDestroy)(void *);        /* moduleObject destructor */
} moduleOwner;

/* The filter chains */
moduleObject *chain[FIL_END] = {NULL};

/* Registry */
shlibObject *modules = NULL;
shlibObject *currentlyLoading = NULL;

Ddef(shlibObject)
{
#if 0
    if (this->refCount > 0) {
        debug(92, 5) ("shlibUnload: revived %s\n", this->name);
        return;
    }
#endif
    debug(92, 4) ("shlibUnload: unloading %s\n", this->name);
    if (modules == this) {
        assert(this->prev == NULL);
        modules = this->next;
    }
    if (this->next)
        this->next->prev = this->prev;
    if (this->prev)
        this->prev->next = this->next;
    xfree(leakFree(this->name));
    dlclose(this->hndl);
}

static void shlibUnloadCB(void *p)
{
    shlibObject *o = p;
    debug(92, 8) ("shlibUnloadCB: %s refCount=%d\n", o->name, o->refCount);
    cbdataUnlock(o);
    UNREF(o); /* calls the shlibObject dtor if needed */
}

/* The actual unloading via dlclose() of a module is deferred
   because it must be called from the main loop directly, not via
   UNREF. Otherwise, a module could possibly unload its own code
   segment. With several seconds delay, a module which is unchanged
   after reconfig will just be reused (but not its moduleObject(s)!)
*/
static void moduleDestroyProxy(void *obj)
{
    moduleOwner *o = ((moduleObject*)obj)->info.owner;
    debug(92, 8) ("moduleDestroyProxy: %s %s\n", o->owner->name,
                  ((moduleObject*)obj)->description);
    assert(cbdataValid(o));
    o->realDestroy(obj);
    cbdataLock(o->owner);
    eventAddIsh("shlibUnload", shlibUnloadCB, o->owner, 15.0, 1);
    cbdataUnlock(o);
    cbdataFree(o);
}

/* ---------- Module loader, implementation ---------- */

/* Register a moduleObject. Called from the modules' moduleInit functions. */
void moduleRegister(moduleObject *theModule)
{
    moduleObject *p;
    moduleOwner *o;
    CBDATA_TYPE(moduleOwner);
    
    if (theModule->info.version != MODULE_API_VERSION) {
        debug(92, 1) ("moduleRegister: rejecting incompatible module\n");
        return;
    }
    debug(92, 4) ("moduleRegister: registering %s %s\n",
                  theModule->description,
                  theModule->trigger ? theModule->trigger : "");
    p = chain[theModule->chain.typ];
    if (p) {
        for (; p->chain.next; p = p->chain.next);
        p->chain.next = REF(theModule);
    } else {
        p = REF(theModule);
        chain[theModule->chain.typ] = p;
    }
    theModule->chain.next = NULL;
    assert(currentlyLoading);
/*
    o = leakAdd(xmalloc(sizeof(moduleOwner)));
    cbdataAdd(o, cbdataLfree, 0);
*/
    CBDATA_INIT_TYPE(moduleOwner);
    o = cbdataAlloc(moduleOwner);
    cbdataLock(o);
    o->owner = REF(currentlyLoading);
    o->realDestroy = theModule->destroy;
    theModule->info.owner = o;
    theModule->destroy = moduleDestroyProxy;
}

void moduleLoad(const char *module, const wordlist *args)
{
    void (*initf)(const wordlist *);
    const char *dle;
    shl_t hndl;
    shlibObject *sc;
    const char *name = strrchr(module, '/');
    if (name)
        ++name;
    else
        name = module;

    debug(92, 4) ("moduleLoad: loading %s\n", name);

    hndl = dlopen(module, RTLD_NOW|RTLD_GLOBAL);
    if (!hndl) {
        dle = dlerror();
        if (!dle)
            dle = "(unknown)";
        debug(92, 1) ("moduleLoad: %s: dlopen error: %s\n", module, dle);
        return;
    }
    xdlsym(initf, hndl, "moduleInit");
    if (!initf) {
        dle = dlerror();
        if (!dle)
            dle = "NULL";
        debug(92, 1) ("moduleInit not found: %s\n", dle);
        dlclose(hndl);
        return;
    }
    for (sc=modules; sc; sc=sc->next) {
        if (hndl == sc->hndl) {
#ifdef HAVE_LIBDL
            dlclose(hndl);
            /* for HAVE_LIBDLD: do not unload here! Refcounting seems to be
               broken, at least for HPUX 10.20. */
#endif
            break;
        }
    }
    if (!sc) {
        sc = new(shlibObject);
        sc->next = modules;
        sc->prev = NULL;
        sc->name = leakAdd(xstrdup(name));
        sc->hndl = hndl;
        if (modules)
            modules->prev = sc;
        modules = sc;
    }
    currentlyLoading = REF(sc);
    initf(args);
    UNREF(currentlyLoading);
}

void moduleLoadAll(void)
{
    load_module *list;
    wordlist *w;
    for (list = Config.modules; list; list = list->next) {
        w = list->params;
        moduleLoad(list->module, w);
    }
}

void moduleUnloadAll(void)
{
    int i, m = 0;
    moduleObject *p, *p0;

    for (i=0; i<FIL_END; ++i) {
        for (p = chain[i]; p; p = p0) {
            p0 = p->chain.next;
            debug(92, 5) ("moduleUnloadAll: unregistering %s\n",
                          p->description);
            UNREF(p);
            ++m;
        }
        chain[i] = NULL;
    }
    debug(92, 4) ("moduleUnloadAll: unregistered %d objects\n", m);
}


/* ---------- Filtering hooks ---------- */

char *canonType(const char *t)
{
    char *x, *p;
    if (!t)
	return NULL;
    p = x = xstrdup(t);
    while (*p) {
	if (*p==';') {
	    *p='\0';
	    return x;
	}
	*p = tolower(*p);
	++p;
    }
    return x;
}

int mtmatch(const char *ct, const char *tr)
{
    if (tr[0] == '*' && tr[1] == '/') {
	for (; *ct && *ct != '/'; ++ct);
	if (*ct)
	    ++ct;
	tr += 2;
    }
    while (*ct) {
	if (*tr == '*')
	    return 1;
	if (*ct != *tr)
	    return 0;
	++ct;
	++tr;
    }
    return (*tr == 0);
}

char *moduleUrlClean(const char *uri)
{
    static char buf[MAX_URL];
    xstrncpy(buf, uri, MAX_URL);
    if (Config.onoff.strip_query_terms) {
	char *p = strchr(buf, '?');
	if (p)
	    p[1] = '\0';
    }
    if (stringHasCntl(buf))
	return rfc1738_escape_unescaped(buf);
    return buf;
}

char *moduleRedirect(const char *uri,  const char *client_addr, const char *auth_user)
{
    const char *res = uri, *tmp;
    moduleObject *f;
    redParam rd;

    rd.uri = res;
    rd.client_addr = client_addr;
    rd.auth_user = auth_user;

    for (f = chain[FIL_REDIRECT]; f; f = f->chain.next) {
        tmp = f->filter(f, &rd);
	if (tmp)
	    res = tmp;
    }
    return (char *)res; /* XX */
}


char *moduleRejecttype(const char *typ, const char *uri, const char *client_addr, const char *auth_user)
{
    moduleObject *f;
    typParam tp;
    char *r = (char *)uri; /* XX */
    tp.uri = r;
    tp.typ = canonType(typ);
    tp.client_addr = client_addr;
    tp.auth_user = auth_user;
    for (f = chain[FIL_REJECTTYPE]; f; f = f->chain.next) {
	if (!*f->trigger || (tp.typ && mtmatch(tp.typ, f->trigger))) {
	    r = f->filter(f, &tp);
	    break;
	}
    }
    xfree(tp.typ);
    return r;
}


void moduleReqHeader(const char *uri, HttpHeader *hdr, const char *client_addr,const char *auth_user)
{
    hdrParam hp;
    moduleObject *f;
    hp.uri = uri;
    hp.hdr = hdr;
    hp.client_addr=client_addr;
    hp.auth_user=auth_user;
    for (f = chain[FIL_REQHEADER]; f; f = f->chain.next) {
        (void) f->filter(f, &hp);
    }
}

void moduleRepHeader(const char *uri, HttpHeader *hdr)
{
    hdrParam hp;
    moduleObject *f;
    hp.uri = uri;
    hp.hdr = hdr;
    for (f = chain[FIL_REPHEADER]; f; f = f->chain.next) {
        (void) f->filter(f, &hp);
    }
}


filterObject *moduleCFilterNew(const char *typ, const char *uri,
                               HttpReply *rep, const char *client_addr, const char *content_type, const char *auth_user)
{
    repParam rp;
    moduleObject *f;
    filterObject *fo = NULL, *fo1 = NULL;
    rp.uri = uri;
    rp.rep = rep;
    rp.client_addr = client_addr;
    rp.content_type = content_type;
    rp.auth_user = auth_user;
    if (typ) {
	char *tt = canonType(typ);
        for (f = chain[FIL_CONTFILTER]; f; f = f->chain.next) {
            if (mtmatch(tt, f->trigger)) {
                filterObject *n = f->filter(f, &rp);
                if (n) {
		    n->owner = REF(f);
		    n->uri = leakAdd(xstrdup(uri));
		    if (fo) {
			fo->next = REF(n);
			fo = fo->next;
		    } else {
			fo = fo1 = REF(n);
		    }
		}
            }
        }
	xfree(tt);
    }
    return fo1;
}

Ddef(filterObject)
{
    UNREF(this->next);
    UNREF(this->owner);
    xfree(leakFree(this->uri));
}

void moduleCFilterDestroy(void *f)
{
    filterObject *o = f;
    UNREF(o);
}

int moduleCFilter(void *filter, MemBuf *target, const char *buf, int len,
                 const char* content_type, const char* client_addr, const char* auth_user)
{
    filterObject *f;
    MemBuf tmp[2];
    int i = 0, r = 0;
    char tmpu[2] = {0, 0};

    for (f = filter; f->next; f = f->next) {
        if (tmpu[i]) {
            memBufReset(&tmp[i]);
        } else {
            memBufDefInit(&tmp[i]);
            tmpu[i] = 1;
        }
	if (f->filter(f, &tmp[i], buf, len, content_type, client_addr, auth_user) < 0) {
            debug(92, 4) ("moduleCFilter: aborted by %s\n",
                          f->owner->description);
            r = -1;
            break;
        }
        buf = tmp[i].buf;
        len = tmp[i].size;
        i = 1-i;
    }
    if (r >= 0) {
	if (f->filter(f, target, buf, len, content_type, client_addr, auth_user) < 0) {
            debug(92, 4) ("moduleCFilter: aborted by %s\n",
                          f->owner->description);
            r = -1;
        }
    }
    for (i=0; i<2; ++i)
        if (tmpu[i])
            memBufClean(&tmp[i]);
    return r;
}
