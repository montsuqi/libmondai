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

#ifndef _INC_VALUE_H
#define _INC_VALUE_H

#include <stdint.h>
#include <time.h>

#define SIZE_OID 4

#define SIZE_GROWN 256 /*	LBS grown unit	*/
#define SIZE_BUFF 65536

#define SIZE_SYMBOL 255
#define SIZE_NUMBUF 60

#ifndef SIZE_NAME
#define SIZE_NAME 64
#endif
#ifndef SIZE_LONGNAME
#define SIZE_LONGNAME 1024
#endif

#include <stdio.h>
#include <glib.h>
#include "types.h"
#include "LBSfunc.h"
#include "fixed_v.h"

#ifndef PacketClass
#define PacketClass uint8_t
#endif

typedef PacketClass PacketDataType;
typedef uint16_t ValueAttributeType;
typedef uint64_t MonObjectType;

#define IS_OBJECT_NULL(obj) ((obj) == 0)

typedef struct _ValueStruct {
  char *name;
  PacketDataType type;
  ValueAttributeType attr;
  LargeByteString *str;
  struct _ValueStruct *parent;
  int index;
  void *body;
} ValueStruct;

typedef struct _ArrayData {
  size_t count;
  struct _ValueStruct *prototype, **item;
} ArrayData;

typedef struct _RecordData {
  size_t count;
  struct _ValueStruct **item;
  char **names;
  GHashTable *members;
} RecordData;

typedef struct _CharData {
  size_t length, asize;
  unsigned char *sval;
} CharData;

typedef struct _ValueData {
  size_t count;
  struct _ValueStruct **item;
} ValuesData;

typedef struct _ObjectData {
  MonObjectType oid;
  char *file;
} ObjectData;

#define GL_TYPE_CLASS (PacketDataType)0xF0
#define GL_TYPE_NULL (PacketDataType)0x00

#define GL_TYPE_NUMERIC (PacketDataType)0x10
#define GL_TYPE_INT (PacketDataType)0x11
#define GL_TYPE_BOOL (PacketDataType)0x12
#define GL_TYPE_FLOAT (PacketDataType)0x13
#define GL_TYPE_NUMBER (PacketDataType)0x14

#define GL_TYPE_STRING (PacketDataType)0x20
#define GL_TYPE_CHAR (PacketDataType)0x21
#define GL_TYPE_TEXT (PacketDataType)0x22
#define GL_TYPE_VARCHAR (PacketDataType)0x23
#define GL_TYPE_DBCODE (PacketDataType)0x24
#define GL_TYPE_POINTER (PacketDataType)0x25
#define GL_TYPE_SYMBOL (PacketDataType)0x26

#define GL_TYPE_BITS (PacketDataType)0x30
#define GL_TYPE_BYTE (PacketDataType)0x31
#define GL_TYPE_BINARY (PacketDataType)0x32

#define GL_TYPE_OBJECT (PacketDataType)0x40

#define GL_TYPE_TIMESTAMP (PacketDataType)0x50
#define GL_TYPE_DATE (PacketDataType)0x51
#define GL_TYPE_TIME (PacketDataType)0x52

#define GL_TYPE_STRUCTURE (PacketDataType)0x80
#define GL_TYPE_ARRAY (PacketDataType)0x81
#define GL_TYPE_RECORD (PacketDataType)0x82
#define GL_TYPE_ALIAS (PacketDataType)0x83
#define GL_TYPE_VALUES (PacketDataType)0x84

#define GL_ATTR_NIL (ValueAttributeType)0x8000
#define GL_ATTR_EXPANDABLE (ValueAttributeType)0x0080
#define GL_ATTR_UNIQ (ValueAttributeType)0x0040
#define GL_ATTR_DESC (ValueAttributeType)0x0020
#define GL_ATTR_ALIAS (ValueAttributeType)0x0010

#define GL_ATTR_VIRTUAL (ValueAttributeType)0x0008
#define GL_ATTR_INPUT (ValueAttributeType)0x0004
#define GL_ATTR_OUTPUT (ValueAttributeType)0x0002
#define GL_ATTR_UPDATE (ValueAttributeType)0x0001
#define GL_ATTR_NULL (ValueAttributeType)0x0000

#define CHAR_NIL 0x01

#define IS_VALUE_NIL(v) (((v)->attr & GL_ATTR_NIL) == GL_ATTR_NIL)
#define IS_VALUE_VIRTUAL(v) (((v)->attr & GL_ATTR_VIRTUAL) == GL_ATTR_VIRTUAL)
#define IS_VALUE_ALIAS(v) (((v)->attr & GL_ATTR_ALIAS) == GL_ATTR_ALIAS)
#define IS_VALUE_UPDATE(v) (((v)->attr & GL_ATTR_UPDATE) == GL_ATTR_UPDATE)
#define IS_VALUE_DESC(v) (((v)->attr & GL_ATTR_DESC) == GL_ATTR_DESC)
#define IS_VALUE_UNIQ(v) (((v)->attr & GL_ATTR_UNIQ) == GL_ATTR_UNIQ)
#define IS_VALUE_EXPANDABLE(v) (((v)->attr & GL_ATTR_EXPANDABLE) == GL_ATTR_EXPANDABLE)

#define GL_OBJ_NULL 0

#define IS_VALUE_RECORD(v) ((v)->type == GL_TYPE_RECORD)
#define IS_VALUE_ARRAY(v) ((v)->type == GL_TYPE_ARRAY)
#define IS_VALUE_NUMERIC(v) (((v)->type & GL_TYPE_CLASS) == GL_TYPE_NUMERIC)
#define IS_VALUE_STRING(v) (((v)->type & GL_TYPE_CLASS) == GL_TYPE_STRING)
#define IS_VALUE_BITS(v) (((v)->type & GL_TYPE_CLASS) == GL_TYPE_BITS)
#define IS_VALUE_STRUCTURE(v) (((v)->type & GL_TYPE_CLASS) == GL_TYPE_STRUCTURE)

#define ValueName(v) ((v)->name)
#define ValueType(v) ((v)->type)
#define ValueSize(v) LBS_StringLength((v)->str)
#define ValueStr(v) ((v)->str)
#define ValueStrBody(v) (char *)LBS_Body(((v)->str))

#define ValueAttribute(v) ((v)->attr)
#define ValueIsNil(v) ((v)->attr |= GL_ATTR_NIL)
#define ValueIsNonNil(v) ((v)->attr &= ~GL_ATTR_NIL)
#define ValueIsUpdate(v) ((v)->attr |= GL_ATTR_UPDATE)
#define ValueIsNotUpdate(v) ((v)->attr &= ~GL_ATTR_UPDATE)
#define ValueOrderIsDesc(v) ((v)->attr |= GL_ATTR_DESC)
#define ValueOrderIsAsc(v) ((v)->attr &= ~GL_ATTR_DESC)
#define ValueIsExpandable(v) ((v)->attr |= GL_ATTR_EXPANDABLE)
#define ValueIsNonExpandable(v) ((v)->attr &= ~GL_ATTR_EXPANDABLE)

#define ValueParent(v) ((v)->parent)
#define ValueIndex(v) ((v)->index)
#define ValueBody(v) ((v)->body)

/*CharData*/
#define ValueCharData(v)      ((CharData*)ValueBody(v))
#define ValueStringPointer(v) ((ValueCharData(v))->sval)
#define ValueStringLength(v)  ((ValueCharData(v))->length)
#ifdef __VALUE_DIRECT
#define ValueString(v)        (ValueStringPointer(v))
#define ValueStringSize(v)    (ValueByteSize(v))
#endif

/*Byte*/
#define ValueByte(v)          (ValueStringPointer(v))
#define ValueByteLength(v)    (ValueStringLength(v))
#define ValueByteSize(v)      ((ValueCharData(v))->asize)

/*DateTime*/
#define ValueDateTime(v)      ((struct tm*)ValueBody(v))
#define ValueDateTimeSec(v)   ((ValueDateTime(v))->tm_sec)
#define ValueDateTimeMin(v)   ((ValueDateTime(v))->tm_min)
#define ValueDateTimeHour(v)  ((ValueDateTime(v))->tm_hour)
#define ValueDateTimeMDay(v)  ((ValueDateTime(v))->tm_mday)
#define ValueDateTimeMon(v)   ((ValueDateTime(v))->tm_mon)
#define ValueDateTimeYear(v)  ((ValueDateTime(v))->tm_year)
#define ValueDateTimeWDay(v)  ((ValueDateTime(v))->tm_wday)
#define ValueDateTimeYDay(v)  ((ValueDateTime(v))->tm_yday)
#define ValueDateTimeIsdst(v) ((ValueDateTime(v))->tm_isdst)

/*Fixed*/
#define ValueFixed(v)       ((Fixed*)ValueBody(v))
#define ValueFixedLength(v) ((ValueFixed(v))->flen)
#define ValueFixedSlen(v)   ((ValueFixed(v))->slen)
#define ValueFixedBody(v)   ((ValueFixed(v))->sval)

/*ArrayData*/
#define ValueArray(v)          ((ArrayData*)ValueBody(v))
#define ValueArraySize(v)      ((ValueArray(v))->count)
#define ValueArrayPrototype(v) ((ValueArray(v))->prototype)
#define ValueArrayItems(v)     ((ValueArray(v))->item)
#define ValueArrayItem(v, i)   ((ValueArray(v))->item[(i)])

/*RecordData*/
#define ValueRecord(v)        ((RecordData*)ValueBody(v))
#define ValueRecordSize(v)    ((ValueRecord(v))->count)
#define ValueRecordItems(v)   ((ValueRecord(v))->item)
#define ValueRecordItem(v, i) ((ValueRecord(v))->item[(i)])
#define ValueRecordNames(v)   ((ValueRecord(v))->names)
#define ValueRecordName(v, i) ((ValueRecord(v))->names[(i)])
#define ValueRecordMembers(v) ((ValueRecord(v))->members)

/*ObjecData*/
#define ValueObject(v)     ((ObjectData*)ValueBody(v))
#define ValueObjectId(v)   ((ValueObject(v))->oid)
#define ValueObjectFile(v) ((ValueObject(v))->file)

/*Values*/
#define ValueValues(v)        ((ValuesData*)ValueBody(v))
#define ValueValuesSize(v)    ((ValueValues(v))->count)
#define ValueValuesItems(v)   ((ValueValues(v))->item)
#define ValueValuesItem(v, i) ((ValueValues(v))->item[(i)])

/*misc*/
#define ValueInteger(v)        ((int64_t)ValueBody(v))
#define ValueBool(v)           ((int64_t)ValueBody(v))
#define ValueFloat(v)          (*((double*)(ValueBody(v))))
#define ValueAliasName(v)      ((char*)ValueBody(v))
#define ValuePointer(v)        ((ValueStruct*)ValueBody(v))

extern ValueStruct *NewValue(PacketDataType type);
extern void FreeValueStruct(ValueStruct *val);
extern void DumpValueStruct(ValueStruct *val);

extern char *GetValueName(ValueStruct *val);
extern char *GetValueLongName(ValueStruct *val);

extern void ValueAddRecordItem(ValueStruct *upper, char *name,
                               ValueStruct *value);
extern void ValueAddArrayItem(ValueStruct *upper, int ix, ValueStruct *value);
extern ValueStruct **MakeValueArray(ValueStruct *template, size_t count,
                                    Bool fCopy);
extern ValueStruct *DuplicateValue(ValueStruct *template, Bool fCopy);

extern ValueStruct *GetRecordItem(ValueStruct *value, char *name);
extern ValueStruct *GetArrayItem(ValueStruct *value, int i);
extern ValueStruct *GetValuesItem(ValueStruct *value, int i);

extern ValueStruct *GetItemLongName(ValueStruct *root, char *longname);

extern void InitializeValue(ValueStruct *value);
extern void FillValue(ValueStruct *value);
extern void CopyValue(ValueStruct *vd, ValueStruct *vs);
extern int CompareValue(ValueStruct *vl, ValueStruct *vr);
extern Bool EqualValue(ValueStruct *vl, ValueStruct *vr);

extern void AssignValue(ValueStruct *to, ValueStruct *from);
extern void MoveValue(ValueStruct *to, ValueStruct *from);

#endif
