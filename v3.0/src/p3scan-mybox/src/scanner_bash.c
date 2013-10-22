/***************************************************************************
 *   Copyright (C) 2003-2007 by Jack S. Lai                                *
 *   laitcg at gmail dot com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <malloc.h>
#ifdef PCRE
#ifdef PCRE_HEADER
#include <pcre.h>
#endif
#ifdef PCRE_DIR_HEADER
#include <pcre/pcre.h>
#endif
#endif

#include "p3scan.h"
#include "parsefile.h"

extern int checkbuff(int fdc);
extern int checktimeout(struct proxycontext *p);
extern char *strreplace(char *haystack,char *needle,char *rstr);
struct configuration_t * config;

static int scan(struct proxycontext *p, char ** virname){
   int ret,ret2,fdc;
   char * command;
   int len;
   FILE * scanner;
   static char  line[4096*16];
#ifdef PCRE
   pcre * rx;
   const char *pcre_error;
   int pcre_erroffset;
#endif
   int offsets[50];
#define VISIZE 1000
   char *vi=malloc(VISIZE);
   int vipos = 0;
   int vcodndx;

   do_log(LOG_DEBUG, "Bash scanner says hello");

   if (vi==NULL) return SCANNER_RET_ERR;

#ifdef PCRE
   if (config->virusregexp){
      rx = pcre_compile(config->virusregexp, PCRE_UNGREEDY /* | PCRE_CASELESS */ ,
         &pcre_error, &pcre_erroffset, NULL);
      if (!rx) {
         /* should not happen, because init1 has already tested and set to NULL on error */
         do_log(LOG_WARNING, "Ouch! Can't compile regular expression: %s (char %i)",
         pcre_error, pcre_erroffset);
      }
   } else {
      rx=NULL;
   }
#else
   do_log(LOG_EMERG,"PCRE Disabled! Can't use bash scanner!");
#endif
   /*
   %MAILFROM%
   %MAILTO%
   %USERNAME%
   %SUBJECT%
   %MAILDATE%
   %SERVERIP%
   %SERVERPORT%
   %CLIENTIP%
   %CLIENTPORT%
   %PROTOCOL%
   %PROGNAME%
   %VERSION%
   %VDINFO%
   */
   len=strlen(config->virusscanner) + 1 + strlen(p->scanthis) + 1;
   if (strlen(NONULL(paramlist_get(p->params, "%MAILFROM%"))))   len=len+strlen(paramlist_get(p->params, "%MAILFROM%"))+3;
   else{
      len=len+9;
      paramlist_set(p->params, "%MAILFROM%", "(null)");
   }
   if (strlen(NONULL(paramlist_get(p->params, "%MAILTO%"))))     len=len+strlen(paramlist_get(p->params, "%MAILTO%"))+3;
   else{
      len=len+9;
      paramlist_set(p->params, "%MAILTO%", "(null)");
   }
   if (strlen(NONULL(paramlist_get(p->params, "%USERNAME%"))))   len=len+strlen(paramlist_get(p->params, "%USERNAME%"))+3;
   else{
      len=len+9;
      paramlist_set(p->params, "%USERNAME%", "(null)");
   }
   if (strlen(NONULL(paramlist_get(p->params, "%SUBJECT%"))))    len=len+strlen(paramlist_get(p->params, "%SUBJECT%"))+3;
   else{
     len=len+9;
     paramlist_set(p->params, "%SUBJECT%", "(null)");
   }
   if (strlen(NONULL(paramlist_get(p->params, "%MAILDATE%"))))   len=len+strlen(paramlist_get(p->params, "%MAILDATE%"))+3;
   else{
     len=len+9;
     paramlist_set(p->params, "%MAILDATE%", "(null)");
   }
   if (strlen(NONULL(paramlist_get(p->params, "%SERVERIP%"))))   len=len+strlen(paramlist_get(p->params, "%SERVERIP%"))+3;
   else{
     len=len+9;
     paramlist_set(p->params, "%SERVERIP%", "(null)");
   }
   if (strlen(NONULL(paramlist_get(p->params, "%SERVERPORT%")))) len=len+strlen(paramlist_get(p->params, "%SERVERPORT%"))+3;
   else{
     len=len+9;
     paramlist_set(p->params, "%SERVERPORT%", "(null)");
   }
   if (strlen(NONULL(paramlist_get(p->params, "%CLIENTIP%"))))   len=len+strlen(paramlist_get(p->params, "%CLIENTIP%"))+3;
   else{
     len=len+9;
     paramlist_set(p->params, "%CLIENTIP%", "(null)");
   }
   if (strlen(NONULL(paramlist_get(p->params, "%CLIENTPORT%")))) len=len+strlen(paramlist_get(p->params, "%CLIENTPORT%"))+3;
   else{
     len=len+9;
     paramlist_set(p->params, "%CLIENTPORT%", "(null)");
   }
   if (strlen(NONULL(paramlist_get(p->params, "%PROTOCOL%"))))   len=len+strlen(paramlist_get(p->params, "%PROTOCOL%"))+3;
   else{
     len=len+9;
     paramlist_set(p->params, "%PROTOCOL%", "(null)");
   }
   if (strlen(NONULL(paramlist_get(p->params, "%PROGNAME%"))))   len=len+strlen(paramlist_get(p->params, "%PROGNAME%"))+3;
   else{
     len=len+9;
     paramlist_set(p->params, "%PROGNAME%", "(null)");
   }
   if (strlen(NONULL(paramlist_get(p->params, "%VERSION%"))))    len=len+strlen(paramlist_get(p->params, "%VERSION%"))+3;
   else{
     len=len+9;
     paramlist_set(p->params, "%VERSION%", "(null)");
   }
   if (strlen(NONULL(paramlist_get(p->params, "%VDINFO%"))))     len=len+strlen(paramlist_get(p->params, "%VDINFO%"))+3;
   else{
     len=len+9;
     paramlist_set(p->params, "%VDINFO%", "(null)");
   }
   len=len+30;
   if ((command=malloc(len+1))==NULL) return SCANNER_RET_ERR;
   snprintf(command, len, "%s '%s' '%s' '%s' '%s' '%s' '%s' '%s' '%s' '%s' '%s' '%s' '%s' '%s' '%s' 2>&1 ",
            config->virusscanner, p->scanthis,
            strreplace(paramlist_get(p->params, "%MAILFROM%"  ),"'"," "),
            strreplace(paramlist_get(p->params, "%MAILTO%"    ),"'"," "),
            strreplace(paramlist_get(p->params, "%USERNAME%"  ),"'"," "),
            strreplace(paramlist_get(p->params, "%SUBJECT%"   ),"'"," "),
            strreplace(paramlist_get(p->params, "%MAILDATE%"  ),"'"," "),
            paramlist_get(p->params, "%SERVERIP%"  ),
            paramlist_get(p->params, "%SERVERPORT%"),
            paramlist_get(p->params, "%CLIENTIP%"  ),
            paramlist_get(p->params, "%CLIENTPORT%"),
            paramlist_get(p->params, "%PROTOCOL%"  ),
            paramlist_get(p->params, "%PROGNAME%"  ),
            paramlist_get(p->params, "%VERSION%"   ),
            strreplace(paramlist_get(p->params, "%VDINFO%"    ),"'"," "));

   do_log(LOG_DEBUG, "popen %s", command);

   if ((scanner=popen(command, "r"))==NULL){
      do_log(LOG_ALERT, "Can't start scanner '%s' !!!", command);
      free(vi);
      free(command);
      return SCANNER_RET_ERR;
   }
   fdc=fileno(scanner);
   ret2=checkbuff(fdc);
   if (ret2 > 1) return SCANNER_RET_CRIT;
   while (!ret2){
      ret2=checkbuff(fdc);
      if (ret2 > 1) return SCANNER_RET_CRIT;
      ret=checktimeout(p);
      if (ret < 0) return SCANNER_RET_CRIT;
   }
   vi[0]='\0';
   *virname=vi;
   while ((fgets(line, 4095, scanner))!=NULL){
      line[strlen(line)-1]='\0';
      if (config->debug_scanning) do_log(LOG_DEBUG, "ScannerLine: '%s'", line);
#ifdef PCRE
      if (rx){
         ret = pcre_exec(rx, NULL, line, strlen(line), 0, 0, offsets, 50);
         if (ret > config->virusregexpsub){
            len=pcre_copy_substring(line, offsets, ret, config->virusregexpsub, vi+vipos, VISIZE  - vipos -4 );
            if (len==PCRE_ERROR_NOMEMORY) break;
            if (len>0) vipos+=len;
            vi[vipos]=' '; vipos++;
            vi[vipos]='&'; vipos++;
            vi[vipos]=' '; vipos++;
         }
      }
#else
   do_log(LOG_EMERG,"PCRE Disabled!");
#endif
   }
   ret=pclose(scanner);
   free(command);
   if (vipos > 3) vi[vipos-3]='\0';
   do_log(LOG_DEBUG, "vi : '%s'", vi);
#ifdef PCRE
   if (rx) pcre_free(rx);
#endif
   if (!WIFEXITED(ret)){
     do_log(LOG_ALERT, "Scanner returned abnormal signal (%i)", ret);
     return SCANNER_RET_ERR;
   } else
     do_log(LOG_DEBUG, "Scanner returned signal %i", WEXITSTATUS(ret));
     ret=WEXITSTATUS(ret);
     for (vcodndx = 0; vcodndx < MAX_VIRUS_CODES && config->viruscode[vcodndx] != -1; vcodndx++) {
       if (ret == config->viruscode[vcodndx]) return SCANNER_RET_VIRUS; /* contains a virus */
     }
     for (vcodndx = 0; vcodndx < MAX_VIRUS_CODES && config->gvcode[vcodndx] != -1; vcodndx++) {
       if (ret == config->gvcode[vcodndx]){
         ret = 0;
         do_log(LOG_DEBUG, "Basic scanner says goodbye (goodcode)");
         return SCANNER_RET_OK; /* good return code */
       }
     }
     if (ret!=0){
       do_log(LOG_ALERT, "WARNING: Your scanner returned neither 0, a viruscode, nor a good viruscode, but %i", ret);
       return SCANNER_RET_ERR;
     }
     do_log(LOG_DEBUG, "Basic scanner says goodbye");
     return SCANNER_RET_OK; /* all ok, no virus */
}

static int init1(void){
#ifdef PCRE
   pcre * rx;
   const char *pcre_error;
   int pcre_erroffset;
#endif
   if (strlen(NONULL(config->virusscanner))<1){
      do_log(LOG_CRIT, "no scanner was defined. scanning completely disabled");
   return SCANNER_RET_ERR;
   }
   if (strlen(NONULL(config->virusregexp))>0){
#ifdef PCRE
      rx = pcre_compile(config->virusregexp, PCRE_UNGREEDY /* | PCRE_CASELESS */ ,
         &pcre_error, &pcre_erroffset, NULL);
      if (!rx) {
         do_log(LOG_WARNING, "Can't compile regular expression: %s (char %i). Virusnames can't be extracted",
         pcre_error, pcre_erroffset);
         config->virusregexp=NULL;
      } else {
         do_log(LOG_DEBUG, "RX compiled succesfully");
         pcre_free(rx);
      }
#else
      do_log(LOG_EMERG,"PCRE Disabled!");
#endif
   } else {
      config->virusregexp=NULL;
      do_log(LOG_WARNING, "No Regular Expression given! Virusnames can't be extracted");
   }
   return 0;
}

scanner_t scanner_bash = {
   "bash",                          /* name */
   "Bash file invocation scanner",  /* description */
   &init1,                          /* init1 (once, afer startup) */
   NULL,                            /* init2 (every connection before first mail) */
   &scan,                           /* scan */
   NULL,                            /* uninit2 */
   NULL,                            /* uninit1 */
   1                                /* dirscan */
};
