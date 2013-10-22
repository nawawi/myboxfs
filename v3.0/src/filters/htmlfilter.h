#ifndef HTMLFILTER_H
#define HTMLFILTER_H

#include "squid.h"
#include "module.h"

/* Use of htmlfilter:

   Create a htmlModuleObject (or a subclass thereof) and set its
   processTag element, then call htmlfilterRegister() with that object
   (instead of moduleRegister()). This arranges for the processTag
   function to be called for every tag encountered in the source with
   the following arguments:

   cf - an htmlFilterObject structure containing info about the processed file
   target - target buffer
   nattribs - max no of elements in the following arrays
   attribs - attributes array
   values - values array
   attribs[0] is the tag name.
   attribs[x]==NULL means "ignore this".
   values[x]==NULL means "attribute without value".

   This routine may call insertTag() with arguments like itself to
   insert an HTML tag into the output stream. It may modify attrs and
   valus and the buffers they point to; these are temp buffers and
   such modifications are ignored after exit from this function. It
   should return 1 if it has processed the tag. If it returns 0,
   htmlfilter.c inserts the original tag (unmodified) into the output.

   It may also set cf->eating. As long as this is non-zero, output is
   suppressed.
*/

typedef enum {  /* parsing state */
    T_TEXT=0,           /* in ordinary text */
    T_STAG,             /* at start of tag */
    T_TAG,              /* inside tag */
    T_EQU,              /* inside tag after an equals sign */
    T_META,             /* inside a meta-tag */
    T_COMM,             /* inside a comment */
    T_DQUOT='"',        /* in double-quoted string */
    T_SQUOT='\''        /* in single-quoted string */
} tpos;

typedef int processTagFunc(htmlFilterObject *cf, MemBuf *target,
                           int nattribs, char *attribs[], char *values[]);

classdef(private htmlModuleObject, moduleObject, ({
    moduleObject *libObject;    /* Lock to htmlfilter library */
    processTagFunc *processTag; /* Do-it func SET BY SUB */
}))

classdef(private htmlFilterObject, filterObject, ({
    int contlen;                /* Content-Length of original HTML page */
    int posin, posout;          /* Input/output positions */
    MemBuf tag;                 /* Buffer holding incomplete tag */
    tpos inTag;                 /* current parsing state */
    int inBSQuot;               /* currently parsing \-quote? */
    int eating;                 /* Currently in eat mode? */
}))

/* Insert a tag into the output stream */
void insertTag(htmlFilterObject *cf, MemBuf *target,
               int nattribs, char *attribs[], char *values[]);

/* Init and register a client module */
void htmlfilterRegister(htmlModuleObject *this, const char *description);

#endif
