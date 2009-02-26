/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2007-2009 Ogochan.
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
#include	<ctype.h>
#include    <sys/types.h>

#include	"types.h"
#include	"misc_v.h"
#include	"monstring.h"
#include	"memory_v.h"
#include	"others.h"
#include	"value.h"
#include	"getset.h"
#include	"json_v.h"
#include	"debug.h"

#define	SKIP_SPACE(p)								\
			while	(	( *(p)  !=  0     )			\
					&&	(  isspace(*(p))  ) )	(p) ++

static	size_t
ParseString(
	byte	*p,
	byte	**str)
{
	size_t	size;
	byte	*s;

	size = 0;
	s = *str;
	while	(	(  *s  !=  0  )
			&&	(  *s  !=  '"'  ) ) {
		switch	(*s) {
		  case	'\\':
			s ++;
			switch	(*s) {
			  case	'u':
				*p =  ( *(s+1) << 12 )
					| ( *(s+2) << 8  )
					| ( *(s+3) << 4  )
					| ( *(s+4)       );
				s += 4;
			  case	'n':
				*p = '\n';
				break;
			  case	'r':
				*p = '\r';
				break;
			  case	't':
				*p = '\t';
				break;
			  default:
				*p = *s;
				break;
			}
			break;
		  default:
			*p = *s;
			break;
		}
		p ++;
		s ++;
		size ++;
	}
	*p = 0;
	(*str) = s;

	return	(size);
}

static	size_t
ParseStringSize(
	char	*p)
{
	size_t	size;

	size = 0;
	while	(	(  *p  !=  0  )
			&&	(  *p  !=  '"'  ) ) {
		if		(  *p  ==  '\\'  ) {
			p ++;
			switch	(*p) {
			  case	'u':
				p+= 4;
				break;
			  default:
				break;
			}
		}
		p ++;
		size ++;
	}
	return	(size);
}

static	size_t
_JSON_UnPackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value)
{
	char	name[SIZE_LONGNAME+1]
		,	buff[SIZE_BUFF];
	char	*str;
	byte	*pp;
	size_t	size;
	ValueStruct	*e;
	int		i;

ENTER_FUNC;
	pp = p;
	if		(  value  !=  NULL  ) {
		SKIP_SPACE(p);
		dbgprintf("(%c)",*p);
		switch	(*p) {
		  case	'{':
			p ++;
			SKIP_SPACE(p);
			while	(	(  *p  !=  0    )
					&&	(  *p  !=  '}'  ) ) {
				if		(  *p  ==  '"'  ) {
					p ++;
					(void)ParseString(name,&p);
					dbgprintf("name = [%s]",name);
					e = GetRecordItem(value,name);
					p ++;
					dbgprintf("[%c]",*p);
					SKIP_SPACE(p);
					if		(  *p  !=  ':'  )	break;
					p ++;
					SKIP_SPACE(p);
					p += _JSON_UnPackValue(opt,p,e);
					SKIP_SPACE(p);
					if		(  *p  ==  ','  ) {
						p ++;
						SKIP_SPACE(p);
					}
				} else
					break;
			}
			if		(  *p  !=  0  )	p ++;
			break;
		  case	'[':
			p ++;
			SKIP_SPACE(p);
			i = 0;
			while	(	(  *p  !=  0    )
					&&	(  *p  !=  ']'  ) ) {
				p += _JSON_UnPackValue(opt,p,GetArrayItem(value,i));
				i ++;
				SKIP_SPACE(p);
				if		(  *p  ==  ','  ) {
					p ++;
					SKIP_SPACE(p);
				}
			}
			if		(  *p  !=  0  )	p ++;
			break;
		  case	'"':
			p ++;
			size = ParseStringSize(p);
			str = (byte *)xmalloc(size+1);
			size = ParseString(str,&p);
			dbgprintf("str = %d",(int)size);
			SetValueBinary(value,str,size);
			xfree(str);
			if		(  *p  !=  0  )	p ++;
			break;
		  default:
			if		(  strlcmp(p,"true")  ==  0  ) {
				SetValueBool(value,TRUE);
				p += 4;
			} else
			if		(  strlcmp(p,"false")  ==  0  ) {
				SetValueBool(value,FALSE);
				p += 5;
			} else
			if		(  strlcmp(p,"null")  ==  0  ) {
				ValueIsNil(value);
				p += 4;
			} else
			if		(	(  *p  ==  '-'  )
					||	(  isdigit(*p)  ) ) {
				str = buff;
				while	(	(  *p  ==  '-'  )
						||	(  *p  ==  '+'  )
						||	(  *p  ==  '.'  )
						||	(  *p  ==  'e'  )
						||	(  *p  ==  'E'  )
						||	(  isdigit(*p)  ) ) {
					*str ++ = *p ++;
				}
				*str = 0;
				dbgprintf("buff = [%s]",buff);
				SetValueString(value,buff,"utf-8");
			}
			break;
		}
		if		(  *p  ==  0  )	goto	quit;
	}
  quit:;
LEAVE_FUNC;
	return	(p-pp);
}

extern	size_t
JSON_UnPackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value)
{
	byte	*pp;

ENTER_FUNC;
	pp = p;
	SKIP_SPACE(p);
	p += _JSON_UnPackValue(opt,p,value);
LEAVE_FUNC;
	return	(p-pp);
}

static	size_t
_JSON_PackValue(
	CONVOPT	*opt,
	byte		*p,
	ValueStruct	*value)
{
	int		i;
	byte	*pp;
	char	*str;

ENTER_FUNC;
	pp = p;
	if		(	(  value  ==  NULL      )
			||	(  IS_VALUE_NIL(value)  ) ) {
		p += sprintf(p,"null");
	} else {
		switch	(value->type) {
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_SYMBOL:
		  case	GL_TYPE_ALIAS:
		  case	GL_TYPE_OBJECT:
			str = ValueToString(value,"utf-8");
			*p ++ = '"';
			p += EncodeStringBackslash(p,str);
			*p ++ = '"';
			break;
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BINARY:
#if	0
			str = ValueToString(value,"utf-8");
			p += sprintf(p,"\"%s\"",str);
#else
			p += sprintf(p,"\"\"");
#endif
			break;
		  case	GL_TYPE_BOOL:
			if		(  ValueBool(value)  ) {
				p += sprintf(p,"true");
			} else {
				p += sprintf(p,"false");
			}
			break;
		  case	GL_TYPE_INT:
			p += sprintf(p,"%d",ValueInteger(value));
			break;
		  case	GL_TYPE_NUMBER:
		  case	GL_TYPE_FLOAT:
			str = ValueToString(value,"utf-8");
			while	(  *str  ==  '0'  )	str ++;
			if		(  *str  ==  0  )	str --;
			p += sprintf(p,"%s",str);
			break;
		  case	GL_TYPE_ARRAY:
			opt->nIndent ++;
			p += sprintf(p,"[");
			p += PutCR(opt,p);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p += IndentLine(opt,p);
				p += _JSON_PackValue(opt,p,ValueArrayItem(value,i));
				if		(  i  <  ValueArraySize(value) - 1  ) {
					p += sprintf(p,",");
				}
				p += PutCR(opt,p);
			}
			opt->nIndent --;
			p += IndentLine(opt,p);
			p += sprintf(p,"]");
			break;
		  case	GL_TYPE_RECORD:
			opt->nIndent ++;
			p += sprintf(p,"{");
			p += PutCR(opt,p);
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p += IndentLine(opt,p);
				p += sprintf(p,"\"%s\":",ValueRecordName(value,i));
				p += _JSON_PackValue(opt,p,ValueRecordItem(value,i));
				if		(  i  <  ValueRecordSize(value) - 1  ) {
					p += sprintf(p,",");
				}
				p += PutCR(opt,p);
			}
			opt->nIndent --;
			p += IndentLine(opt,p);
			p += sprintf(p,"}");
			break;
		  default:
			break;
		}
	}
LEAVE_FUNC;
	return	(p-pp);
}

extern	size_t
JSON_PackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value)
{
	size_t	ret;

ENTER_FUNC;
	ret = _JSON_PackValue(opt,p,value);
	if		(  ret  >  0  ) {
		*(p+ret) = 0;
	}
LEAVE_FUNC;
	return	(ret);
}

static	size_t
_JSON_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value)
{
	int		i;
	size_t	ret;
	char	*str;

ENTER_FUNC;
	if		(	(  value  ==  NULL  )
			||	(  IS_VALUE_NIL(value)  ) ) {
		ret = 4;	//	null
	} else {
		ret = 0;
		switch	(ValueType(value)) {
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_SYMBOL:
			ret += EncodeLength(opt,ValueToString(value,"utf-8")) + 2;
			break;
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BINARY:
#if	0
			ret += strlen(ValueToString(value,"utf-8")) + 2;
#else
			ret = 2;
#endif
			break;
		  case	GL_TYPE_BOOL:
			if		(  ValueBool(value)  ) {
				ret += 4;	//	true
			} else {
				ret += 5;	//	false
			}
			break;
		  case	GL_TYPE_INT:
		  case	GL_TYPE_FLOAT:
		  case	GL_TYPE_OBJECT:
		  case	GL_TYPE_NUMBER:
			str = ValueToString(value,"utf-8");
			while	(  *str  ==  '0'  )	str ++;
			if		(  *str  ==  0  )	str --;
			ret += EncodeLength(opt,str);
			break;
		  case	GL_TYPE_ARRAY:
			ret += 2;
			ret += PutCR(opt,NULL);
			opt->nIndent ++;
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				ret += IndentLine(opt,NULL);
				ret += _JSON_SizeValue(opt,ValueArrayItem(value,i));
				if		(  i  <  ValueArraySize(value) - 1  ) {
					ret ++;
				}
				ret += PutCR(opt,NULL);
			}
			opt->nIndent --;
			ret += IndentLine(opt,NULL);
			break;
		  case	GL_TYPE_RECORD:
			ret += 2;
			ret += PutCR(opt,NULL);
			opt->nIndent ++;
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				ret += IndentLine(opt,NULL);
				ret += EncodeLength(opt,ValueRecordName(value,i)) + 3;	//	"...":
				ret += _JSON_SizeValue(opt,ValueRecordItem(value,i));
				if		(  i  <  ValueRecordSize(value) - 1  ) {
					ret ++;
				}
				ret += PutCR(opt,NULL);
			}
			opt->nIndent --;
			ret += IndentLine(opt,NULL);
			break;
		  case	GL_TYPE_ALIAS:
		  default:
			ret = 0;
			break;
		}
	}
LEAVE_FUNC;
	return	(ret);
}

extern	size_t
JSON_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value)
{
	ConvSetEncoding(opt,STRING_ENCODING_BACKSLASH);
	return	(_JSON_SizeValue(opt,value));
}

static	size_t
_JSON_Parse(
	byte		*p,
	ValueStruct	**ret)
{
	ValueStruct	*value
		,		*lower;
	size_t	size;
	Bool	fFloat;
	int		i;
	char	name[SIZE_LONGNAME+1]
		,	buff[SIZE_BUFF];
	char	*str;
	byte	*pp;

ENTER_FUNC;
	pp = p;
	SKIP_SPACE(p);
	if		(  *p  !=  0  ) {
		SKIP_SPACE(p);
		dbgprintf("(%c)",*p);
		switch	(*p) {
		  case	'{':
			p ++;
			SKIP_SPACE(p);
			value = NewValue(GL_TYPE_RECORD);
			while	(	(  *p  !=  0    )
					&&	(  *p  !=  '}'  ) ) {
				if		(  *p  ==  '"'  ) {
					p ++;
					(void)ParseString(name,&p);
					dbgprintf("name = [%s]",name);
					p ++;
					SKIP_SPACE(p);
					if		(  *p  !=  ':'  )	break;
					p ++;
					SKIP_SPACE(p);
					p += _JSON_Parse(p,&lower);
					SKIP_SPACE(p);
					if		(  *p  ==  ','  ) {
						p ++;
						SKIP_SPACE(p);
					}
					ValueAddRecordItem(value,name,lower);
				} else
					break;
			}
			if		(  *p  !=  0  )	p ++;
			break;
		  case	'[':
			p ++;
			SKIP_SPACE(p);
			value = NewValue(GL_TYPE_ARRAY);
			i = 0;
			while	(	(  *p  !=  0    )
					&&	(  *p  !=  ']'  ) ) {
				p += _JSON_Parse(p,&lower);
				ValueAddArrayItem(value,i,lower);
				i ++;
				SKIP_SPACE(p);
				if		(  *p  ==  ','  ) {
					p ++;
					SKIP_SPACE(p);
				}
			}
			if		(  *p  !=  0  )	p ++;
			break;
		  case	'"':
			p ++;
			size = ParseStringSize(p);
			str = (char *)xmalloc(size+1);
			(void)ParseString(str,&p);
			dbgprintf("str = [%s]%d",str,(int)size);
			value = NewValue(GL_TYPE_TEXT);
			SetValueString(value,str,"utf8");
			xfree(str);
			if		(  *p  !=  0  )	p ++;
			break;
		  default:
			if		(  strlcmp(p,"true")  ==  0  ) {
				value = NewValue(GL_TYPE_BOOL);
				SetValueBool(value,TRUE);
				p += 4;
			} else
			if		(  strlcmp(p,"false")  ==  0  ) {
				value = NewValue(GL_TYPE_BOOL);
				SetValueBool(value,FALSE);
				p += 5;
			} else
			if		(  strlcmp(p,"null")  ==  0  ) {
				value = NewValue(GL_TYPE_BOOL);
				ValueIsNil(value);
				p += 4;
			} else
			if		(	(  *p  ==  '-'  )
					||	(  isdigit(*p)  ) ) {
				str = buff;
				fFloat = FALSE;
				while	(	(  *p  ==  '-'  )
						||	(  *p  ==  '+'  )
						||	(  *p  ==  '.'  )
						||	(  *p  ==  'e'  )
						||	(  *p  ==  'E'  )
						||	(  isdigit(*p)  ) ) {
					if		(	(  *p  ==  '.'  )
							||	(  *p  ==  'e'  )
							||	(  *p  ==  'E'  ) ) {
						fFloat = TRUE;
					}
					*str ++ = *p ++;
				}
				*str = 0;
				dbgprintf("buff = [%s]",buff);
				if		(  fFloat  ) {
					value = NewValue(GL_TYPE_FLOAT);
				} else {
					value = NewValue(GL_TYPE_INT);
				}
				SetValueString(value,buff,"utf-8");
			} else {
				value = NULL;
			}
			break;
		}
		*ret = value;
	} else {
		*ret = NULL;
	}
LEAVE_FUNC;
	return	(p-pp);
}
	
extern	size_t
JSON_Parse(
	char	*str,
	ValueStruct	**ret)
{
	size_t	size;
ENTER_FUNC;
	SKIP_SPACE(str);
	size = _JSON_Parse(str,ret);
LEAVE_FUNC;
	return	(size);
}
