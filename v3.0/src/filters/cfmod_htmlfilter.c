/* Common code for HTML filters.
   This is not a filter on its own, but a library to be used by modules
   which "do something" with the HTML tags. See script.c for how to write
   those.
 */

#include "squid.h"
#include "module.h"
#include "patfile.h"
#include "htmlfilter.h"
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#define CFMOD_HTMLFILTER_C
#include <classes.dh>

Ddef(htmlModuleObject)
{
    UNREF(this->libObject);
}

Ddef(htmlFilterObject)
{
    memBufClean(&this->tag);
}

#define DEBUG_TAG_PARSER

/* Insert a tag. Can be called from the module. */
void insertTag(htmlFilterObject *this, MemBuf *target,
               int nattribs, char *attribs[], char *values[])
{
    int i, len;
    int *alen, *vlen;
    if (this->eating)
        return;

#ifdef HAVE_ALLOCA
    alen = alloca(2*nattribs*sizeof(int));
#else
    alen = xmalloc(2*nattribs*sizeof(int));
#endif
    vlen = alen + nattribs;
    for (i=1, len=2+strlen(attribs[0]); i<nattribs; ++i)
        if (attribs[i]) {
            alen[i] = strlen(attribs[i]);
            len += alen[i] + 1;
            if (values[i]) {
                vlen[i] = strlen(values[i]);
                len += vlen[i] + 1;
            }
        }
    if (this->contlen > 0 && this->posout+len > this->posin) {
        debug(94, 3) ("htmlfilter.c: %s: insertTag (%s): no space!\n",
                      moduleUrlClean(this->uri), attribs[0]);
#ifndef HAVE_ALLOCA
        xfree(alen);
#endif
        return;
    }

    memBufAppend(target, "<", 1);
    memBufAppend(target, attribs[0], strlen(attribs[0]));
    for (i=1; i<nattribs; ++i)
        if (attribs[i]) {
            memBufAppend(target, " ", 1);
            memBufAppend(target, attribs[i], alen[i]);
            if (values[i]) {
                memBufAppend(target, "=", 1);
                memBufAppend(target, values[i], vlen[i]);
            }
        }
    memBufAppend(target, ">", 1);
    this->posout += len;
#ifndef HAVE_ALLOCA
    xfree(alen);
#endif
}

#ifdef DEBUG_TAG_PARSER
static void debug_tag_parser(int nattribs, char *attribs[], char *values[])
{
    int i;
    MemBuf mb;
    memBufDefInit(&mb);
    for (i=0; i<nattribs; ++i) {
        memBufAppend(&mb, "\t", 1);
        if (attribs[i]) {
            memBufAppend(&mb, attribs[i], strlen(attribs[i]));
        }
        if (values[i]) {
            memBufAppend(&mb, "\t", 1);
            memBufAppend(&mb, values[i], strlen(values[i]));
        }
        memBufAppend(&mb, "\n", 1);
    }
    memBufAppend(&mb, "\0", 1);
    debug(94, 9) ("htmlfilter.c: processTag: %d\n%s", nattribs, mb.buf);
    memBufClean(&mb);
}
#endif

typedef enum {
    V_WSP, V_COMM, V_ATT, V_SVAL, V_VAL, V_DQUOT='"', V_SQUOT='\''
} vpos;

/* Parse a tag, hand it over to processTag() */
/* Recognizes ATTR=VAL, ATTR="VAL" and ATTR='VAL'. */
/* Handles SGML -- comments -- as an (empty) attribute. */
static int processTag0(htmlFilterObject *this, MemBuf *target)
{
#ifdef HAVE_ALLOCA
    char *scratch = alloca(this->tag.size);
    /* Worst case: <A B C D ...> gives length/2 attributes */
    char **atts = alloca(this->tag.size*sizeof(char *));
#else
    char *scratch = xmalloc(this->tag.size);
    char **atts = xcalloc(this->tag.size, sizeof(char *));
#endif
    char **vals = atts+this->tag.size/2;
    int ret, i, n = 0;
    vpos v = V_WSP, v0 = V_WSP;
    char *p = scratch+1;

#ifdef HAVE_ALLOCA
    memset(atts, 0, this->tag.size*sizeof(char *));
#endif

    assert(this->tag.buf[0]=='<');
    xmemcpy(scratch, this->tag.buf, this->tag.size);
    scratch[this->tag.size-1] = '\0';
    i = 1;
    for (; i<this->tag.size-1; ++i) {
        switch (v) {
        case V_WSP:
            if (isspace(scratch[i]))
                continue;
            p = scratch + i;
            if (scratch[i]=='=' && n>0) {
                ++p;
                v = V_VAL;
                continue;
            }
            if (scratch[i]=='-' && scratch[i+1]=='-') {
                v = V_COMM;
                i+=2;
                continue;
            }
            v = V_ATT;
            continue;
        case V_ATT:
            if (isspace(scratch[i])) {
                scratch[i] = '\0';
                atts[n++] = p;
                v = V_WSP;
            } else if (scratch[i]=='=') {
                scratch[i] = '\0';
                atts[n++] = p;
                p = scratch + i + 1;
                v = V_SVAL;
            }
            continue;
        case V_SVAL:
            if (isspace(scratch[i]))
                continue;
            p = scratch + i;
            v = V_VAL;
            /* FALL THRU */
        case V_VAL:
            if (isspace(scratch[i])) {
                if (v0 == V_DQUOT || v0 == V_SQUOT) {
                    scratch[i-1] = '\0';
                    vals[n-1] = p+1;
                } else {
                    scratch[i] = '\0';
                    vals[n-1] = p;
                }
                v0 = v = V_WSP;
            } else if (scratch[i]=='\'' || scratch[i]=='"') {
                v0 = v = scratch[i];
            }
            continue;
        case V_DQUOT:
        case V_SQUOT:
            if (scratch[i]==v && scratch[i-1]!='\\')
                v = V_VAL;
            continue;
        case V_COMM:
            if (i>2 && isspace(scratch[i]) &&
                scratch[i-1]=='-' && scratch[i-2]=='-') {
                scratch[i] = '\0';
                atts[n++] = p;
                v = V_WSP;
            }
            continue;
        }
    }
    if (v==V_ATT || v==V_COMM) {
        atts[n++]=p;
    } else if (v==V_VAL) {
        if (v0==V_DQUOT || v0==V_SQUOT) {
            scratch[i-1] = '\0';
            vals[n-1] = p+1;
        } else {
            vals[n-1] = p;
        }
    }

    if (n > 0) {
#ifdef DEBUG_TAG_PARSER
        if (debugLevels[94]>=9)
            debug_tag_parser(n, atts, vals);
#endif
        ret = CC(this->owner,htmlModuleObject)->processTag(this, target,
                                                           n, atts, vals);
    } else {
        ret = 0;
    }
#ifndef HAVE_ALLOCA
    xfree(scratch);
    xfree(atts);
#endif
    return ret;
}

static int htmlCfFilter(filterObject *obj, MemBuf *target,
                        char *buf, int len)
{
    htmlFilterObject *this = CC(obj, htmlFilterObject);
    int i;
    debug(94, 9) ("htmlfilter: %s: pos %d/%d (%d) len %d\n",
		  moduleUrlClean(this->uri),
                  this->posin, this->posout, this->contlen, len);
    this->posin += len;
    for (i=0; i<len; ++i) {
        if (buf[i]=='\0') {
#ifdef FIX_BROKEN_HTML
            buf[i]=' ';
#else
            debug(94, 1) ("htmlfilter: %s: no HTML, killing off\n",
                          moduleUrlClean(this->uri));
            return -1;
#endif
        }
        switch (this->inTag) {
        case T_TEXT:
            if (buf[i]=='<') {
                this->inTag = T_STAG;
                goto storeTag;
            }
        storeText:
            if (!this->eating) {
                memBufAppend(target, buf+i, 1);
                ++this->posout;
            }
            continue;

        case T_META:
            if (this->inBSQuot>=0) {
                if (buf[i]=='-') {
                    if (++this->inBSQuot >= 2) {
                        this->inTag = T_COMM;
                        this->inBSQuot = 0;
                    }
                } else {
                    this->inBSQuot = -1;
                }
            }
            if (buf[i]=='>')
                this->inTag = T_TEXT;
            goto storeText;

        case T_COMM:
            if (buf[i]=='>' && this->inBSQuot >= 2)
                this->inTag = T_TEXT;
            if (buf[i]=='-')
                ++this->inBSQuot;
            else
                this->inBSQuot = 0;
            goto storeText;

        case T_STAG:
            if (buf[i]=='!') {
                /* Push back SGML meta-tags including legal HTML coments */
                if (!this->eating) {
                    memBufAppend(target, "<!", 2);
                    this->posout += 2;
                }
                memBufReset(&this->tag);
                this->inTag = T_META;
                this->inBSQuot = 0;
                continue;
            }
            if (buf[i]=='<')
                continue; /* handle bad <<TAG> syntax */
            if (!isspace(buf[i]))
                this->inTag = T_TAG;
            /* FALL THRU */
        case T_EQU:
            if ((buf[i]=='"' || buf[i]=='\'') && !this->inBSQuot) {
                memBufAppend(&this->tag, buf+i, 1);
                this->inTag = buf[i];
                continue;
            }
            this->inTag = T_TAG;
            /* FALL THRU */
        case T_TAG:
            switch (buf[i]) {
            case '>':
                memBufAppend(&this->tag, ">", 1);
                if (!processTag0(this, target) && !this->eating) {
                    memBufAppend(target, this->tag.buf, this->tag.size);
                    this->posout += this->tag.size;
                }
                memBufReset(&this->tag);
                this->inTag = T_TEXT;
                continue;
            case '<':
                /* syntax error, but best effort to recover - open new tag */
                memBufAppend(&this->tag, ">", 1);
                if (!processTag0(this, target) && !this->eating) {
                    memBufAppend(target, this->tag.buf, this->tag.size);
                    this->posout += this->tag.size;
                }
                memBufReset(&this->tag);
                this->inTag = T_STAG;
                goto storeTag;
            case '=':
                this->inTag = T_EQU;
                /* FALL THRU */
            }
            /* FALL THRU */
        case T_DQUOT:
        case T_SQUOT:
            if (!this->inBSQuot) {
                if (buf[i]=='\\') {
                    this->inBSQuot = 1;
                } else if (buf[i]==this->inTag) {
                    this->inTag = T_TAG;
                }
            }
            /* FALL THRU */
        storeTag:
            memBufAppend(&this->tag, buf+i, 1);
            this->inBSQuot = 0;
        }
    }
    if (this->contlen>=0 && this->posin>=this->contlen) {
        /* Append blanks to end of file if needed */
        for (i=this->posout; i<this->contlen; ++i)
            memBufAppend(target, " ", 1);
    }
    return 0;
}

/* moduleObject->filter for FIL_CONTFILTER:
   filterObject constructor.
*/

static void *htmlContFilter(htmlModuleObject *this, const void *arg)
{
    htmlFilterObject *cf;
    const repParam *rp = arg;

    if (this->patFile && (patfileCheckReload(this->patFile),
                          patfileMatch(this->patFile, rp->uri)))
        return NULL;

    cf = new(htmlFilterObject);
    cf->filter = htmlCfFilter;
    cf->contlen = rp->rep->content_length;
    memBufDefInit(&cf->tag);

    return cf;
}

/* moduleObject holding the library module.
   This gets REFd by every client, which means it will be freed and
   release the library when the last client exits.
   This is about the only legitimate use for a global variable in a module. */
static moduleObject *libObject = NULL;

/* Register a htmlfilter derivative */
void htmlfilterRegister(htmlModuleObject *this, const char *description)
{
    this->chain.typ = FIL_CONTFILTER;
    this->info.version = MODULE_API_VERSION;
    this->description = description;
    this->trigger = "text/html";
    this->filter = htmlContFilter;
    this->libObject = REF(libObject);

    moduleRegister(CC(this, moduleObject));
}

/* Register the library itself */
void moduleInit(const wordlist *args)
{
    moduleObject *m = new(moduleObject);
    m->chain.typ = FIL_DUMMY;
    m->info.version = MODULE_API_VERSION;
    m->description = "HTML Filter Library";
    moduleRegister(m);
    libObject = m; /* weak reference! */
}
