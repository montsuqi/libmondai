/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2001-2003 Ogochan & JMA (Japan Medical Association).
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
#include	"misc_v.h"
#include	"memory_v.h"
#include	"value.h"
#include	"cobolvalue.h"
#include	"others.h"
#include	"monstring.h"
#include	"OpenCOBOL_v.h"
#include	"getset.h"
#include	"debug.h"

#define	IntegerC2Cobol	IntegerCobol2C
static	void
IntegerCobol2C(
	CONVOPT	*opt,
	int		*ival)
{
	char	*p
	,		c;

#ifndef	WORDS_BIGENDIAN
	if		(  opt->fBigEndian  ) {
		p = (char *)ival;
		c = p[3];
		p[3] = p[0];
		p[0] = c;
		c = p[2];
		p[2] = p[1];
		p[1] = c;
	}
#else
	if		(  !opt->fBigEndian  ) {
		p = (char *)ival;
		c = p[3];
		p[3] = p[0];
		p[0] = c;
		c = p[2];
		p[2] = p[1];
		p[1] = c;
	}
#endif
}

#define COBOL_1_0
#ifdef COBOL_1_0
static	void
FixedCobol2C(
	char	*buff,
	size_t	size)
{
	buff[size] = 0;
	if (buff[size - 1] & 0x40) {
		buff[0] |= 0x40;
		buff[size - 1] ^= 0x40;
	}
}

static	void
FixedC2Cobol(
	char	*buff,
	size_t	size)
{
	if (buff[0] & 0x40) {
		buff[0] ^= 0x40;
		buff[size - 1] |= 0x40;
	}
}

#else

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
#endif

extern	size_t
OpenCOBOL_UnPackValue(
	CONVOPT	*opt,
	unsigned char	*p,
	ValueStruct	*value)
{
	int		i;
	char	buff[SIZE_NUMBUF+1];
	unsigned char	*q;

ENTER_FUNC;
	q = p;
	if		(  value  !=  NULL  ) {
		ValueIsNonNil(value);
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			ValueInteger(value) = *(int *)p;
			IntegerCobol2C(opt,&ValueInteger(value));
			p += sizeof(int);
			break;
		  case	GL_TYPE_FLOAT:
			ValueFloat(value) = *(double *)p;
			p += sizeof(double);
			break;
		  case	GL_TYPE_BOOL:
			ValueBool(value) = ( *(char *)p == 'T' ) ? TRUE : FALSE;
			p ++;
			break;
		  case	GL_TYPE_OBJECT:
			if (IsCobolSpace(p, sizeof(ValueObjectId(value)))){
				ValueObjectId(value) = 0;
			} else {
				ValueObjectId(value) = *(MonObjectType *)p;
			}
			p += sizeof(ValueObjectId(value));
			if		(  ValueObjectFile(value)  !=  NULL  ) {
				xfree(ValueObjectFile(value));
				ValueObjectFile(value) = NULL;
			}
			break;
		  case	GL_TYPE_BYTE:
			memcpy(ValueByte(value),p,ValueByteLength(value));
			p += ValueByteLength(value);
			break;
		  case	GL_TYPE_BINARY:
			SetValueBinary(value,p,opt->textsize);
			p += opt->textsize;
			break;
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_SYMBOL:
			StringCobol2C(p,opt->textsize);
			SetValueStringWithLength(value,p,opt->textsize,ConvCodeset(opt));
			p += opt->textsize;
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			StringCobol2C(p,ValueStringLength(value));
			SetValueStringWithLength(value,p,ValueStringLength(value),ConvCodeset(opt));
			p += ValueStringLength(value);
			break;
		  case	GL_TYPE_NUMBER:
			memcpy(buff,p,ValueFixedLength(value));
			FixedCobol2C(buff,ValueFixedLength(value));
			strcpy(ValueFixedBody(value),buff);
			p += ValueFixedLength(value);
			break;
		  case	GL_TYPE_TIMESTAMP:
			ValueDateTimeYear(value) = StrToInt(p,4);	p += 4;
			ValueDateTimeMon(value) = StrToInt(p,2);	p += 2;
			ValueDateTimeMDay(value) = StrToInt(p,2);	p += 2;
			ValueDateTimeHour(value) = StrToInt(p,2);	p += 2;
			ValueDateTimeMin(value) = StrToInt(p,2);	p += 2;
			ValueDateTimeSec(value) = StrToInt(p,2);	p += 2;
			mktime(ValueDateTime(value));
			break;
		  case	GL_TYPE_TIME:
			ValueDateTimeYear(value) = 0;
			ValueDateTimeMon(value) = 0;
			ValueDateTimeMDay(value) = 0;
			ValueDateTimeHour(value) = StrToInt(p,2);	p += 2;
			ValueDateTimeMin(value) = StrToInt(p,2);	p += 2;
			ValueDateTimeSec(value) = StrToInt(p,2);	p += 2;
			break;
		  case	GL_TYPE_DATE:
			ValueDateTimeYear(value) = StrToInt(p,2);	p += 4;
			ValueDateTimeMon(value) = StrToInt(p,2);	p += 2;
			ValueDateTimeMDay(value) = StrToInt(p,2);	p += 2;
			ValueDateTimeHour(value) = 0;
			ValueDateTimeMin(value) = 0;
			ValueDateTimeSec(value) = 0;
			mktime(ValueDateTime(value));
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p += OpenCOBOL_UnPackValue(opt,p,ValueArrayItem(value,i));
				ValueParent(ValueArrayItem(value,i)) = value;
				ValueIndex(ValueArrayItem(value,i)) = i;
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p += OpenCOBOL_UnPackValue(opt,p,ValueRecordItem(value,i));
				ValueParent(ValueRecordItem(value,i)) = value;
				ValueIndex(ValueRecordItem(value,i)) = i;
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
OpenCOBOL_PackValue(
	CONVOPT	*opt,
	unsigned char	*p,
	ValueStruct	*value)
{
	int		i;
	size_t	size;
	unsigned char	*pp;

ENTER_FUNC;
	pp = p; 
	if		(  value  !=  NULL  ) {
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			*(int *)p = ValueInteger(value);
			IntegerC2Cobol(opt,(int *)p);
			p += sizeof(int);
			break;
		  case	GL_TYPE_FLOAT:
			*(double *)p = ValueFloat(value);
			p += sizeof(double);
			break;
		  case	GL_TYPE_BOOL:
			*(char *)p = ValueBool(value) ? 'T' : 'F';
			p ++;
			break;
		  case	GL_TYPE_BYTE:
			memcpy(p,ValueByte(value),ValueByteLength(value));
			p += ValueByteLength(value);
			break;
		  case	GL_TYPE_BINARY:
			memclear(p,opt->textsize);
			size = ( opt->textsize < ValueByteLength(value) ) ? opt->textsize : ValueByteLength(value);
			memcpy(p,ValueToBinary(value),size);
			p += opt->textsize;
			break;
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_SYMBOL:
			size = ( opt->textsize < ValueStringLength(value) ) ? opt->textsize : ValueStringLength(value);
			memcpy(p,ValueToString(value,ConvCodeset(opt)),size);
			StringC2Cobol(p,opt->textsize);
			p += opt->textsize;
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			if		(  IS_VALUE_NIL(value)  ) {
				memclear(p,ValueStringLength(value));	/*	LOW-VALUE	*/
			} else {
				strncpy(p,ValueToString(value,ConvCodeset(opt)),ValueStringLength(value));
				StringC2Cobol(p,ValueStringLength(value));
			}
			p += ValueStringLength(value);
			break;
		  case	GL_TYPE_NUMBER:
			memcpy(p,ValueFixedBody(value),ValueFixedLength(value));
			FixedC2Cobol(p,ValueFixedLength(value));
			p += ValueFixedLength(value);
			break;
		  case	GL_TYPE_OBJECT:
			*(MonObjectType *)p = ValueObjectId(value);
			p += sizeof(ValueObjectId(value));
			break;
		  case	GL_TYPE_TIMESTAMP:
			p += sprintf(p,"%04d%02d%02d%02d%02d%02d",
						 ValueDateTimeYear(value),
						 ValueDateTimeMon(value),
						 ValueDateTimeMDay(value),
						 ValueDateTimeHour(value),
						 ValueDateTimeMin(value),
						 ValueDateTimeSec(value));
			break;
		  case	GL_TYPE_DATE:
			p += sprintf(p,"%04d%02d%02d",
						 ValueDateTimeYear(value),
						 ValueDateTimeMon(value),
						 ValueDateTimeMDay(value));
			break;
		  case	GL_TYPE_TIME:
			p += sprintf(p,"%02d%02d%02d",
						 ValueDateTimeHour(value),
						 ValueDateTimeMin(value),
						 ValueDateTimeSec(value));
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p += OpenCOBOL_PackValue(opt,p,ValueArrayItem(value,i));
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p += OpenCOBOL_PackValue(opt,p,ValueRecordItem(value,i));
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
OpenCOBOL_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value)
{
	int		i;
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
	  case	GL_TYPE_BINARY:
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_SYMBOL:
		ret = opt->textsize;
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
	  case	GL_TYPE_OBJECT:
		ret = sizeof(ValueObjectId(value));
		break;
	  case	GL_TYPE_TIMESTAMP:
		ret = 8 + 6;
		break;
	  case	GL_TYPE_DATE:
		ret = 8;
		break;
	  case	GL_TYPE_TIME:
		ret = 6;
		break;
	  case	GL_TYPE_ARRAY:
		ret = 0;
		for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
			ret += OpenCOBOL_SizeValue(opt,ValueArrayItem(value,i));
		}
		break;
	  case	GL_TYPE_RECORD:
		ret = 0;
		for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
			ret += OpenCOBOL_SizeValue(opt,ValueRecordItem(value,i));
		}
		break;
	  default:
		ret = 0;
		break;
	}
dbgmsg("<OpenCOBOL_SizeValue");
	return	(ret);
}

