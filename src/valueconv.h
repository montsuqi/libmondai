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
}	CONVOPT;

#define	ConvEncoding(opt)			((opt)->encode)

typedef	struct {
	char	*name;
	char	*fsep;
	char	*bsep;
	byte	*(*PackValue)(CONVOPT *opt, byte *p, ValueStruct *value);
	byte	*(*UnPackValue)(CONVOPT *opt, byte *p, ValueStruct *value);
	size_t	(*SizeValue)(CONVOPT *opt, ValueStruct *value);
}	ConvFuncs;

#undef	GLOBAL
#ifdef	_VALUECONV
#define	GLOBAL		/*	*/
#else
#define	GLOBAL		extern
#endif

GLOBAL	byte	*(*PackValue)(CONVOPT *opt, byte *p, ValueStruct *value);
GLOBAL	byte	*(*UnPackValue)(CONVOPT *opt, byte *p, ValueStruct *value);
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

#define	ConvCodeset(opt)			((opt)->codeset)

#endif

