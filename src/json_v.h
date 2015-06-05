/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2007-2008 Ogochan.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef	_INC_JSON_VALUE_H
#define	_INC_JSON_VALUE_H

#include	"valueconv.h"
#include	<json.h>

extern	size_t	JSON_UnPackValue(CONVOPT *opt, unsigned char *p, ValueStruct *value);
extern	size_t	JSON_UnPackValueOmmit(CONVOPT *opt, unsigned char *p, ValueStruct *value);
extern	size_t	JSON_PackValue(CONVOPT *opt, unsigned char *p, ValueStruct *value);
extern	size_t	JSON_PackValueOmmit(CONVOPT *opt, unsigned char *p, ValueStruct *value);
extern	size_t	JSON_SizeValue(CONVOPT *opt, ValueStruct *value);
extern	size_t	JSON_SizeValueOmmit(CONVOPT *opt, ValueStruct *value);
extern	size_t	JSON_Parse(char *str, ValueStruct **ret);
extern	Bool	CheckJSONObject(json_object *obj,enum json_type type);

#endif
