/*
 * DEBUG: section 92     Module loader
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

#ifndef MODULE_H
#define MODULE_H

#include "squid.h"

/* Define this to track REF/UNREF usage */
/* #define REF_DEBUG */

#define MODULE_API_VERSION 0xa040

/* -------------------------- Object framework ----------------------------- */

/* Create global cbdata type for each class */

#define ADD_CLASSDEF(t) CBDATA_GLOBAL_TYPE(t);

/* The general structure of an object is specified by this definition: */

#include <classes.th>

classdef(Object,, ({
    int refCount;                       /* Reference count */
    void (*destroy)(void *this);        /* Destructor function */
}))

/* See the classdef program (in filters/) on syntax and destructor semantics.

   REF and UNREF usage:

   Whenever a pointer to an object is stored somewhere, do it like this:
       genericObject *stuff = REF(someobject);
   Whenever such a stored pointer goes out of use, do this:
       UNREF(stuff);
   (most importantly, do this in destructors for objects pointed to)
   This ensures the destructor will be called at the right time.
   Do not explicitly call destructors.
   The arguments to REF and UNREF must be suitable pointer _variables_.
   The redundant assignments help ensure this.

   All objects are implicitly cbdata, by default unlocked.
*/

#ifdef REF_DEBUG
#ifdef __GNUC__
#define REF(obj) \
    ({ if ((obj=obj)) { \
         debug(92,9)("%s:%d: REF %d\n",__FILE__,__LINE__,(obj)->refCount); \
         ++(obj)->refCount; \
       } else { \
         debug(92,9)("%s:%d: REF (null)\n",__FILE__,__LINE__); \
       } obj; })
#else
#define REF(obj) (((obj=obj)) ? (++(obj)->refCount, obj) : NULL)
#endif
#define UNREF(ptr) \
    do{ if (ptr) { \
          debug(92,9)("%s:%d: UNREF %d\n",__FILE__,__LINE__,(ptr)->refCount); \
          if((--(ptr)->refCount)<=0) (ptr)->destroy(ptr); \
          (ptr)=NULL; \
        } else { \
          debug(92,9)("%s:%d: UNREF (null)\n",__FILE__,__LINE__); \
        } }while(0)
#else

#define REF(obj) (((obj=obj)) ? (++(obj)->refCount, obj) : NULL)
#define UNREF(ptr) \
    do{ if ((ptr) && ((--(ptr)->refCount)<=0)) { \
          (ptr)->destroy(ptr); (ptr)=NULL; } \
    }while(0)

#endif

/* The global ctor; the macro makes it easier to use cbdata */
#define void_O_CTOR(s,d,t) (CBDATA_INIT_TYPE(t) >= 0 ? void_O_CTOR_((Object *)cbdataAlloc(t), d) : NULL)
 
extern void *void_O_CTOR_(Object *this, _DTOR dtor);

/* Used for narrowing casts. Should find a way to check types here? */
#define CC(ptr,tptrtype) ((tptrtype*)(ptr))

/* -------------------------- Module stuff --------------------------------- */

/* Filter types */

typedef enum _squidFilterType {
    FIL_DUMMY,                          /* Library module */
    FIL_AUTH,                           /* Authentication checker */
    FIL_REDIRECT,                       /* URI redirector */
    FIL_REQHEADER,                      /* Request header filter */
    FIL_REPHEADER,                      /* Reply header filter */
    FIL_REJECTTYPE,                     /* Reject based on content type */
    FIL_CONTFILTER,                     /* Content filter */
    FIL_END
} squidFilterType;

/* The module object */

classdef(moduleObject, Object, ({
    union {                             /* Modified by moduleRegister */
        int typ;                        /* - Filter type */
        struct _moduleObject *next;     /* - Next in chain */
    } chain;
    union {                             /* Modified by moduleRegister */
        int version;                    /* - must be MODULE_API_VERSION */
        struct _moduleOwner *owner;     /* - shlib info */
    } info;
    const char *description;            /* Description of filter purpose */
    const char *trigger;                /* Trigger argument (like file type) */
    void *(*filter)(class *this, const void *arg);
                                        /* Either: module filter function */
                                        /*  Or: filter object constructor */
    patFileObject *patFile;             /* pattern file */
}))

/* Called from core: Initialize the module. */
void moduleInit(const wordlist *args);

/* Called from module: Register a newly created module object. */
void moduleRegister(moduleObject *theModule);

/* The filter object */

classdef(filterObject, Object, ({
    moduleObject *owner;                /* Owning module */
    char *uri;                          /* Request URI */
    struct _filterObject *next;         /* Next filter in chain */
    int (*filter)(struct _filterObject *this, MemBuf *target,
                 const char *buf, int len, const char* content_type, const char* client_addr, const char* auth_user);                                     
	/* Filtering function */
}))

void moduleLoad(const char *module, const wordlist *args);
void moduleLoadAll(void);
void moduleUnloadAll(void);

/* --------------------------- Filtering hooks ----------------------------- */

char *canonType(const char *t);
int mtmatch(const char *ct, const char *tr);
char *moduleUrlClean(const char *uri);

char *moduleRedirect(const char *uri, const char *client_addr, const char *auth_user);
typedef struct _redParam {
    const char *uri;
    const char *client_addr;
    const char *auth_user;
} redParam;

typedef struct _typParam {
    const char *uri;
    char *typ;
    const char *client_addr;
    const char *auth_user;
} typParam;

char *moduleRejecttype(const char *typ, const char *uri, const char *client_addr, const char *auth_user);

typedef struct _hdrParam {
    const char *uri;
    HttpHeader *hdr;
    const char *client_addr;
    const char *auth_user;
} hdrParam;

void moduleReqHeader(const char *uri, HttpHeader *hdr, const char *client_addr,const char *auth_user);
void moduleRepHeader(const char *uri, HttpHeader *hdr);

typedef struct _repParam {
    const char *uri;
    HttpReply *rep;
    const char *client_addr;
    const char *content_type;
    const char *auth_user;
} repParam;

filterObject *moduleCFilterNew(const char *typ, const char *uri,
                               HttpReply *rep, const char *client_addr, const char *content_type, const char *auth_user);
int moduleCFilter(void *filter, MemBuf *target, const char *buf, int len,
                 const char* content_type, const char* client_addr, const char* auth_user);
void moduleCFilterDestroy(void *filter);

#define RREJECT ((char*)-1)

#endif
