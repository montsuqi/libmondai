/*	PANDA -- a simple transaction monitor

Copyright (C) 2000-2003 Ogochan & JMA (Japan Medical Association).

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


#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<glib.h>

#define	_DD_PARSER
#define	__VALUE_DIRECT
#include	"types.h"
#include	"hash.h"
#include	"value.h"
#include	"misc.h"
#include	"monstring.h"
#include	"memory.h"
#include	"others.h"
#include	"Lex.h"
#include	"DDparser.h"
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
#define	T_VIRTUAL		(T_YYBASE +13)
#define	T_ALIAS			(T_YYBASE +14)

static	TokenTable	tokentable[] = {
	{	"bool"		,T_BOOL		},
	{	"byte"		,T_BYTE		},
	{	"char"		,T_CHAR		},
	{	"varchar"	,T_VARCHAR	},
	{	"float"		,T_FLOAT	},
	{	"input"		,T_INPUT	},
	{	"int"		,T_INT		},
	{	"number"	,T_NUMBER	},
	{	"output"	,T_OUTPUT	},
	{	"text"		,T_TEXT		},
	{	"object"	,T_OBJECT	},
	{	"dbcode"	,T_DBCODE	},
	{	"virtual"	,T_VIRTUAL	},
	{	"alias"		,T_ALIAS	},
	{	""			,0	}
};

static	GHashTable	*Reserved;

static	void
SetAttribute(
	ValueStruct	*val,
	ValueAttributeType	attr)
{
	int		i;

	val->attr |= attr;
	switch	(ValueType(val)) {
	  case	GL_TYPE_NUMBER:
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_INT:
	  case	GL_TYPE_BOOL:
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_OBJECT:
		break;
	  case	GL_TYPE_ARRAY:
		for	( i = 0 ; i < ValueArraySize(val) ; i ++ ) {
			SetAttribute(ValueArrayItem(val,i),attr);
		}
		break;
	  case	GL_TYPE_RECORD:
		for	( i = 0 ; i < ValueRecordSize(val) ; i ++ ) {
			SetAttribute(ValueRecordItem(val,i),attr);
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
#define	Error(msg)		{CURR->fError=TRUE;_Error((msg),CURR->fn,CURR->cLine);}

typedef	struct _ArrayDimension	{
	int		count;
	struct	_ArrayDimension	*next;
}	ArrayDimension;

static	void
ParValueDefine(
	ValueStruct	*upper)
{
	size_t		size
	,			ssize;
	char		name[SIZE_SYMBOL+1]
	,			buff[SIZE_LONGNAME+1];
	char		*p;
	ValueStruct	*value
	,			*array;
	ValueAttributeType	attr;
	int			token;
	ArrayDimension	*next
	,				*curr;

dbgmsg(">ParValueDefine");
	value = NULL;
	while	(  ComToken  ==  T_SYMBOL  ) {
		attr = ValueAttribute(upper);
		strcpy(name,ComSymbol);
		switch	(GetSymbol) {
		  case	T_ALIAS:
		  case	'=':
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
						p += sprintf(p,"[%d]",size);
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
			token = ComToken;
			size = 0;
			ssize = 0;
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
				size = 1;
			}
			if		(  !CURR->fError  ) {
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
				  default:
					break;
				}
				if		(  value->type  ==  GL_TYPE_NUMBER  ) {
					ValueFixedLength(value) = size;
					ValueFixedSlen(value) = ssize;
					ValueFixedBody(value) = (char *)xmalloc(size + 1);
					ValueFixedBody(value)[size] = 0;
					memset(ValueFixedBody(value),'0',size);
				} else {
					ValueStringLength(value) = size;
					ValueStringSize(value) = size+1;
					ValueString(value) = (char *)xmalloc(ValueStringSize(value));
					memclear(ValueString(value),ValueStringSize(value));
				}
			}
			break;
		  case	T_TEXT:
			value = NewValue(GL_TYPE_TEXT);
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
		  case	T_BOOL:
			value = NewValue(GL_TYPE_BOOL);
			GetSymbol;
			break;
		  case	T_OBJECT:
			value = NewValue(GL_TYPE_OBJECT);
			GetSymbol;
			break;
		  case	'{':
			value = NewValue(GL_TYPE_RECORD);
			GetName;
			ParValueDefine(value);
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
				GetSymbol;
			} else {
				curr->count = 0;
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
			if		(  curr->count  >  0  ) {
				ValueArrayItems(array) = MakeValueArray(value,curr->count);
			} else {
				ValueArrayItems(array) = MakeValueArray(value,1);
			}
			next = curr->next;
			xfree(curr);
			curr = next;
			value = array;
		}
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
			  default:
				Error("invalit attribute modifier");
				break;
			}
			GetSymbol;
		}
		if		(  ComToken  ==  ';'  ) {
			GetSymbol;
			/*	OK	*/
		} else {
			Error("; missing");
		}
		if		(  !CURR->fError  ) {
			ValueAttribute(value) = GL_ATTR_NULL;
			SetAttribute(value,attr);
			ValueAddRecordItem(upper,name,value);
		}
	}
	if		(	(  !CURR->fError      )
			 &&	(  ComToken  ==  '}'  ) ) {
		GetSymbol;
		/*	OK	*/
	} else {
		printf("token = %d [%c]\n",ComToken,ComToken);
		Error("syntax error");
	}
#if	0
	if		(  value  ==  NULL  ) {
		value = NewValue(GL_TYPE_INT);
		ValueAttribute(value) = ValueAttribute(upper);
		ValueAddRecordItem(upper,NULL,value);
	}
#endif
dbgmsg("<ParValueDefine");
}


extern	void
DD_ParserInit(void)
{
	LexInit();
	Reserved = MakeReservedTable(tokentable);
	ValueName = (char *)xmalloc(1);
}

extern	ValueStruct	*
DD_ParseMain(void)
{
	ValueStruct	*ret;
	ValueAttributeType	attr;

dbgmsg(">DD_ParseMain");
	SetReserved(Reserved); 
	ret = NULL;
	if		(  GetSymbol  ==  T_VIRTUAL  ) {
		attr = GL_ATTR_VIRTUAL;
		GetSymbol;
	} else {
		attr = GL_ATTR_NULL;
	}
	if		(  ComToken  ==  T_SYMBOL  ) {
		xfree(ValueName);
		ValueName = StrDup(ComSymbol);
		if		(  GetSymbol  == '{'  ) {
			ret = NewValue(GL_TYPE_RECORD);
			ValueAttribute(ret) = attr;
			GetName;
			ParValueDefine(ret);
			if		(  CURR->fError  ) {
				ret = NULL;
			}
		} else {
			ret = NULL;
			Error("syntax error");
		}
	} else {
		ret = NULL;
	}
dbgmsg("<DD_ParseMain");
	return	(ret);
}

extern	ValueStruct	*
DD_ParseValue(
	char	*name)
{
	ValueStruct	*ret;

dbgmsg(">DD_ParseValue");
	if		(  PushLexInfo(name,RecordDir,Reserved)  !=  NULL  ) {
		ret = DD_ParseMain();
		DropLexInfo();
	} else {
		ret = NULL;
	}
dbgmsg("<DD_ParseValue");
	return	(ret);
}
