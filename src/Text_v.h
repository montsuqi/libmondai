/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2003 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2004-2008 Ogochan.
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

#ifndef _TEXT_VALUE_H
#define _TEXT_VALUE_H

#include "valueconv.h"

extern size_t CSV_UnPackValue(CONVOPT *opt, unsigned char *p,
                              ValueStruct *value);

extern size_t CSV1_PackValue(CONVOPT *opt, unsigned char *p,
                             ValueStruct *value);
extern size_t CSV2_PackValue(CONVOPT *opt, unsigned char *p,
                             ValueStruct *value);
extern size_t CSV3_PackValue(CONVOPT *opt, unsigned char *p,
                             ValueStruct *value);
extern size_t CSVE_PackValue(CONVOPT *opt, unsigned char *p,
                             ValueStruct *value);

extern size_t CSV1_SizeValue(CONVOPT *opt, ValueStruct *val);
extern size_t CSV2_SizeValue(CONVOPT *opt, ValueStruct *val);
extern size_t CSV3_SizeValue(CONVOPT *opt, ValueStruct *val);
extern size_t CSVE_SizeValue(CONVOPT *opt, ValueStruct *val);

#define CSV_SizeValue CSV3_SizeValue
#define CSV_PackValue CSV3_PackValue

extern size_t SQL_UnPackValue(CONVOPT *opt, unsigned char *p,
                              ValueStruct *value);
extern size_t SQL_PackValue(CONVOPT *opt, unsigned char *p, ValueStruct *value);
extern size_t SQL_SizeValue(CONVOPT *opt, ValueStruct *value);

extern size_t RFC822_UnPackValue(CONVOPT *opt, unsigned char *p,
                                 ValueStruct *value);
extern size_t RFC822_PackValue(CONVOPT *opt, unsigned char *p,
                               ValueStruct *value);
extern size_t RFC822_SizeValue(CONVOPT *opt, ValueStruct *value);

extern size_t CGI_UnPackValue(CONVOPT *opt, unsigned char *p,
                              ValueStruct *value);
extern size_t CGI_PackValue(CONVOPT *opt, unsigned char *p, ValueStruct *value);
extern size_t CGI_SizeValue(CONVOPT *opt, ValueStruct *value);

#endif
