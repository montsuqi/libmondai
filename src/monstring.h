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

#ifndef	_INC_MONDAI_STRING_H
#define	_INC_MONDAI_STRING_H

#include	<strings.h>
#include	"types.h"

#define	memclear(b,s)	memset((b),0,(s))
#define	strlcmp(s1,s2)	strncmp((s1),(s2),strlen(s2))
#define	strlicmp(s1,s2)	strnicmp((s1),(s2),strlen(s2))

#ifdef	__GNUC__
extern	int		stricmp(char *s1, char *s2);
extern	int		strnicmp(char *s1, char *s2, size_t l);
#endif

extern	char	*StrDup(char *s);
extern	char	*IntStrDup(int val);
extern	char	*StringChop(char *str);
extern	long	StrToInt(char *str, size_t len);
extern	long	HexToInt(char *str, size_t len);
extern	char	*IntToStr(char *str, long val, size_t len);
extern	void	PrintFixString(char *s, int len);
extern	size_t	CharLength(byte c);

#endif
