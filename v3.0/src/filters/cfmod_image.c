/* mod_image - remove Image from HTML pages
   nawawi - 21-Jul-2007
*/
#define _GNU_SOURCE
#include <string.h>
#include "squid.h"
#include "module.h"
#include "htmlfilter.h"
#include "patfile.h"

#define MODNAME "cfmod_image"

#define CFMOD_IMAGE_C
#include <classes.dh>

static int imageProcessTag(htmlFilterObject *cf, MemBuf *target, int nattribs, char *attrs[], char *valus[]) {
        if(!strcasecmp(attrs[0], "IMG")) {
                debug(93, 3) (MODNAME ": %s: removing IMAGE\n",moduleUrlClean(cf->uri));
                return 1;
        }
	int i, x;
        for(i=1; i<nattribs; i++) {
        	if(attrs[i] && valus[i] && !strcasecmp(attrs[i], "BACKGROUND")) {
			memBufAppend(target, "<", 1);
			memBufAppend(target, attrs[0], strlen(attrs[0]));
			for(x=1;x<nattribs; x++) {
				if(x==i) continue;
				memBufAppend(target, " ", 1);
				memBufAppend(target, attrs[x], strlen(attrs[x]));
				memBufAppend(target, "=", 1);
				memBufAppend(target, valus[x], strlen(valus[x]));
			}
			memBufAppend(target, ">", 1);
			debug(93, 3) (MODNAME ": %s: removing BACKGROUND IMAGE\n",moduleUrlClean(cf->uri));
			return 1;
		}
		// img in stylesheet
		if(attrs[i] && valus[i] && !strcasecmp(attrs[i], "STYLE") && strcasestr(valus[i],"background-image")!=NULL) {
			memBufAppend(target, "<", 1);
			memBufAppend(target, attrs[0], strlen(attrs[0]));
			for(x=1;x<nattribs; x++) {
				if(x==i) continue;
				memBufAppend(target, " ", 1);
				memBufAppend(target, attrs[x], strlen(attrs[x]));
				memBufAppend(target, "=", 1);
				memBufAppend(target, valus[x], strlen(valus[x]));
			}
			memBufAppend(target, ">", 1);
			debug(93, 3) (MODNAME ": %s: removing BACKGROUND IMAGE\n",moduleUrlClean(cf->uri));
			return 1;
		}
	}
        return 0;
}

void moduleInit(const wordlist *args) {
    htmlModuleObject *m = new(htmlModuleObject);
    patFileObject *pf = args ? patfileNew(args->key, 0) : NULL;
    m->patFile = REF(pf);
    m->processTag = imageProcessTag;
    htmlfilterRegister(m, "Image filter");
}
