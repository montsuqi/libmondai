/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2006-2008 Ogochan.
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
 *	This module is not supporting datetime, date, time.
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
#include <ctype.h>
#include <sys/types.h>

#include "types.h"
#include "misc_v.h"
#include "monstring.h"
#include "memory_v.h"
#include "others.h"
#include "value.h"
#include "getset.h"
#include "php_v.h"
#include "debug.h"

static size_t _PHP_UnPackItem(ValueStruct *e, unsigned char *p) {
  ValueStruct *a;
  Bool bval;
  size_t slen, alen;
  char *q;
  unsigned char *pp;
  char buff[SIZE_BUFF + 1];
  int i;

  pp = p;
  switch (*p++) {
  case 'i':
  case 'd':
    q = buff;
    while ((*p != 0) && (*p != ';')) {
      *q++ = *p++;
    }
    *q = 0;
    SetValueString(e, buff, NULL);
    p++;
    break;
  case 's':
    q = buff;
    while ((*p != 0) && (*p != ':')) {
      *q++ = *p++;
    }
    *q = 0;
    slen = atoi(buff);
    if (*p != '"')
      break;
    p++;
    q = buff;
    for (i = 0; i < slen; i++) {
      *q++ = *p++;
    }
    *q = 0;
    if (*p != '"')
      break;
    SetValueString(e, buff, NULL);
    p++;
    break;
  case 'b':
    if (*p == '1') {
      bval = TRUE;
    } else {
      bval = FALSE;
    }
    SetValueBool(e, bval);
    p++;
    break;
  case 'a':
    q = buff;
    while ((*p != 0) && (*p != ':')) {
      *q++ = *p++;
    }
    *q = 0;
    alen = atoi(buff);
    if (*p != '{')
      break;
    p++;
    for (i = 0; i < alen; i++) {
      a = ValueArrayItem(e, i);
      p += _PHP_UnPackItem(a, p);
      if (*p == ';')
        p++;
    }
    if (*p == '}')
      p++;
    break;
  default:
    break;
  }
  return (p - pp);
}

static size_t _PHP_UnPackValue(CONVOPT *opt, unsigned char *p,
                               ValueStruct *value) {
  Bool fNull;
  char name[SIZE_LONGNAME + 1];
  char *q;
  unsigned char *pp;
  ValueStruct *e;

  pp = p;
  if (value != NULL) {
    while (*p != 0) {
      q = name;
      if (*p == '!') {
        fNull = TRUE;
        p++;
      } else {
        fNull = FALSE;
      }
      while ((*p != 0) && (*p != '|')) {
        *q++ = *p++;
      }
      *q = 0;
      if (*p != 0)
        p++;
      if ((e = GetItemLongName(value, name)) != NULL) {
        if (fNull) {
          ValueIsNil(e);
        } else {
          p += _PHP_UnPackItem(e, p);
        }
      }
    }
  }
  return (p - pp);
}

extern size_t PHP_UnPackValue(CONVOPT *opt, unsigned char *p,
                              ValueStruct *value) {
  size_t ret;

  ret = _PHP_UnPackValue(opt, p, value);
  return (ret);
}

static size_t _PHP_PackValue(CONVOPT *opt, unsigned char *p, ValueStruct *value,
                             char *name, char *longname) {
  int i;
  unsigned char *pp;
  char *str;

  pp = p;
  if (value != NULL) {
    switch (ValueType(value)) {
    case GL_TYPE_CHAR:
    case GL_TYPE_VARCHAR:
    case GL_TYPE_DBCODE:
    case GL_TYPE_TEXT:
    case GL_TYPE_SYMBOL:
      str = ValueToString(value, ConvCodeset(opt));
      p += sprintf(p, "s:%zd:\"%s\"", strlen(str), str);
      break;
    case GL_TYPE_BOOL:
      if (ValueBool(value)) {
        p += sprintf(p, "b:1;");
      } else {
        p += sprintf(p, "b:0;");
      }
      break;
    case GL_TYPE_INT:
      p += sprintf(p, "i:%d;", ValueInteger(value));
      break;
    case GL_TYPE_NUMBER:
    case GL_TYPE_FLOAT:
      str = ValueToString(value, ConvCodeset(opt));
      p += sprintf(p, "d:%s;", str);
      break;
    case GL_TYPE_ARRAY:
      p += sprintf(p, "{");
      for (i = 0; i < ValueArraySize(value); i++) {
        p += _PHP_PackValue(opt, p, ValueArrayItem(value, i),
                            name + strlen(name), longname);
      }
      p += sprintf(p, "}");
      break;
    case GL_TYPE_RECORD:
      for (i = 0; i < ValueRecordSize(value); i++) {
        if (*longname == 0) {
          sprintf(name, "%s", ValueRecordName(value, i));
        } else {
          sprintf(name, ".%s", ValueRecordName(value, i));
        }
        p += _PHP_PackValue(opt, p, ValueRecordItem(value, i),
                            name + strlen(name), longname);
      }
      break;
    case GL_TYPE_BYTE:
    case GL_TYPE_BINARY:
    case GL_TYPE_ALIAS:
    case GL_TYPE_OBJECT:
    default:
      break;
    }
  }
  return (p - pp);
}

extern size_t PHP_PackValue(CONVOPT *opt, unsigned char *p,
                            ValueStruct *value) {
  char longname[SIZE_LONGNAME + 1];
  size_t ret;

  ConvSetEncoding(opt, STRING_ENCODING_URL);
  memclear(longname, SIZE_LONGNAME);
  if (opt->recname != NULL) {
    strcpy(longname, opt->recname);
  }
  ret = _PHP_PackValue(opt, p, value, (longname + strlen(longname)), longname);
  if (ret > 0) {
    *(p + ret - 1) = 0;
  }
  return (ret);
}

static size_t _PHP_SizeValue(CONVOPT *opt, ValueStruct *value, char *name,
                             char *longname) {
  int i;
  size_t ret;

  dbgmsg(">_PHP_SizeValue");
  if (value == NULL)
    return (0);
  switch (ValueType(value)) {
  case GL_TYPE_CHAR:
  case GL_TYPE_VARCHAR:
  case GL_TYPE_DBCODE:
  case GL_TYPE_TEXT:
  case GL_TYPE_SYMBOL:
  case GL_TYPE_BYTE:
  case GL_TYPE_BINARY:
  case GL_TYPE_BOOL:
  case GL_TYPE_NUMBER:
  case GL_TYPE_INT:
  case GL_TYPE_FLOAT:
  case GL_TYPE_OBJECT:
    ret = EncodeLength(opt, ValueToString(value, ConvCodeset(opt))) +
          strlen(longname) + 2;
    break;
  case GL_TYPE_ARRAY:
    ret = 0;
    for (i = 0; i < ValueArraySize(value); i++) {
      sprintf(name, "[%d]", i);
      ret += _PHP_SizeValue(opt, ValueArrayItem(value, i), name + strlen(name),
                            longname);
    }
    break;
  case GL_TYPE_RECORD:
    ret = 0;
    for (i = 0; i < ValueRecordSize(value); i++) {
      sprintf(name, ".%s", ValueRecordName(value, i));
      ret += _PHP_SizeValue(opt, ValueRecordItem(value, i), name + strlen(name),
                            longname);
    }
    break;
  case GL_TYPE_ALIAS:
  default:
    ret = 0;
    break;
  }
  dbgmsg("<_PHP_SizeValue");
  return (ret);
}

extern size_t PHP_SizeValue(CONVOPT *opt, ValueStruct *value) {
  char longname[SIZE_LONGNAME + 1];

  ConvSetEncoding(opt, STRING_ENCODING_URL);
  strcpy(longname, opt->recname);
  return (_PHP_SizeValue(opt, value, (longname + strlen(longname)), longname));
}
