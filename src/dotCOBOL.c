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

extern	char	*
dotCOBOL_UnPackValue(
	char	*p,
	ValueStruct	*value,
	size_t	textsize)
{
	int		i;
	char	buff[SIZE_BUFF];

dbgmsg(">dotCOBOL_UnPackValue");
	if		(  value  !=  NULL  ) {
		switch	(value->type) {
		  case	GL_TYPE_INT:
			value->body.IntegerData = *(int *)p;
			dotCOBOL_IntegerCobol2C(&value->body.IntegerData);
			p += sizeof(int);
			break;
		  case	GL_TYPE_BOOL:
			value->body.BoolData = ( *(char *)p == 'T' ) ? TRUE : FALSE;
			p ++;
			break;
		  case	GL_TYPE_BYTE:
			memcpy(value->body.CharData.sval,p,value->body.CharData.len);
			p += value->body.CharData.len;
			break;
		  case	GL_TYPE_TEXT:
			if		(  value->body.CharData.sval  !=  NULL  ) {
				if		(  value->body.CharData.len  <  textsize  ) {
					xfree(value->body.CharData.sval);
					value->body.CharData.sval = (char *)xmalloc(textsize + 1);
					value->body.CharData.len = textsize;
				}
			} else {
				value->body.CharData.sval = (char *)xmalloc(textsize + 1);
				value->body.CharData.len = textsize;
			}
			memcpy(value->body.CharData.sval,p,value->body.CharData.len);
			p += textsize;
			StringCobol2C(value->body.CharData.sval,value->body.CharData.len);
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			memcpy(value->body.CharData.sval,p,value->body.CharData.len);
			p += value->body.CharData.len;
			StringCobol2C(value->body.CharData.sval,value->body.CharData.len);
			break;
		  case	GL_TYPE_NUMBER:
			memcpy(buff,p,ValueFixed(value)->flen);
			FixedCobol2C(buff,ValueFixed(value)->flen);
			strcpy(ValueFixed(value)->sval,buff);
			p += value->body.FixedData.flen;
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < value->body.ArrayData.count ; i ++ ) {
				p = dotCOBOL_UnPackValue(p,value->body.ArrayData.item[i],textsize);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < value->body.RecordData.count ; i ++ ) {
				p = dotCOBOL_UnPackValue(p,value->body.RecordData.item[i],textsize);
			}
			break;
		  default:
			break;
		}
	}
dbgmsg("<dotCOBOL_UnPackValue");
	return	(p);
}

extern	char	*
dotCOBOL_PackValue(
	char	*p,
	ValueStruct	*value,
	size_t	textsize)
{
	int		i;
	size_t	size;

dbgmsg(">dotCOBOL_PackValue");
	if		(  value  !=  NULL  ) {
		switch	(value->type) {
		  case	GL_TYPE_INT:
			*(int *)p = value->body.IntegerData;
			dotCOBOL_IntegerC2Cobol((int *)p);
			p += sizeof(int);
			break;
		  case	GL_TYPE_BOOL:
			*(char *)p = value->body.BoolData ? 'T' : 'F';
			p ++;
			break;
		  case	GL_TYPE_BYTE:
			memcpy(p,value->body.CharData.sval,value->body.CharData.len);
			p += value->body.CharData.len;
			break;
		  case	GL_TYPE_TEXT:
			size = ( textsize < value->body.CharData.len ) ? textsize :
				value->body.CharData.len;
			memcpy(p,value->body.CharData.sval,size);
			StringC2Cobol(p,textsize);
			p += textsize;
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			memcpy(p,value->body.CharData.sval,value->body.CharData.len);
			StringC2Cobol(p,value->body.CharData.len);
			p += value->body.CharData.len;
			break;
		  case	GL_TYPE_NUMBER:
			memcpy(p,ValueFixed(value)->sval,ValueFixed(value)->flen);
			FixedC2Cobol(p,ValueFixed(value)->flen);
			p += value->body.FixedData.flen;
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < value->body.ArrayData.count ; i ++ ) {
				p = dotCOBOL_PackValue(p,value->body.ArrayData.item[i],textsize);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < value->body.RecordData.count ; i ++ ) {
				p = dotCOBOL_PackValue(p,value->body.RecordData.item[i],textsize);
			}
			break;
		  default:
			break;
		}
	}
dbgmsg("<dotCOBOL_PackValue");
	return	(p);
}
