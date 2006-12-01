/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2003 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2004-2006 Ogochan.
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

#ifndef	_INC_OTHERS_H
#define	_INC_OTHERS_H
#include	<glib.h>
#include	"types.h"

extern	char		**ParCommandLine(char *line);
extern	char		*ExpandPath(char *org,char *base);

extern	size_t		DecodeStringURL(byte *q, char *p);
extern	size_t		EncodeStringURL(char *q, char *p);
extern	size_t		EncodeStringLengthURL(char *q);
extern	size_t		EncodeBase64(char *out, int size, byte *in, size_t len);
extern	size_t		DecodeBase64(byte *out, int size, char *in, size_t len);
extern	size_t		EncodeLengthBase64(char *str);

#define	BASE64SIZE(s)	((((s)+2)/2)*3)

#endif
