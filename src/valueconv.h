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

#undef	GLOBAL
#ifdef	_VALUECONV
#define	GLOBAL		/*	*/
#else
#define	GLOBAL		extern
#endif

GLOBAL	char	*(*PackValue)(char *p, ValueStruct *value, size_t textsize);
GLOBAL	char	*(*UnPackValue)(char *p, ValueStruct *value, size_t textsize);

#undef	GLOBAL

extern	void	SetLanguage(char *name);
extern	char	*NativeUnPackValue(char *p, ValueStruct *value);
extern	char	*NativePackValue(char *p, ValueStruct *value);

#endif

