/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2001-2003 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2004-2008 Ogochan.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
#define	DEBUG
#define	TRACE
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include	<errno.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<glib.h>

#include	"types.h"
#include	"misc_v.h"

#include	"value.h"
#include	"cobolvalue.h"
#include	"debug.h"


extern	void
StringCobol2C(
	char	*str,
	size_t	size)
{
	char	*p;

	if (*str == 0){
		return;
	}
	
	for	( p = str + size - 1 ; p >= str ; p -- ) {
		if		(  *p  ==  ' '  ) {
			*p = 0;
		} else
		if		(  *p  ==  0  ) {
			/*	*/
		} else
			break;
	}
}

extern	void
StringC2Cobol(
	char	*p,
	size_t	size)
{
	int		i;
	size_t		left;

	left = size;
	while ( (*p != 0) && (left > 0) ){
		++p;
		--left;
	}
	memset(p, ' ', left);
	p += left;
}

