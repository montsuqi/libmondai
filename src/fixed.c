/*
 * libmondai -- MONTSUQI data access library
 * 
 * Copyright (C) 2000-2004 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2005-2008 Ogochan.
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
#include	"monstring.h"
#include	"value.h"
#include	"memory_v.h"
#include	"fixed_v.h"
#include	"debug.h"

extern	void
FreeFixed(
	Fixed	*xval)
{
	if		(  xval->sval  !=  NULL  ) {
		xfree(xval->sval);
	}
	xfree(xval);
}

extern	Fixed	*
NewFixed(
	int		flen,
	int		slen)
{
	Fixed	*xval;

	xval = New(Fixed);
	xval->flen = flen;
	xval->slen = slen;
	if		(  flen  >  0  ) {
		xval->sval = (char *)xmalloc(flen+1);
	}
	return	(xval);
}

extern	void
FixedRescale(
	Fixed	*to,
	Fixed	*fr)
{
	char	*p
	,		*q;
	size_t	len;
	Bool	fMinus;

	if		(  ( *fr->sval & 0x40 )  ==  0x40  ) {
		fMinus = TRUE;
	} else {
		fMinus = FALSE;
	}
	memset(to->sval,'0',to->flen);
	to->sval[to->flen] = 0;
	p = fr->sval + ( fr->flen - fr->slen );
	q = to->sval + ( to->flen - to->slen );
	len = ( fr->slen > to->slen ) ? to->slen : fr->slen;
	for	( ; len > 0 ; len -- ) {
		*q ++ = ( *p & 0x3F );
		p ++;
	}
	p = fr->sval + ( fr->flen - fr->slen ) - 1;
	q = to->sval + ( to->flen - to->slen ) - 1;
	len = ( ( fr->flen - fr->slen ) > ( to->flen - to->slen ) )
		? ( to->flen - to->slen ) : ( fr->flen - fr->slen );
	for	( ; len > 0 ; len -- ) {
		*q -- = ( *p & 0x3F );
		p --;
	}
	if		(  fMinus  ) {
		*to->sval |= 0x40;
	}
}

extern	void
FloatToFixed(
	Fixed	*xval,
	double	fval)
{
	char	str[SIZE_NUMBUF+1];
	char	*p
	,		*q;
	Bool	fMinus;
	int len;

	if		(  fval  <  0  ) {
		fval = - fval;
		fMinus = TRUE;
	} else {
		fMinus = FALSE;
	}

	len = (int)xval->flen;
	if (xval->slen > 0) {
            /* ignore decimal point */
	    len++;
	}
	sprintf(str, "%0*.*f", len, (int)xval->slen, fval);
	p = str;
	q = xval->sval;
	while	(  *p  !=  0  ) {
		if		( *p  !=  '.'  ) {
			*q = *p;
			q ++;
		}
		p ++;
	}
	if		(  fMinus  ) {
		*xval->sval |= 0x40;
	}
}

extern	void
IntToFixed(
	Fixed	*xval,
	int		ival)
{
	char	str[SIZE_NUMBUF+1];
	Bool	fMinus;

	if		(  ival  <  0  ) {
		ival = - ival;
		fMinus = TRUE;
	} else {
		fMinus = FALSE;
	}
	sprintf(str, "%d",ival);
	xval->sval = StrDup(str);
	xval->flen = strlen(str);
	xval->slen = 0;
	if		(  fMinus  ) {
		*xval->sval |= 0x40;
	}
}

extern	int
FixedToInt(
	Fixed	*xval)
{
	int		ival;
	int		i;

	ival = StrToInt(xval->sval,strlen(xval->sval));
	for	( i = 0 ; i < xval->slen ; i ++ ) {
		ival /= 10;
	}
	return	(ival);
}

extern	double
FixedToFloat(
	Fixed	*xval)
{
	double	fval;
	int		i;
	Bool	fMinus;
	unsigned char work[SIZE_NUMBUF+1];

	strcpy(work,xval->sval);
	if		(  *work  >=  0x70  ) {
		*work ^= 0x40;
		fMinus = TRUE;
	} else {
		fMinus = FALSE;
	}
	fval = atof(work);
	for	( i = 0 ; i < xval->slen ; i ++ ) {
		fval /= 10.0;
	}
	if		(  fMinus  ) {
		fval = - fval;
	}
	return	(fval);
}

