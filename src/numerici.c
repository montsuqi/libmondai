/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2003 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2004-2007 Ogochan.
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

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<glib.h>
#include	<math.h>

#include	"types.h"
#include	"misc_v.h"
#include	"memory_v.h"
#include	"value.h"
#include	"numeric.h"
#include	"debug.h"

extern	Numeric
FixedToNumeric(
	Fixed	*xval)
{
	Numeric	value
	,		value2;
	Bool	fMinus;

	if		(  ( xval->sval[0] & 0x40 )  !=  0  ) {
		xval->sval[0] &= 0x3F;
		fMinus = TRUE;
	} else {
		fMinus = FALSE;
	}
	value = NumericInput(xval->sval,xval->flen,xval->slen);
	value2 = NumericShift(value,- xval->slen);
	NumericFree(value);
	if		(  fMinus  ) {
		value = NumericUMinus(value2);
		NumericFree(value2);
	} else {
		value = value2;
	}
	return	(value);
}

extern	char	*
NumericToFixed(
	Numeric	value,
	int		precision,
	int		scale)
{
	char	*fr
	,		*to;
	char	*p
	,		*q;
	char	*str;
	size_t	len;
	Bool	fMinus;

	str = NumericOutput(value);
	fr = str;
	if		(  *fr  ==  '-'  ) {
		fMinus = TRUE;
		fr ++;
	} else {
		fMinus = FALSE;
	}
	to = (char *)xmalloc(precision+1);
	memset(to,'0',precision);
	to[precision] = 0;
	if		(  ( p = strchr(fr,'.') )  !=  NULL  ) {
		p ++;
		q = to + ( precision - scale );
		len = ( strlen(p) > scale ) ? scale : strlen(p);
		for	( ; len > 0 ; len -- ) {
			*q ++ = *p ++;
		}
	}
	if		(  ( p = strchr(fr,'.') )  !=  NULL  ) {
		p --;
	} else {
		p = fr + strlen(fr) - 1;
	}
	q = to + ( precision - scale ) - 1;
	len = ( ( p - fr + 1 ) > ( precision - scale ) )
		? ( precision - scale ) : ( p - fr + 1 );
	for	( ; len > 0 ; len -- ) {
		*q -- = *p --;
	}
	if		(  fMinus  ) {
		*to |= 0x40;
	}
	xfree(str);
	return	(to);
}

