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

#define	_DD_PARSER

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<glib.h>
#include	"types.h"
#include	"hash.h"
#include	"value.h"
#include	"misc.h"
#include	"monstring.h"
#include	"DDlex.h"
#include	"DDparser.h"
#include	"debug.h"

static	void
SetAttribute(
	ValueStruct	*val,
	ValueAttributeType	attr)
{
	int		i;

	val->attr |= attr;
	switch	(val->type) {
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
		for	( i = 0 ; i < val->body.ArrayData.count ; i ++ ) {
			SetAttribute(val->body.ArrayData.item[i],attr);
		}
		break;
	  case	GL_TYPE_RECORD:
		for	( i = 0 ; i < val->body.RecordData.count ; i ++ ) {
			SetAttribute(val->body.RecordData.item[i],attr);
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
#define	Error(msg)	{fDD_Error=TRUE;_Error((msg),DD_FileName,DD_cLine);}
#define	GetSymbol	(DD_Token = DD_Lex(FALSE))
#define	GetName		(DD_Token = DD_Lex(TRUE))

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
	char		name[SIZE_SYMBOL+1];
	ValueStruct	*value
	,			*array;
	ValueAttributeType	attr;
	int			token;
	ArrayDimension	*next
	,				*curr;

dbgmsg(">ParValueDefine");
	value = NULL;
	while	(  DD_Token  ==  T_SYMBOL  ) {
		attr = GL_ATTR_NULL;
		strcpy(name,DD_ComSymbol);
		switch	(GetSymbol) {
		  case	T_BYTE:
		  case	T_CHAR:
		  case	T_NUMBER:
		  case	T_DBCODE:
		  case	T_VARCHAR:
			token = DD_Token;
			size = 0;
			ssize = 0;
			if		(  GetSymbol  ==  '('  ) {
				if		(  GetSymbol  ==  T_ICONST  ) {
					size = DD_ComInt;
					if		(  token  ==  T_NUMBER  ) {
						switch	(GetSymbol) {
						  case	')':
							ssize = 0;
							break;
						  case	',':
							if		(  GetSymbol  ==  T_ICONST  ) {
								ssize = DD_ComInt;
							} else {
								Error("field size invalud");
							}
							GetSymbol;
							break;
						}
					} else {
						GetSymbol;
					}
					if		(  DD_Token  !=  ')'  ) {
						Error("not closed left paren");
					}
				} else {
					Error("invalid size definition");
				}
				GetSymbol;
			} else {
				size = 1;
			}
			if		(  !fDD_Error  ) {
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
					value->body.FixedData.flen = size;
					value->body.FixedData.slen = ssize;
					value->body.FixedData.sval = (char *)xmalloc(size + 1);
					value->body.FixedData.sval[size] = 0;
					memset(value->body.FixedData.sval,'0',value->body.FixedData.flen);
				} else {
					value->body.CharData.len = size;
					value->body.CharData.sval = (char *)xmalloc(size + 1);
					memclear(value->body.CharData.sval,size + 1);
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
		while	(  DD_Token  ==  '['  ) {	
			curr = New(ArrayDimension);
			if		(  GetSymbol  == T_ICONST  ) {
				curr->count = DD_ComInt;
				GetSymbol;
			} else {
				curr->count = 0;
			}
			curr->next = next;
			next = curr;
			if		(  DD_Token  !=  ']'  ) {
				Error("unmatch lbracket");
				break;
			}
			GetSymbol;
		}
		for	( curr = next ; curr != NULL ; ) {
			array = New(ValueStruct);
			array->type = GL_TYPE_ARRAY;
			array->attr = GL_ATTR_NULL;
			array->body.ArrayData.count = curr->count;
			if		(  curr->count  >  0  ) {
				array->body.ArrayData.item = MakeValueArray(value,curr->count);
			} else {
				array->body.ArrayData.item = MakeValueArray(value,1);
			}
			next = curr->next;
			xfree(curr);
			curr = next;
			value = array;
		}
		while	(  DD_Token  ==  ','  ) {
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
		if		(  DD_Token  ==  ';'  ) {
			GetSymbol;
			/*	OK	*/
		} else {
			Error("; missing");
		}
		if		(  !fDD_Error  ) {
			value->attr = GL_ATTR_NULL;
			SetAttribute(value,attr);
			ValueAddRecordItem(upper,name,value);
		}
	}
	if		(	(  !fDD_Error         )
			 &&	(  DD_Token  ==  '}'  ) ) {
		GetSymbol;
		/*	OK	*/
	} else {
		printf("token = %d [%c]\n",DD_Token,DD_Token);
		Error("syntax error");
	}
	if		(  value  ==  NULL  ) {
		value = NewValue(GL_TYPE_INT);
		value->attr = GL_ATTR_NULL;
		ValueAddRecordItem(upper,NULL,value);
	}
dbgmsg("<ParValueDefine");
}

extern	void
DD_ParserInit(void)
{
	DD_LexInit();
	ValueName = (char *)xmalloc(1);;
}

extern	ValueStruct	*
DD_ParseMain(
	FILE	*fp,
	char	*name)
{
	ValueStruct	*ret;

dbgmsg(">DD_ParseMain");
	DD_FileName = StrDup(name);
	DD_cLine = 1;
	DD_File = fp;
	ret = NULL;

	if		(  GetSymbol  ==  T_SYMBOL  ) {
		xfree(ValueName);
		ValueName = StrDup(DD_ComSymbol);
		if		(  GetSymbol  == '{'  ) {
			ret = NewValue(GL_TYPE_RECORD);
			ret->attr = GL_ATTR_NULL;
			GetName;
			ParValueDefine(ret);
			if		(  fDD_Error  ) {
				ret = NULL;
			}
		} else {
			ret = NULL;
			Error("syntax error");
		}
	} else {
		ret = NULL;
	}
	xfree(DD_FileName);
dbgmsg("<DD_ParseMain");
	return	(ret);
}

extern	ValueStruct	*
DD_ParseValue(
	char	*name)
{
	FILE	*fp;
	ValueStruct	*ret;

dbgmsg(">DD_ParseValue");
	fDD_Error = FALSE;
	if		(  ( fp = fopen(name,"r") )  !=  NULL  ) {
		ret = DD_ParseMain(fp,name);
		fclose(fp);
	} else {
		ret = NULL;
	}
dbgmsg("<DD_ParseValue");
	return	(ret);
}
