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

#include	"types.h"
#include	"misc.h"
#include	"value.h"
#include	"Native_v.h"
#include	"debug.h"

extern	char	*
NativeUnPackValue(
	CONVOPT	*opt,
	char	*p,
	ValueStruct	*value)
{
	int		i;
	size_t	size;

dbgmsg(">NativeUnPackValue");
	if		(  value  !=  NULL  ) {
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			ValueInteger(value) = *(int *)p;
			p += sizeof(int);
			break;
		  case	GL_TYPE_BOOL:
			ValueBool(value) = ( *(char *)p == 'T' ) ? TRUE : FALSE;
			p ++;
			break;
		  case	GL_TYPE_BYTE:
			memcpy(ValueByte(value),p,ValueByteLength(value));
			p += ValueByteLength(value);
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			memcpy(ValueString(value),p,ValueStringLength(value));
			ValueString(value)[ValueStringLength(value)] = 0;
			p += ValueStringLength(value);
			break;
		  case	GL_TYPE_TEXT:
			memcpy(&size,p,sizeof(size_t));
			p += sizeof(size_t);
			if		(  size  !=  ValueStringLength(value)  ) {
				xfree(ValueString(value));
				ValueString(value) = (char *)xmalloc(size+1);
				ValueStringLength(value) = size;
			}
			memcpy(ValueString(value),p,size);
			p += size;
			break;
		  case	GL_TYPE_NUMBER:
			memcpy(ValueFixedBody(value),p,ValueFixedLength(value));
			p += ValueFixedLength(value);
			break;
		  case	GL_TYPE_OBJECT:
			ValueObjectPlace(value) = *(int *)p;
			p += sizeof(int);
			ValueObjectID(value) = *(int *)p;
			p += sizeof(int);
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p = NativeUnPackValue(opt,p,ValueArrayItem(value,i));
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
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

extern	char	*
NativePackValue(
	CONVOPT	*opt,
	char	*p,
	ValueStruct	*value)
{
	int		i;

dbgmsg(">NativePackValue");
	if		(  value  !=  NULL  ) {
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			*(int *)p = ValueInteger(value);
			p += sizeof(int);
			break;
		  case	GL_TYPE_BOOL:
			*(char *)p = ValueBool(value) ? 'T' : 'F';
			p ++;
			break;
		  case	GL_TYPE_BYTE:
			memcpy(p,ValueByte(value),ValueByteLength(value));
			p += ValueByteLength(value);
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			memcpy(p,ValueString(value),ValueStringLength(value));
			p += ValueStringLength(value);
			break;
		  case	GL_TYPE_TEXT:
			*(size_t *)p = ValueStringLength(value);
			p += sizeof(size_t);
			memcpy(p,ValueString(value),ValueStringLength(value));
			p += ValueStringLength(value);
			break;
		  case	GL_TYPE_NUMBER:
			memcpy(p,ValueFixedBody(value),ValueFixedLength(value));
			p += ValueFixedLength(value);
			break;
		  case	GL_TYPE_OBJECT:
			*(int *)p = ValueObjectPlace(value);
			p += sizeof(int);
			*(int *)p = ValueObjectID(value);
			p += sizeof(int);
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p = NativePackValue(opt,p,ValueArrayItem(value,i));
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
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
	int		i
	,		n;
	size_t	ret;

	if		(  val  ==  NULL  )	return	(0);
dbgmsg(">NativeSizeValue");
	switch	(ValueType(val)) {
	  case	GL_TYPE_INT:
		ret = sizeof(int);
		break;
	  case	GL_TYPE_FLOAT:
		ret = sizeof(double);
		break;
	  case	GL_TYPE_BOOL:
		ret = 1;
		break;
	  case	GL_TYPE_BYTE:
		ret = ValueByteLength(val);
		break;
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
		ret = ValueStringLength(val);
		break;
	  case	GL_TYPE_NUMBER:
		ret = ValueFixedLength(val);
		break;
	  case	GL_TYPE_TEXT:
		ret = ValueStringLength(val) + sizeof(size_t);
		break;
	  case	GL_TYPE_ARRAY:
		n = ValueArraySize(val);
		ret = NativeSizeValue(opt,ValueArrayItem(val,0)) * n;
		break;
	  case	GL_TYPE_RECORD:
		ret = 0;
		for	( i = 0 ; i < ValueRecordSize(val) ; i ++ ) {
			ret += NativeSizeValue(opt,ValueRecordItem(val,i));
		}
		break;
	  default:
		ret = 0;
		break;
	}
dbgmsg("<NativeSizeValue");
	return	(ret);
}
