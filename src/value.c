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
#include	<math.h>

#define	__VALUE_DIRECT
#include	"types.h"
#include	"misc.h"
#include	"monstring.h"
#include	"value.h"
#include	"memory.h"
#include	"getset.h"
#include	"hash.h"
#include	"debug.h"

#define	DUMP_LOCALE		"euc-jp"

extern	ValueStruct	*
NewValue(
	PacketDataType	type)
{
	ValueStruct	*ret;

dbgmsg(">NewValue");
	ret = New(ValueStruct);
	ValueAttribute(ret) = GL_ATTR_NULL;
	ValueStr(ret) = NULL;
	ValueType(ret) = type;
	switch	(type) {
	  case	GL_TYPE_BYTE:
		ValueByteLength(ret) = 0;
		ValueByte(ret) = NULL;
		break;
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		ValueStringLength(ret) = 0;
		ValueStringSize(ret) = 0;
		ValueString(ret) = NULL;
		break;
	  case	GL_TYPE_NUMBER:
		ValueFixedLength(ret) = 0;
		ValueFixedSlen(ret) = 0;
		ValueFixedBody(ret) = NULL;
		break;
	  case	GL_TYPE_INT:
		ValueInteger(ret) = 0;
		break;
	  case	GL_TYPE_FLOAT:
		ValueFloat(ret) = 0.0;
		break;
	  case	GL_TYPE_BOOL:
		ValueBool(ret) = FALSE;
		break;
	  case	GL_TYPE_OBJECT:
		ValueObjectPlace(ret) = 0;
		ValueObjectID(ret) = 0;
		break;
	  case	GL_TYPE_RECORD:
		ValueRecordSize(ret) = 0;
		ValueRecordMembers(ret) = NewNameHash();
		ValueRecordItems(ret) = NULL;
		ValueRecordNames(ret) = NULL;
		break;
	  case	GL_TYPE_ARRAY:
		ValueArraySize(ret) = 0;
		ValueArrayItems(ret) = NULL;
		break;
	  case	GL_TYPE_ALIAS:
		ValueAliasName(ret) = NULL;
		break;
	  default:
		xfree(ret);
		ret = NULL;
		break;
	}
dbgmsg("<NewValue");
	return	(ret);
}

extern	void
FreeValueStruct(
	ValueStruct	*val)
{
	int		i;

	if		(  val  !=  NULL  ) {
		switch	(val->type) {
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(val) ; i ++ ) {
				FreeValueStruct(ValueArrayItem(val,i));
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(val) ; i ++ ) {
				FreeValueStruct(ValueRecordItem(val,i));
			}
			break;
		  case	GL_TYPE_BYTE:
			if		(  ValueByte(val)  !=  NULL  ) {
				xfree(ValueByte(val));
			}
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
			if		(  ValueString(val)  !=  NULL  ) {
				xfree(ValueString(val));
			}
			break;
		  case	GL_TYPE_NUMBER:
			if		(  ValueFixedBody(val)  !=  NULL  ) {
				xfree(ValueFixedBody(val));
			}
			break;
		  case	GL_TYPE_ALIAS:
		  default:
			break;
		}
		if		(  ValueStr(val)  !=  NULL  ) {
			FreeLBS(ValueStr(val));
		}
		xfree(val);
	}
}

extern	void
FreeFixed(
	Fixed	*xval)
{
	if		(  xval->sval  !=  NULL  ) {
		xfree(xval->sval);
	}
	xfree(xval);
}

extern	Fixed	*
NewFixed(
	int		flen,
	int		slen)
{
	Fixed	*xval;

	xval = New(Fixed);
	xval->flen = flen;
	xval->slen = slen;
	if		(  flen  >  0  ) {
		xval->sval = (char *)xmalloc(flen+1);
	}
	return	(xval);
}

extern	ValueStruct	*
GetRecordItem(
	ValueStruct	*value,
	char		*name)
{	
	gpointer	p;
	ValueStruct	*item;

	if		(  value->type  ==  GL_TYPE_RECORD  ) {
		if		(  ( p = g_hash_table_lookup(ValueRecordMembers(value),name) )
				   ==  NULL  ) {
			item = NULL;
		} else {
			item = ValueRecordItem(value,(int)p-1);
		}
	} else {
		item = NULL;
	}
	return	(item);
}

extern	ValueStruct	*
GetArrayItem(
	ValueStruct	*value,
	int			i)
{
	ValueStruct	*item;
	
	if		(	(  i  >=  0                      )
			&&	(  i  <   ValueArraySize(value)  ) )	{
		item = ValueArrayItem(value,i);
	} else {
		item = NULL;
	}
	return	(item);
}

extern	void
FloatToFixed(
	Fixed	*xval,
	double	fval)
{
	char	str[SIZE_NUMBUF+1];
	char	*p
	,		*q;
	Bool	fMinus;

	if		(  fval  <  0  ) {
		fval = - fval;
		fMinus = TRUE;
	} else {
		fMinus = FALSE;
	}
	sprintf(str, "%0*.*f", (int)xval->flen+1, (int)xval->slen, fval);
	p = str;
	q = xval->sval;
	while	(  *p  !=  0  ) {
		if		( *p  !=  '.'  ) {
			*q = *p;
			q ++;
		}
		p ++;
	}
	if		(  fMinus  ) {
		*xval->sval |= 0x40;
	}
}

extern	void
IntToFixed(
	Fixed	*xval,
	int		ival)
{
	char	str[SIZE_NUMBUF+1];
	Bool	fMinus;

	if		(  ival  <  0  ) {
		ival = - ival;
		fMinus = TRUE;
	} else {
		fMinus = FALSE;
	}
	sprintf(str, "%d",ival);
	xval->sval = StrDup(str);
	xval->flen = strlen(str);
	xval->slen = 0;
	if		(  fMinus  ) {
		*xval->sval |= 0x40;
	}
}

extern	int
FixedToInt(
	Fixed	*xval)
{
	int		ival;
	int		i;

	ival = StrToInt(xval->sval,strlen(xval->sval));
	for	( i = 0 ; i < xval->slen ; i ++ ) {
		ival /= 10;
	}
	return	(ival);
}

extern	double
FixedToFloat(
	Fixed	*xval)
{
	double	fval;
	int		i;
	Bool	fMinus;

	if		(  *xval->sval  >=  0x70  ) {
		*xval->sval ^= 0x40;
		fMinus = TRUE;
	} else {
		fMinus = FALSE;
	}
	fval = atof(xval->sval);
	for	( i = 0 ; i < xval->slen ; i ++ ) {
		fval /= 10.0;
	}
	if		(  fMinus  ) {
		fval = - fval;
	}
	return	(fval);
}

extern	ValueStruct	*
GetItemLongName(
	ValueStruct		*root,
	char			*longname)
{
	char	item[SIZE_NAME+1]
	,		*p
	,		*q;
	int		n;
	ValueStruct	*val;

dbgmsg(">GetItemLongName");
	if		(  root  ==  NULL  ) { 
		printf("no root ValueStruct [%s]\n",longname);
		return	(FALSE);
	}
	p = longname; 
	val = root;
	while	(  *p  !=  0  ) {
		q = item;
		if		(  *p  ==  '['  ) {
			p ++;
			while	(  isdigit(*p)  ) {
				*q ++ = *p ++;
			}
			if		(  *p  !=  ']'  ) {
				/*	fatal error	*/
			}
			*q = 0;
			p ++;
			n = StrToInt(item,strlen(item));
			val = GetArrayItem(val,n);
		} else {
			while	(	(  *p  !=  0    )
					&&	(  *p  !=  '.'  )
					&&	(  *p  !=  '['  ) ) {
				*q ++ = *p ++;
			}
			*q = 0;
			if		(  *item  !=  0  ) {
				val = GetRecordItem(val,item);
			}
		}
		if		(  *p   ==  '.'   )	p ++;
		if		(  val  ==  NULL  )	{
			printf("no ValueStruct [%s]\n",longname);
			break;
		}
	}
dbgmsg("<GetItemLongName");
	return	(val); 
}

static	void
DumpItem(
	char		*name,
	ValueStruct	*value)
{
	printf("%s:",name);
	printf("%s",((value->attr&GL_ATTR_INPUT) == GL_ATTR_INPUT) ? "I" : "O");
	printf("%s",((value->attr&GL_ATTR_ALIAS) == GL_ATTR_ALIAS) ? " ALIAS" : "");
	printf("%s",((value->attr&GL_ATTR_NIL) == GL_ATTR_NIL) ? " NIL:" : ":");
	DumpValueStruct(value);
}

extern	void
DumpValueStruct(
	ValueStruct	*val)
{
	int		i;

	if		(  val  ==  NULL  )	{
		printf("null value\n");
	} else {
		switch	(ValueType(val)) {
		  case	GL_TYPE_INT:
			printf("integer[%d]\n",ValueInteger(val));
			fflush(stdout);
			break;
		  case	GL_TYPE_FLOAT:
			printf("float[%g]\n",ValueFloat(val));
			fflush(stdout);
			break;
		  case	GL_TYPE_BOOL:
			printf("Bool[%s]\n",(ValueBool(val) ? "T" : "F"));
			fflush(stdout);
			break;
		  case	GL_TYPE_CHAR:
			printf("char(%d,%d) [",ValueStringSize(val),ValueStringLength(val));
			if		(  !IS_VALUE_NIL(val)  ) {
				PrintFixString(ValueToString(val,DUMP_LOCALE),ValueStringLength(val));
			}
			printf("]\n");
			fflush(stdout);
			break;
		  case	GL_TYPE_VARCHAR:
			printf("varchar(%d,%d)",ValueStringSize(val),ValueStringLength(val));
			if		(  !IS_VALUE_NIL(val)  ) {
				printf(" [%s]",ValueToString(val,DUMP_LOCALE));
			}
			printf("\n");
			fflush(stdout);
			break;
		  case	GL_TYPE_DBCODE:
			printf("code(%d,%d) [%s]\n",ValueStringSize(val),ValueStringLength(val),
				   ValueString(val));
			fflush(stdout);
			break;
		  case	GL_TYPE_NUMBER:
			printf("number(%d,%d) [%s]\n",ValueFixedLength(val),
				   ValueFixedSlen(val),
				   ValueFixedBody(val));
			fflush(stdout);
			break;
		  case	GL_TYPE_TEXT:
			printf("text(%d,%d)",ValueStringSize(val),ValueStringLength(val));
			fflush(stdout);
			if		(  !IS_VALUE_NIL(val)  ) {
				printf(" [%s]\n",ValueStringPointer(val));
				printf(" [%s]\n",ValueToString(val,DUMP_LOCALE));
			}
			fflush(stdout);
			break;
		  case	GL_TYPE_OBJECT:
			printf("object [%d:%d]\n",ValueObjectPlace(val), ValueObjectID(val));
			fflush(stdout);
			break;
		  case	GL_TYPE_ARRAY:
			printf("array size = %d\n",ValueArraySize(val));
			fflush(stdout);
			for	( i = 0 ; i < ValueArraySize(val) ; i ++ ) {
				DumpValueStruct(ValueArrayItem(val,i));
			}
			break;
		  case	GL_TYPE_RECORD:
			printf("record members = %d\n",ValueRecordSize(val));
			fflush(stdout);
			for	( i = 0 ; i < ValueRecordSize(val) ; i ++ ) {
				DumpItem(ValueRecordName(val,i),ValueRecordItem(val,i));
			}
			printf("--\n");
			break;
		  case	GL_TYPE_ALIAS:
			printf("alias name = [%s]\n",ValueAliasName(val));
			fflush(stdout);
			break;
		  default:
			break;
		}
	}
}

extern	void
InitializeValue(
	ValueStruct	*value)
{
	int		i;

dbgmsg(">InitializeValue");
	if		(  value  ==  NULL  )	return;
	if		(  ValueStr(value)  !=  NULL  ) {
		FreeLBS(ValueStr(value));
	}
	ValueStr(value) = NULL;
	switch	(ValueType(value)) {
	  case	GL_TYPE_INT:
		ValueInteger(value) = 0;
		break;
	  case	GL_TYPE_FLOAT:
		ValueFloat(value) = 0.0;
		break;
	  case	GL_TYPE_BOOL:
		ValueBool(value) = FALSE;
		break;
	  case	GL_TYPE_OBJECT:
		ValueObjectPlace(value) = 0;
		ValueObjectID(value) = 0;
		break;
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
		memclear(ValueString(value),ValueStringSize(value));
		break;
	  case	GL_TYPE_TEXT:
		if		(  ValueString(value)  !=  NULL  ) {
			xfree(ValueString(value));
		}
		ValueString(value) = NULL;
		ValueStringLength(value) = 0;
		ValueStringSize(value) = 0;
		break;
	  case	GL_TYPE_NUMBER:
		if		(  ValueFixedLength(value)  >  0  ) {
			memset(ValueFixedBody(value),'0',ValueFixedLength(value));
		}
		ValueFixedBody(value)[ValueFixedLength(value)] = 0;
		break;
	  case	GL_TYPE_ARRAY:
		for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
			InitializeValue(ValueArrayItem(value,i));
		}
		break;
	  case	GL_TYPE_RECORD:
		for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
			InitializeValue(ValueRecordItem(value,i));
		}
		break;
	  case	GL_TYPE_ALIAS:
	  default:
		break;
	}
dbgmsg("<InitializeValue");
}

/*
 *	moves simple data only
 */
extern	void
MoveValue(
	ValueStruct	*to,
	ValueStruct	*from)
{
	Fixed	*xval;

ENTER_FUNC;
	switch	(ValueType(to)) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		if		(  ValueStringSize(from)  >  ValueStringSize(to)  ) {
			if		(  ValueString(to)  !=  NULL  ) {
				xfree(ValueString(to));
			}
			ValueStringSize(to) = ValueStringSize(from);
			ValueString(to) = (char *)xmalloc(ValueStringSize(to));
		}
		memclear(ValueString(to),ValueStringSize(to));
		memcpy(ValueString(to),ValueString(from),ValueStringSize(to));
		if		(  ValueType(to)  ==  GL_TYPE_TEXT  ) {
			ValueStringLength(to) = ValueStringLength(from);
		}
		break;
	  case	GL_TYPE_NUMBER:
		xval = ValueToFixed(from);
		SetValueFixed(to,xval);
		FreeFixed(xval);
		break;
	  case	GL_TYPE_INT:
		SetValueInteger(to,ValueToInteger(from));
		break;
	  case	GL_TYPE_FLOAT:
		SetValueFloat(to,ValueToFloat(from));
		break;
	  case	GL_TYPE_BOOL:
		SetValueBool(to,ValueToBool(from));
		break;
	  case	GL_TYPE_ALIAS:
	  default:
		break;
	}
LEAVE_FUNC;
}

/*
 *	copies same structure
 */
extern	void
CopyValue(
	ValueStruct	*vd,
	ValueStruct	*vs)
{
	int		i;

ENTER_FUNC;
	if		(  vd  ==  NULL  )	return;
	if		(  vs  ==  NULL  )	return;
	switch	(vs->type) {
	  case	GL_TYPE_INT:
		ValueInteger(vd) = ValueInteger(vs);
		break;
	  case	GL_TYPE_FLOAT:
		ValueFloat(vd) = ValueFloat(vs);
		break;
	  case	GL_TYPE_BOOL:
		ValueBool(vd) = ValueBool(vs);
		break;
	  case	GL_TYPE_OBJECT:
		ValueObjectPlace(vd) = ValueObjectPlace(vs);
		ValueObjectID(vd) = ValueObjectID(vs);
		break;
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
		memcpy(ValueString(vd),ValueString(vs),ValueStringSize(vd));
		break;
	  case	GL_TYPE_NUMBER:
		if		(  ValueFixedLength(vd)  >  0  ) {
			memcpy(ValueFixedBody(vd),ValueFixedBody(vs),ValueFixedLength(vs));
		}
		ValueFixedBody(vd)[ValueFixedLength(vd)] = 0;
		ValueFixedLength(vd) = ValueFixedLength(vs);
		break;
	  case	GL_TYPE_ARRAY:
		for	( i = 0 ; i < ValueArraySize(vs) ; i ++ ) {
			CopyValue(ValueArrayItem(vd,i),ValueArrayItem(vs,i));
		}
		break;
	  case	GL_TYPE_RECORD:
		for	( i = 0 ; i < ValueRecordSize(vs) ; i ++ ) {
			CopyValue(ValueRecordItem(vd,i),ValueRecordItem(vs,i));
		}
		ValueRecordMembers(vd) = ValueRecordMembers(vs);
		ValueRecordNames(vd) = ValueRecordNames(vs);
		break;
	  case	GL_TYPE_ALIAS:
	  default:
		break;
	}
LEAVE_FUNC;
}

extern	ValueStruct	**
MakeValueArray(
	ValueStruct	*template,
	size_t		count)
{
	ValueStruct	**ret;
	int			i;

	ret = (ValueStruct **)xmalloc(sizeof(ValueStruct *) * count);
	for	( i = 0 ; i < count ; i ++ ) {
		ret[i] = DuplicateValue(template);
	}
	return	(ret);
}

extern	ValueStruct	*
DuplicateValue(
	ValueStruct	*template)
{
	ValueStruct	*p;
	int			i;

	p = New(ValueStruct);
	ValueType(p) = ValueType(template);
	ValueAttribute(p) = ValueAttribute(template);
	ValueStr(p) = NULL;

	switch	(ValueType(template)) {
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		if		(  ValueStringSize(template)  >  0  ) {
			ValueString(p) = (char *)xmalloc(ValueStringSize(template));
			memclear(ValueString(p),ValueStringSize(template));
		}
		ValueStringLength(p) = ValueStringLength(template);
		ValueStringSize(p) = ValueStringSize(template);
		break;
	  case	GL_TYPE_NUMBER:
		ValueFixedBody(p) = (char *)xmalloc(ValueFixedLength(template)+1);
		memcpy(ValueFixedBody(p),
			   ValueFixedBody(template),
			   ValueFixedLength(template)+1);
		ValueFixedLength(p) = ValueFixedLength(template);
		ValueFixedSlen(p) = ValueFixedSlen(template);
		break;
	  case	GL_TYPE_ARRAY:
		ValueArrayItems(p) = 
			MakeValueArray(ValueArrayItem(template,0),
						   ValueArraySize(template));
		ValueArraySize(p) = ValueArraySize(template);
		break;
	  case	GL_TYPE_INT:
		ValueInteger(p) = 0;
		break;
	  case	GL_TYPE_BOOL:
		ValueBool(p) = FALSE;
		break;
	  case	GL_TYPE_OBJECT:
		ValueObjectPlace(p) = 0;
		ValueObjectID(p) = 0;
		break;
	  case	GL_TYPE_RECORD:
		/*	share name table		*/
		ValueRecordMembers(p) = ValueRecordMembers(template);
		ValueRecordNames(p) = ValueRecordNames(template);
		/*	duplicate data space	*/
		ValueRecordItems(p) = 
			(ValueStruct **)xmalloc(sizeof(ValueStruct *) * ValueRecordSize(template));
		ValueRecordSize(p) = ValueRecordSize(template);
		for	( i = 0 ; i < ValueRecordSize(template) ; i ++ ) {
			ValueRecordItem(p,i) = 
				DuplicateValue(ValueRecordItem(template,i));
		}
		break;
	  case	GL_TYPE_ALIAS:
	  default:
		break;
	}
	return	(p);
}

extern	void
ValueAddRecordItem(
	ValueStruct	*upper,
	char		*name,
	ValueStruct	*value)
{
	ValueStruct	**items;
	char		**names;
	char		*dname;
	size_t		nsize;

dbgmsg(">ValueAddRecordItem");
#ifdef	TRACE
	printf("name = [%s]\n",name); 
#endif
	nsize = ValueRecordSize(upper) + 1;
	items = (ValueStruct **)
		xmalloc(sizeof(ValueStruct *) * nsize);
	names = (char **)
		xmalloc(sizeof(char *) * nsize);
	if		(  ValueRecordSize(upper)  >  0  ) {
		memcpy(items, ValueRecordItems(upper), 
			   sizeof(ValueStruct *) * ValueRecordSize(upper) );
		memcpy(names, ValueRecordNames(upper),
			   sizeof(char *) * ValueRecordSize(upper) );
		xfree(ValueRecordItems(upper));
		xfree(ValueRecordNames(upper));
	}
	items[ValueRecordSize(upper)] = value;
	dname = StrDup(name);
	names[ValueRecordSize(upper)] = dname;
	if		(  name  !=  NULL  ) {
		if		(  g_hash_table_lookup(ValueRecordMembers(upper),name)  ==  NULL  ) {
			g_hash_table_insert(ValueRecordMembers(upper),
								(gpointer)dname,
								(gpointer)(ValueRecordSize(upper)+1));
		} else {
			printf("name = [%s]\t",name);
			Error("name duplicate");
		}
	}
	ValueRecordItems(upper) = items;
	ValueRecordNames(upper) = names;
	ValueRecordSize(upper) = nsize;
dbgmsg("<ValueAddRecordItem");
}

