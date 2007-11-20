/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 1989-2007 Ogochan.
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

#define	DEBUG
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
#include	"monstring.h"
#include	"memory_v.h"
#include	"debug.h"

#ifdef	__GNUC__
extern	int
stricmp(
	char	*s1,
	char	*s2)
{
	for	( ; *s1  !=  0 ; s1 ++ , s2 ++ )	{
		if		(  toupper(*s1)  !=  toupper(*s2)  )
			break;
	}
	return	(toupper(*s1) - toupper(*s2));
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
{
	long	ret
		,	sign;

	if		(  *str  ==  '-'  ) {
		sign = -1;
		str ++;
	} else {
		sign = 1;
	}
	ret = 0L;
	for	( ; len > 0 ; len -- )	{
		if		(  isdigit(*str)  )	{
			ret = ret * 10 + ( *str - '0' );
		}
		str ++;
	}
	return	(ret*sign);
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

	if		(  ( p = strrchr(str,'\r') )  !=  NULL  ) {
		*p = 0;
	}
	if		(  ( p = strrchr(str,'\n') )  !=  NULL  ) {
		*p = 0;
	}
	return	(str);
}

/*
 *	for utf8
 */
extern	size_t
CharLength(
	byte	c)
{
	size_t	len;

	if		(  ( c & 0x80 )  ==  0  ) {
		len = 1;
	} else
	if		(  ( c & 0xE0 )  ==  0xC0  ) {
		len = 2;
	} else
	if		(  ( c & 0xE0 )  ==  0xE0  ) {
		len = 3;
	} else
	if		(  ( c & 0xF8 )  ==  0xF0  ) {
		len = 4;
	} else {
		len = 0;
	}
	return	(len);
}
