/*	PANDA -- a simple transaction monitor

Copyright (C) 2001-2003 Ogochan & JMA (Japan Medical Association).

This module is part of PANDA.

	PANDA is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves
any particular purpose or works at all, unless he says so in writing.
Refer to the GNU General Public License for full details. 

	Everyone is granted permission to copy, modify and redistribute
PANDA, but only under the conditions described in the GNU General
Public License.  A copy of this license is supposed to have been given
to you along with PANDA so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies. 
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
#include	"misc.h"
#include	"memory.h"
#include	"value.h"
#include	"Native_v.h"
#include	"debug.h"

extern	byte	*
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

dbgmsg(">NativeUnPackValue");
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
		ValueAttribute(value) =	( ValueAttribute(value) & ~GL_ATTR_NIL )
			| ( attr & GL_ATTR_NIL );
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
			memcpy(ValueFixedBody(value),p,ValueFixedLength(value));
			p += ValueFixedLength(value);
			break;
		  case	GL_TYPE_BYTE:
			memcpy(ValueByte(value),p,ValueByteLength(value));
			p += ValueByteLength(value);
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
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
			memcpy(ValueString(value),p,size);
			p += size;
			ValueString(value)[size-1] = 0;
			if		(  ValueType(value)  ==  GL_TYPE_TEXT  ) {
				ValueStringLength(value) = len;
			}
			break;
		  case	GL_TYPE_OBJECT:
			ValueObjectPlace(value) = *(int *)p;
			p += sizeof(int);
			ValueObjectID(value) = *(int *)p;
			p += sizeof(int);
			break;
		  case	GL_TYPE_ARRAY:
			ValueArraySize(value) = *(size_t *)p;
			p += sizeof(size_t);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p = NativeUnPackValue(opt,p,ValueArrayItem(value,i));
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
				p = NativeUnPackValue(opt,p,ValueRecordItem(value,i));
			}
			break;
		  default:
			break;
		}
	}
dbgmsg("<NativeUnPackValue");
	return	(p);
}

extern	byte	*
NativePackValue(
	CONVOPT	*opt,
	byte	*p,
	ValueStruct	*value)
{
	int		i;
	Bool	fName;
	size_t	size;

dbgmsg(">NativePackValue");
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
			memcpy(p,ValueFixedBody(value),ValueFixedLength(value));
			p += ValueFixedLength(value);
			break;
		  case	GL_TYPE_BYTE:
			memcpy(p,ValueByte(value),ValueByteLength(value));
			p += ValueByteLength(value);
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
			size = ValueStringSize(value);
			*(size_t *)p = size;
			p += sizeof(size_t);
			*(size_t *)p = ValueStringLength(value);
			p += sizeof(size_t);
			memcpy(p,ValueString(value),size);
			p += size;
			break;
		  case	GL_TYPE_OBJECT:
			*(int *)p = ValueObjectPlace(value);
			p += sizeof(int);
			*(int *)p = ValueObjectID(value);
			p += sizeof(int);
			break;
		  case	GL_TYPE_ARRAY:
			*(size_t *)p = ValueArraySize(value);
			p += sizeof(size_t);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p = NativePackValue(opt,p,ValueArrayItem(value,i));
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
				p = NativePackValue(opt,p,ValueRecordItem(value,i));
			}
			break;
		  default:
			break;
		}
	}
dbgmsg("<NativePackValue");
	return	(p);
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
dbgmsg(">NativeSizeValue");
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
		ret += ValueFixedLength(val);
		break;
	  case	GL_TYPE_BYTE:
		ret += ValueByteLength(val);
		break;
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		ret += sizeof(size_t) + sizeof(size_t) + ValueStringSize(val);
		break;
	  case	GL_TYPE_OBJECT:
		ret += sizeof(int) + sizeof(int);
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
dbgmsg("<NativeSizeValue");
	return	(ret);
}
