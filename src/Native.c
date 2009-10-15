/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2001-2004 Ogochan & JMA (Japan Medical Association).
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

/*
#define	DEBUG
#define	TRACE
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include    <sys/types.h>

#define	__VALUE_DIRECT
#include	"types.h"
#include	"misc_v.h"
#include	"others.h"
#include	"monstring.h"
#include	"memory_v.h"
#include	"value.h"
#include	"Native_v.h"
#include	"debug.h"

extern	size_t 
NativeUnPackValueNew(
	CONVOPT	*opt,
	byte		*p,
	ValueStruct	**ret)
{
	ValueStruct	*value
		,		*lower;
	int		i;
	size_t	size
	,		len;
	PacketDataType	type;
	ValueAttributeType	attr;
	byte	*q;
	char	*name;

ENTER_FUNC;
	q = p; 
	*ret = NULL;
	if (p != NULL &&
		( type = *(PacketDataType *)p )  !=  GL_TYPE_NULL  ) {
		value = *ret = NewValue(type);
		p += sizeof(PacketDataType);
		attr = *(ValueAttributeType *)p;
		p += sizeof(ValueAttributeType);
		ValueAttribute(value) = attr;
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			ValueInteger(value) = *(int *)p;
			p += sizeof(int);
			break;
		  case	GL_TYPE_TIMESTAMP:
		  case	GL_TYPE_DATE:
		  case	GL_TYPE_TIME:
			ValueDateTimeSec(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeMin(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeHour(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeMDay(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeMon(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeYear(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeWDay(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeYDay(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeIsdst(value) = *(int *)p;	p += sizeof(int);
			break;
		  case	GL_TYPE_BOOL:
			ValueBool(value) = ( *(char *)p == 'T' ) ? TRUE : FALSE;
			p ++;
			break;
		  case	GL_TYPE_FLOAT:
			ValueFloat(value) = *(double *)p;
			p += sizeof(double);
			break;
		  case	GL_TYPE_NUMBER:
			size = *(size_t *)p;
			p += sizeof(size_t);
			if		(  size  >  0  ) {
				ValueFixedBody(value) = (char *)xmalloc(size+1);
				ValueFixedLength(value) = size;
				ValueFixedSlen(value) = *(size_t *)p;
				p += sizeof(size_t);
				memcpy(ValueFixedBody(value),p,ValueFixedLength(value));
				ValueFixedBody(value)[ValueFixedLength(value)] = 0;
				p += ValueFixedLength(value);
			} else {
				p += sizeof(size_t);
			}
			break;
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BINARY:
			size = *(size_t *)p;
			p += sizeof(size_t);
			if		(  size  >  0  )	{
				ValueByteSize(value) = size;
				ValueByte(value) = (byte *)xmalloc(ValueByteSize(value));
			}
			if		(  size  >  0  ) {
				memclear(ValueByte(value),size);
				memcpy(ValueByte(value),p,size);
				p += size;
			}
			ValueByteLength(value) = size;
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_SYMBOL:
			size = *(size_t *)p;
			p += sizeof(size_t);
			len = *(size_t *)p;
			p += sizeof(size_t);
			if 		(  strlen(p) + 1   >  size  ) {
				size = strlen(p) + 1;
				MonWarningPrintf("%s:ValueStringSize is wrong",GetValueLongName(value));
			}
			if		(  size  >  0  )	{
				ValueStringSize(value) = size;
				ValueString(value) = (byte *)xmalloc(ValueStringSize(value));
				memclear(ValueString(value),size);
				strcpy(ValueString(value),p);
				p += strlen(p) + 1;
			}
			ValueStringLength(value) = len;
			break;
		  case	GL_TYPE_OBJECT:
			ValueObjectId(value) = *(MonObjectType *)p;
			p += sizeof(MonObjectType);
			if		(  ( size = *(size_t *)p )  >  0  ) {
				p += sizeof(size_t);
				ValueObjectFile(value) = (char *)xmalloc(size);
				strcpy(ValueObjectFile(value),p);
				p += size;
			} else {
				p += sizeof(size_t);
				ValueObjectFile(value) = NULL;
			}
			break;
		  case	GL_TYPE_ARRAY:
			size = *(size_t *)p;
			ValueArraySize(value) = size;
			p += sizeof(size_t);
			ValueArrayItems(value) = (ValueStruct **)xmalloc(sizeof(ValueStruct *) * size);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p += NativeUnPackValueNew(opt,p,&ValueArrayItem(value,i));
				ValueParent(ValueArrayItem(value,i)) = value;
				ValueIndex(ValueArrayItem(value,i)) = i;
			}
			break;
		  case	GL_TYPE_RECORD:
			size = *(size_t *)p;
			ValueRecordSize(value) = size;
			p += sizeof(size_t);
			ValueRecordItems(value) = (ValueStruct **)xmalloc(sizeof(ValueStruct *) * size);
			ValueRecordNames(value) = (char **)xmalloc(sizeof(char *) * size);
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				name = StrDup(p);
				p += strlen(name)+1;
				p += NativeUnPackValueNew(opt,p,&lower);
				ValueRecordItem(value,i) = lower;
				ValueRecordName(value,i) = name;
				g_hash_table_insert(ValueRecordMembers(value),
									(gpointer)ValueRecordName(value,i),
									(gpointer)((long)i+1));
				ValueParent(ValueRecordItem(value,i)) = value;
				ValueIndex(ValueRecordItem(value,i)) = i;
				dbgprintf("name = [%s]\n",ValueRecordName(value,i));
			}
			break;
		  case	GL_TYPE_ALIAS:
			name = StrDup(p);
			p += strlen(name)+1;
			ValueAliasName(value) = name;
			break;
		  default:
			ValueIsNil(value);
			break;
		}
	}
LEAVE_FUNC;
	return	(p-q);
}


extern	size_t
NativeUnPackValue(
	CONVOPT	*opt,
	byte	*p,
	ValueStruct	*value)
{
	int		rc
	,		i;
	size_t	size
	,		len;
	PacketDataType	type;
	ValueAttributeType	attr;
	byte	*q;
	char	*name;

ENTER_FUNC;
	q = p; 
	if		(  value  !=  NULL  ) {
		type = *(PacketDataType *)p;
		p += sizeof(PacketDataType);
		attr = *(ValueAttributeType *)p;
		p += sizeof(ValueAttributeType);
		if		(  type  !=  ValueType(value)  ) {
			MonWarningPrintf("unmatch type [%X:%X].\n",(int)type,(int)ValueType(value));
			return -1;
		}
		ValueAttribute(value) = attr;
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			dbgprintf("GL_TYPE_INT value[%d]", *(int *)p);
			ValueInteger(value) = *(int *)p;
			p += sizeof(int);
			break;
		  case	GL_TYPE_TIMESTAMP:
		  case	GL_TYPE_DATE:
		  case	GL_TYPE_TIME:
			dbgmsg("GL_TYPE_{TIMESTAMP,DATA,TIME}");
			ValueDateTimeSec(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeMin(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeHour(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeMDay(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeMon(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeYear(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeWDay(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeYDay(value) = *(int *)p;	p += sizeof(int);
			ValueDateTimeIsdst(value) = *(int *)p;	p += sizeof(int);
			break;
		  case	GL_TYPE_BOOL:
			dbgprintf("GL_TYPE_BOOL value[%c]", *(char *)p);
			ValueBool(value) = ( *(char *)p == 'T' ) ? TRUE : FALSE;
			p ++;
			break;
		  case	GL_TYPE_FLOAT:
			dbgprintf("GL_TYPE_FLOAT value[%lf]", *(double *)p);
			ValueFloat(value) = *(double *)p;
			p += sizeof(double);
			break;
		  case	GL_TYPE_NUMBER:
			dbgmsg("GL_TYPE_NUMBER");
			size = *(size_t *)p;
			p += sizeof(size_t);
			if		(  size  >  ValueFixedLength(value)  ) {
				if		(  ValueFixedBody(value)  !=  NULL  ) {
					xfree(ValueFixedBody(value));
				}
				ValueFixedBody(value) = (char *)xmalloc(size+1);
			}
			ValueFixedLength(value) = size;
			ValueFixedSlen(value) = *(size_t *)p;
			p += sizeof(size_t);
			memcpy(ValueFixedBody(value),p,ValueFixedLength(value));
			ValueFixedBody(value)[ValueFixedLength(value)] = 0;
			p += ValueFixedLength(value);
			dbgprintf("sval[%s]", ValueFixedBody(value));
			break;
		  case	GL_TYPE_BYTE:
			dbgmsg("GL_TYPE_BYTE");
			size = *(size_t *)p;
			p += sizeof(size_t);
			if (size > ValueByteSize(value)) {
				memcpy(ValueByte(value),p,ValueByteSize(value));
				ValueByteLength(value) = ValueByteSize(value);
			} else {
				memcpy(ValueByte(value),p,size);
				ValueByteLength(value) = size;
			}
			p += size;
			break;
		  case	GL_TYPE_BINARY:
			dbgmsg("GL_TYPE_BYNARY");
			size = *(size_t *)p;
			p += sizeof(size_t);
			if		(  size  >  ValueByteSize(value)  ) {
				if		(  ValueByte(value)  !=  NULL  ) {
					xfree(ValueByte(value));
				}
				ValueByteSize(value) = size;
				ValueByte(value) = (byte *)xmalloc(ValueByteSize(value));
			}
			if		(  size  >  0  ) {
				memclear(ValueByte(value),size);
				memcpy(ValueByte(value),p,size);
				p += size;
			}
			ValueByteLength(value) = size;
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_SYMBOL:
			dbgmsg("GL_TYPE_{CHAR,VARCAHR,DBCODE,TEXT,SYMBOL}");
			size = *(size_t *)p;
			p += sizeof(size_t);
			len = *(size_t *)p;
			p += sizeof(size_t);
			if 		(  strlen(p) + 1   >  size  ) {
				size = strlen(p) + 1;
				MonWarningPrintf("%s:ValueStringSize is wrong",GetValueLongName(value));
			}
			if		(  size  >  ValueStringSize(value)  ) {
				if		(  ValueString(value)  !=  NULL  ) {
					xfree(ValueString(value));
				}
				ValueStringSize(value) = size;
				ValueString(value) = (byte *)xmalloc(ValueStringSize(value));
			}
			if		(  size  >  0  ) {
				memclear(ValueString(value),size);
				strcpy(ValueString(value),p);
				p += strlen(p) + 1;
			} else {
				p ++;
			}
			if		(	(  ValueType(value)  ==  GL_TYPE_TEXT    )
					||	(  ValueType(value)  ==  GL_TYPE_SYMBOL  ) ) {
				ValueStringLength(value) = len;
			}
			dbgprintf("value[%s]", ValueString(value));
			break;
		  case	GL_TYPE_OBJECT:
			dbgmsg("GL_TYPE_OBJECT");
			ValueObjectId(value) = *(MonObjectType *)p;
			p += sizeof(ValueObjectId(value));
			if		(  ValueObjectFile(value)  !=  NULL  ) {
				xfree(ValueObjectFile(value));
			}
			if		(  ( size = *(size_t *)p )  >  0  ) {
				p += sizeof(size_t);
				ValueObjectFile(value) = (char *)xmalloc(size);
				strcpy(ValueObjectFile(value),p);
				p += size;
			} else {
				p += sizeof(size_t);
				ValueObjectFile(value) = NULL;
			}
			break;
		  case	GL_TYPE_ARRAY:
			dbgmsg("GL_TYPE_ARRAY");
			ValueArraySize(value) = *(size_t *)p;
			p += sizeof(size_t);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				dbgprintf("child[%d]",i);
				rc = NativeUnPackValue(opt,p,ValueArrayItem(value,i));
				ValueParent(ValueArrayItem(value,i)) = value;
				ValueIndex(ValueArrayItem(value,i)) = i;
				if (rc > 0) {
					p += rc;
				} else {
					return rc;
				}
			}
			break;
		  case	GL_TYPE_RECORD:
			dbgmsg("GL_TYPE_RECORD");
			ValueRecordSize(value) = *(size_t *)p;
			p += sizeof(size_t);
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				if (strcmp(p,ValueRecordName(value,i))) {
					MonWarningPrintf("unmatch record name src[%s] dst[%s]", p, ValueRecordName(value,i));
					return -1;
				}
				dbgprintf("child[%d][%s]",i, p);
				p += strlen(p) + 1;
				rc = NativeUnPackValue(opt,p,ValueRecordItem(value,i));
				ValueParent(ValueRecordItem(value,i)) = value;
				ValueIndex(ValueRecordItem(value,i)) = i;
				dbgprintf("rc[%d]",rc);
				if (rc > 0 ) {
					p += rc;
				} else {
					return rc;
				}
			}
			break;
		  case  GL_TYPE_ALIAS:
			dbgmsg("GL_TYPE_ALIAS");
			name = StrDup(p);
			p += strlen(name) + 1;
			ValueAliasName(value) = name;
			break;
		  default:
			dbgmsg("default");
			ValueIsNil(value);
			break;
		}
	}
LEAVE_FUNC;
	return	(p-q);
}

extern	size_t
NativeSizeValue(
	CONVOPT	*opt,
	ValueStruct	*value)
{
	size_t	esize
		,	size;
	int		i;

ENTER_FUNC;
	esize = 0;
	if		(  value  !=  NULL  ) {
		esize += sizeof(PacketDataType);
		esize += sizeof(ValueAttributeType);
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			esize += sizeof(int);
			break;
		  case	GL_TYPE_TIMESTAMP:
		  case	GL_TYPE_DATE:
		  case	GL_TYPE_TIME:
			esize += sizeof(int) * 9;
			break;
		  case	GL_TYPE_BOOL:
			esize ++;
			break;
		  case	GL_TYPE_FLOAT:
			esize += sizeof(double);
			break;
		  case	GL_TYPE_NUMBER:
			esize += sizeof(size_t);
			esize += sizeof(size_t);
			esize += ValueFixedLength(value);
			break;
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BINARY:
			size = ValueByteLength(value);
			esize += sizeof(size_t);
			esize += size;
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_SYMBOL:
			esize += sizeof(size_t);
			esize += sizeof(size_t);
			if (ValueString(value) != NULL) {
				esize += strlen(ValueString(value)) + 1;
			}
			break;
		  case	GL_TYPE_OBJECT:
			esize += sizeof(MonObjectType);
			esize += sizeof(size_t);
			if		(  ValueObjectFile(value)  !=  NULL  ) {
				esize += strlen(ValueObjectFile(value)) + 1;
			}
			break;
		  case	GL_TYPE_ARRAY:
			esize += sizeof(size_t);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				esize += NativeSizeValue(opt,ValueArrayItem(value,i));
			}
			break;
		  case	GL_TYPE_RECORD:
			esize += sizeof(size_t);
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				esize += strlen(ValueRecordName(value,i))+1;
				esize += NativeSizeValue(opt,ValueRecordItem(value,i));
			}
			break;
		  case	GL_TYPE_ALIAS:
			esize += strlen(ValueAliasName(value))+1;
			break;
		  default:
			break;
		}
	}
LEAVE_FUNC;
	return	(esize);
}

extern	size_t
NativePackValue(
	CONVOPT	*opt,
	byte	*p,
	ValueStruct	*value)
{
	size_t	size;
	int		i;
	byte	*pp;

ENTER_FUNC;
	pp = p;
	if		(  value  !=  NULL  ) {
		*(PacketDataType *)p = ValueType(value);
		p += sizeof(PacketDataType);
		*(ValueAttributeType *)p = ValueAttribute(value);
		p += sizeof(ValueAttributeType);
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			*(int *)p = ValueInteger(value);
			p += sizeof(int);
			break;
		  case	GL_TYPE_TIMESTAMP:
		  case	GL_TYPE_DATE:
		  case	GL_TYPE_TIME:
			*(int *)p = ValueDateTimeSec(value);	p += sizeof(int);
			*(int *)p = ValueDateTimeMin(value);	p += sizeof(int);
			*(int *)p = ValueDateTimeHour(value);	p += sizeof(int);
			*(int *)p = ValueDateTimeMDay(value);	p += sizeof(int);
			*(int *)p = ValueDateTimeMon(value);	p += sizeof(int);
			*(int *)p = ValueDateTimeYear(value);	p += sizeof(int);
			*(int *)p = ValueDateTimeWDay(value);	p += sizeof(int);
			*(int *)p = ValueDateTimeYDay(value);	p += sizeof(int);
			*(int *)p = ValueDateTimeIsdst(value);	p += sizeof(int);
			break;
		  case	GL_TYPE_BOOL:
			*(char *)p = ValueBool(value) ? 'T' : 'F';
			p ++;
			break;
		  case	GL_TYPE_FLOAT:
			*(double *)p = ValueFloat(value);
			p += sizeof(double);
			break;
		  case	GL_TYPE_NUMBER:
			*(size_t *)p = ValueFixedLength(value);
			p += sizeof(size_t);
			*(size_t *)p = ValueFixedSlen(value);
			p += sizeof(size_t);
			memcpy(p,ValueFixedBody(value),ValueFixedLength(value));
			p += ValueFixedLength(value);
			break;
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BINARY:
			size = ValueByteLength(value);
			*(size_t *)p = size;
			p += sizeof(size_t);
			memcpy(p,ValueByte(value),size);
			p += size;
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_SYMBOL:
			size = ValueStringSize(value);
			*(size_t *)p = size;
			p += sizeof(size_t);
			*(size_t *)p = ValueStringLength(value);
			p += sizeof(size_t);
			if (ValueString(value) != NULL) {
				strcpy(p,ValueString(value));
				p += strlen(ValueString(value)) + 1;
			}
			break;
		  case	GL_TYPE_OBJECT:
			*(MonObjectType *)p = ValueObjectId(value);
			p += sizeof(MonObjectType);
			if		(  ValueObjectFile(value)  !=  NULL  ) {
				size = strlen(ValueObjectFile(value)) + 1;
				*(size_t *)p = size;
				p += sizeof(size_t);
				strcpy(p,ValueObjectFile(value));
				p += size;
			} else {
				*(size_t *)p = 0;
				p += sizeof(size_t);
			}
			break;
		  case	GL_TYPE_ARRAY:
			*(size_t *)p = ValueArraySize(value);
			p += sizeof(size_t);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p += NativePackValue(opt,p,ValueArrayItem(value,i));
			}
			break;
		  case	GL_TYPE_RECORD:
			*(size_t *)p = ValueRecordSize(value);
			p += sizeof(size_t);
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				strcpy(p,ValueRecordName(value,i));
				p += strlen(ValueRecordName(value,i))+1;
				p += NativePackValue(opt,p,ValueRecordItem(value,i));
			}
			break;
		  case	GL_TYPE_ALIAS:
			strcpy(p,ValueAliasName(value));
			p += strlen(ValueAliasName(value))+1;
			break;
		  default:
			break;
		}
	}
LEAVE_FUNC;
	return	(p-pp);
}

extern	size_t
NativeSaveSize(
	ValueStruct	*value,
	Bool		fData)
{
	size_t	esize
		,	size;
	int		i;

ENTER_FUNC;
	esize = 0;
	if		(  value  !=  NULL  ) {
		esize += sizeof(PacketDataType);
		esize += sizeof(ValueAttributeType);
		if		(  IS_VALUE_NIL(value)  ) {
			fData = FALSE;
		}
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			if		(  fData  ) {
				esize += sizeof(int);
			}
			break;
		  case	GL_TYPE_TIMESTAMP:
		  case	GL_TYPE_DATE:
		  case	GL_TYPE_TIME:
			if		(  fData  ) {
				esize += sizeof(int) * 9;
			}
			break;
		  case	GL_TYPE_BOOL:
			if		(  fData  ) {
				esize ++;
			}
			break;
		  case	GL_TYPE_FLOAT:
			if		(  fData  ) {
				esize += sizeof(double);
			}
			break;
		  case	GL_TYPE_NUMBER:
			esize += sizeof(size_t);
			esize += sizeof(size_t);
			if		(  fData  ) {
				esize += ValueFixedLength(value);
			}
			break;
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BINARY:
			size = ValueByteLength(value);
			esize += sizeof(size_t);
			if		(  fData  ) {
				esize += size;
			}
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_SYMBOL:
			esize += sizeof(size_t);
			esize += sizeof(size_t);
			if		(  fData  ) {
				esize += strlen(ValueString(value)) + 1;
			}
			break;
		  case	GL_TYPE_OBJECT:
			if		(  fData  ) {
				esize += sizeof(MonObjectType);
				esize += sizeof(size_t);
				if		(  ValueObjectFile(value)  !=  NULL  ) {
					esize += strlen(ValueObjectFile(value)) + 1;
				}
			}
			break;
		  case	GL_TYPE_ARRAY:
			esize += sizeof(size_t);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				esize += NativeSaveSize(ValueArrayItem(value,i),fData);
			}
			break;
		  case	GL_TYPE_RECORD:
			esize += sizeof(size_t);
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				esize += strlen(ValueRecordName(value,i))+1;
				esize += NativeSaveSize(ValueRecordItem(value,i),fData);
			}
			break;
		  case	GL_TYPE_ALIAS:
			esize += strlen(ValueAliasName(value))+1;
			break;
		  default:
			break;
		}
	}
LEAVE_FUNC;
	return	(esize);
}

extern	size_t
NativeSaveValue(
	byte		*p,
	ValueStruct	*value,
	Bool		fData)
{
	size_t	size;
	int		i;
	byte	*pp;

ENTER_FUNC;
	pp = p;
	if		(  value  !=  NULL  ) {
		*(PacketDataType *)p = ValueType(value);
		p += sizeof(PacketDataType);
		*(ValueAttributeType *)p = ValueAttribute(value);
		p += sizeof(ValueAttributeType);
		if		(  IS_VALUE_NIL(value)  ) {
			fData = FALSE;
		}
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			if		(  fData  ) {
				*(int *)p = ValueInteger(value);
				p += sizeof(int);
			}
			break;
		  case	GL_TYPE_TIMESTAMP:
		  case	GL_TYPE_DATE:
		  case	GL_TYPE_TIME:
			if		(  fData  ) {
				*(int *)p = ValueDateTimeSec(value);	p += sizeof(int);
				*(int *)p = ValueDateTimeMin(value);	p += sizeof(int);
				*(int *)p = ValueDateTimeHour(value);	p += sizeof(int);
				*(int *)p = ValueDateTimeMDay(value);	p += sizeof(int);
				*(int *)p = ValueDateTimeMon(value);	p += sizeof(int);
				*(int *)p = ValueDateTimeYear(value);	p += sizeof(int);
				*(int *)p = ValueDateTimeWDay(value);	p += sizeof(int);
				*(int *)p = ValueDateTimeYDay(value);	p += sizeof(int);
				*(int *)p = ValueDateTimeIsdst(value);	p += sizeof(int);
			}
			break;
		  case	GL_TYPE_BOOL:
			if		(  fData  ) {
				*(char *)p = ValueBool(value) ? 'T' : 'F';
				p ++;
			}
			break;
		  case	GL_TYPE_FLOAT:
			if		(  fData  ) {
				*(double *)p = ValueFloat(value);
				p += sizeof(double);
			}
			break;
		  case	GL_TYPE_NUMBER:
			*(size_t *)p = ValueFixedLength(value);
			p += sizeof(size_t);
			*(size_t *)p = ValueFixedSlen(value);
			p += sizeof(size_t);
			if		(  fData  ) {
				memcpy(p,ValueFixedBody(value),ValueFixedLength(value));
				p += ValueFixedLength(value);
			}
			break;
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BINARY:
			size = ValueByteLength(value);
			*(size_t *)p = size;
			p += sizeof(size_t);
			if		(  fData  ) {
				memcpy(p,ValueByte(value),size);
				p += size;
			}
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_SYMBOL:
			size = ValueStringSize(value);
			*(size_t *)p = size;
			p += sizeof(size_t);
			*(size_t *)p = ValueStringLength(value);
			p += sizeof(size_t);
			if		(  fData  ) {
				strcpy(p,ValueString(value));
				p += strlen(ValueString(value)) + 1;
			}
			break;
		  case	GL_TYPE_OBJECT:
			if		(  fData  ) {
				*(MonObjectType *)p = ValueObjectId(value);
				p += sizeof(MonObjectType);
				if		(  ValueObjectFile(value)  !=  NULL  ) {
					size = strlen(ValueObjectFile(value)) + 1;
					*(size_t *)p = size;
					p += sizeof(size_t);
					strcpy(p,ValueObjectFile(value));
					p += size;
				} else {
					*(size_t *)p = 0;
					p += sizeof(size_t);
				}
			}
			break;
		  case	GL_TYPE_ARRAY:
			*(size_t *)p = ValueArraySize(value);
			p += sizeof(size_t);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p += NativeSaveValue(p,ValueArrayItem(value,i),fData);
			}
			break;
		  case	GL_TYPE_RECORD:
			*(size_t *)p = ValueRecordSize(value);
			p += sizeof(size_t);
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				strcpy(p,ValueRecordName(value,i));
				p += strlen(ValueRecordName(value,i))+1;
				p += NativeSaveValue(p,ValueRecordItem(value,i),fData);
			}
			break;
		  case	GL_TYPE_ALIAS:
			strcpy(p,ValueAliasName(value));
			p += strlen(ValueAliasName(value))+1;
			break;
		  default:
			break;
		}
	}
LEAVE_FUNC;
	return	(p-pp);
}

extern	size_t
_NativeRestoreValue(
	byte		*p,
	ValueStruct	**ret,
	Bool		fData)
{
	ValueStruct	*value
		,		*lower;
	int		i;
	size_t	size
	,		len;
	PacketDataType	type;
	ValueAttributeType	attr;
	byte	*q;
	char	*name;

ENTER_FUNC;
	q = p; 
	if		(  ( type = *(PacketDataType *)p )  !=  GL_TYPE_NULL  ) {
		value = NewValue(type);
		*ret = value;
		p += sizeof(PacketDataType);
		attr = *(ValueAttributeType *)p;
		p += sizeof(ValueAttributeType);
		ValueAttribute(value) = attr;
		if		(  IS_VALUE_NIL(value)  ) {
			fData = FALSE;
		}
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			if		(  fData  ) {
				ValueInteger(value) = *(int *)p;
				p += sizeof(int);
			} else {
				ValueInteger(value) = 0;
			}
			break;
		  case	GL_TYPE_TIMESTAMP:
		  case	GL_TYPE_DATE:
		  case	GL_TYPE_TIME:
			if		(  fData  ) {
				ValueDateTimeSec(value) = *(int *)p;	p += sizeof(int);
				ValueDateTimeMin(value) = *(int *)p;	p += sizeof(int);
				ValueDateTimeHour(value) = *(int *)p;	p += sizeof(int);
				ValueDateTimeMDay(value) = *(int *)p;	p += sizeof(int);
				ValueDateTimeMon(value) = *(int *)p;	p += sizeof(int);
				ValueDateTimeYear(value) = *(int *)p;	p += sizeof(int);
				ValueDateTimeWDay(value) = *(int *)p;	p += sizeof(int);
				ValueDateTimeYDay(value) = *(int *)p;	p += sizeof(int);
				ValueDateTimeIsdst(value) = *(int *)p;	p += sizeof(int);
			} else {
				ValueDateTimeSec(value) = 0;
				ValueDateTimeMin(value) = 0;
				ValueDateTimeHour(value) = 0;
				ValueDateTimeMDay(value) = 0;
				ValueDateTimeMon(value) = 0;
				ValueDateTimeYear(value) = 0;
				ValueDateTimeWDay(value) = 0;
				ValueDateTimeYDay(value) = 0;
				ValueDateTimeIsdst(value) = 0;
			}
			break;
		  case	GL_TYPE_BOOL:
			if		(  fData  ) {
				ValueBool(value) = ( *(char *)p == 'T' ) ? TRUE : FALSE;
				p ++;
			} else {
				ValueBool(value) = FALSE;
			}
			break;
		  case	GL_TYPE_FLOAT:
			if		(  fData  ) {
				ValueFloat(value) = *(double *)p;
				p += sizeof(double);
			} else {
				ValueFloat(value) = 0;
			}
			break;
		  case	GL_TYPE_NUMBER:
			size = *(size_t *)p;
			p += sizeof(size_t);
			if		(  size  >  0  ) {
				ValueFixedBody(value) = (char *)xmalloc(size+1);
				ValueFixedLength(value) = size;
				ValueFixedSlen(value) = *(size_t *)p;
				p += sizeof(size_t);
				if		(  fData  ) {
					memcpy(ValueFixedBody(value),p,ValueFixedLength(value));
					ValueFixedBody(value)[ValueFixedLength(value)] = 0;
					p += ValueFixedLength(value);
				} else {
					memset(ValueFixedBody(value),'0',ValueFixedLength(value));
					ValueFixedBody(value)[ValueFixedLength(value)] = 0;
				}
			} else {
				p += sizeof(size_t);
			}
			break;
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BINARY:
			size = *(size_t *)p;
			p += sizeof(size_t);
			if		(  size  >  0  )	{
				ValueByteSize(value) = size;
				ValueByte(value) = (byte *)xmalloc(ValueByteSize(value));
			}
			if		(  size  >  0  ) {
				if		(  fData  ) {
					memclear(ValueByte(value),size);
					memcpy(ValueByte(value),p,size);
					p += size;
				} else {
					memclear(ValueByte(value),size);
				}
			}
			ValueByteLength(value) = size;
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_SYMBOL:
			size = *(size_t *)p;
			p += sizeof(size_t);
			len = *(size_t *)p;
			p += sizeof(size_t);
			if		(  size  >  0  )	{
				ValueStringSize(value) = size;
				ValueString(value) = (byte *)xmalloc(ValueStringSize(value));
				memclear(ValueString(value),size);
				if		(  fData  ) {
					strcpy(ValueString(value),p);
					p += strlen(p) + 1;
				}
			}
			ValueStringLength(value) = len;
			break;
		  case	GL_TYPE_OBJECT:
			if		(  fData  ) {
				ValueObjectId(value) = *(MonObjectType *)p;
				p += sizeof(MonObjectType);
				if		(  ( size = *(size_t *)p )  >  0  ) {
					p += sizeof(size_t);
					ValueObjectFile(value) = (char *)xmalloc(size);
					strcpy(ValueObjectFile(value),p);
					p += size;
				} else {
					p += sizeof(size_t);
					ValueObjectFile(value) = NULL;
				}
			} else {
				ValueObjectId(value) = (MonObjectType)0;
				ValueObjectFile(value) = NULL;
			}
			break;
		  case	GL_TYPE_ARRAY:
			size = *(size_t *)p;
			ValueArraySize(value) = size;
			p += sizeof(size_t);
			ValueArrayItems(value) = (ValueStruct **)xmalloc(sizeof(ValueStruct *) * size);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p += _NativeRestoreValue(p,&ValueArrayItem(value,i),fData);
			}
			break;
		  case	GL_TYPE_RECORD:
			size = *(size_t *)p;
			ValueRecordSize(value) = size;
			p += sizeof(size_t);
			ValueRecordItems(value) = (ValueStruct **)xmalloc(sizeof(ValueStruct *) * size);
			ValueRecordNames(value) = (char **)xmalloc(sizeof(char *) * size);
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				name = StrDup(p);
				p += strlen(name)+1;
				p += _NativeRestoreValue(p,&lower,fData);
				ValueRecordItem(value,i) = lower;
				ValueRecordName(value,i) = name;
				g_hash_table_insert(ValueRecordMembers(value),
									(gpointer)ValueRecordName(value,i),
									(gpointer)((long)i+1));
				dbgprintf("name = [%s]\n",ValueRecordName(value,i));
			}
			break;
		  case	GL_TYPE_ALIAS:
			name = StrDup(p);
			p += strlen(name)+1;
			ValueAliasName(value) = name;
			break;
		  default:
			ValueIsNil(value);
			break;
		}
	} else {
		*ret = NULL;
	}
LEAVE_FUNC;
	return	(p-q);
}

extern	ValueStruct	*
NativeRestoreValue(
	byte		*p,
	Bool		fData)
{
	ValueStruct	*val;

	_NativeRestoreValue(p,&val,fData);
	return	(val);
}

