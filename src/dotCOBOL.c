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
#include	"cobolvalue.h"
#include	"dotCOBOL_v.h"
#include	"memory.h"
#include	"getset.h"
#include	"debug.h"

extern	void
dotCOBOL_IntegerCobol2C(
	int		*ival)
{
	char	*p
	,		c;
	p = (char *)ival;
	c = p[3];
	p[3] = p[0];
	p[0] = c;
	c = p[2];
	p[2] = p[1];
	p[1] = c;
}

static	void
FixedCobol2C(
	char	*buff,
	size_t	size)
{
	buff[size] = 0;
	if		(  buff[size - 1]  >=  0x70  ) {
		buff[size - 1] ^= 0x40;
		*buff |= 0x40;
	}
}

static	void
FixedC2Cobol(
	char	*buff,
	size_t	size)
{
	if		(  *buff  >=  0x70  ) {
		*buff ^= 0x40;
		buff[size - 1] |= 0x40;
	}
}

extern	byte	*
dotCOBOL_UnPackValue(
	CONVOPT *opt,
	byte	*p,
	ValueStruct	*value)
{
	int		i;
	char	buff[SIZE_BUFF];
	char	*str;

dbgmsg(">dotCOBOL_UnPackValue");
	if		(  value  !=  NULL  ) {
		ValueIsNonNil(value);
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			ValueInteger(value) = *(int *)p;
			dotCOBOL_IntegerCobol2C(&ValueInteger(value));
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
		  case	GL_TYPE_TEXT:
			str = (char *)xmalloc((ValueStringLength(value)+1)*sizeof(char));
			memcpy(str,p,ValueStringLength(value));
			str[ValueStringLength(value)] = 0;
			p += opt->textsize;
			StringCobol2C(str,ValueStringLength(value));
			SetValueString(value,str,ConvCodeset(opt));
			xfree(str);
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			str = (char *)xmalloc((ValueStringLength(value)+1)*sizeof(char));
			memcpy(str,p,ValueStringLength(value));
			str[ValueStringLength(value)] = 0;
			p += ValueStringLength(value);
			StringCobol2C(str,ValueStringLength(value));
			SetValueString(value,str,ConvCodeset(opt));
			xfree(str);
			break;
		  case	GL_TYPE_NUMBER:
			memcpy(buff,p,ValueFixedLength(value));
			FixedCobol2C(buff,ValueFixedLength(value));
			strcpy(ValueFixedBody(value),buff);
			p += ValueFixedLength(value);
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p = dotCOBOL_UnPackValue(opt,p,ValueArrayItem(value,i));
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p = dotCOBOL_UnPackValue(opt,p,ValueRecordItem(value,i));
			}
			break;
		  default:
			ValueIsNil(value);
			break;
		}
	}
dbgmsg("<dotCOBOL_UnPackValue");
	return	(p);
}

extern	byte	*
dotCOBOL_PackValue(
	CONVOPT	*opt,
	byte	*p,
	ValueStruct	*value)
{
	int		i;
	size_t	size;

dbgmsg(">dotCOBOL_PackValue");
	if		(  value  !=  NULL  ) {
		switch	(value->type) {
		  case	GL_TYPE_INT:
			*(int *)p = ValueInteger(value);
			dotCOBOL_IntegerC2Cobol((int *)p);
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
		  case	GL_TYPE_TEXT:
			size = ( opt->textsize < ValueStringLength(value) ) ? opt->textsize : ValueStringLength(value);
			memcpy(p,ValueToString(value,ConvCodeset(opt)),size);
			StringC2Cobol(p,opt->textsize);
			p += opt->textsize;
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			memcpy(p,ValueToString(value,ConvCodeset(opt)),ValueStringLength(value));
			StringC2Cobol(p,ValueStringLength(value));
			p += ValueStringLength(value);
			break;
		  case	GL_TYPE_NUMBER:
			memcpy(p,ValueFixedBody(value),ValueFixedLength(value));
			FixedC2Cobol(p,ValueFixedLength(value));
			p += ValueFixedLength(value);
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p = dotCOBOL_PackValue(opt,p,ValueArrayItem(value,i));
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p = dotCOBOL_PackValue(opt,p,ValueRecordItem(value,i));
			}
			break;
		  default:
			break;
		}
	}
dbgmsg("<dotCOBOL_PackValue");
	return	(p);
}

extern	size_t
dotCOBOL_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value)
{
	int		i
	,		n;
	size_t	ret;

	if		(  value  ==  NULL  )	return	(0);
dbgmsg(">dotCOBOL_SizeValue");
	switch	(value->type) {
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
		ret = ( opt->textsize < ValueByteLength(value) ) ? opt->textsize :
			ValueByteLength(value);
		break;
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
		ret = ValueStringLength(value);
		break;
	  case	GL_TYPE_NUMBER:
		ret = ValueFixedLength(value);
		break;
	  case	GL_TYPE_ARRAY:
		if		(  ValueArraySize(value)  >  0  ) {
			n = ValueArraySize(value);
		} else {
			n = opt->arraysize;
		}
		ret = dotCOBOL_SizeValue(opt,ValueArrayItem(value,0)) * n;
		break;
	  case	GL_TYPE_RECORD:
		ret = 0;
		for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
			ret += dotCOBOL_SizeValue(opt,ValueRecordItem(value,i));
		}
		break;
	  default:
		ret = 0;
		break;
	}
dbgmsg("<dotCOBOL_SizeValue");
	return	(ret);
}

