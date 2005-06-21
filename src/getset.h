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

#ifndef	_INC_GETSET_H
#define	_INC_GETSET_H

extern	Bool		SetValueStringWithLength(ValueStruct *val, char *str, size_t slen,
											 char *locale);
extern	Bool		SetValueInteger(ValueStruct *val, int ival);
extern	Bool		SetValueChar(ValueStruct *val, char cval);
extern	Bool		SetValueBool(ValueStruct *val, Bool bval);
extern	Bool		SetValueFloat(ValueStruct *val, double bval);
extern	Bool		SetValueFixed(ValueStruct *val, Fixed *fval);
extern	Bool		SetValueBinary(ValueStruct *val, byte *str, size_t slen);

extern	int			ValueToInteger(ValueStruct *val);
extern	double		ValueToFloat(ValueStruct *val);
extern	Fixed		*ValueToFixed(ValueStruct *val);
extern	Bool		ValueToBool(ValueStruct *val);
extern	LargeByteString	*ValueToLBS(ValueStruct *val, char *codeset);
extern	char		*ValueToString(ValueStruct *value, char *locale);
extern	byte		*ValueToBinary(ValueStruct *val);

#define	SetValueString(val,st,loc)	SetValueStringWithLength((val),(st),strlen(st),(loc))
#define	SetValueLBS(val,lbs)		SetValueBinary((val),LBS_Body(lbs),LBS_Size(lbs))

#endif
