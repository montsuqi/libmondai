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
#ifndef	_TEXT_VALUE_H
#define	_TEXT_VALUE_H

#include	"valueconv.h"

extern	char	*CSV_UnPackValue(CONVOPT *opt, char *p, ValueStruct *value);

extern	char	*CSV1_PackValue(CONVOPT *opt,char *p, ValueStruct *value);
extern	char	*CSV2_PackValue(CONVOPT *opt,char *p, ValueStruct *value);
extern	char	*CSV3_PackValue(CONVOPT *opt,char *p, ValueStruct *value);
extern	char	*CSVE_PackValue(CONVOPT *opt,char *p, ValueStruct *value);

extern	size_t	CSV1_SizeValue(CONVOPT *opt,ValueStruct *val);
extern	size_t	CSV2_SizeValue(CONVOPT *opt,ValueStruct *val);
extern	size_t	CSV3_SizeValue(CONVOPT *opt,ValueStruct *val);
extern	size_t	CSVE_SizeValue(CONVOPT *opt,ValueStruct *val);

#define	CSV_SizeValue	CSV3_SizeValue
#define	CSV_PackValue	CSV3_PackValue

extern	char	*RFC822_UnPackValue(CONVOPT *opt,char *p, ValueStruct *value);
extern	char	*RFC822_PackValue(CONVOPT *opt,char *p, ValueStruct *value);
extern	size_t	RFC822_SizeValue(CONVOPT *opt,ValueStruct *value);

extern	char	*CGI_UnPackValue(CONVOPT *opt,char *p, ValueStruct *value);
extern	char	*CGI_PackValue(CONVOPT *opt,char *p, ValueStruct *value);
extern	size_t	CGI_SizeValue(CONVOPT *opt,ValueStruct *value);

#endif