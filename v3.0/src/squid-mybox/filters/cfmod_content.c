/* 
	mod_content - remove Javascript/Script/Flash/Java/Image from HTML pages
*/

#include "squid.h"
#include "module.h"
#include "htmlfilter.h"
#include "patfile.h"
#include "lib.h"

#define MODNAME "cfmod_content"
#define ALLOWFILE "/etc/cf/content.allow"

classdef(private scriptModuleObject, htmlModuleObject, ({
	regex_t jsStyleRE;
	regex_t jsEntityRE;
	int js;
	int image;
	int object;
}))

#define CFMOD_CONTENT_C
#include <classes.dh>

Ddef(scriptModuleObject)
{
	regfree(&this->jsStyleRE);
	regfree(&this->jsEntityRE);
	this->js=0;
	this->image=0;
	this->object=0;
}

/* the isJSAttrib function */
#include "jsattrib.c"
/* the HTML_* REs */
#include "jsre.h"

/* SUBMIT tag */
static char *submit_attrs[]={ "INPUT", "TYPE" };
static char *submit_valus[]={ NULL, "SUBMIT" };
#define submit_nattribs 2

static int scriptProcessTag(htmlFilterObject *cf, MemBuf *target,int nattribs, char *attrs[], char *valus[]) {
	int i, mod=0, sub=0, x=0;
   
	for (i=1; i<nattribs; ++i) {
		if(valus[i] && strstr(valus[i],myboxuri)) {
			return 0;
		}
	}

	if(CC(cf->owner, scriptModuleObject)->image) {
		if(!strcasecmp(attrs[0], "IMG")) {
			debug(93, 3) (MODNAME ": %s: removing IMAGE\n",moduleUrlClean(cf->uri));
			return 1;
		}
		for(i=1; i<nattribs; i++) {
			if(attrs[i] && valus[i] && !strcasecmp(attrs[i], "BACKGROUND")) {
				attrs[i]=NULL;
				valus[i]=NULL;
				x=1;
			}
			// img in stylesheet
			if(attrs[i] && valus[i] && !strcasecmp(attrs[i], "STYLE") && strcasestr(valus[i],"background-image")!=NULL) {
				attrs[i]=NULL;
				valus[i]=NULL;
				x=1;
			}
		}
		if(x!=0) {
			debug(93, 3) (MODNAME ": %s: removing IMAGE\n",moduleUrlClean(cf->uri));
			insertTag(cf, target, nattribs, attrs, valus);
			return 1;
		}
        }

	if(CC(cf->owner, scriptModuleObject)->object) {
		if(!strcasecmp(attrs[0], "OBJECT") || !strcasecmp(attrs[0], "EMBED") || !strcasecmp(attrs[0], "PARAM")) {
			debug(93, 3) (MODNAME ": %s: removing ActiveX/Java/Flash OBJECT\n",moduleUrlClean(cf->uri));
			cf->eating = 3;
			return 1;
		}
 		if(cf->eating==3 && (!strcasecmp(attrs[0], "/OBJECT") || !strcasecmp(attrs[0], "/EMBED") || !strcasecmp(attrs[0], "/PARAM"))) {
			cf->eating = 0;
			return 1;
		}
	}

	if(CC(cf->owner, scriptModuleObject)->js) {
		if(!strcasecmp(attrs[0], "SCRIPT")) {
			debug(93, 3) (MODNAME ": %s: removing SCRIPT\n",moduleUrlClean(cf->uri));
			cf->eating=1;
			return 1;
		}
		if(cf->eating==1 && !strcasecmp(attrs[0], "/SCRIPT")) {
			cf->eating=0;
			return 1;
		}
		if(!strcasecmp(attrs[0], "STYLE")) {
			for (i=1; i<nattribs; ++i) {
				if(attrs[i] && valus[i] &&!regexec(&CC(cf->owner, scriptModuleObject)->jsStyleRE,valus[i], 0, NULL, 0)) {
 					debug(93, 3) (MODNAME ": %s: removing javascript STYLE\n",moduleUrlClean(cf->uri));
					cf->eating=2;
					return 1;
				}
			}
		}
		if(cf->eating==2 && !strcasecmp(attrs[0], "/STYLE")) {
			cf->eating=0;
			return 1;
		}

		for (i=1; i<nattribs; ++i) {
			if(attrs[i] && isJSAttrib(attrs[i], strlen(attrs[i]))) {
				debug(93, 3) (MODNAME ": %s: removing %s from %s\n",moduleUrlClean(cf->uri), attrs[i], attrs[0]);
				mod=1;
				/* If the Javascript attribute contains a submit function,
				fake a submit button. Otherwise we may end up with
				a non-submittable form */
				if(valus[i] && strstr(valus[i], "submit()")) sub=1;
				attrs[i]=NULL;
			} else if(attrs[i] && valus[i] && !regexec(&CC(cf->owner, scriptModuleObject)->jsEntityRE,valus[i], 0, NULL, 0)) {
				debug(93, 3) (MODNAME ": %s: removing %s from %s\n",moduleUrlClean(cf->uri), valus[i], attrs[0]);
				if(valus[i] && strstr(valus[i], "submit()")) sub=1;
				mod=1;
				attrs[i]=NULL;
			}
		}
		if(!mod) return 0;

		/* Insert the cleaned tag */
		insertTag(cf, target, nattribs, attrs, valus);
		if(sub) {
			debug(93, 3) (MODNAME ": %s: faking SUBMIT\n",moduleUrlClean(cf->uri));
 			insertTag(cf, target, submit_nattribs, submit_attrs, submit_valus);
		}
		return 1;
	}
	return 0;
}

void moduleInit(const wordlist *args) {
	scriptModuleObject *m=new(scriptModuleObject);
	patFileObject *pf;
	if(!args) {
		debug(93, 1) (MODNAME ": needs argument\n");
		return;
	}

	if(__file_exists(ALLOWFILE)) {
		pf=patfileNew(ALLOWFILE,0);
	} else {
		pf=NULL;
	}
	if(args) {
		if(strstr(args->key,"js")) m->js=1;
                if(strstr(args->key,"image")) m->image=1;
                if(strstr(args->key,"object")) m->object=1;
	}
	m->patFile=REF(pf);
	m->processTag=scriptProcessTag;
	if(m->js) {
		int e;

		e=regcomp(&m->jsStyleRE,  "text/javascript",REG_EXTENDED|REG_ICASE|REG_NOSUB);
		if(e) debug(93, 1) (MODNAME ": regcomp 1: %d\n", e);
		e=regcomp(&m->jsEntityRE,"&\\{.*\\}|" HTML_mocha ":|" HTML_javascript ":|"HTML_livescript ":|" HTML_eval "\\(.*\\)",REG_EXTENDED|REG_ICASE|REG_NOSUB);
		if(e) debug(93, 1) (MODNAME ": regcomp 2: %d\n", e);
	}

	htmlfilterRegister(CC(m, htmlModuleObject), "Content removal filter");
}
