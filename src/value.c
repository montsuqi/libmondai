/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2004 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2005-2008 Ogochan.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <glib.h>
#include <math.h>

#define __VALUE_DIRECT
#include "types.h"
#include "misc_v.h"
#include "monstring.h"
#include "numeric.h"
#include "numerici.h"
#include "value.h"
#include "memory_v.h"
#include "getset.h"
#include "hash_v.h"
#include "debug.h"

#define DUMP_LOCALE NULL

extern ValueStruct *NewValue(PacketDataType type) {
  ValueStruct *ret;

  ret = New(ValueStruct);
  dbgprintf("value = %p\n", ret);
  ValueAttribute(ret) = GL_ATTR_NIL;
  ValueStr(ret) = NULL;
  ValueType(ret) = type;
  ValueParent(ret) = NULL;
  switch (type) {
  case GL_TYPE_BYTE:
  case GL_TYPE_BINARY:
    ret->body = New(CharData);
    ValueByteLength(ret) = 0;
    ValueByteSize(ret) = 0;
    ValueByte(ret) = NULL;
    break;
  case GL_TYPE_CHAR:
  case GL_TYPE_VARCHAR:
  case GL_TYPE_DBCODE:
  case GL_TYPE_TEXT:
  case GL_TYPE_SYMBOL:
    ret->body = New(CharData);
    ValueStringLength(ret) = 0;
    ValueStringSize(ret) = 0;
    ValueString(ret) = NULL;
    ValueIsExpandable(ret);
    break;
  case GL_TYPE_NUMBER:
    ret->body = New(Fixed);
    ValueFixedLength(ret) = 0;
    ValueFixedSlen(ret) = 0;
    ValueFixedBody(ret) = NULL;
    break;
  case GL_TYPE_INT:
    SetValueInteger(ret,0);
    break;
  case GL_TYPE_FLOAT:
    ret->body = xmalloc(sizeof(double));
    ValueFloat(ret) = 0.0;
    break;
  case GL_TYPE_BOOL:
    SetValueBool(ret,FALSE);
    break;
  case GL_TYPE_OBJECT:
    ret->body = xmalloc(SIZE_UUID+1);
    memset(ret->body,0,SIZE_UUID+1);
    break;
  case GL_TYPE_ROOT_RECORD:
  case GL_TYPE_RECORD:
    if (type == GL_TYPE_ROOT_RECORD) {
      ret->body = New(RootRecordData);
      ValueRootRecordName(ret) = NULL;
    } else {
      ret->body = New(RecordData);
    }
    ValueRecordSize(ret) = 0;
    ValueRecordMembers(ret) = NewNameHash();
    ValueRecordItems(ret) = NULL;
    ValueRecordNames(ret) = NULL;
    ValueAttribute(ret) = GL_ATTR_NULL;
    break;
  case GL_TYPE_ARRAY:
    ret->body = New(ArrayData);
    ValueArraySize(ret) = 0;
    ValueArrayPrototype(ret) = NULL;
    ValueArrayItems(ret) = NULL;
    ValueAttribute(ret) = GL_ATTR_NULL;
    break;
  case GL_TYPE_ALIAS:
    SetValueAliasName(ret,NULL);
    ValueAttribute(ret) = GL_ATTR_NULL;
    break;
  case GL_TYPE_VALUES:
    ret->body = New(ValuesData);
    ValueValuesSize(ret) = 0;
    ValueValuesItems(ret) = NULL;
    ValueAttribute(ret) = GL_ATTR_NULL;
    break;
  case GL_TYPE_TIMESTAMP:
  case GL_TYPE_DATE:
  case GL_TYPE_TIME:
    ret->body = (struct tm *)xmalloc(sizeof(struct tm));
    ValueDateTimeSec(ret) = 0;
    ValueDateTimeMin(ret) = 0;
    ValueDateTimeHour(ret) = 0;
    ValueDateTimeMDay(ret) = 0;
    ValueDateTimeMon(ret) = 0;
    ValueDateTimeYear(ret) = 0;
    ValueDateTimeWDay(ret) = 0;
    ValueDateTimeYDay(ret) = 0;
    ValueDateTimeIsdst(ret) = 0;
    break;
    break;
  default:
    MonWarningPrintf("invalid type:%#X", type);
    xfree(ret);
    ret = NULL;
    break;
  }
  return (ret);
}

extern void FreeValueStruct(ValueStruct *val) {
  int i,type;

  if (val != NULL) {
    type = ValueType(val);
    switch (type) {
    case GL_TYPE_ARRAY:
      for (i = 0; i < ValueArraySize(val); i++) {
        FreeValueStruct(ValueArrayItem(val, i));
      }
      if (ValueArrayPrototype(val) != NULL) {
        FreeValueStruct(ValueArrayPrototype(val));
      }
      if (ValueArrayItems(val) != NULL) {
        xfree(ValueArrayItems(val));
      }
      xfree(ValueBody(val));
      break;
    case GL_TYPE_ROOT_RECORD:
    case GL_TYPE_RECORD:
      if (type == GL_TYPE_ROOT_RECORD) {
        xfree(ValueRootRecordName(val));
      }
      for (i = 0; i < ValueRecordSize(val); i++) {
        FreeValueStruct(ValueRecordItem(val, i));
        xfree(ValueRecordName(val, i));
      }
      DestroyHashTable(ValueRecordMembers(val));
      if (ValueRecordNames(val) != NULL) {
        xfree(ValueRecordNames(val));
      }
      if (ValueRecordItems(val) != NULL) {
        xfree(ValueRecordItems(val));
      }
      xfree(ValueBody(val));
      break;
    case GL_TYPE_BYTE:
    case GL_TYPE_BINARY:
      xfree(ValueByte(val));
      xfree(ValueBody(val));
      break;
    case GL_TYPE_CHAR:
    case GL_TYPE_VARCHAR:
    case GL_TYPE_DBCODE:
    case GL_TYPE_TEXT:
    case GL_TYPE_SYMBOL:
      xfree(ValueString(val));
      xfree(ValueBody(val));
      break;
    case GL_TYPE_TIMESTAMP:
    case GL_TYPE_DATE:
    case GL_TYPE_TIME:
    case GL_TYPE_OBJECT:
    case GL_TYPE_FLOAT:
    case GL_TYPE_ALIAS:
      xfree(ValueBody(val));
      break;
    case GL_TYPE_NUMBER:
      xfree(ValueFixedBody(val));
      xfree(ValueBody(val));
      break;
    case GL_TYPE_VALUES:
      if (ValueValuesItems(val) != NULL) {
        xfree(ValueValuesItems(val));
      }
      xfree(ValueBody(val));
    default:
      break;
    }
    FreeLBS(ValueStr(val));
    xfree(val);
  }
}

extern char *GetValueName(ValueStruct *value) {
  char *ret;
  char buff[16];
  int index;
  ValueStruct *parent;

  ret = NULL;
  if (value != NULL) {
    if ((parent = ValueParent(value)) != NULL) {
      index = ValueIndex(value);
      switch (ValueType(parent)) {
      case GL_TYPE_ROOT_RECORD:
      case GL_TYPE_RECORD:
        ret = StrDup(ValueRecordName(parent, index));
        break;
      case GL_TYPE_ARRAY:
        sprintf(buff, "[%d]", index);
        ret = StrDup(buff);
        break;
      default:
        fprintf(stderr, "invalid parent type\n");
        break;
      }
    }
  }
  if (ret == NULL) {
    ret = StrDup("");
  }
  return ret;
}

extern char *GetValueLongName(ValueStruct *value) {
  char *lname;
  char *name;
  char *tmp;
  ValueStruct *parent;

  lname = NULL;
  if (value != NULL) {
    parent = value;
    while (ValueParent(parent) != NULL) {
      name = GetValueName(parent);
      if (lname == NULL) {
        lname = name;
      } else {
        tmp = xmalloc(SIZE_LONGNAME + 1);
        snprintf(tmp, SIZE_LONGNAME, "%s.%s", name, lname);
        xfree(name);
        xfree(lname);
        lname = tmp;
      }
      parent = ValueParent(parent);
    }
  }
  return lname;
}

extern ValueStruct *GetRecordItem(ValueStruct *value, char *name) {
  gpointer p;
  ValueStruct *item;
  int type;

  if (value != NULL) {
    type = ValueType(value);
    if (type == GL_TYPE_RECORD || type == GL_TYPE_ROOT_RECORD) {
      if ((p = g_hash_table_lookup(ValueRecordMembers(value), name)) == NULL) {
        item = NULL;
      } else {
        item = ValueRecordItem(value, (long)p - 1);
      }
    } else {
      item = NULL;
    }
  } else {
    item = NULL;
  }
  return (item);
}

extern ValueStruct *GetArrayItem(ValueStruct *value, int index) {
  ValueStruct *item, **items;
  size_t size;
  int i;

  if ((index >= 0) && (index < ValueArraySize(value))) {
    item = ValueArrayItem(value, index);
  } else {
    if (IS_VALUE_EXPANDABLE(value)) {
      size = index + 1;
      items = (ValueStruct **)xmalloc(sizeof(ValueStruct *) * size);
      memclear(items, sizeof(ValueStruct *) * size);
      if (ValueArrayItems(value) != NULL) {
        memcpy(items, ValueArrayItems(value),
               ValueArraySize(value) * sizeof(ValueStruct *));
        xfree(ValueArrayItems(value));
      }
      for (i = ValueArraySize(value); i < size; i++) {
        items[i] = DuplicateValue(ValueArrayPrototype(value), FALSE);
      }
      ValueArrayItems(value) = items;
      ValueArraySize(value) = size;
      item = ValueArrayItem(value, index);
    } else {
      item = NULL;
    }
  }
  return (item);
}

extern ValueStruct *GetValuesItem(ValueStruct *value, int i) {
  ValueStruct *item;

  if ((i >= 0) && (i < ValueValuesSize(value))) {
    item = ValueValuesItem(value, i);
  } else {
    item = NULL;
  }
  return (item);
}

extern ValueStruct *GetItemLongName(ValueStruct *root, char *longname) {
  char item[SIZE_LONGNAME + 1], *p, *q;
  int n;
  ValueStruct *val;

  if (root == NULL) {
    printf("no root ValueStruct [%s]\n", longname);
    return (FALSE);
  }
  p = longname;
  val = root;
  while (*p != 0) {
    q = item;
    if (*p == '[') {
      p++;
      while (isdigit(*p)) {
        *q++ = *p++;
      }
      if (*p != ']') {
        /*	fatal error	*/
      }
      *q = 0;
      p++;
      n = StrToInt(item, strlen(item));
      val = GetArrayItem(val, n);
    } else {
      while ((*p != 0) && (*p != '.') && (*p != '[')) {
        *q++ = *p++;
      }
      *q = 0;
      if (*item != 0) {
        val = GetRecordItem(val, item);
      }
    }
    if (*p == '.')
      p++;
    if (val == NULL) {
      break;
    }
  }
  return (val);
}

static void _DumpValueStruct(ValueStruct *val, int level) {
  int i, j;

  if (val == NULL) {
    fprintf(stderr, "[null]\n");
  } else {
    switch (ValueType(val)) {
    case GL_TYPE_INT:
      fprintf(stderr, "%ld\n", ValueInteger(val));
      break;
    case GL_TYPE_FLOAT:
      fprintf(stderr, "%g\n", ValueFloat(val));
      break;
    case GL_TYPE_BOOL:
      fprintf(stderr, "%s\n", (ValueBool(val) ? "T" : "F"));
      break;
    case GL_TYPE_CHAR:
    case GL_TYPE_VARCHAR:
    case GL_TYPE_TEXT:
      fprintf(stderr, "\"%s\"\n", ValueToString(val, NULL));
      break;
    case GL_TYPE_DBCODE:
      fprintf(stderr, "[%s]\n", ValueString(val));
      break;
    case GL_TYPE_NUMBER:
      fprintf(stderr, "%s\n", ValueFixedBody(val));
      break;
    case GL_TYPE_OBJECT:
      fprintf(stderr, "%s\n", ValueObjectId(val));
      break;
    case GL_TYPE_TIMESTAMP:
      fprintf(stderr, "[%s]\n", ValueToString(val, DUMP_LOCALE));
      break;
    case GL_TYPE_DATE:
      fprintf(stderr, "[%s]\n", ValueToString(val, DUMP_LOCALE));
      break;
    case GL_TYPE_TIME:
      fprintf(stderr, "[%s]\n", ValueToString(val, DUMP_LOCALE));
      break;
    case GL_TYPE_ARRAY:
      fprintf(stderr, "[\n");
      for (i = 0; i < ValueArraySize(val); i++) {
        for (j = 0; j < level; j++) {
          fprintf(stderr, "  ");
        }
        fprintf(stderr, "%d: ", i);
        _DumpValueStruct(ValueArrayItem(val, i), level + 1);
      }
      for (j = 0; j < (level - 1); j++) {
        fprintf(stderr, "  ");
      }
      fprintf(stderr, "]\n");
      break;
    case GL_TYPE_ROOT_RECORD:
    case GL_TYPE_RECORD:
      fprintf(stderr, "{\n");

      for (i = 0; i < ValueRecordSize(val); i++) {
        for (j = 0; j < level; j++) {
          fprintf(stderr, "  ");
        }
        fprintf(stderr, "%s: ", ValueRecordName(val, i));
        _DumpValueStruct(ValueRecordItem(val, i), level + 1);
      }

      for (j = 0; j < (level - 1); j++) {
        fprintf(stderr, "  ");
      }
      fprintf(stderr, "}\n");
      break;
    default:
      break;
    }
  }
}

extern void DumpValueStruct(ValueStruct *val) { _DumpValueStruct(val, 1); }

extern void InitializeValue(ValueStruct *value) {
  int i,type;

  if (value == NULL) {
    return;
  }
  if (ValueStr(value) != NULL) {
    FreeLBS(ValueStr(value));
  }
  ValueStr(value) = NULL;
  type = ValueType(value);
  switch (type) {
  case GL_TYPE_ARRAY:
  case GL_TYPE_VALUES:
  case GL_TYPE_ROOT_RECORD:
  case GL_TYPE_RECORD:
  case GL_TYPE_ALIAS:
    break;
  default:
    ValueIsNil(value);
    break;
  }
  switch (type) {
  case GL_TYPE_INT:
    SetValueInteger(value, 0);
    break;
  case GL_TYPE_FLOAT:
    ValueFloat(value) = 0.0;
    break;
  case GL_TYPE_BOOL:
    SetValueBool(value, FALSE);
    break;
  case GL_TYPE_OBJECT:
    memset(ValueBody(value),0,SIZE_UUID+1);
    break;
  case GL_TYPE_BYTE:
  case GL_TYPE_CHAR:
  case GL_TYPE_VARCHAR:
  case GL_TYPE_DBCODE:
    memclear(ValueString(value), ValueStringSize(value));
    break;
  case GL_TYPE_TEXT:
  case GL_TYPE_SYMBOL:
    if (ValueString(value) != NULL) {
      xfree(ValueString(value));
    }
    ValueString(value) = NULL;
    ValueStringLength(value) = 0;
    ValueStringSize(value) = 0;
    break;
  case GL_TYPE_BINARY:
    if (ValueByte(value) != NULL) {
      xfree(ValueByte(value));
    }
    ValueByte(value) = NULL;
    ValueByteLength(value) = 0;
    ValueByteSize(value) = 0;
    break;
  case GL_TYPE_NUMBER:
    if (ValueFixedLength(value) > 0) {
      memset(ValueFixedBody(value), '0', ValueFixedLength(value));
    }
    ValueFixedBody(value)[ValueFixedLength(value)] = 0;
    break;
  case GL_TYPE_TIMESTAMP:
  case GL_TYPE_DATE:
  case GL_TYPE_TIME:
    ValueDateTimeSec(value) = 0;
    ValueDateTimeMin(value) = 0;
    ValueDateTimeHour(value) = 0;
    ValueDateTimeMDay(value) = 0;
    ValueDateTimeMon(value) = 0;
    ValueDateTimeYear(value) = 0;
    ValueDateTimeWDay(value) = 0;
    ValueDateTimeYDay(value) = 0;
    ValueDateTimeIsdst(value) = 0;
    break;
  case GL_TYPE_ARRAY:
    for (i = 0; i < ValueArraySize(value); i++) {
      InitializeValue(ValueArrayItem(value, i));
    }
    break;
  case GL_TYPE_VALUES:
    for (i = 0; i < ValueValuesSize(value); i++) {
      InitializeValue(ValueValuesItem(value, i));
    }
    break;
  case GL_TYPE_ROOT_RECORD:
  case GL_TYPE_RECORD:
    for (i = 0; i < ValueRecordSize(value); i++) {
      InitializeValue(ValueRecordItem(value, i));
    }
    break;
  case GL_TYPE_ALIAS:
  default:
    break;
  }
}

extern void FillValue(ValueStruct *value) {
  int i,type;
  char *buf;

  if (value == NULL) {
    return;
  }
  if (ValueStr(value) != NULL) {
    FreeLBS(ValueStr(value));
  }
  ValueStr(value) = NULL;
  type = ValueType(value);

  switch (type) {
  case GL_TYPE_ARRAY:
  case GL_TYPE_VALUES:
  case GL_TYPE_ROOT_RECORD:
  case GL_TYPE_RECORD:
  case GL_TYPE_ALIAS:
    break;
  default:
    ValueIsNil(value);
    break;
  }

  switch (type) {
  case GL_TYPE_INT:
    SetValueInteger(value, 1);
    break;
  case GL_TYPE_FLOAT:
    ValueFloat(value) = 1.0;
    break;
  case GL_TYPE_BOOL:
    SetValueBool(value, FALSE);
    break;
  case GL_TYPE_OBJECT:
    memset(ValueBody(value),0,SIZE_UUID+1);
    break;
  case GL_TYPE_BYTE:
  case GL_TYPE_CHAR:
  case GL_TYPE_VARCHAR:
  case GL_TYPE_DBCODE:
    buf = xmalloc(ValueStringSize(value));
    memclear(buf, ValueStringSize(value));
    memset(buf, 'a', ValueStringSize(value) - 1);
    SetValueString(value, buf, NULL);
    break;
  case GL_TYPE_TEXT:
  case GL_TYPE_SYMBOL:
    if (ValueString(value) != NULL) {
      xfree(ValueString(value));
    }
    ValueString(value) = NULL;
    ValueStringLength(value) = 0;
    ValueStringSize(value) = 0;
    break;
  case GL_TYPE_BINARY:
    if (ValueByte(value) != NULL) {
      xfree(ValueByte(value));
    }
    ValueByte(value) = NULL;
    ValueByteLength(value) = 0;
    ValueByteSize(value) = 0;
    break;
  case GL_TYPE_NUMBER:
    if (ValueFixedLength(value) > 0) {
      memset(ValueFixedBody(value), '1', ValueFixedLength(value));
    }
    ValueFixedBody(value)[ValueFixedLength(value)] = 0;
    break;
  case GL_TYPE_TIMESTAMP:
  case GL_TYPE_DATE:
  case GL_TYPE_TIME:
    ValueDateTimeSec(value) = 0;
    ValueDateTimeMin(value) = 0;
    ValueDateTimeHour(value) = 0;
    ValueDateTimeMDay(value) = 0;
    ValueDateTimeMon(value) = 0;
    ValueDateTimeYear(value) = 0;
    ValueDateTimeWDay(value) = 0;
    ValueDateTimeYDay(value) = 0;
    ValueDateTimeIsdst(value) = 0;
    break;
  case GL_TYPE_ARRAY:
    for (i = 0; i < ValueArraySize(value); i++) {
      InitializeValue(ValueArrayItem(value, i));
    }
    break;
  case GL_TYPE_VALUES:
    for (i = 0; i < ValueValuesSize(value); i++) {
      InitializeValue(ValueValuesItem(value, i));
    }
    break;
  case GL_TYPE_ROOT_RECORD:
  case GL_TYPE_RECORD:
    for (i = 0; i < ValueRecordSize(value); i++) {
      InitializeValue(ValueRecordItem(value, i));
    }
    break;
  case GL_TYPE_ALIAS:
  default:
    break;
  }
}

/*
 *	moves simple data only
 */
extern void MoveValue(ValueStruct *to, ValueStruct *from) {
  int type;
  Fixed *xval;

  ValueAttribute(to) = ValueAttribute(from);
  type = ValueType(to);
  switch (type) {
  case GL_TYPE_CHAR:
  case GL_TYPE_VARCHAR:
  case GL_TYPE_DBCODE:
  case GL_TYPE_TEXT:
  case GL_TYPE_SYMBOL:
    if (ValueStringSize(from) > ValueStringSize(to)) {
      if (ValueString(to) != NULL) {
        xfree(ValueString(to));
      }
      ValueStringSize(to) = ValueStringSize(from);
      ValueString(to) = (unsigned char *)xmalloc(ValueStringSize(to));
    }
    memclear(ValueString(to), ValueStringSize(to));
    memcpy(ValueString(to), ValueString(from), ValueStringSize(from));
    if (ValueType(to) == GL_TYPE_TEXT) {
      ValueStringLength(to) = ValueStringLength(from);
    }
    break;
  case GL_TYPE_BYTE:
  case GL_TYPE_BINARY:
    if (ValueByteSize(from) > ValueByteSize(to)) {
      if (ValueByte(to) != NULL) {
        xfree(ValueByte(to));
      }
      ValueByteSize(to) = ValueByteSize(from);
      ValueByte(to) = (unsigned char *)xmalloc(ValueByteSize(to));
    }
    memclear(ValueByte(to), ValueByteSize(to));
    memcpy(ValueByte(to), ValueByte(from), ValueByteSize(from));
    if (ValueType(to) == GL_TYPE_BINARY) {
      ValueByteLength(to) = ValueByteLength(from);
    }
    break;
  case GL_TYPE_NUMBER:
    xval = ValueToFixed(from);
    SetValueFixed(to, xval);
    FreeFixed(xval);
    break;
  case GL_TYPE_INT:
    SetValueInteger(to, ValueToInteger(from));
    break;
  case GL_TYPE_FLOAT:
    SetValueFloat(to, ValueToFloat(from));
    break;
  case GL_TYPE_BOOL:
    SetValueBool(to, ValueToBool(from));
    break;
  case GL_TYPE_TIMESTAMP:
  case GL_TYPE_DATE:
  case GL_TYPE_TIME:
    SetValueDateTime(to, ValueToDateTime(from));
    break;
  case GL_TYPE_OBJECT:
    memset(ValueBody(to),0,SIZE_UUID+1);
    memcpy(ValueBody(to),ValueBody(from),SIZE_UUID);
    break;
  case GL_TYPE_ALIAS:
  default:
    break;
  }
}

/*
 *	copies same structure
 */
extern void CopyValue(ValueStruct *vd, ValueStruct *vs) {
  int i,type;

  if (vd == NULL || vs == NULL || IS_VALUE_NIL(vs)) {
    return;
  }
  ValueAttribute(vd) = ValueAttribute(vs);
  type = ValueType(vs);

  switch (type) {
  case GL_TYPE_INT:
    SetValueInteger(vd, ValueInteger(vs));
    break;
  case GL_TYPE_FLOAT:
    ValueFloat(vd) = ValueFloat(vs);
    break;
  case GL_TYPE_BOOL:
    SetValueBool(vd, ValueBool(vs));
    break;
  case GL_TYPE_OBJECT:
    memset(ValueBody(vd),0,SIZE_UUID+1);
    memcpy(ValueBody(vd),ValueBody(vs),SIZE_UUID);
    break;
  case GL_TYPE_TIMESTAMP:
  case GL_TYPE_DATE:
  case GL_TYPE_TIME:
    memcpy(ValueBody(vd),ValueBody(vs),sizeof(struct tm));
    break;
  case GL_TYPE_TEXT:
  case GL_TYPE_SYMBOL:
  case GL_TYPE_CHAR:
  case GL_TYPE_VARCHAR:
  case GL_TYPE_DBCODE:
  case GL_TYPE_BYTE:
  case GL_TYPE_BINARY:
    MoveValue(vd, vs);
    break;
  case GL_TYPE_NUMBER:
    if (ValueFixedLength(vd) > 0) {
      memcpy(ValueFixedBody(vd), ValueFixedBody(vs), ValueFixedLength(vs));
    }
    ValueFixedBody(vd)[ValueFixedLength(vd)] = 0;
    ValueFixedLength(vd) = ValueFixedLength(vs);
    break;
  case GL_TYPE_ARRAY:
    for (i = 0; i < ValueArraySize(vs); i++) {
      CopyValue(ValueArrayItem(vd, i), ValueArrayItem(vs, i));
    }
    break;
  case GL_TYPE_VALUES:
    for (i = 0; i < ValueValuesSize(vs); i++) {
      CopyValue(ValueValuesItem(vd, i), ValueValuesItem(vs, i));
    }
    break;
  case GL_TYPE_ROOT_RECORD:
  case GL_TYPE_RECORD:
    for (i = 0; i < ValueRecordSize(vs); i++) {
      CopyValue(ValueRecordItem(vd, i), ValueRecordItem(vs, i));
    }
    break;
  case GL_TYPE_ALIAS:
  default:
    break;
  }
}

/*
 *	assign compatible structure
 */
extern void AssignValue(ValueStruct *vd, ValueStruct *vs) {
  int i,type;

  if (vd == NULL || vs == NULL) {
    return;
  }
  type = ValueType(vd);
  switch (type) {
  case GL_TYPE_INT:
  case GL_TYPE_FLOAT:
  case GL_TYPE_BOOL:
  case GL_TYPE_OBJECT:
  case GL_TYPE_TEXT:
  case GL_TYPE_SYMBOL:
  case GL_TYPE_CHAR:
  case GL_TYPE_VARCHAR:
  case GL_TYPE_DBCODE:
  case GL_TYPE_BYTE:
  case GL_TYPE_BINARY:
  case GL_TYPE_NUMBER:
  case GL_TYPE_TIMESTAMP:
  case GL_TYPE_DATE:
  case GL_TYPE_TIME:
    SetValueString(vd, ValueToString(vs, NULL), NULL);
    break;
  case GL_TYPE_ARRAY:
    for (i = 0; i < ValueArraySize(vs); i++) {
      AssignValue(ValueArrayItem(vd, i), ValueArrayItem(vs, i));
    }
    break;
  case GL_TYPE_VALUES:
    for (i = 0; i < ValueValuesSize(vs); i++) {
      AssignValue(ValueValuesItem(vd, i), ValueValuesItem(vs, i));
    }
    break;
  case GL_TYPE_ROOT_RECORD:
  case GL_TYPE_RECORD:
    for (i = 0; i < ValueRecordSize(vs); i++) {
      AssignValue(ValueRecordItem(vd, i), ValueRecordItem(vs, i));
    }
    break;
  case GL_TYPE_ALIAS:
  default:
    break;
  }
}

/*
 *	compare same structure
 */
extern int CompareValue(ValueStruct *vl, ValueStruct *vr) {
  int i, res;
  Fixed *fl, *fr;
  Numeric nl, nr;

  res = 0;
  if (vl == NULL) {
    res = -1;
  } else if (vr == NULL) {
    res = 1;
  } else if (IS_VALUE_NIL(vl)) {
    res = -1;
  } else if (IS_VALUE_NIL(vr)) {
    res = 1;
#if 0
	} else
	if		(  ( ValueType(vl) - ValueType(vr) )  !=  0  ) {
		res = ValueType(vl) - ValueType(vr);
#endif
  } else
    switch (ValueType(vl)) {
    case GL_TYPE_INT:
      if (IS_VALUE_DESC(vr)) {
        res = ValueToInteger(vr) - ValueToInteger(vl);
      } else {
        res = ValueToInteger(vl) - ValueToInteger(vr);
      }
      break;
    case GL_TYPE_TIMESTAMP:
    case GL_TYPE_DATE:
    case GL_TYPE_TIME:
    case GL_TYPE_FLOAT:
      if (IS_VALUE_DESC(vr)) {
        res = (int)(ValueToFloat(vr) - ValueToFloat(vl));
      } else {
        res = (int)(ValueToFloat(vl) - ValueToFloat(vr));
      }
      break;
    case GL_TYPE_BOOL:
      if (IS_VALUE_DESC(vr)) {
        res = (int)(ValueToBool(vr) - ValueToBool(vl));
      } else {
        res = (int)(ValueToBool(vl) - ValueToBool(vr));
      }
      break;
    case GL_TYPE_TEXT:
    case GL_TYPE_SYMBOL:
    case GL_TYPE_CHAR:
    case GL_TYPE_VARCHAR:
    case GL_TYPE_DBCODE:
    case GL_TYPE_BYTE:
    case GL_TYPE_BINARY:
      if (IS_VALUE_DESC(vr)) {
        res = strcmp(ValueToString(vr, NULL), ValueToString(vl, NULL));
      } else {
        res = strcmp(ValueToString(vl, NULL), ValueToString(vr, NULL));
      }
      break;
    case GL_TYPE_NUMBER:
      fl = ValueToFixed(vl);
      fr = ValueToFixed(vr);
      nl = FixedToNumeric(fl);
      nr = FixedToNumeric(fr);
      if (IS_VALUE_DESC(vr)) {
        res = NumericCmp(nr, nl);
      } else {
        res = NumericCmp(nl, nr);
      }
      NumericFree(nl);
      NumericFree(nr);
      FreeFixed(fl);
      FreeFixed(fr);
      break;
    case GL_TYPE_ARRAY:
      for (i = 0; i < ValueArraySize(vr); i++) {
        if ((res = CompareValue(ValueArrayItem(vl, i),
                                ValueArrayItem(vr, i))) != 0)
          break;
      }
      break;
    case GL_TYPE_VALUES:
      for (i = 0; i < ValueValuesSize(vr); i++) {
        if ((res = CompareValue(ValueValuesItem(vl, i),
                                ValueValuesItem(vr, i))) != 0)
          break;
      }
      break;
    case GL_TYPE_ROOT_RECORD:
    case GL_TYPE_RECORD:
      for (i = 0; i < ValueRecordSize(vr); i++) {
        if ((res = CompareValue(ValueRecordItem(vl, i), ValueRecordItem(vr, i))) != 0)
          break;
      }
      break;
    case GL_TYPE_OBJECT:
      res = memcmp(ValueBody(vl),ValueBody(vr),SIZE_UUID);
      break;
    case GL_TYPE_ALIAS:
    default:
      res = 0;
      break;
    }
  return (res);
}

/*
 *	compare structure
 */
extern Bool EqualValue(ValueStruct *vl, ValueStruct *vr) {
  int i;
  Bool ret;

  if (ValueType(vl) == ValueType(vr)) {
    ret = TRUE;
    switch (ValueType(vl)) {
    case GL_TYPE_CHAR:
    case GL_TYPE_VARCHAR:
    case GL_TYPE_DBCODE:
    case GL_TYPE_TEXT:
    case GL_TYPE_SYMBOL:
      if (ValueStringSize(vl) == ValueStringSize(vr)) {
        ret = TRUE;
      } else {
        ret = FALSE;
      }
      break;
    case GL_TYPE_BYTE:
    case GL_TYPE_BINARY:
      if (ValueByteSize(vl) == ValueByteSize(vr)) {
        ret = TRUE;
      } else {
        ret = FALSE;
      }
      break;
    case GL_TYPE_NUMBER:
      if ((ValueFixedLength(vl) == ValueFixedLength(vr)) &&
          (ValueFixedSlen(vl) == ValueFixedSlen(vr))) {
        ret = TRUE;
      } else {
        ret = FALSE;
      }
      break;
    case GL_TYPE_OBJECT:
      ret = !memcmp(ValueBody(vl),ValueBody(vr),SIZE_UUID);
      break;
    case GL_TYPE_ARRAY:
      if (ValueArraySize(vl) != ValueArraySize(vr)) {
        ret = FALSE;
      } else
        for (i = 0; i < ValueArraySize(vr); i++) {
          if (!EqualValue(ValueArrayItem(vl, i), ValueArrayItem(vr, i))) {
            ret = FALSE;
            break;
          }
        }
      break;
    case GL_TYPE_VALUES:
      if (ValueValuesSize(vl) != ValueValuesSize(vr)) {
        ret = FALSE;
      } else
        for (i = 0; i < ValueValuesSize(vr); i++) {
          if (!EqualValue(ValueValuesItem(vl, i), ValueValuesItem(vr, i))) {
            ret = FALSE;
            break;
          }
        }
      break;
    case GL_TYPE_ROOT_RECORD:
    case GL_TYPE_RECORD:
      if (ValueRecordSize(vl) != ValueRecordSize(vr)) {
        ret = FALSE;
      } else
        for (i = 0; i < ValueRecordSize(vr); i++) {
          if ((strcmp(ValueRecordName(vl, i), ValueRecordName(vr, i)) != 0) ||
              (!EqualValue(ValueRecordItem(vl, i), ValueRecordItem(vr, i)))) {
            ret = FALSE;
            break;
          }
        }
      break;
    default:
      break;
    }
  } else {
    ret = FALSE;
  }
  return (ret);
}

extern ValueStruct **MakeValueArray(ValueStruct *template, size_t count,
                                    Bool fCopy) {
  ValueStruct **ret;
  int i;

  ret = (ValueStruct **)xmalloc(sizeof(ValueStruct *) * count);
  for (i = 0; i < count; i++) {
    ret[i] = DuplicateValue(template, fCopy);
  }
  return (ret);
}

extern ValueStruct *DuplicateValue(ValueStruct *template, Bool fCopy) {
  ValueStruct *p;
  ValueStruct **ret;
  int i,type;

  if (template != NULL) {
    p = NewValue(ValueType(template));
    ValueAttribute(p) = ValueAttribute(template);
    ValueStr(p) = NULL;
    type = ValueType(template);
    switch (type) {
    case GL_TYPE_INT:
      if (fCopy) {
        SetValueInteger(p, ValueInteger(template));
      } else {
        SetValueInteger(p, 0);
      }
      break;
    case GL_TYPE_FLOAT:
      if (fCopy) {
        ValueFloat(p) = ValueFloat(template);
      } else {
        ValueFloat(p) = 0.0;
      }
      break;
    case GL_TYPE_BOOL:
      if (fCopy) {
        SetValueBool(p, ValueBool(template));
      } else {
        SetValueBool(p, FALSE);
      }
      break;
    case GL_TYPE_OBJECT:
      if (fCopy) {
        memclear(ValueBody(p), SIZE_UUID+1);
        memcpy(ValueBody(p), ValueBody(template), SIZE_UUID);
      }
      break;
    case GL_TYPE_TIMESTAMP:
    case GL_TYPE_DATE:
    case GL_TYPE_TIME:
      if (fCopy) {
        memcpy(ValueBody(p), ValueBody(template),sizeof(struct tm));
      } else {
        ValueDateTimeSec(p) = 0;
        ValueDateTimeMin(p) = 0;
        ValueDateTimeHour(p) = 0;
        ValueDateTimeMDay(p) = 0;
        ValueDateTimeMon(p) = 0;
        ValueDateTimeYear(p) = 0;
        ValueDateTimeWDay(p) = 0;
        ValueDateTimeYDay(p) = 0;
        ValueDateTimeIsdst(p) = 0;
      }
      break;
    case GL_TYPE_CHAR:
    case GL_TYPE_VARCHAR:
    case GL_TYPE_DBCODE:
    case GL_TYPE_TEXT:
    case GL_TYPE_SYMBOL:
      if (ValueStringSize(template) > 0) {
        ValueString(p) = (unsigned char *)xmalloc(ValueStringSize(template));
        if (fCopy) {
          memcpy(ValueString(p), ValueString(template),
                 ValueStringSize(template));
        } else {
          memclear(ValueString(p), ValueStringSize(template));
        }
      } else {
        ValueString(p) = NULL;
      }
      ValueStringLength(p) = ValueStringLength(template);
      ValueStringSize(p) = ValueStringSize(template);
      break;
    case GL_TYPE_BYTE:
    case GL_TYPE_BINARY:
      if (ValueByteSize(template) > 0) {
        ValueByte(p) = (unsigned char *)xmalloc(ValueByteSize(template));
        if (fCopy) {
          memcpy(ValueString(p), ValueString(template),
                 ValueStringSize(template));
        } else {
          memclear(ValueByte(p), ValueByteSize(template));
        }
      } else {
        ValueByte(p) = NULL;
      }
      ValueByteLength(p) = ValueByteLength(template);
      ValueByteSize(p) = ValueByteSize(template);
      break;
    case GL_TYPE_NUMBER:
      ValueFixedBody(p) = (char *)xmalloc(ValueFixedLength(template) + 1);
      ValueFixedLength(p) = ValueFixedLength(template);
      ValueFixedSlen(p) = ValueFixedSlen(template);
      if (fCopy) {
        memcpy(ValueFixedBody(p), ValueFixedBody(template),
               ValueFixedLength(template) + 1);
      } else {
        SetValueStringWithLength(p, "0.0", strlen("0.0"), NULL);
      }
      break;
    case GL_TYPE_ARRAY:
      ret = (ValueStruct **)xmalloc(sizeof(ValueStruct *) *
                                    ValueArraySize(template));
      for (i = 0; i < ValueArraySize(template); i++) {
        ret[i] = DuplicateValue(ValueArrayItem(template, i), fCopy);
        ValueParent(ret[i]) = p;
      }
      ValueArrayItems(p) = ret;
      ValueArraySize(p) = ValueArraySize(template);
      break;
    case GL_TYPE_ROOT_RECORD:
    case GL_TYPE_RECORD:
      if (type == GL_TYPE_ROOT_RECORD) {
        if (ValueRootRecordName(template) != NULL) {
          ValueRootRecordName(p) = StrDup(ValueRootRecordName(template));
        }
      }
      /*	share name table		*/
      ValueRecordNames(p) = (char **)xmalloc(sizeof(char *) * ValueRecordSize(template));
      /*	duplicate data space	*/
      ValueRecordItems(p) = (ValueStruct **)xmalloc(sizeof(ValueStruct *) * ValueRecordSize(template));
      ValueRecordSize(p) = ValueRecordSize(template);
      for (i = 0; i < ValueRecordSize(template); i++) {
        ValueRecordItem(p, i) = DuplicateValue(ValueRecordItem(template, i), fCopy);
        ValueRecordName(p, i) = StrDup(ValueRecordName(template, i));
        if (g_hash_table_lookup(ValueRecordMembers(p), ValueRecordName(p, i)) !=
            NULL) {
          MonWarningPrintf("duplicate value:%s", ValueRecordName(p, i));
        }
        g_hash_table_insert(ValueRecordMembers(p), (gpointer)ValueRecordName(p, i), (gpointer)((long)i + 1));
        ValueParent(ValueRecordItem(p, i)) = p;
      }
      break;
    case GL_TYPE_VALUES:
      ValueValuesItems(p) = (ValueStruct **)xmalloc(sizeof(ValueStruct *) *
                                                    ValueValuesSize(template));
      ValueValuesSize(p) = ValueValuesSize(template);
      for (i = 0; i < ValueValuesSize(template); i++) {
        ValueValuesItem(p, i) =
            DuplicateValue(ValueValuesItem(template, i), fCopy);
        ValueParent(ValueValuesItem(p, i)) = p;
      }
      break;
    case GL_TYPE_ALIAS:
      SetValueAliasName(p, StrDup(ValueAliasName(template)));
      break;
    default:
      break;
    }
  } else {
    p = NULL;
  }
  return (p);
}

extern void ValueAddRecordItem(ValueStruct *upper, char *name,
                               ValueStruct *value) {
  ValueStruct **items;
  char **names;
  char *dname;
  size_t nsize;

  dbgprintf("name = [%s]\n", name);
  nsize = ValueRecordSize(upper) + 1;
  items = (ValueStruct **)xmalloc(sizeof(ValueStruct *) * nsize);
  names = (char **)xmalloc(sizeof(char *) * nsize);
  if (ValueRecordSize(upper) > 0) {
    memcpy(items, ValueRecordItems(upper),
           sizeof(ValueStruct *) * ValueRecordSize(upper));
    memcpy(names, ValueRecordNames(upper),
           sizeof(char *) * ValueRecordSize(upper));
    xfree(ValueRecordItems(upper));
    xfree(ValueRecordNames(upper));
  }
  items[ValueRecordSize(upper)] = value;
  dname = StrDup(name);
  names[ValueRecordSize(upper)] = dname;
  ValueParent(value) = upper;
  if (name != NULL) {
    if (g_hash_table_lookup(ValueRecordMembers(upper), name) == NULL) {
      g_hash_table_insert(ValueRecordMembers(upper), (gpointer)dname,
                          (gpointer)(ValueRecordSize(upper) + 1));
    } else {
      MonWarningPrintf("name duplicate [%s]", name);
    }
  }
  ValueRecordItems(upper) = items;
  ValueRecordNames(upper) = names;
  ValueRecordSize(upper) = nsize;
}

extern void ValueAddArrayItem(ValueStruct *upper, int ix, ValueStruct *value) {
  ValueStruct **items;
  size_t nsize;

  if (ix < 0) {
    ix = ValueArraySize(upper);
  }
  if (ix >= ValueArraySize(upper)) {
    nsize = ix + 1;
    items = (ValueStruct **)xmalloc(sizeof(ValueStruct *) * nsize);
    memclear(items, sizeof(ValueStruct *) * nsize);
    if (ValueArraySize(upper) > 0) {
      memcpy(items, ValueArrayItems(upper),
             sizeof(ValueStruct *) * ValueArraySize(upper));
      xfree(ValueArrayItems(upper));
    }
    ValueArrayItems(upper) = items;
    ValueArraySize(upper) = nsize;
  }
  ValueArrayItem(upper, ix) = value;
  ValueParent(value) = upper;
}

extern int ValueIndex(ValueStruct *val) {
  ValueStruct *p;
  p = ValueParent(val);
  if (p == NULL) {
    return 0;
  }

  int i,size;
  switch (ValueType(p)) {
  case GL_TYPE_ROOT_RECORD:
  case GL_TYPE_RECORD:
    size = ValueRecordSize(p);
    for (i = 0; i < size; i++) {
      if (val == ValueRecordItem(p, i)) {
        return i;
      }
    }
    break;
  case GL_TYPE_ARRAY:
    size = ValueArraySize(p);
    for (i = 0; i < size; i++) {
      if (val == ValueArrayItem(p, i)) {
        return i;
      }
    }
    break;
  }
  return 0;
}
