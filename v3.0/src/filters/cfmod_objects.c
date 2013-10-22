/* mod_object - remove ActiveX/Java/Flash OBJECTs from HTML pages
   nawawi - 21-Jul-2007
*/

#include "squid.h"
#include "module.h"
#include "htmlfilter.h"
#include "patfile.h"

#define MODNAME	"cfmod_object"

#define CFMOD_OBJECT_C
#include <classes.dh>

static int objectProcessTag(htmlFilterObject *cf, MemBuf *target,int nattribs, char *attrs[], char *valus[]) {
	if(!strcasecmp(attrs[0], "OBJECT") || !strcasecmp(attrs[0], "EMBED") || !strcasecmp(attrs[0], "PARAM")) {
		debug(93, 3) (MODNAME ": %s: removing ActiveX/Java/Flash OBJECT\n",moduleUrlClean(cf->uri));
		cf->eating = 1;
		return 1;
	}
 	if(cf->eating==1 && (!strcasecmp(attrs[0], "/OBJECT") || !strcasecmp(attrs[0], "/EMBED") ||
		!strcasecmp(attrs[0], "/PARAM")) ) {
		cf->eating = 0;
		return 1;
	}
	return 0;
}

void moduleInit(const wordlist *args) {
	htmlModuleObject *m = new(htmlModuleObject);
	patFileObject *pf = args ? patfileNew(args->key, 0) : NULL;
	m->patFile = REF(pf);
	m->processTag = objectProcessTag;
	htmlfilterRegister(m, "ActiveX/Java/Flash filter");
}
