/* mod_script - remove Javascript from HTML pages
   This removes all SCRIPT elements as well as known event handler
   attributes, javascript entities and "mocha" URLs from arbitrary tags.
*/

#include "squid.h"
#include "module.h"
#include "htmlfilter.h"
#include "patfile.h"

#define MODNAME "cfmod_script"

classdef(private jscriptModuleObject, htmlModuleObject, ({
    regex_t jsStyleRE;
    regex_t jsEntityRE;
}))

#define CFMOD_SCRIPT_C
#include <classes.dh>

Ddef(jscriptModuleObject)
{
    regfree(&this->jsStyleRE);
    regfree(&this->jsEntityRE);
}

/* the isJSAttrib function */
#include "jsattrib.c"
/* the HTML_* REs */
#include "jsre.h"

/* SUBMIT tag */
static char *submit_attrs[] = { "INPUT", "TYPE" };
static char *submit_valus[] = { NULL, "SUBMIT" };
#define submit_nattribs 2


/* Currently known ways to specify Javascript scripts:
   <SCRIPT> ... </SCRIPT>
   <STYLE TYPE="text/javascript"> ... </STYLE>  (Netscape)
   <anytag anyparam="&{ ... }">                 (Netscape)
   <anytag anyparam="mocha: ... ">              (Netscape)
   <anytag anyparam="expression(eval( ... ))">  (MSIE)
*/

static int scriptProcessTag(htmlFilterObject *cf, MemBuf *target,
                            int nattribs, char *attrs[], char *valus[])
{
    int i, mod = 0, sub = 0;

    /* Opening SCRIPT: eat this tag and everything that follows */
    if (!strcasecmp(attrs[0], "SCRIPT")) {
        debug(93, 3) (MODNAME ": %s: removing SCRIPT\n",
		      moduleUrlClean(cf->uri));
        cf->eating = 1;
        return 1;
    }

    /* Closing SCRIPT: eat this tag, resume normal operation */
    if (cf->eating==1 && !strcasecmp(attrs[0], "/SCRIPT")) {
        cf->eating = 0;
        return 1;
    }

    /* Opening STYLE: start eat if javascript style */
    if (!strcasecmp(attrs[0], "STYLE"))
        for (i=1; i<nattribs; ++i)
            if (attrs[i] && valus[i] &&
                !regexec(&CC(cf->owner, jscriptModuleObject)->jsStyleRE,
                         valus[i], 0, NULL, 0)) {
                debug(93, 3) (MODNAME ": %s: removing javascript STYLE\n",
                              moduleUrlClean(cf->uri));
                cf->eating = 2;
                return 1;
            }

    /* Closing STYLE: eat this tag, resume normal operation */
    if (cf->eating==2 && !strcasecmp(attrs[0], "/STYLE")) {
        cf->eating = 0;
        return 1;
    }

    /* Remove Javascript attributes from tags */
    for (i=1; i<nattribs; ++i) {
        if (attrs[i] && isJSAttrib(attrs[i], strlen(attrs[i]))) {
            debug(93, 3) (MODNAME ": %s: removing %s from %s\n",
                          moduleUrlClean(cf->uri), attrs[i], attrs[0]);
            mod = 1;
            /* If the Javascript attribute contains a submit function,
               fake a submit button. Otherwise we may end up with
               a non-submittable form */
            if (valus[i] && strstr(valus[i], "submit()"))
                sub = 1;
            attrs[i] = NULL;
        } else if (attrs[i] && valus[i] &&
            !regexec(&CC(cf->owner, jscriptModuleObject)->jsEntityRE,
                     valus[i], 0, NULL, 0)) {
            debug(93, 3) (MODNAME ": %s: removing %s from %s\n",
                          moduleUrlClean(cf->uri), valus[i], attrs[0]);
            if (valus[i] && strstr(valus[i], "submit()"))
                sub = 1;
            mod = 1;
            attrs[i] = NULL;
        }
    }
    if (!mod)
        return 0;

    /* Insert the cleaned tag */
    insertTag(cf, target, nattribs, attrs, valus);
    if (sub) {
        debug(93, 3) (MODNAME ": %s: faking SUBMIT\n",
		      moduleUrlClean(cf->uri));
        insertTag(cf, target, submit_nattribs, submit_attrs, submit_valus);
    }
    return 1;
}

void moduleInit(const wordlist *args)
{
    jscriptModuleObject *m = new(jscriptModuleObject);
    patFileObject *pf = args ? patfileNew(args->key, 0) : NULL;
    int e;

    m->patFile = REF(pf);
    m->processTag = scriptProcessTag;
    e = regcomp(&m->jsStyleRE,  "text/javascript",
                REG_EXTENDED|REG_ICASE|REG_NOSUB);
    if (e)
        debug(93, 1) (MODNAME ": regcomp 1: %d\n", e);
    e = regcomp(&m->jsEntityRE,
                "&\\{.*\\}|" HTML_mocha ":|" HTML_javascript ":|"
                HTML_livescript ":|" HTML_eval "\\(.*\\)",
                REG_EXTENDED|REG_ICASE|REG_NOSUB);
    if (e)
        debug(93, 1) (MODNAME ": regcomp 2: %d\n", e);
    htmlfilterRegister(CC(m, htmlModuleObject), "Javascript filter");
}
