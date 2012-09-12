/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2003 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2005-2008 Ogochan.
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


#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<glib.h>

#define	_REC_PARSER
#define	__VALUE_DIRECT
#include	"types.h"
#include	"hash_v.h"
#include	"value.h"
#include	"misc_v.h"
#include	"monstring.h"
#include	"memory_v.h"
#include	"others.h"
#include	"Lex.h"
#include	"RecParser.h"
#include	"debug.h"

#define	T_CHAR			(T_YYBASE +1)
#define	T_TEXT			(T_YYBASE +2)
#define	T_INT			(T_YYBASE +3)
#define	T_INPUT			(T_YYBASE +4)
#define	T_OUTPUT		(T_YYBASE +5)
#define	T_BOOL			(T_YYBASE +6)
#define	T_BYTE			(T_YYBASE +7)
#define	T_NUMBER		(T_YYBASE +8)
#define	T_VARCHAR		(T_YYBASE +9)
#define	T_DBCODE		(T_YYBASE +10)
#define	T_OBJECT		(T_YYBASE +11)
#define	T_FLOAT			(T_YYBASE +12)
#define	T_TIMESTAMP		(T_YYBASE +13)
#define	T_DATE			(T_YYBASE +14)
#define	T_TIME			(T_YYBASE +15)

#define	T_VIRTUAL		(T_YYBASE +16)
#define	T_ALIAS			(T_YYBASE +17)
#define	T_BINARY		(T_YYBASE +18)
#define	T_UNIQ			(T_YYBASE +19)
#define	T_PRIMARY		(T_YYBASE +20)
#define	T_KEY			(T_YYBASE +21)

static	void	ParValueDefines(CURFILE *in, ValueStruct *upper);

static	TokenTable	tokentable[] = {
	{	"bool"		,T_BOOL		},
	{	"byte"		,T_BYTE		},
	{	"binary"	,T_BINARY	},
	{	"char"		,T_CHAR		},
	{	"varchar"	,T_VARCHAR	},
	{	"float"		,T_FLOAT	},
	{	"timestamp"	,T_TIMESTAMP},
	{	"date"		,T_DATE		},
	{	"time"		,T_TIME		},
	{	"input"		,T_INPUT	},
	{	"int"		,T_INT		},
	{	"integer"	,T_INT		},
	{	"number"	,T_NUMBER	},
	{	"output"	,T_OUTPUT	},
	{	"text"		,T_TEXT		},
	{	"object"	,T_OBJECT	},
	{	"dbcode"	,T_DBCODE	},
	{	"virtual"	,T_VIRTUAL	},
	{	"alias"		,T_ALIAS	},
	{	"primary"	,T_PRIMARY	},
	{	"key"		,T_KEY		},
	{	""			,0			}
};

static	GHashTable	*Reserved;
static	GHashTable	*ParsedRec;

extern	void
SetValueAttribute(
	ValueStruct			*val,
	ValueAttributeType	attr)
{
	int		i;

	ValueAttribute(val) |= attr;
	switch	(ValueType(val)) {
	  case	GL_TYPE_ARRAY:
		for	( i = 0 ; i < ValueArraySize(val) ; i ++ ) {
			SetValueAttribute(ValueArrayItem(val,i),attr);
		}
		break;
	  case	GL_TYPE_RECORD:
		for	( i = 0 ; i < ValueRecordSize(val) ; i ++ ) {
			SetValueAttribute(ValueRecordItem(val,i),attr);
		}
		break;
	  default:
		break;
	}
}

static	void
_Error(
	char	*msg,
	char	*fn,
	int		line)
{
	printf("%s:%d:%s\n",fn,line,msg);
}
#undef	Error
#define	Error(msg)		{in->fError=TRUE;_Error((msg),in->fn,in->cLine);}

typedef	struct _ArrayDimension	{
	int		count;
	Bool	fExpandable;
	struct	_ArrayDimension	*next;
}	ArrayDimension;

extern	ValueStruct	*
ParValueDefine(
	CURFILE	*in)
{
	size_t		size
	,			ssize;
	char		buff[SIZE_LONGNAME+1];
	char		*p;
	int			token
	,			i;
	ValueStruct	*value
	,			*array;
	ArrayDimension	*next
	,				*curr;
	Bool		fExpandable;

ENTER_FUNC;
	SetReserved(in,Reserved); 
	value = NULL;
	switch	(GetSymbol) {
	  case	T_ALIAS:
	  case	T_EQ:
		value = NewValue(GL_TYPE_ALIAS);
		p = buff;
		while	(	(  GetSymbol  !=  ','  )
				&&	(  ComToken   !=  ';'  ) ) {
			switch	(ComToken) {
			  case	T_SYMBOL:
				p += sprintf(p,"%s",ComSymbol);
				break;
			  case	'[':
				if		(  GetSymbol  ==  T_ICONST  ) {
					size = ComInt;
					GetSymbol;
				} else {
					size = 0;
				}
				if		(  ComToken  ==  ']'  ) {
					p += sprintf(p,"[%d]",(int)size);
				} else {
					Error("invalid sufix");
				}
				break;
			  case	'.':
				p += sprintf(p,".");
				break;
			  default:
				Error("invalid char in alias");
				break;
			}
			ValueAliasName(value) = StrDup(buff);
		}
		break;
	  case	T_BYTE:
	  case	T_CHAR:
	  case	T_NUMBER:
	  case	T_DBCODE:
	  case	T_VARCHAR:
	  case	T_TEXT:
		token = ComToken;
		size = 0;
		ssize = 0;
		fExpandable = FALSE;
		if		(  GetSymbol  ==  '('  ) {
			if		(  GetSymbol  ==  T_ICONST  ) {
				size = ComInt;
				if		(  token  ==  T_NUMBER  ) {
					switch	(GetSymbol) {
					  case	')':
						ssize = 0;
						break;
					  case	',':
						if		(  GetSymbol  ==  T_ICONST  ) {
							ssize = ComInt;
						} else {
							Error("field size invalud");
						}
						GetSymbol;
						break;
					}
				} else {
					GetSymbol;
				}
				if		(  ComToken  !=  ')'  ) {
					Error("not closed left paren");
				}
			} else {
				Error("invalid size definition");
			}
			GetSymbol;
		} else {
			if		(	(  token  ==  T_DBCODE  )
					||	(  token  ==  T_TEXT    ) ) {
				fExpandable = TRUE;
			} else {
				size = 1;
			}
		}
		if		(  !in->fError  ) {
			switch	(token) {
			  case	T_BYTE:
				value = NewValue(GL_TYPE_BYTE);
				break;
			  case	T_CHAR:
				value = NewValue(GL_TYPE_CHAR);
				break;
			  case	T_VARCHAR:
				value = NewValue(GL_TYPE_VARCHAR);
				break;
			  case	T_DBCODE:
				value = NewValue(GL_TYPE_DBCODE);
				break;
			  case	T_NUMBER:
				value = NewValue(GL_TYPE_NUMBER);
				break;
			  case	T_TEXT:
				value = NewValue(GL_TYPE_TEXT);
			  default:
				break;
			}
			switch(ValueType(value)) {
			  case	GL_TYPE_NUMBER:
				ValueFixedLength(value) = size;
				ValueFixedSlen(value) = ssize;
				ValueFixedBody(value) = (char *)xmalloc(size + 1);
				ValueFixedBody(value)[size] = 0;
				memset(ValueFixedBody(value),'0',size);
				break;
			  case	GL_TYPE_BYTE:
				ValueByteLength(value) = size;
				ValueByteSize(value) = size;
				ValueByte(value) = (unsigned char *)xmalloc(ValueByteSize(value));
				memclear(ValueByte(value),ValueByteSize(value));
				break;
			  case GL_TYPE_OBJECT:
				break;
			  default:
				if		(  fExpandable  ) {
					ValueIsExpandable(value);
				} else {
					ValueIsNonExpandable(value);
					ValueStringLength(value) = size;
					ValueStringSize(value) = size+1;
					ValueString(value) = (unsigned char *)xmalloc(ValueStringSize(value));
					memclear(ValueString(value),ValueStringSize(value));
				}
				break;
			}
		} else {
			value = NULL;
		}
		break;
	  case	T_BINARY:
		value = NewValue(GL_TYPE_BINARY);
		ValueIsExpandable(value);
		GetSymbol;
		break;
	  case	T_INT:
		value = NewValue(GL_TYPE_INT);
		GetSymbol;
		break;
	  case	T_FLOAT:
		value = NewValue(GL_TYPE_FLOAT);
		GetSymbol;
		break;
	  case	T_TIMESTAMP:
		value = NewValue(GL_TYPE_TIMESTAMP);
		GetSymbol;
		break;
	  case	T_DATE:
		value = NewValue(GL_TYPE_DATE);
		GetSymbol;
		break;
	  case	T_TIME:
		value = NewValue(GL_TYPE_TIME);
		GetSymbol;
		break;
	  case	T_BOOL:
		value = NewValue(GL_TYPE_BOOL);
		GetSymbol;
		break;
	  case	T_OBJECT:
		value = NewValue(GL_TYPE_OBJECT);
		GetSymbol;
		break;
	  case	'{':
	  case	'(':
		value = NewValue(GL_TYPE_RECORD);
		GetName;
		ParValueDefines(in,value);
		break;
	  default:
		value = NULL;
		Error("not supported");
		break;
	}
	next = NULL;
	while	(  ComToken  ==  '['  ) {	
		curr = New(ArrayDimension);
		if		(  GetSymbol  == T_ICONST  ) {
			curr->count = ComInt;
			curr->fExpandable = FALSE;
			GetSymbol;
		} else {
			curr->count = 0;
			curr->fExpandable = TRUE;
		}
		curr->next = next;
		next = curr;
		if		(  ComToken  !=  ']'  ) {
			Error("unmatch lbracket");
			break;
		}
		GetSymbol;
	}
	for	( curr = next ; curr != NULL ; ) {
		array = NewValue(GL_TYPE_ARRAY);
		ValueArraySize(array) = curr->count;
		ValueArrayPrototype(array) = value;
		if		(  curr->fExpandable  ) {
			ValueIsExpandable(array);
			ValueArrayItems(array) = NULL;
		} else {
			ValueIsNonExpandable(array);
			ValueArrayItems(array) = MakeValueArray(value,curr->count,FALSE);
			for(i = 0; i < ValueArraySize(array); i++) {
				ValueParent(ValueArrayItem(array,i)) = array;
				ValueIndex(ValueArrayItem(array,i)) = i;
			}
		}
		next = curr->next;
		xfree(curr);
		curr = next;
		value = array;
	}
LEAVE_FUNC;
	return	(value);
}

static	void
ParValueDefines(
	CURFILE		*in,
	ValueStruct	*upper)
{
	ValueAttributeType	attr;
	ValueStruct			*value;
	char				name[SIZE_SYMBOL+1];

ENTER_FUNC;
	while	(  ComToken  ==  T_SYMBOL  ) {
		strcpy(name,ComSymbol);
		value = ParValueDefine(in);
		attr = ValueAttribute(upper);
		while	(  ComToken  ==  ','  ) {
			switch	(GetSymbol) {
			  case	T_INPUT:
				attr |= GL_ATTR_INPUT;
				break;
			  case	T_OUTPUT:
				attr |= GL_ATTR_OUTPUT;
				break;
			  case	T_VIRTUAL:
				attr |= GL_ATTR_VIRTUAL;
				break;
			  case	T_UNIQ:
				attr |= GL_ATTR_UNIQ;
				break;
			  default:
				Error("invalid attribute modifier");
				break;
			}
			GetSymbol;
		}
		if		(  ComToken  ==  ';'  ) {
			GetName;
		} else {
			Error("; missing");
		}
		if		(  !in->fError  ) {
			SetValueAttribute(value,attr);
			ValueAddRecordItem(upper,name,value);
		}
	}
	if		(	(  !in->fError      )
			&&	(	(  ComToken  ==  '}'  )
				||	(  ComToken  ==  ')'  ) ) ) {
		GetSymbol;
		/*	OK	*/
	} else {
		printf("token = %d [%c]\n",ComToken,ComToken);
		Error("syntax error(invalid structure define(s))");
	}
LEAVE_FUNC;
}


extern	void
RecParserInit(void)
{
	LexInit();
	Reserved = MakeReservedTable(tokentable);
	ParsedRec = NewNameHash();
}

extern	ValueStruct	*
RecParseMain(
	CURFILE	*in)
{
	ValueStruct	*ret;
	ValueAttributeType	attr;

ENTER_FUNC;
	SetReserved(in,Reserved); 
	ret = NULL;
	if		(  GetSymbol  ==  T_VIRTUAL  ) {
		attr = GL_ATTR_VIRTUAL;
		GetSymbol;
	} else {
		attr = GL_ATTR_NULL;
	}
	if		(  ComToken  ==  T_SYMBOL  ) {
		if		(  in->ValueName  !=  NULL  ) {
			xfree(in->ValueName);
		}
		in->ValueName = StrDup(ComSymbol);
		if		(  GetSymbol  == '{'  ) {
			ret = NewValue(GL_TYPE_RECORD);
			ValueAttribute(ret) = attr;
			GetName;
			ParValueDefines(in,ret);
			if		(  in->fError  ) {
				Error("syntax error");
				FreeValueStruct(ret);
				ret = NULL;
			}
		} else {
			ret = NULL;
			Error("syntax error");
		}
	} else {
		ret = NULL;
	}
LEAVE_FUNC;
	return	(ret);
}

extern	ValueStruct	*
RecParseValue(
	char	*name,
	char	**ValueName)
{
	ValueStruct	*ret;
	CURFILE		*in
		,		root;
	
ENTER_FUNC;
	root.next = NULL;
	if ( (ret  = g_hash_table_lookup(ParsedRec, name)) ==  NULL  ){
		if		(  ( in = PushLexInfo(&root,name,RecordDir,Reserved) )  !=  NULL  ) {
			ret = RecParseMain(in);
			if (in->ValueName != NULL) {
				if (ValueName != NULL) {
					*ValueName = StrDup(in->ValueName);
				}
				ValueName(ret) = StrDup(in->ValueName);
			}
			DropLexInfo(&in);
			g_hash_table_insert(ParsedRec, StrDup(name), ret);
		} else {
			ret = NULL;
		}
	} else {
		if		(	ValueName !=  NULL ) {
			*ValueName = GetValueName(ret);
		}
	}
	
LEAVE_FUNC;
	return	(ret);
}

extern	ValueStruct	*
RecParseValueMem(
	char	*mem,
	char	**ValueName)
{
	ValueStruct	*ret;
	CURFILE		*in
		,		root;

ENTER_FUNC;
	root.next = NULL;
	if		(  ( in = PushLexInfoMem(&root,mem,RecordDir,Reserved) )  !=  NULL  ) {
		ret = RecParseMain(in);
		if (ret != NULL) {
			if (in->ValueName != NULL) {
				if (ValueName != NULL) {
					*ValueName = StrDup(in->ValueName);
				}
				ValueName(ret) = StrDup(in->ValueName);
			}
			DropLexInfo(&in);
		}
	} else {
		ret = NULL;
	}
LEAVE_FUNC;
	return	(ret);
}
