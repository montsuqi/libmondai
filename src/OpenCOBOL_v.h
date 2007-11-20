/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2003 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2004-2007 Ogochan.
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

#ifndef	_OPENCOBOL_VALUE_H
#define	_OPENCOBOL_VALUE_H
#include	"valueconv.h"

extern	size_t	OpenCOBOL_UnPackValue(CONVOPT	*opt, byte *p, ValueStruct *value);
extern	size_t	OpenCOBOL_PackValue(CONVOPT *opt, byte *p, ValueStruct *value);
extern	size_t	OpenCOBOL_SizeValue(CONVOPT *opt, ValueStruct *value);
#endif
