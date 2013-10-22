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

#ifndef _SCANNER_H
#define _SCANNER_H
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "p3scan.h"

#ifdef PCRE
extern scanner_t scanner_basic;
extern scanner_t scanner_bash;
#endif
extern scanner_t scanner_sophie;
extern scanner_t scanner_clamd;

scanner_t *scannerlist[] = {   
#ifdef PCRE
	&scanner_basic,
	&scanner_bash,
#endif
	&scanner_sophie,
	&scanner_clamd,
	NULL
};

#endif
