/*
 * DEBUG: section 94     Pattern file library for filters
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
#include "patfile.h"

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#define BUFINCR 1024            /* Size increment of replacement buffer */

typedef struct _patChain {
    struct _patChain *next;             /* next in chain */
    regex_t patbuf;                     /* compiled pattern */
    union {
        int negate;                     /* non-replace: is negative match */
        char *replace;                  /* replace: replacement string */
    } action;
} patChain;

classdef(patFileObject, Object, ({
    struct _patChain *chain;            /* the pattern chain */
    char *fileName;                     /* name of pattern file */
    time_t mtime;                       /* modtime of pattern file */
    int replace;                        /* Process replace rules? */
    char *buf;                          /* buffer space */
    size_t buflen;                      /* buffer size */
}))

#define PATFILE_C
#include <classes.dh>

Ddef(patFileObject)
{
    patfileUnload(this);
    xfree(leakFree(this->fileName));
    xfree(this->buf);
}

patFileObject *patfileNew(const char *name, int replace)
{
    patFileObject *p = new(patFileObject);
    p->fileName = leakAdd(xstrdup(name));
    p->replace = replace;
    return p;
}

void patfileCheckReload(patFileObject *this)
{
    struct stat st;
    FILE *f;
    int ln = 0, np = 0, e, neg, fl;
    int skip;
    char buf[256];
    char *p0, *p1, *p2, *bang = NULL;
    patChain *n, *l = NULL;

    if (stat(this->fileName, &st)<0) {
        debug(94, 1) ("%s: stat: %s\n", this->fileName, xstrerror());
        return;
    }
    if (this->mtime >= st.st_mtime)
        return;
    this->mtime = st.st_mtime;

    f = fopen(this->fileName, "r");
    if (!f) {
        debug(94, 1) ("%s: open: %s\n", this->fileName, xstrerror());
        return;
    }
    debug(94, 4) ("patfile: reloading %s\n", this->fileName);
    patfileUnload(this);

    while (fgets(buf, sizeof(buf), f)) {
        ++ln;
        neg = 0;
	skip=0;
        fl = this->replace ? REG_EXTENDED : REG_EXTENDED|REG_NOSUB;
        for (p0=buf; *p0==' ' || *p0=='\t'; ++p0);
        if (*p0=='#' || *p0=='\n' || *p0=='\0')
            continue;
        for (p1=p0; *p1 && *p1!='\t' && *p1!='\n'; ++p1);
        if (*p1) {
            for (*p1++='\0'; *p1=='\t'; ++p1);
            for (p2=p1; *p2 && *p2!='\n'; ++p2);
            *p2='\0';
            if (*p1=='\t' || *p1=='\n' || *p1=='!' || *p1=='\0')
                p1=NULL;
        } else {
            p1=NULL;
        }
        if (*p0=='-') {
            ++p0;
            fl |= REG_ICASE;
        }
        if (*p0=='!') {
            if (this->replace) {
                if (bang)
                    debug(94, 1) ("%s:%d: duplicate reject definition\n",
                                  this->fileName, ln);
                else
                    if (p1)
                        bang=xstrdup(p1);
                continue;
            } else {
                ++p0;
                neg = 1;
            }
        }
	if(this->replace && *p0=='%') {
		p0++;
		skip=1;
	}
        n = leakAdd(xmalloc(sizeof(patChain)));
        e = regcomp(&n->patbuf, p0, fl);
        if (e) {
            (void)regerror(e, &n->patbuf, buf, sizeof(buf));
            debug(94, 1) ("%s:%d: regex error: %s\n", this->fileName, ln, buf);
            xfree(n);
            continue;
        }
        if (this->replace) {
		if(skip) {
			n->action.replace = xstrdup(p0);
		} else {
            		if (p1) {
                		n->action.replace = xstrdup(p1);
            		} else if (bang) {
                		n->action.replace = xstrdup(bang);
            		} else {
				debug(94, 1) ("%s:%d: missing replacement\n",this->fileName, ln);
                		n->action.replace = NULL;
	    		}
		}
        } else {
            n->action.negate = neg;
        }
        n->next = NULL;
        if (l)
            l->next = n;
        else
            this->chain = n;
        l = n;
        ++np;
    }
    fclose(f);
    if (bang)
        xfree(bang);
    debug(94, 4) ("%s: loaded %d patterns\n", this->fileName, np);
}

void patfileUnload(patFileObject *this)
{
    patChain *p0, *p;
    for (p = this->chain; p; p = p0) {
        p0 = p->next;
        regfree(&p->patbuf);
        if (this->replace)
            xfree(p->action.replace);
        xfree(leakFree(p));
    }
    this->chain = NULL;
}

#define MAXSUBPAT 10

/* Do the \0..\9 substitutions in the replacement pattern */
static char *patfileSubst(patFileObject *this, const char *uri,
			  patChain *r, const regmatch_t *subs)
{
    char *p0;
    int n, i, k;
    char c;

    if (!(p0 = r->action.replace))
	return NULL; /* Should return generic reject URI */

	if(strcmp(uri,p0)==0) {
		debug(93, 3) ("TEST TEST TEST %s %s\n", uri, p0);
		return p0; // skip
	}
    n = 0;
    i = k = -1;
    while (*p0 || i>=0) {
	if (k >= 0) {
	    if (i < subs[k].rm_eo) {
		c = uri[i++];
	    } else {
		k = i = -1;
		continue;
	    }
	} else if (*p0=='\\' && p0[1]>='0' && p0[1]<='9') {
	    k = p0[1] - '0';
	    p0 += 2;
	    if ((i = subs[k].rm_so) < 0)
		k = -1;
	    continue;
	} else {
	    c = *p0++;
	}
	if (n+1 >= this->buflen) {
	    this->buflen += BUFINCR;
	    this->buf = xrealloc(this->buf, this->buflen);
	}
	this->buf[n++] = c;
    }
    this->buf[n] = '\0';
    return this->buf;
}

char *patfileMatch(patFileObject *this, const char *uri)
{
    patChain *r;
    regmatch_t subs[MAXSUBPAT];
    int i;

    for (r=this->chain, i=0; r; r=r->next, ++i) {
        if (!regexec(&r->patbuf, uri, MAXSUBPAT, subs, 0)) {
            debug(94, 8) ("patfileMatch: matched pattern %d\n", i);
	    if (this->replace)
		return patfileSubst(this, uri, r, subs);
	    else
                return r->action.negate ? NULL : (char*)uri; /* XX */
        }
    }
    return NULL;
}

