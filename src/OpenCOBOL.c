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
#include	"memory.h"
#include	"value.h"
#include	"cobolvalue.h"
#include	"OpenCOBOL_v.h"
#include	"getset.h"
#include	"debug.h"

#define	IntegerC2Cobol	IntegerCobol2C
static	void
IntegerCobol2C(
	int		*ival)
{
	/*	NOP	*/
}

/*
  unsigned	0123456789
  plus		0123456789
  minus		@ABCDEFGHI
*/

static	void
FixedCobol2C(
	char	*buff,
	size_t	size)
{
	buff[size] = 0;
	if		(	(  buff[size - 1]  >=  '@'  )
			&&	(  buff[size - 1]  <=  'I'  ) ) {
		buff[size - 1] = '0' + ( buff[size - 1] - '@' );
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
		buff[size - 1]  -= '0' - '@';
	}
}

extern	byte	*
OpenCOBOL_UnPackValue(
	CONVOPT	*opt,
	byte	*p,
	ValueStruct	*value)
{
	int		i;
	char	buff[SIZE_NUMBUF+1];
	char	*str;

ENTER_FUNC;
	if		(  value  !=  NULL  ) {
		ValueIsNonNil(value);
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			value->body.IntegerData = *(int *)p;
			IntegerCobol2C(&value->body.IntegerData);
			p += sizeof(int);
			break;
		  case	GL_TYPE_BOOL:
			value->body.BoolData = ( *(char *)p == 'T' ) ? TRUE : FALSE;
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
			for	( i = 0 ; i < value->body.ArrayData.count ; i ++ ) {
				p = OpenCOBOL_UnPackValue(opt,p,value->body.ArrayData.item[i]);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < value->body.RecordData.count ; i ++ ) {
				p = OpenCOBOL_UnPackValue(opt,p,value->body.RecordData.item[i]);
			}
			break;
		  default:
			ValueIsNil(value);
			break;
		}
	}
LEAVE_FUNC;
	return	(p);
}

extern	byte	*
OpenCOBOL_PackValue(
	CONVOPT	*opt,
	byte	*p,
	ValueStruct	*value)
{
	int		i;
	size_t	size;

ENTER_FUNC;
	if		(  value  !=  NULL  ) {
		switch	(value->type) {
		  case	GL_TYPE_INT:
			*(int *)p = value->body.IntegerData;
			IntegerC2Cobol((int *)p);
			p += sizeof(int);
			break;
		  case	GL_TYPE_BOOL:
			*(char *)p = value->body.BoolData ? 'T' : 'F';
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
			for	( i = 0 ; i < value->body.ArrayData.count ; i ++ ) {
				p = OpenCOBOL_PackValue(opt,p,value->body.ArrayData.item[i]);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < value->body.RecordData.count ; i ++ ) {
				p = OpenCOBOL_PackValue(opt,p,value->body.RecordData.item[i]);
			}
			break;
		  default:
			break;
		}
	}
LEAVE_FUNC;
	return	(p);
}

extern	size_t
OpenCOBOL_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value)
{
	int		i
	,		n;
	size_t	ret;

	if		(  value  ==  NULL  )	return	(0);
dbgmsg(">OpenCOBOL_SizeValue");
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
	  case	GL_TYPE_TEXT:
		ret = ( opt->textsize > ValueStringLength(value) ) ? opt->textsize :
			ValueStringLength(value);
		break;
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
		ret = ValueStringLength(value);
		break;
	  case	GL_TYPE_NUMBER:
		ret = ValueFixedLength(value);
		break;
	  case	GL_TYPE_ARRAY:
		if		(  value->body.ArrayData.count  >  0  ) {
			n = value->body.ArrayData.count;
		} else {
			n = opt->arraysize;
		}
		ret = OpenCOBOL_SizeValue(opt,value->body.ArrayData.item[0]) * n;
		break;
	  case	GL_TYPE_RECORD:
		ret = 0;
		for	( i = 0 ; i < value->body.RecordData.count ; i ++ ) {
			ret += OpenCOBOL_SizeValue(opt,value->body.RecordData.item[i]);
		}
		break;
	  default:
		ret = 0;
		break;
	}
dbgmsg("<OpenCOBOL_SizeValue");
	return	(ret);
}

