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

#ifndef	_INC_GETSET_H
#define	_INC_GETSET_H

extern	Bool		SetValueString(ValueStruct *val, char *str, char *locale);
extern	Bool		SetValueInteger(ValueStruct *val, int ival);
extern	Bool		SetValueBool(ValueStruct *val, Bool bval);
extern	Bool		SetValueFloat(ValueStruct *val, double bval);
extern	Bool		SetValueFixed(ValueStruct *val, Fixed *fval);

extern	int			ValueToInteger(ValueStruct *val);
extern	double		ValueToFloat(ValueStruct *val);
extern	Fixed		*ValueToFixed(ValueStruct *val);
extern	Bool		ValueToBool(ValueStruct *val);
extern	char		*ValueToString(ValueStruct *value, char *locale);

#endif
