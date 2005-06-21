/*
libmondai -- MONTSUQI data access library
Copyright (C) 1989-2005 Ogochan.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

/*
#define	TRACE
*/
/*
*	misc library module
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<dirent.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	"types.h"
#include	"misc_v.h"
#include	"debug.h"

extern	FILE	*
Fopen(
	char	*name,
	char	*mode)
{
	char	*p
	,		buff[MAXNAMLEN+1];

	for ( p = name ; ( p = strchr(p, '/') ) != NULL ; p ++ )	{
		memcpy(buff, name, p - name);
		buff[p - name] = 0;
		mkdir(buff,0755);
	}
	return	(fopen(name,mode));
}

extern	void
MakeCobolX(
	char	*to,
	size_t	len,
	char	*from)
{
	for	( ; len > 0 ; len -- )	{
		if		(  *from  ==  0  )	{
			*to = ' ';
		} else {
			*to = *from ++;
		}
		to ++;
	}
	*to = 0;
}

extern	void
CopyCobol(
	char	*to,
	char	*from)
{
	while	(  !isspace(*from)  )	{
		*to ++ = *from ++ ;
	}
	*to = 0;
}

extern	void
PrintFixString(
	char	*s,
	int		len)
{	int		i;

	for	( i = 0 ; i < len ; i ++ )	{
		if		(  *s  ==  0  ) {
			putchar('.');
		} else {
			putchar(*s);
		}
		s ++;
	}
}

extern	void
AdjustByteOrder(
	void	*to,
	void	*from,
	size_t	size)
#ifndef	BIG_ENDIAN
{
	memcpy(to,from,size);
}
#else
{	byte	*p
	,		*q;

	p = (byte *)to + size - 1;
	for	( q = from ; p >= (byte *)to ; p -- , q ++ )	{
		*p = *q;
	}
}
#endif
#if	0
extern	Bool
IsAlnum(
	int		c)
{	int		ret;

	ret = 	(	( c >= 0x20 )
			 && ( c <  0x7F ) ) ? TRUE : FALSE;
	return	(ret);
}
#endif
