/*	PANDA -- a simple transaction monitor

Copyright (C) 1989-2003 Ogochan.

This module is part of PANDA.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the 
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA  02111-1307, USA.
*/

/*
#define	TRACE
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>
#include	"types.h"
#include	"misc.h"
#include	"monstring.h"
#include	"debug.h"

#ifdef	__GNUC__
extern	int
stricmp(
	char	*s1,
	char	*s2)
{	int		ret;

	ret = 0;
	for	( ; *s1  !=  0 ; s1 ++ , s2 ++ )	{
		if		(  ( ret = toupper(*s1) - toupper(*s2) )  !=  0  )
			break;
	}
	return	(ret);
}
extern	int
strnicmp(
	char	*s1,
	char	*s2,
	size_t	l)
{	int		ret;

	ret = 0;
	for	( ; l > 0 ; l -- , s1 ++ , s2 ++ )	{
		if		(  ( ret = toupper(*s1) - toupper(*s2) )  !=  0  )
			break;
	}
	return	(ret);
}
#endif

extern	char	*
StrDup(
	char	*s)
{
	char	*str;

	if		(  s  !=  NULL  ) {
		str = xmalloc(strlen(s)+1);
		strcpy(str,s);
	} else {
		str = NULL;
	}
	return	(str);
}

extern	long
StrToInt(
	char	*str,
	size_t	len)
{	long	ret;

	ret = 0L;
	for	( ; len > 0 ; len -- )	{
		if		(  isdigit(*str)  )	{
			ret = ret * 10 + ( *str - '0' );
		}
		str ++;
	}
	return	(ret);
}

extern	long
HexToInt(
	char	*str,
	size_t	len)
{
	long	ret;

	ret = 0L;
	for	( ; len > 0 ; len -- )	{
		ret <<= 4;
		if		(  isdigit(*str)  )	{
			ret += ( *str - '0' );
		} else
		if		(  isxdigit(*str)  )	{
			ret += ( toupper(*str) - 'A' + 10 );
		}
		str ++;
	}
	return	(ret);
}

extern	char	*
IntToStr(
	char	*str,
	long	val,
	size_t	len)
{
	str += len;
#if	0
	*str = 0;
#endif
	for	( ; len > 0 ; len -- )	{
		str --;
		*str = (char)(( val % 10 ) + '0');
		val /= 10;
	}
	return	(str);
}

extern	char	*
IntStrDup(
	int		val)
{
	char	buff[20];

	sprintf(buff,"%d",val);
	return	(StrDup(buff));
}

extern	char	*
StringChop(
	char	*str)
{
	char	*p;

	p = str + strlen(str) - 1;
	while	(	(  *p  ==  '\r'  )
			||	(  *p  ==  '\n'  ) ) {
		*p = 0;
		if		(  p  ==  str  )	break;
		p --;
	}
	return	(str);
}
