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

#ifndef _INC_XML_VALUE_H
#define _INC_XML_VALUE_H
#include "valueconv.h"

#define NS_URI "http://panda.montsuqui.org/libmondai"

#define XML_OUT_HEADER 0x01
#define XML_OUT_BODY 0x02
#define XML_OUT_TAILER 0x04
#define XML_OUT_ALL 0x07

typedef struct {
  unsigned char fOutput;
  int type;
} XMLOPT;

#define XML_TYPE1 1
#define XML_TYPE2 2

#define ConvOutput(opt)                                                        \
  (((opt)->appendix == NULL) ? XML_OUT_ALL                                     \
                             : (((XMLOPT *)(opt)->appendix)->fOutput))

#define ConvXmlType(opt)                                                       \
  (((opt)->appendix == NULL) ? XML_TYPE1 : (((XMLOPT *)(opt)->appendix)->type))

extern void ConvSetXmlType(CONVOPT *opt, int type);
extern void ConvSetOutput(CONVOPT *opt, unsigned char v);

extern size_t XML_UnPackValue(CONVOPT *opt, unsigned char *p,
                              ValueStruct *value);
extern size_t XML_PackValue(CONVOPT *opt, unsigned char *p, ValueStruct *value);
extern size_t XML_SizeValue(CONVOPT *opt, ValueStruct *value);

extern size_t XML1_UnPackValue(CONVOPT *opt, unsigned char *p,
                               ValueStruct *value);
extern size_t XML1_PackValue(CONVOPT *opt, unsigned char *p,
                             ValueStruct *value);
extern size_t XML1_SizeValue(CONVOPT *opt, ValueStruct *value);

extern size_t XML2_UnPackValue(CONVOPT *opt, unsigned char *p,
                               ValueStruct *value);
extern size_t XML2_PackValue(CONVOPT *opt, unsigned char *p,
                             ValueStruct *value);
extern size_t XML2_SizeValue(CONVOPT *opt, ValueStruct *value);

extern void DestroyXMLOPT(XMLOPT *opt);
extern void DestroyConvOptXML(CONVOPT *opt);

#endif
