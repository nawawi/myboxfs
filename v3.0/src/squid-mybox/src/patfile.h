/* The pattern file library.
 */

#ifndef PATFILE_H
#define PATFILE_H

#include "module.h"

/* Create a patFileObject.
   name - the file name.
   replace - 0 if a "match" aka "allow list" file, 1 if a "replacement" file.
*/
patFileObject *patfileNew(const char *name, int replace);

/* Check if file has changed, reload if necessary.
   Usually called before each patfileMatch().
*/
void patfileCheckReload(patFileObject *this);

/* Unload the patterns, leave empty. Usually no need to call this. */
void patfileUnload(patFileObject *this);

/* Check for match.
   uri - the URI to check.
   Returns - NULL, if no match was found.
             uri, if match was found in a "match" file.
             replacement, if match was found in a "replacement" file.
*/
char *patfileMatch(patFileObject *this, const char *uri);
const char *myboxuri="mybox.internal.proxy";
#endif
