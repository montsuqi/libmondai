/*
 * libmondai -- MONTSUQI data access library
 *
 * Copyright (C) 2001-2003 Ogochan & JMA (Japan Medical Association).
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

/*
#define	DEBUG
#define	TRACE
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "types.h"
#include "misc_v.h"
#include "value.h"
#include "cobolvalue.h"
#include "monstring.h"
#include "others.h"
#include "dotCOBOL_v.h"
#include "memory_v.h"
#include "getset.h"
#include "debug.h"

extern void dotCOBOL_IntegerCobol2C(int *ival) {
  char *p, c;
  p = (char *)ival;
  c = p[3];
  p[3] = p[0];
  p[0] = c;
  c = p[2];
  p[2] = p[1];
  p[1] = c;
}

static void FixedCobol2C(char *buff, size_t size) {
  buff[size] = 0;
  if (buff[size - 1] >= 0x70) {
    buff[size - 1] ^= 0x40;
    *buff |= 0x40;
  }
}

static void FixedC2Cobol(char *buff, size_t size) {
  if (*buff >= 0x70) {
    *buff ^= 0x40;
    buff[size - 1] |= 0x40;
  }
}

extern size_t dotCOBOL_UnPackValue(CONVOPT *opt, unsigned char *p,
                                   ValueStruct *value) {
  int i;
  char buff[SIZE_BUFF];
  unsigned char *str;
  unsigned char *pp;

  dbgmsg(">dotCOBOL_UnPackValue");
  pp = p;
  if (value != NULL) {
    ValueIsNonNil(value);
    switch (ValueType(value)) {
    case GL_TYPE_INT:
      ValueInteger(value) = *(int *)p;
      dotCOBOL_IntegerCobol2C(&ValueInteger(value));
      p += sizeof(int);
      break;
    case GL_TYPE_FLOAT:
      ValueFloat(value) = *(double *)p;
      p += sizeof(double);
      break;
    case GL_TYPE_BOOL:
      ValueBool(value) = (*(char *)p == 'T') ? TRUE : FALSE;
      p++;
      break;
    case GL_TYPE_BYTE:
      memcpy(ValueByte(value), p, ValueByteLength(value));
      p += ValueByteLength(value);
      break;
    case GL_TYPE_BINARY:
      str = (unsigned char *)xmalloc((opt->textsize) * sizeof(unsigned char));
      memcpy(str, p, opt->textsize);
      p += opt->textsize;
      SetValueBinary(value, str, opt->textsize);
      xfree(str);
      break;
    case GL_TYPE_TEXT:
    case GL_TYPE_SYMBOL:
      str = (unsigned char *)xmalloc((opt->textsize + 1) * sizeof(char));
      memcpy(str, p, opt->textsize);
      str[opt->textsize] = 0;
      p += opt->textsize;
      StringCobol2C(str, opt->textsize);
      SetValueString(value, str, ConvCodeset(opt));
      xfree(str);
      break;
    case GL_TYPE_CHAR:
    case GL_TYPE_VARCHAR:
    case GL_TYPE_DBCODE:
      str = (unsigned char *)xmalloc((ValueStringLength(value) + 1) *
                                     sizeof(char));
      memcpy(str, p, ValueStringLength(value));
      str[ValueStringLength(value)] = 0;
      p += ValueStringLength(value);
      StringCobol2C(str, ValueStringLength(value));
      SetValueString(value, str, ConvCodeset(opt));
      xfree(str);
      break;
    case GL_TYPE_NUMBER:
      memcpy(buff, p, ValueFixedLength(value));
      FixedCobol2C(buff, ValueFixedLength(value));
      strcpy(ValueFixedBody(value), buff);
      p += ValueFixedLength(value);
      break;
    case GL_TYPE_OBJECT:
      ValueObjectId(value) = *(MonObjectType *)p;
      p += sizeof(ValueObject(value));
      if (ValueObjectFile(value) != NULL) {
        xfree(ValueObjectFile(value));
        ValueObjectFile(value) = NULL;
      }
      break;
    case GL_TYPE_TIMESTAMP:
      ValueDateTimeYear(value) = StrToInt(p, 2);
      p += 4;
      ValueDateTimeMon(value) = StrToInt(p, 2) - 1;
      p += 2;
      ValueDateTimeMDay(value) = StrToInt(p, 2);
      p += 2;
      ValueDateTimeHour(value) = StrToInt(p, 2);
      p += 2;
      ValueDateTimeMin(value) = StrToInt(p, 2);
      p += 2;
      ValueDateTimeSec(value) = StrToInt(p, 2);
      p += 2;
      mktime(ValueDateTime(value));
      break;
    case GL_TYPE_TIME:
      ValueDateTimeYear(value) = 0;
      ValueDateTimeMon(value) = 0;
      ValueDateTimeMDay(value) = 0;
      ValueDateTimeHour(value) = StrToInt(p, 2);
      p += 2;
      ValueDateTimeMin(value) = StrToInt(p, 2);
      p += 2;
      ValueDateTimeSec(value) = StrToInt(p, 2);
      p += 2;
      break;
    case GL_TYPE_DATE:
      ValueDateTimeYear(value) = StrToInt(p, 2);
      p += 4;
      ValueDateTimeMon(value) = StrToInt(p, 2) - 1;
      p += 2;
      ValueDateTimeMDay(value) = StrToInt(p, 2);
      p += 2;
      ValueDateTimeHour(value) = 0;
      ValueDateTimeMin(value) = 0;
      ValueDateTimeSec(value) = 0;
      mktime(ValueDateTime(value));
      break;
    case GL_TYPE_ARRAY:
      for (i = 0; i < ValueArraySize(value); i++) {
        p += dotCOBOL_UnPackValue(opt, p, ValueArrayItem(value, i));
      }
      break;
    case GL_TYPE_RECORD:
      for (i = 0; i < ValueRecordSize(value); i++) {
        p += dotCOBOL_UnPackValue(opt, p, ValueRecordItem(value, i));
      }
      break;
    default:
      ValueIsNil(value);
      break;
    }
  }
  dbgmsg("<dotCOBOL_UnPackValue");
  return (p - pp);
}

extern size_t dotCOBOL_PackValue(CONVOPT *opt, unsigned char *p,
                                 ValueStruct *value) {
  int i;
  size_t size;
  unsigned char *pp;

  dbgmsg(">dotCOBOL_PackValue");
  pp = p;
  if (value != NULL) {
    switch (ValueType(value)) {
    case GL_TYPE_INT:
      *(int *)p = ValueInteger(value);
      dotCOBOL_IntegerC2Cobol((int *)p);
      p += sizeof(int);
      break;
    case GL_TYPE_FLOAT:
      *(double *)p = ValueFloat(value);
      p += sizeof(double);
      break;
    case GL_TYPE_BOOL:
      *(char *)p = ValueBool(value) ? 'T' : 'F';
      p++;
      break;
    case GL_TYPE_BYTE:
      memcpy(p, ValueByte(value), ValueByteLength(value));
      p += ValueByteLength(value);
      break;
    case GL_TYPE_BINARY:
      memclear(p, opt->textsize);
      size = (opt->textsize < ValueByteLength(value)) ? opt->textsize
                                                      : ValueByteLength(value);
      memcpy(p, ValueToBinary(value), size);
      p += opt->textsize;
      break;
    case GL_TYPE_TEXT:
    case GL_TYPE_SYMBOL:
      memclear(p, opt->textsize);
      size = (opt->textsize < ValueStringLength(value))
                 ? opt->textsize
                 : ValueStringLength(value);
      memcpy(p, ValueToString(value, ConvCodeset(opt)), size);
      StringC2Cobol(p, opt->textsize);
      p += opt->textsize;
      break;
    case GL_TYPE_CHAR:
    case GL_TYPE_VARCHAR:
    case GL_TYPE_DBCODE:
      if (IS_VALUE_NIL(value)) {
        memclear(p, ValueStringLength(value)); /*	LOW-VALUE	*/
      } else {
        memcpy(p, ValueToString(value, ConvCodeset(opt)),
               ValueStringLength(value));
        StringC2Cobol(p, ValueStringLength(value));
      }
      p += ValueStringLength(value);
      break;
    case GL_TYPE_NUMBER:
      memcpy(p, ValueFixedBody(value), ValueFixedLength(value));
      FixedC2Cobol(p, ValueFixedLength(value));
      p += ValueFixedLength(value);
      break;
    case GL_TYPE_OBJECT:
      *(MonObjectType *)p = ValueObjectId(value);
      p += sizeof(ValueObject(value));
      break;
    case GL_TYPE_TIMESTAMP:
      p += sprintf(p, "%04d%02d%02d%02d%02d%02d", ValueDateTimeYear(value),
                   ValueDateTimeMon(value) + 1, ValueDateTimeMDay(value),
                   ValueDateTimeHour(value), ValueDateTimeMin(value),
                   ValueDateTimeSec(value));
      break;
    case GL_TYPE_DATE:
      p += sprintf(p, "%04d%02d%02d", ValueDateTimeYear(value),
                   ValueDateTimeMon(value) + 1, ValueDateTimeMDay(value));
      break;
    case GL_TYPE_TIME:
      p += sprintf(p, "%02d%02d%02d", ValueDateTimeHour(value),
                   ValueDateTimeMin(value), ValueDateTimeSec(value));
      break;
    case GL_TYPE_ARRAY:
      for (i = 0; i < ValueArraySize(value); i++) {
        p += dotCOBOL_PackValue(opt, p, ValueArrayItem(value, i));
      }
      break;
    case GL_TYPE_RECORD:
      for (i = 0; i < ValueRecordSize(value); i++) {
        p += dotCOBOL_PackValue(opt, p, ValueRecordItem(value, i));
      }
      break;
    default:
      break;
    }
  }
  dbgmsg("<dotCOBOL_PackValue");
  return (p - pp);
}

extern size_t dotCOBOL_SizeValue(CONVOPT *opt, ValueStruct *value) {
  int i, n;
  size_t ret;

  if (value == NULL)
    return (0);
  dbgmsg(">dotCOBOL_SizeValue");
  switch (ValueType(value)) {
  case GL_TYPE_INT:
    ret = sizeof(int);
    break;
  case GL_TYPE_FLOAT:
    ret = sizeof(double);
    break;
  case GL_TYPE_BOOL:
    ret = 1;
    break;
  case GL_TYPE_BINARY:
  case GL_TYPE_TEXT:
  case GL_TYPE_SYMBOL:
    ret = opt->textsize;
    break;
  case GL_TYPE_BYTE:
  case GL_TYPE_CHAR:
  case GL_TYPE_VARCHAR:
  case GL_TYPE_DBCODE:
    ret = ValueStringLength(value);
    break;
  case GL_TYPE_NUMBER:
    ret = ValueFixedLength(value);
    break;
  case GL_TYPE_OBJECT:
    ret = sizeof(ValueObject(value));
    break;
  case GL_TYPE_TIMESTAMP:
    ret = 8 + 6;
    break;
  case GL_TYPE_DATE:
    ret = 8;
    break;
  case GL_TYPE_TIME:
    ret = 6;
    break;
  case GL_TYPE_ARRAY:
    if (ValueArraySize(value) > 0) {
      n = ValueArraySize(value);
    } else {
      n = opt->arraysize;
    }
    ret = dotCOBOL_SizeValue(opt, ValueArrayItem(value, 0)) * n;
    break;
  case GL_TYPE_RECORD:
    ret = 0;
    for (i = 0; i < ValueRecordSize(value); i++) {
      ret += dotCOBOL_SizeValue(opt, ValueRecordItem(value, i));
    }
    break;
  default:
    ret = 0;
    break;
  }
  dbgmsg("<dotCOBOL_SizeValue");
  return (ret);
}
