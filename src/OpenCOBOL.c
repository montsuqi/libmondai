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
#include	"OpenCOBOL_v.h"
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

extern	char	*
OpenCOBOL_UnPackValue(
	char	*p,
	ValueStruct	*value,
	size_t	textsize)
{
	int		i;
	char	buff[SIZE_BUFF];

	if		(  value  !=  NULL  ) {
		switch	(value->type) {
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
			if		(  ValueString(value)  !=  NULL  ) {
				if		(  ValueStringLength(value)  <  textsize  ) {
					xfree(ValueString(value));
					ValueString(value) = (char *)xmalloc(textsize + 1);
					ValueStringLength(value) = textsize;
				}
			} else {
				ValueString(value) = (char *)xmalloc(textsize + 1);
				ValueStringLength(value) = textsize;
			}
			memcpy(ValueString(value),p,ValueStringLength(value));
			p += textsize;
			StringCobol2C(ValueString(value),ValueStringLength(value));
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			memcpy(ValueString(value),p,ValueStringLength(value));
			p += ValueStringLength(value);
			StringCobol2C(ValueString(value),ValueStringLength(value));
			break;
		  case	GL_TYPE_NUMBER:
			memcpy(buff,p,ValueFixedLength(value));
			FixedCobol2C(buff,ValueFixedLength(value));
			strcpy(ValueFixedBody(value),buff);
			p += ValueFixedLength(value);
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < value->body.ArrayData.count ; i ++ ) {
				p = OpenCOBOL_UnPackValue(p,value->body.ArrayData.item[i],textsize);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < value->body.RecordData.count ; i ++ ) {
				p = OpenCOBOL_UnPackValue(p,value->body.RecordData.item[i],textsize);
			}
			break;
		  default:
			break;
		}
	}
	return	(p);
}

extern	char	*
OpenCOBOL_PackValue(
	char	*p,
	ValueStruct	*value,
	size_t	textsize)
{
	int		i;
	size_t	size;

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
			size = ( textsize < ValueStringLength(value) ) ? textsize :
				ValueStringLength(value);
			memcpy(p,ValueString(value),size);
			StringC2Cobol(p,textsize);
			p += textsize;
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			StringC2Cobol(ValueString(value),ValueStringLength(value));
			memcpy(p,ValueString(value),ValueStringLength(value));
			p += ValueStringLength(value);
			break;
		  case	GL_TYPE_NUMBER:
			memcpy(p,ValueFixedBody(value),ValueFixedLength(value));
			FixedC2Cobol(p,ValueFixedLength(value));
			p += ValueFixedLength(value);
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < value->body.ArrayData.count ; i ++ ) {
				p = OpenCOBOL_PackValue(p,value->body.ArrayData.item[i],textsize);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < value->body.RecordData.count ; i ++ ) {
				p = OpenCOBOL_PackValue(p,value->body.RecordData.item[i],textsize);
			}
			break;
		  default:
			break;
		}
	}
	return	(p);
}

extern	size_t
OpenCOBOL_SizeValue(
	ValueStruct	*value,
	size_t		arraysize,
	size_t		textsize)
{
	int		i
	,		n
	,		size;
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
		size = ( textsize > ValueStringLength(value) ) ? textsize :
			ValueStringLength(value);
		ret = size;
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
			n = arraysize;
		}
		ret = OpenCOBOL_SizeValue(value->body.ArrayData.item[0],arraysize,textsize) * n;
		break;
	  case	GL_TYPE_RECORD:
		ret = 0;
		for	( i = 0 ; i < value->body.RecordData.count ; i ++ ) {
			ret += OpenCOBOL_SizeValue(value->body.RecordData.item[i],arraysize,textsize);
		}
		break;
	  default:
		ret = 0;
		break;
	}
dbgmsg("<OpenCOBOL_SizeValue");
	return	(ret);
}

