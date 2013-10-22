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

#ifndef _PARSEFILE_H
#define _PARSEFILE_H
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

typedef struct paramlist {
    char * name;
    char * value;
    struct paramlist * next;
} paramlist;

/* parses infile to outfile, all words given in params will be replaced
 * leading (\r\n or \n) is one of WRITELINE_LEADING_[NONE|N|RN]
 */
int parsefile(char * infile, char * outfile, paramlist * params, int leading);

/* parses file descriptor in to out, all words given in params will be replaced
 * leading (\r\n or \n) is one of WRITELINE_LEADING_[NONE|N|RN]
 */
int parsefds(int in, int out , paramlist * params, int leading);

/* Adds/Updates name to paramlist.
 * To delete a name, call it with value set to NULL.
 */
int paramlist_set(struct paramlist * params, const char * name, const char * value);

/* returnes the value of name */
char * paramlist_get(struct paramlist * params, const char * name);

/* initialize paramlist */
struct paramlist * paramlist_init(void);
    
/* unitializes the paramlist */
void paramlist_uninit(struct paramlist ** params);

#endif /* ifndef _PARSEFILE_H */
