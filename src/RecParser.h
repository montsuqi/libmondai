/*
libmondai -- MONTSUQI data access library
Copyright (C) 2000-2003 Ogochan & JMA (Japan Medical Association).
Copyright (C) 2004-2005 Ogochan.

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

#ifndef	_INC_REC_PARSER_H
#define	_INC_REC_PARSER_H
#include	<glib.h>

#include	"Lex.h"

extern	ValueStruct	*ParValueDefine(CURFILE *in);
extern	void		SetValueAttribute(ValueStruct *val, ValueAttributeType attr);
extern	void		RecParserInit(void);
extern	ValueStruct	*RecParseValue(char *name, char **ValueName);
extern	ValueStruct	*RecParseValueMem(char *mem, char **ValueName);
extern	ValueStruct	*RecParseMain(CURFILE *in);

#undef	GLOBAL
#ifdef	_REC_PARSER
#define	GLOBAL		/*	*/
#else
#define	GLOBAL		extern
#endif

GLOBAL	char	*RecordDir;

#undef	GLOBAL
#endif
