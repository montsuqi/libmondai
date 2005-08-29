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

#ifndef	_INC_VALUECONV_H
#define	_INC_VALUECONV_H

#include	"value.h"

#define	STRING_ENCODING_NULL	0
#define	STRING_ENCODING_URL		1
#define	STRING_ENCODING_BASE64	2

typedef	struct {
	char	*codeset;
	char	*recname;
	int		encode;
	size_t	textsize;
	size_t	arraysize;
	Bool	fName;
	void	*appendix;
	int		nIndent;
}	CONVOPT;

#define	ConvEncoding(opt)			((opt)->encode)

typedef	struct {
	char	*name;
	Bool	fBinary;
	char	*fsep;
	char	*bsep;
	size_t	(*PackValue)(CONVOPT *opt, byte *p, ValueStruct *value);
	size_t	(*UnPackValue)(CONVOPT *opt, byte *p, ValueStruct *value);
	size_t	(*SizeValue)(CONVOPT *opt, ValueStruct *value);
}	ConvFuncs;

#undef	GLOBAL
#ifdef	_VALUECONV
#define	GLOBAL		/*	*/
#else
#define	GLOBAL		extern
#endif

GLOBAL	size_t	(*PackValue)(CONVOPT *opt, byte *p, ValueStruct *value);
GLOBAL	size_t	(*UnPackValue)(CONVOPT *opt, byte *p, ValueStruct *value);
GLOBAL	size_t	(*SizeValue)(CONVOPT *opt, ValueStruct *value);

#undef	GLOBAL

extern	ConvFuncs	*GetConvFunc(char *name);
extern	void		ConvSetLanguage(char *name);
extern	CONVOPT		*NewConvOpt(void);
extern	void		DestroyConvOpt(CONVOPT *opt);

#define	ConvSetSize(opt,ts,rs)		(opt)->textsize = (ts), (opt)->arraysize = (rs)
#define	ConvSetCodeset(opt,cod)		(opt)->codeset = (cod)
#define	ConvSetRecName(opt,rec)		(opt)->recname = (rec)
#define	ConvSetEncoding(opt,en)		(opt)->encode = (en)
#define	ConvSetUseName(opt,f)		(opt)->fName = (f)
#define	ConvSetAppendix(opt,a)		(opt)->appendix = (a)

#define	ConvCodeset(opt)			(((opt) == NULL) ? "utf-8" : ((opt)->codeset))

#endif

