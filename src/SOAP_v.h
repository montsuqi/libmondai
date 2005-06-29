/*
libmondai -- MONTSUQI data access library
Copyright (C) 2005 Ogochan.

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

#ifndef	_INC_SOAP_VALUE_H
#define	_INC_SOAP_VALUE_H

#include	"XML_v.h"

extern	size_t	SOAP_PackValue(byte *p, ValueStruct *value, char *method,
							   char *prefix, char *uri, Bool fIndent, Bool fBodyOnly);
extern	void	SOAP_UnPackValue(ValueStruct *val, char *data, char *method);
extern	ValueStruct	*SOAP_LoadValue(char *data, char *method);
//extern	void	SOAP_LoadValue(char *data, ValueStruct **val, char *method);

#endif
