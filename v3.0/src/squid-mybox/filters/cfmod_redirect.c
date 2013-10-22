
#include "squid.h"
#include "module.h"
#include "patfile.h"
#include "lib.h"

#define MODNAME "mod_redirect"
#define REDIRECTFILE "/etc/cf/redirect"
#define REDIRECT_C
#include <classes.dh>

/* moduleObject->filter for FIL_REDIRECT:
   arg is the requested URI.
   Returns the redirected URI. Returns NULL if no redirection.
*/
static void *redirFilter(moduleObject *this, const void *arg) {
	//const char *uri = arg;
 	char *r;
	const redParam *rd = arg;
	if(strstr(rd->uri,myboxuri)) return NULL;
    	if(this->patFile && rd->client_addr && (patfileCheckReload(this->patFile),patfileMatch(this->patFile, rd->uri))) return NULL;
    	if(this->patFile && rd->client_addr && (patfileCheckReload(this->patFile),patfileMatch(this->patFile, rd->client_addr))) return NULL;
    	if(this->patFile && rd->auth_user && (patfileCheckReload(this->patFile),patfileMatch(this->patFile, rd->auth_user))) return NULL;
    
	patfileCheckReload(this->patFile);
	r = patfileMatch(this->patFile, rd->uri);
	if (r) debug(93, 3) (MODNAME ": %s : redirect to '%s'\n", moduleUrlClean(rd->uri), r);
	return r;
}

void moduleInit(const wordlist *args) {
	moduleObject *m;
	patFileObject *pf;
	if(__file_exists(REDIRECTFILE)) {
		pf=patfileNew(REDIRECTFILE,1);
	} else {
		debug(93, 1) (MODNAME ": file '%s' not found\n",REDIRECTFILE);
        	return;
	}
    	m = new(moduleObject);
    	m->chain.typ = FIL_REDIRECT;
    	m->info.version = MODULE_API_VERSION;
    	m->description = "URI Redirector";
    	m->filter = redirFilter;
    	//pf = patfileNew(args->key, 1);
    	m->patFile = REF(pf);
    	moduleRegister(m);
}
