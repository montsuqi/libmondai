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
#include	"monstring.h"
#include	"value.h"
#include	"Text_v.h"
#include	"debug.h"

static	char	*
_CSV_UnPackValue(
	char		*p,
	ValueStruct	*value)
{
	int		i;
	char	buff[SIZE_BUFF];
	char	*q
	,		*s;

	if		(  value  !=  NULL  ) {
		if		(  !IS_VALUE_STRUCTURE(value)  ) {
			memset(buff,0,SIZE_BUFF);
			q = buff;
			if		(  *p  ==  '"'  ) {
				p ++;
				while	(  *p  !=  0  ) {
					if		(	(  *p      ==  '"'  )
							&&	(  *(p+1)  ==  ','  ) )	break;
					switch	(*p) {
					  case	'\\':
						p ++;
						switch	(*p) {
						  case	'n':
							*q = '\n';
							break;
						  default:
							*q = *p;
							break;
						}
						break;
					  case	'"':
						p ++;
						switch	(*p) {
						  case	'"':
							*q = '"';
							break;
						  default:
							break;
						}
					  default:
						*q = *p ++;
						break;
					}
					q ++;
				}
				p ++;
				while	(	(  *p  !=  0    )
						&&	(  *p  !=  ','  ) ) p ++;
				p ++;
			} else {
				while	(	(  *p  !=  0    )
						&&	(  *p  !=  ','  ) ) {
					*q ++ = *p ++;
				}
				p ++;
			}
			*q = 0;
		}
		switch	(value->type) {
		  case	GL_TYPE_INT:
			ValueInteger(value) = StrToInt(buff,strlen(buff));
			break;
		  case	GL_TYPE_BOOL:
			ValueBool(value) = ( *buff == 'T' ) ? TRUE : FALSE;
			break;
		  case	GL_TYPE_FLOAT:
			ValueFloat(value) = atof(buff);
			break;
		  case	GL_TYPE_NUMBER:
			memcpy(ValueFixedBody(value),buff,ValueFixedLength(value));
			break;
		  case	GL_TYPE_TEXT:
			if		(  ValueString(value)  !=  NULL  ) {
				if		(  ValueStringLength(value)  <  strlen(buff)  ) {
					xfree(ValueString(value));
					ValueString(value) = (char *)xmalloc(strlen(buff) + 1);
					ValueStringLength(value) = strlen(buff);
				}
			} else {
				ValueString(value) = (char *)xmalloc(strlen(buff) + 1);
				ValueStringLength(value) = strlen(buff);
			}
			memset(ValueString(value),0,ValueStringLength(value)+1);
			if		(  ValueStringLength(value)  >  0  ) {
				memcpy(ValueString(value),buff,ValueStringLength(value));
			}
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			if		(  ValueString(value)  ==  NULL  ) {
				ValueString(value) = (char *)xmalloc(strlen(buff) + 1);
				ValueStringLength(value) = strlen(buff);
			}
			memcpy(ValueString(value),buff,ValueStringLength(value));
			break;
		  case	GL_TYPE_BYTE:
			s = buff;
			q = ValueByte(value);
			for	( i = 0 ; i < ValueByteLength(value) ; i ++ , q ++ ) {
				if		(  *s  ==  '%'  ) {
					s ++;
					if		(  *s  ==  '%'  ) {
						*q = '%';
						s ++;
					} else {
						*q = (unsigned char)HexToInt(s,2);
						s += 2;
					}
				} else {
					*q = *s;
					s ++;
				}
			}
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p = _CSV_UnPackValue(p,ValueArrayItem(value,i));
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p = _CSV_UnPackValue(p,ValueRecordItem(value,i));
			}
			break;
		  default:
			printf("invalid flag [%d]\n",value->type);
			break;
		}
	}
	return	(p);
}

extern	char	*
CSV_UnPackValue(
	char		*p,
	ValueStruct	*value,
	size_t		textsize)
{
	return	(_CSV_UnPackValue(p,value));
}

static	char	*
CSV_Encode(
	char	*str,
	char	fSsep)
{
	static	char	buff[SIZE_BUFF+1];
	char	*p;

	p = buff;
	for	( ; *str != 0 ; str ++ ) {
		switch	(*str) {
		  case	'"':
			if		(  fSsep  ) {
				*p ++ = '"';
				*p ++ = '"';
			}
			break;
		  case	'\n':
			*p ++ = '\\';
			*p ++ = 'n';
			break;
		  case	'\\':
			*p ++ = '\\';
			*p ++ = '\\';
			break;
		  default:
			*p ++ = *str;
			break;
		}
	}
	*p = 0;
	return	(buff);
}

static	Bool
IsComma(
	char	*str)
{
	Bool	ret;

	ret = FALSE;
	for	( ; *str != 0 ; str ++ ) {
		switch	(*str) {
		  case	',':
			ret = TRUE;
			break;
		  default:
			break;
		}
		if		(  ret  )	break;
	}
	return	(ret);
}

static	char	*
_CSV_PackValue(
	char		*p,
	ValueStruct	*value,
	Bool		fNsep,
	Bool		fSsep,
	Bool		fCesc)
{
	int		i;
	char	*str;

	if		(  value  !=  NULL  ) {
		switch	(value->type) {
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BOOL:
			str = CSV_Encode(ToString(value),fSsep);
			if		(  fSsep  ) {
				p += sprintf(p,"\"%s\",",str);
			} else {
				if		(  IsComma(str)  ) {
					p += sprintf(p,"\"%s\",",str);
				} else {
					p += sprintf(p,"%s,",str);
				}
			}
			break;
		  case	GL_TYPE_NUMBER:
		  case	GL_TYPE_INT:
		  case	GL_TYPE_FLOAT:
			if		(  fNsep  ) {
				p += sprintf(p,"\"%s\",",ToString(value));
			} else {
				p += sprintf(p,"%s,",ToString(value));
			}
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p = _CSV_PackValue(p,ValueArrayItem(value,i),fNsep,fSsep,fCesc);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p = _CSV_PackValue(p,ValueRecordItem(value,i),fNsep,fSsep,fCesc);
			}
			break;
		  default:
			break;
		}
	}
	return	(p);
}

extern	char	*
CSV1_PackValue(
	char	*p,
	ValueStruct	*value,
	size_t	textsize)
{
	char	*ret;

	ret = _CSV_PackValue(p,value,TRUE,TRUE,FALSE);
	ret --;
	*ret = 0;
	return	(ret);
}

extern	char	*
CSV2_PackValue(
	char	*p,
	ValueStruct	*value,
	size_t	textsize)
{
	char	*ret;

	ret = _CSV_PackValue(p,value,FALSE,FALSE,FALSE);
	ret --;
	*ret = 0;
	return	(ret);
}

extern	char	*
CSV3_PackValue(
	char	*p,
	ValueStruct	*value,
	size_t	textsize)
{
	char	*ret;

	ret = _CSV_PackValue(p,value,FALSE,TRUE,FALSE);
	ret --;
	*ret = 0;
	return	(ret);
}

extern	char	*
CSVE_PackValue(
	char	*p,
	ValueStruct	*value,
	size_t	textsize)
{
	char	*ret;

	ret = _CSV_PackValue(p,value,FALSE,FALSE,FALSE);
	ret --;
	*ret = 0;
	return	(ret);
}

static	size_t
_CSV_SizeValue(
	ValueStruct	*value,
	Bool		fNsep,
	Bool		fSsep,
	Bool		fCesc)
{
	int		i;
	size_t	ret;
	char	*str;
	Bool	fComma;

dbgmsg(">CSV_SizeValue");
	if		(  value  ==  NULL  )	return	(0);
	switch	(value->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_BOOL:
		str = ToString(value);
		ret = strlen(str) + 1;
		if		(  fSsep  )	ret += 2;
		fComma = FALSE;
		for	( ; *str != 0 ; str ++ ) {
			switch	(*str) {
			  case	'"':
				if		(  fSsep )	ret ++;
				break;
			  case	'\n':
			  case	'\\':
				ret ++;
				break;
			  case	',':
				fComma = TRUE;
				break;
			  default:
				break;
			}
		}
		if		(	(  fCesc   )
				&&	(  fComma  ) )	ret += 2;
		break;
	  case	GL_TYPE_NUMBER:
	  case	GL_TYPE_INT:
	  case	GL_TYPE_FLOAT:
		ret = strlen(ToString(value)) + 1;
		if		(  fNsep  )	ret += 2;
		break;
	  case	GL_TYPE_ARRAY:
		ret = 0;
		for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
			ret += _CSV_SizeValue(ValueArrayItem(value,i),fNsep,fSsep,fCesc);
		}
		break;
	  case	GL_TYPE_RECORD:
		ret = 0;
		for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
			ret += _CSV_SizeValue(ValueRecordItem(value,i),fNsep,fSsep,fCesc);
		}
		break;
	  case	GL_TYPE_OBJECT:
	  default:
		ret = 0;
		break;
	}
dbgmsg("<_CSV_SizeValue");
	return	(ret);
}

extern	size_t
CSV1_SizeValue(
	ValueStruct	*value,
	size_t		arraysize,
	size_t		textsize)
{
	size_t	ret;

dbgmsg(">CSV1_SizeValue");
	if		(  value  ==  NULL  )	return	(0);
	ret = _CSV_SizeValue(value,TRUE,TRUE,FALSE);
dbgmsg("<CSV1_SizeValue");
	return	(ret);
}

extern	size_t
CSV2_SizeValue(
	ValueStruct	*value,
	size_t		arraysize,
	size_t		textsize)
{
	size_t	ret;

dbgmsg(">CSV1_SizeValue");
	if		(  value  ==  NULL  )	return	(0);
	ret = _CSV_SizeValue(value,FALSE,FALSE,FALSE);
dbgmsg("<CSV1_SizeValue");
	return	(ret);
}

extern	size_t
CSV3_SizeValue(
	ValueStruct	*value,
	size_t		arraysize,
	size_t		textsize)
{
	size_t	ret;

dbgmsg(">CSV3_SizeValue");
	if		(  value  ==  NULL  )	return	(0);
	ret = _CSV_SizeValue(value,FALSE,TRUE,FALSE);
dbgmsg("<CSV3_SizeValue");
	return	(ret);
}

extern	size_t
CSVE_SizeValue(
	ValueStruct	*value,
	size_t		arraysize,
	size_t		textsize)
{
	size_t	ret;

dbgmsg(">CSV3_SizeValue");
	if		(  value  ==  NULL  )	return	(0);
	ret = _CSV_SizeValue(value,FALSE,FALSE,TRUE);
dbgmsg("<CSV3_SizeValue");
	return	(ret);
}

