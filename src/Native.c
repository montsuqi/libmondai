/*
libmondai -- MONTSUQI data access library
Copyright (C) 2001-2004 Ogochan & JMA (Japan Medical Association).
Copyright (C) 2005 Ogochan.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
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
NativeUnPackValue(
	CONVOPT	*opt,
	byte	*p,
	ValueStruct	*value)
{
	int		i;
	size_t	size
	,		len;
	Bool	fName;
	PacketDataType	type;
	ValueAttributeType	attr;
	byte	*q;

ENTER_FUNC;
	q = p; 
	if		(  value  !=  NULL  ) {
		if		(  opt  !=  NULL  ) { 
			fName = opt->fName;
		} else {
			fName = FALSE;
		}
		type = *(PacketDataType *)p;
		p += sizeof(PacketDataType);
		attr = *(ValueAttributeType *)p;
		p += sizeof(ValueAttributeType);
		if		(  type  !=  ValueType(value)  ) {
			fprintf(stdout,"unmatch type [%X:%X].\n",(int)type,(int)ValueType(value));
		}
		ValueAttribute(value) = attr;
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			ValueInteger(value) = *(int *)p;
			p += sizeof(int);
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
			break;
		  case	GL_TYPE_BYTE:
			memcpy(ValueByte(value),p,ValueByteLength(value));
			p += ValueByteLength(value);
			break;
		  case	GL_TYPE_BINARY:
			size = *(size_t *)p;
			p += sizeof(size_t);
			if		(  size  >  ValueByteSize(value)  ) {
				if		(  ValueByte(value)  !=  NULL  ) {
					xfree(ValueByte(value));
				}
				ValueByteSize(value) = size;
				ValueByte(value) = (char *)xmalloc(ValueByteSize(value));
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
			if		(  size  >  ValueStringSize(value)  ) {
				if		(  ValueString(value)  !=  NULL  ) {
					xfree(ValueString(value));
				}
				ValueStringSize(value) = size;
				ValueString(value) = (char *)xmalloc(ValueStringSize(value));
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
			break;
		  case	GL_TYPE_OBJECT:
			ValueObject(value) = *(MonObjectType *)p;
			p += sizeof(MonObjectType);
			break;
		  case	GL_TYPE_ARRAY:
			ValueArraySize(value) = *(size_t *)p;
			p += sizeof(size_t);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p += NativeUnPackValue(opt,p,ValueArrayItem(value,i));
			}
			break;
		  case	GL_TYPE_RECORD:
			ValueRecordSize(value) = *(size_t *)p;
			p += sizeof(size_t);
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				if		(  fName  ) {
					if		(  *p  ==  0xFF  ) {
						p ++;
						strcpy(ValueRecordName(value,i),p);
						p += strlen(ValueRecordName(value,i))+1;
					}
				} else {
					if	(  *p  ==  0xFF  ) {
						p ++;
						p += strlen(p)+1;
					}
				}
				dbgprintf("name = [%s]\n",ValueRecordName(value,i));
				p += NativeUnPackValue(opt,p,ValueRecordItem(value,i));
			}
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
NativePackValue(
	CONVOPT	*opt,
	byte	*p,
	ValueStruct	*value)
{
	int		i;
	Bool	fName;
	size_t	size;
	byte	*pp;

ENTER_FUNC;
	pp = p;
	if		(  value  !=  NULL  ) {
		if		(  opt  !=  NULL  ) { 
			fName = opt->fName;
		} else {
			fName = FALSE;
		}
		*(PacketDataType *)p = ValueType(value);
		p += sizeof(PacketDataType);
		*(ValueAttributeType *)p = ValueAttribute(value);
		p += sizeof(ValueAttributeType);
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			*(int *)p = ValueInteger(value);
			p += sizeof(int);
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
			memcpy(p,ValueByte(value),ValueByteLength(value));
			p += ValueByteLength(value);
			break;
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
			if		(	(  ValueString(value)  ==  NULL  )
					||	(  size                ==  0     ) ) {
				*p = 0;
				p ++;
			} else {
				strcpy(p,ValueString(value));
				p += strlen(ValueString(value)) + 1;
			}
			break;
		  case	GL_TYPE_OBJECT:
			*(MonObjectType *)p = ValueObject(value);
			p += sizeof(MonObjectType);
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
				if		(  fName  ) {
					*p = 0xFF;
					p ++;
					strcpy(p,ValueRecordName(value,i));
					p += strlen(ValueRecordName(value,i))+1;
				}
				p += NativePackValue(opt,p,ValueRecordItem(value,i));
			}
			break;
		  default:
			break;
		}
	}
LEAVE_FUNC;
	return	(p-pp);
}

extern	size_t
NativeSizeValue(
	CONVOPT	*opt,
	ValueStruct	*val)
{
	int		i;
	size_t	ret;
	Bool	fName;

	if		(  val  ==  NULL  )	return	(0);
ENTER_FUNC;
	if		(  opt  !=  NULL  ) { 
		fName = opt->fName;
	} else {
		fName = FALSE;
	}
	ret = sizeof(PacketDataType) + sizeof(ValueAttributeType);
	switch	(ValueType(val)) {
	  case	GL_TYPE_INT:
		ret += sizeof(int);
		break;
	  case	GL_TYPE_BOOL:
		ret += 1;
		break;
	  case	GL_TYPE_FLOAT:
		ret += sizeof(double);
		break;
	  case	GL_TYPE_NUMBER:
		ret += ValueFixedLength(val) + sizeof(size_t) + sizeof(size_t);
		break;
	  case	GL_TYPE_BYTE:
		ret += ValueByteLength(val);
		break;
	  case	GL_TYPE_BINARY:
		ret += sizeof(size_t) + ValueByteLength(val);
		break;
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_SYMBOL:
		ret += sizeof(size_t) + sizeof(size_t) + 1;
		if		(  ValueString(val)  !=  NULL  )	{
			ret += strlen(ValueString(val));
		} else {
			ret ++;
		}
		break;
	  case	GL_TYPE_OBJECT:
		ret += sizeof(MonObjectType);
		break;
	  case	GL_TYPE_ARRAY:
		ret += sizeof(size_t);
		for	( i = 0 ; i < ValueArraySize(val) ; i ++ ) {
			ret += NativeSizeValue(opt,ValueArrayItem(val,i));
		}
		break;
	  case	GL_TYPE_RECORD:
		ret += sizeof(size_t);
		for	( i = 0 ; i < ValueRecordSize(val) ; i ++ ) {
			if		(  fName  ) {
				ret += strlen(ValueRecordName(val,i))+1+1;
			}
			ret += NativeSizeValue(opt,ValueRecordItem(val,i));
		}
		break;
	  default:
		break;
	}
LEAVE_FUNC;
	return	(ret);
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
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			if		(  fData  ) {
				esize += sizeof(int);
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
			esize += size;
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
			esize += ValueStringSize(value);
			esize += sizeof(size_t);
			esize += sizeof(size_t);
			if		(  fData  ) {
				esize += strlen(ValueString(value)) + 1;
			}
			break;
		  case	GL_TYPE_OBJECT:
			if		(  fData  ) {
				esize += sizeof(MonObjectType);
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
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			if		(  fData  ) {
				*(int *)p = ValueInteger(value);
				p += sizeof(int);
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
				*(MonObjectType *)p = ValueObject(value);
				p += sizeof(MonObjectType);
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

static	size_t
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
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			if		(  fData  ) {
				ValueInteger(value) = *(int *)p;
				p += sizeof(int);
			} else {
				ValueInteger(value) = 0;
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
				ValueByte(value) = (char *)xmalloc(ValueByteSize(value));
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
				ValueString(value) = (char *)xmalloc(ValueStringSize(value));
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
				ValueObject(value) = *(MonObjectType *)p;
				p += sizeof(MonObjectType);
			} else {
				ValueObject(value) = (MonObjectType)0;
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
									(gpointer)(i+1));
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
