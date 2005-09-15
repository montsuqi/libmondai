/*
libmondai -- MONTSUQI data access library
Copyright (C) 2000-2004 Ogochan & JMA (Japan Medical Association).
Copyright (C) 2005 Ogochan.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
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
#include	"misc_v.h"
#include	"monstring.h"
#include	"numeric.h"
#include	"numerici.h"
#include	"value.h"
#include	"memory_v.h"
#include	"getset.h"
#include	"hash_v.h"
#include	"debug.h"

#define	DUMP_LOCALE		"euc-jp"

extern	ValueStruct	*
NewValue(
	PacketDataType	type)
{
	ValueStruct	*ret;

ENTER_FUNC;
	ret = New(ValueStruct);
	dbgprintf("value = %p\n",ret);
	ValueAttribute(ret) = GL_ATTR_NIL;
	ValueStr(ret) = NULL;
	ValueType(ret) = type;
	switch	(type) {
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_BINARY:
		ValueByteLength(ret) = 0;
		ValueByteSize(ret) = 0;
		ValueByte(ret) = NULL;
		break;
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_SYMBOL:
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
		ValueObject(ret) = 0;
		break;
	  case	GL_TYPE_RECORD:
		ValueRecordSize(ret) = 0;
		ValueRecordMembers(ret) = NewNameHash();
		ValueRecordItems(ret) = NULL;
		ValueRecordNames(ret) = NULL;
		ValueAttribute(ret) = GL_ATTR_NULL;
		break;
	  case	GL_TYPE_ARRAY:
		ValueArraySize(ret) = 0;
		ValueArrayExpandable(ret) = FALSE;
		ValueArrayPrototype(ret) = NULL;
		ValueArrayItems(ret) = NULL;
		ValueAttribute(ret) = GL_ATTR_NULL;
		break;
	  case	GL_TYPE_ALIAS:
		ValueAliasName(ret) = NULL;
		ValueAttribute(ret) = GL_ATTR_NULL;
		break;
	  case	GL_TYPE_VALUES:
		ValueValuesSize(ret) = 0;
		ValueValuesItems(ret) = NULL;
		ValueAttribute(ret) = GL_ATTR_NULL;
		break;
	  case	GL_TYPE_POINTER:
		ValuePointer(ret) = NULL;
		break;
	  default:
		xfree(ret);
		ret = NULL;
		break;
	}
LEAVE_FUNC;
	return	(ret);
}

extern	void
FreeValueStruct(
	ValueStruct	*val)
{
	int		i;

	if		(  val  !=  NULL  ) {
		dbgprintf("type = %02X\n",val->type);
		switch	(val->type) {
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(val) ; i ++ ) {
				FreeValueStruct(ValueArrayItem(val,i));
			}
			xfree(ValueArrayItems(val));
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(val) ; i ++ ) {
				FreeValueStruct(ValueRecordItem(val,i));
				xfree(ValueRecordName(val,i));
			}
			g_hash_table_destroy(ValueRecordMembers(val));
			xfree(ValueRecordNames(val));
			xfree(ValueRecordItems(val));
			break;
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BINARY:
			if		(  ValueByte(val)  !=  NULL  ) {
				xfree(ValueByte(val));
			}
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_SYMBOL:
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

extern	ValueStruct	*
GetRecordItem(
	ValueStruct	*value,
	char		*name)
{	
	gpointer	p;
	ValueStruct	*item;

ENTER_FUNC;
	if		(  value  !=  NULL  ) {
		if		(  ValueType(value)  ==  GL_TYPE_RECORD  ) {
			if		(  ( p = g_hash_table_lookup(ValueRecordMembers(value),name) )
					   ==  NULL  ) {
				item = NULL;
			} else {
				item = ValueRecordItem(value,(int)p-1);
			}
		} else {
			item = NULL;
		}
	} else {
		item = NULL;
	}
LEAVE_FUNC;
	return	(item);
}

extern	ValueStruct	*
GetArrayItem(
	ValueStruct	*value,
	int			index)
{
	ValueStruct	*item
		,		**items;
	size_t		size;
	int			i;

ENTER_FUNC;	
	if		(	(  index  >=  0                      )
			&&	(  index  <   ValueArraySize(value)  ) )	{
		item = ValueArrayItem(value,index);
	} else {
		if		(  ValueArrayExpandable(value)  ) {
			size = index + 1;
			items = (ValueStruct **)xmalloc(sizeof(ValueStruct *) * size);
			memclear(items,sizeof(ValueStruct *) * size);
			if		(  ValueArrayItems(value)  !=  NULL  ) {
				memcpy(items,ValueArrayItems(value),
					   ValueArraySize(value)*sizeof(ValueStruct *));
				xfree(ValueArrayItems(value));
			}
			for	( i = ValueArraySize(value) ; i < size ; i ++ ) {
				items[i] = DuplicateValue(ValueArrayPrototype(value));
			}
			ValueArrayItems(value) = items;
			ValueArraySize(value) = size;
			item = ValueArrayItem(value,index);
		} else {
			item = NULL;
		}
	}
LEAVE_FUNC;
	return	(item);
}

extern	ValueStruct	*
GetValuesItem(
	ValueStruct	*value,
	int			i)
{
	ValueStruct	*item;
	
	if		(	(  i  >=  0                      )
			&&	(  i  <   ValueValuesSize(value)  ) )	{
		item = ValueValuesItem(value,i);
	} else {
		item = NULL;
	}
	return	(item);
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

ENTER_FUNC;
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
			break;
		}
	}
LEAVE_FUNC;
	return	(val); 
}

static	void
DumpItem(
	char		*name,
	ValueStruct	*value)
{
	if		(  name  !=  NULL  ) {
		printf("%s:",name);
	} else {
		printf(":");
	}
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
			printf("char(%d,%d) [",ValueStringLength(val),ValueStringSize(val));
			if		(  !IS_VALUE_NIL(val)  ) {
				PrintFixString(ValueToString(val,DUMP_LOCALE),ValueStringLength(val));
			}
			printf("]\n");
			fflush(stdout);
			break;
		  case	GL_TYPE_VARCHAR:
			printf("varchar(%d,%d)",ValueStringLength(val),ValueStringSize(val));
			if		(  !IS_VALUE_NIL(val)  ) {
				printf(" [%s]",ValueToString(val,DUMP_LOCALE));
			}
			printf("\n");
			fflush(stdout);
			break;
		  case	GL_TYPE_DBCODE:
			printf("code(%d,%d) [%s]\n",ValueStringLength(val),ValueStringSize(val),
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
			printf("text(%d,%d)",ValueStringLength(val),ValueStringSize(val));
			fflush(stdout);
			if		(  !IS_VALUE_NIL(val)  ) {
				printf(" [%s]\n",ValueStringPointer(val));
				printf(" [%s]\n",ValueToString(val,DUMP_LOCALE));
			}
			fflush(stdout);
			break;
		  case	GL_TYPE_SYMBOL:
			printf("symbol(%d,%d)",ValueStringLength(val),ValueStringSize(val));
			fflush(stdout);
			if		(  !IS_VALUE_NIL(val)  ) {
				printf(" [%s]\n",ValueStringPointer(val));
				printf(" [%s]\n",ValueToString(val,DUMP_LOCALE));
			}
			fflush(stdout);
			break;
		  case	GL_TYPE_BINARY:
			printf("binary(%d,%d)",ValueByteLength(val),ValueByteSize(val));
			fflush(stdout);
			if		(  !IS_VALUE_NIL(val)  ) {
				printf(" [%s]\n",ValueToString(val,DUMP_LOCALE));
			}
			fflush(stdout);
			break;
		  case	GL_TYPE_OBJECT:
			printf("object [%lld]\n",ValueObject(val));
			fflush(stdout);
			break;
		  case	GL_TYPE_ARRAY:
			printf("array size = %d(%s)\n",ValueArraySize(val),
				   ValueArrayExpandable(val) ? "expandable" : "fixed");
			fflush(stdout);
			for	( i = 0 ; i < ValueArraySize(val) ; i ++ ) {
				DumpValueStruct(ValueArrayItem(val,i));
			}
			break;
		  case	GL_TYPE_VALUES:
			printf("values size = %d\n",ValueValuesSize(val));
			fflush(stdout);
			for	( i = 0 ; i < ValueValuesSize(val) ; i ++ ) {
				DumpValueStruct(ValueValuesItem(val,i));
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
#define	_dbgmsg(s)		printf("%s:%d:%s\n",__FILE__,__LINE__,(s));fflush(stdout);
extern	void
InitializeValue(
	ValueStruct	*value)
{
	int		i;

ENTER_FUNC;
	if		(  value  ==  NULL  )	return;
	if		(  ValueStr(value)  !=  NULL  ) {
		FreeLBS(ValueStr(value));
	}
	ValueStr(value) = NULL;
    switch (ValueType(value)) {
	  case	GL_TYPE_ARRAY:
	  case	GL_TYPE_VALUES:
	  case	GL_TYPE_RECORD:
	  case	GL_TYPE_ALIAS:
        break;
      default:
        ValueIsNil(value);
        break;
    }
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
		ValueObject(value) = 0;
		break;
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
		memclear(ValueString(value),ValueStringSize(value));
		break;
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_SYMBOL:
		if		(  ValueString(value)  !=  NULL  ) {
			xfree(ValueString(value));
		}
		ValueString(value) = NULL;
		ValueStringLength(value) = 0;
		ValueStringSize(value) = 0;
		break;
	  case	GL_TYPE_BINARY:
		if		(  ValueByte(value)  !=  NULL  ) {
			xfree(ValueByte(value));
		}
		ValueByte(value) = NULL;
		ValueByteLength(value) = 0;
		ValueByteSize(value) = 0;
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
	  case	GL_TYPE_VALUES:
		for	( i = 0 ; i < ValueValuesSize(value) ; i ++ ) {
			InitializeValue(ValueValuesItem(value,i));
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
LEAVE_FUNC;
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
	ValueAttribute(to) = ValueAttribute(from);
	switch	(ValueType(to)) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_SYMBOL:
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
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_BINARY:
		if		(  ValueByteSize(from)  >  ValueByteSize(to)  ) {
			if		(  ValueByte(to)  !=  NULL  ) {
				xfree(ValueByte(to));
			}
			ValueByteSize(to) = ValueByteSize(from);
			ValueByte(to) = (char *)xmalloc(ValueByteSize(to));
		}
		memclear(ValueByte(to),ValueByteSize(to));
		memcpy(ValueByte(to),ValueByte(from),ValueByteSize(to));
		if		(  ValueType(to)  ==  GL_TYPE_BINARY  ) {
			ValueByteLength(to) = ValueByteLength(from);
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
	ValueAttribute(vd) = ValueAttribute(vs);
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
		ValueObject(vd) = ValueObject(vs);
		break;
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_SYMBOL:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_BINARY:
		MoveValue(vd,vs);
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
	  case	GL_TYPE_VALUES:
		for	( i = 0 ; i < ValueValuesSize(vs) ; i ++ ) {
			CopyValue(ValueValuesItem(vd,i),ValueValuesItem(vs,i));
		}
		break;
	  case	GL_TYPE_RECORD:
		for	( i = 0 ; i < ValueRecordSize(vs) ; i ++ ) {
			CopyValue(ValueRecordItem(vd,i),ValueRecordItem(vs,i));
		}
		break;
	  case	GL_TYPE_ALIAS:
	  default:
		break;
	}
LEAVE_FUNC;
}

/*
 *	assign compatible structure
 */
extern	void
AssignValue(
	ValueStruct	*vd,
	ValueStruct	*vs)
{
	int		i;

ENTER_FUNC;
	if		(  vd  ==  NULL  )	return;
	if		(  vs  ==  NULL  )	return;
	switch	(ValueType(vd)) {
	  case	GL_TYPE_INT:
	  case	GL_TYPE_FLOAT:
	  case	GL_TYPE_BOOL:
	  case	GL_TYPE_OBJECT:
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_SYMBOL:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_BINARY:
	  case	GL_TYPE_NUMBER:
		SetValueString(vd,ValueToString(vs,NULL),NULL);
		break;
	  case	GL_TYPE_ARRAY:
		for	( i = 0 ; i < ValueArraySize(vs) ; i ++ ) {
			AssignValue(ValueArrayItem(vd,i),ValueArrayItem(vs,i));
		}
		break;
	  case	GL_TYPE_VALUES:
		for	( i = 0 ; i < ValueValuesSize(vs) ; i ++ ) {
			AssignValue(ValueValuesItem(vd,i),ValueValuesItem(vs,i));
		}
		break;
	  case	GL_TYPE_RECORD:
		for	( i = 0 ; i < ValueRecordSize(vs) ; i ++ ) {
			AssignValue(ValueRecordItem(vd,i),ValueRecordItem(vs,i));
		}
		break;
	  case	GL_TYPE_ALIAS:
	  default:
		break;
	}
LEAVE_FUNC;
}

/*
 *	compare same structure
 */
extern	int
CompareValue(
	ValueStruct	*vl,
	ValueStruct	*vr)
{
	int		i;
	int		res;
	Fixed	*fl
		,	*fr;
	Numeric	nl
		,	nr;

ENTER_FUNC;
	res = 0;
	if		(  vl  ==  NULL  ) {
		res = -1;
	} else
	if		(  vr  ==  NULL  )	{
		res = 1;
	} else
	if		(  IS_VALUE_NIL(vl)  )	{
		res = -1;
	} else
	if		(  IS_VALUE_NIL(vr)  )	{
		res = 1;
#if	0
	} else
	if		(  ( ValueType(vl) - ValueType(vr) )  !=  0  ) {
		res = ValueType(vl) - ValueType(vr);
#endif
	} else
	switch	(ValueType(vl)) {
	  case	GL_TYPE_INT:
		if		(  IS_VALUE_DESC(vr)  ) {
			res = ValueToInteger(vr) - ValueToInteger(vl);
		} else {
			res = ValueToInteger(vl) - ValueToInteger(vr);
		}
		break;
	  case	GL_TYPE_FLOAT:
		if		(  IS_VALUE_DESC(vr)  ) {
			res = (int)(ValueToFloat(vr) - ValueToFloat(vl));
		} else {
			res = (int)(ValueToFloat(vl) - ValueToFloat(vr));
		}
		break;
	  case	GL_TYPE_BOOL:
		if		(  IS_VALUE_DESC(vr)  ) {
			res = (int)(ValueToBool(vr) - ValueToBool(vl));
		} else {
			res = (int)(ValueToBool(vl) - ValueToBool(vr));
		}
		break;
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_SYMBOL:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_BINARY:
		if		(  IS_VALUE_DESC(vr)  ) {
			res = strcmp(ValueToString(vr,NULL),ValueToString(vl,NULL));
		} else {
			res = strcmp(ValueToString(vl,NULL),ValueToString(vr,NULL));
		}
		break;
	  case	GL_TYPE_NUMBER:
		fl = ValueToFixed(vl);
		fr = ValueToFixed(vr);
		nl = FixedToNumeric(fl);
		nr = FixedToNumeric(fr);
		if		(  IS_VALUE_DESC(vr)  ) {
			res = NumericCmp(nr,nl);
		} else {
			res = NumericCmp(nl,nr);
		}
		NumericFree(nl);
		NumericFree(nr);
		FreeFixed(fl);
		FreeFixed(fr);
		break;
	  case	GL_TYPE_ARRAY:
		for	( i = 0 ; i < ValueArraySize(vr) ; i ++ ) {
			if		( ( res = CompareValue(ValueArrayItem(vl,i),ValueArrayItem(vr,i)) )
					  !=  0  )	break;
		}
		break;
	  case	GL_TYPE_VALUES:
		for	( i = 0 ; i < ValueValuesSize(vr) ; i ++ ) {
			if		(  ( res = CompareValue(ValueValuesItem(vl,i),ValueValuesItem(vr,i)) )
					   !=  0  )	break;
		}
		break;
	  case	GL_TYPE_RECORD:
		for	( i = 0 ; i < ValueRecordSize(vr) ; i ++ ) {
			if		(  ( res = CompareValue(ValueRecordItem(vl,i),ValueRecordItem(vr,i)) )
					   !=  0  )	break;
		}
		break;
	  case	GL_TYPE_OBJECT:
	  case	GL_TYPE_ALIAS:
	  default:
		res = 0;
		break;
	}
LEAVE_FUNC;
	return	(res);
}

/*
 *	compare structure
 */
extern	Bool
EqualValue(
	ValueStruct	*vl,
	ValueStruct	*vr)
{
	int		i;
	Bool	ret;

ENTER_FUNC;
	if		(  ValueType(vl)  ==  ValueType(vr)  ) {
		ret = TRUE;
		switch	(ValueType(vl)) {
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_SYMBOL:
			if		(  ValueStringSize(vl)  ==  ValueStringSize(vr)  ) {
				ret = TRUE;
			} else {
				ret = FALSE;
			}
			break;
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BINARY:
			if		(  ValueByteSize(vl)  ==  ValueByteSize(vr)  ) {
				ret = TRUE;
			} else {
				ret = FALSE;
			}
			break;
		  case	GL_TYPE_NUMBER:
			if		(	(  ValueFixedLength(vl)  ==  ValueFixedLength(vr)  )
					&&	(  ValueFixedSlen(vl)    ==  ValueFixedSlen(vr)    ) ) {
				ret = TRUE;
			} else {
				ret = FALSE;
			}
			break;
		  case	GL_TYPE_ARRAY:
			if		(  ValueArraySize(vl)  !=  ValueArraySize(vr)  ) {
				ret = FALSE;
			} else
			for	( i = 0 ; i < ValueArraySize(vr) ; i ++ ) {
				if		(  !EqualValue(ValueArrayItem(vl,i),ValueArrayItem(vr,i))  ) {
					ret = FALSE;
					break;
				}
			}
			break;
		  case	GL_TYPE_VALUES:
			if		(  ValueValuesSize(vl)  !=  ValueValuesSize(vr)  ) {
				ret = FALSE;
			} else
			for	( i = 0 ; i < ValueValuesSize(vr) ; i ++ ) {
				if		(  !EqualValue(ValueValuesItem(vl,i),ValueValuesItem(vr,i))  ) {
					ret = FALSE;
					break;
				}
			}
			break;
		  case	GL_TYPE_RECORD:
			if		(  ValueRecordSize(vl)  !=  ValueRecordSize(vr)  ) {
				ret = FALSE;
			} else
			for	( i = 0 ; i < ValueRecordSize(vr) ; i ++ ) {
				if		(	(  strcmp(ValueRecordName(vl,i),ValueRecordName(vr,i))  !=  0  )
						||	(  !EqualValue(ValueRecordItem(vl,i),ValueRecordItem(vr,i))  ) ) {
					ret = FALSE;
					break;
				}
			}
			break;
		  default:
			break;
		}
	} else {
		ret = FALSE;
	}
LEAVE_FUNC;
	return	(ret);
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
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_SYMBOL:
		if		(  ValueStringSize(template)  >  0  ) {
			ValueString(p) = (char *)xmalloc(ValueStringSize(template));
			memclear(ValueString(p),ValueStringSize(template));
		} else {
			ValueString(p) = NULL;
        }
		ValueStringLength(p) = ValueStringLength(template);
		ValueStringSize(p) = ValueStringSize(template);
		break;
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_BINARY:
		if		(  ValueByteSize(template)  >  0  ) {
			ValueByte(p) = (char *)xmalloc(ValueByteSize(template));
			memclear(ValueByte(p),ValueByteSize(template));
		} else {
			ValueByte(p) = NULL;
        }
		ValueByteLength(p) = ValueByteLength(template);
		ValueByteSize(p) = ValueByteSize(template);
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
			MakeValueArray(ValueArrayPrototype(template),
						   ValueArraySize(template));
		ValueArraySize(p) = ValueArraySize(template);
		ValueArrayExpandable(p) = ValueArrayExpandable(template);
		break;
	  case	GL_TYPE_INT:
		ValueInteger(p) = 0;
		break;
	  case	GL_TYPE_BOOL:
		ValueBool(p) = FALSE;
		break;
	  case	GL_TYPE_OBJECT:
		ValueObject(p) = 0;
		break;
	  case	GL_TYPE_RECORD:
		/*	share name table		*/
		ValueRecordMembers(p) = NewNameHash();
		ValueRecordNames(p) =
			(char **)xmalloc(sizeof(char *) * ValueRecordSize(template));
		/*	duplicate data space	*/
		ValueRecordItems(p) = 
			(ValueStruct **)xmalloc(sizeof(ValueStruct *) * ValueRecordSize(template));
		ValueRecordSize(p) = ValueRecordSize(template);
		for	( i = 0 ; i < ValueRecordSize(template) ; i ++ ) {
			ValueRecordItem(p,i) = 
				DuplicateValue(ValueRecordItem(template,i));
			ValueRecordName(p,i) = StrDup(ValueRecordName(template,i));
			g_hash_table_insert(ValueRecordMembers(p),
								(gpointer)ValueRecordName(p,i),
								(gpointer)(i+1));
		}
		break;
	  case	GL_TYPE_VALUES:
		ValueValuesItems(p) = 
			(ValueStruct **)xmalloc(sizeof(ValueStruct *) * ValueValuesSize(template));
		ValueValuesSize(p) = ValueValuesSize(template);
		for	( i = 0 ; i < ValueValuesSize(template) ; i ++ ) {
			ValueValuesItem(p,i) = 
				DuplicateValue(ValueValuesItem(template,i));
		}
		break;
	  case	GL_TYPE_ALIAS:
		ValueAliasName(p) = StrDup(ValueAliasName(template));
		break;
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

ENTER_FUNC;
	dbgprintf("name = [%s]\n",name); 
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
LEAVE_FUNC;
}

extern	void
ValueAddArrayItem(
	ValueStruct	*upper,
	int			ix,
	ValueStruct	*value)
{
	ValueStruct	**items;
	size_t		nsize;

ENTER_FUNC;
	if		(  ix  <  0  ) {
		ix = ValueArraySize(upper);
	}
	if		(  ix  >=  ValueArraySize(upper)  ) {
		nsize = ix + 1;
		items = (ValueStruct **)xmalloc(sizeof(ValueStruct *) * nsize);
		memclear(items,sizeof(ValueStruct *) * nsize);
		if		(  ValueArraySize(upper)  >  0  ) {
			memcpy(items, ValueArrayItems(upper), 
				   sizeof(ValueStruct *) * ValueArraySize(upper) );
			xfree(ValueArrayItems(upper));
		}
		ValueArrayItems(upper) = items;
		ValueArraySize(upper) = nsize;
	}
	ValueArrayItem(upper,ix) = value;
LEAVE_FUNC;
}

