/*	PANDA -- a simple transaction monitor

Copyright (C) 2000-2003 Ogochan & JMA (Japan Medical Association).

This module is part of PANDA.

	PANDA is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves
any particular purpose or works at all, unless he says so in writing.
Refer to the GNU General Public License for full details. 

	Everyone is granted permission to copy, modify and redistribute
PANDA, but only under the conditions described in the GNU General
Public License.  A copy of this license is supposed to have been given
to you along with PANDA so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies. 
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
#include	"misc.h"
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

