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

#include	"types.h"
#include	"misc.h"
#include	"value.h"
#include	"hash.h"
#include	"debug.h"

static	Chunk	*HashTables = NULL;

extern	ValueStruct	*
NewValue(
	PacketDataType	type)
{
	ValueStruct	*ret;

dbgmsg(">NewValue");
	ret = New(ValueStruct);
	ret->fUpdate = FALSE;
	ret->attr = GL_ATTR_NULL;
	ret->type = type;
	switch	(type) {
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		ret->body.CharData.len = 0;
		ret->body.CharData.sval = NULL;
		break;
	  case	GL_TYPE_NUMBER:
		ret->body.FixedData.flen = 0;
		ret->body.FixedData.slen = 0;
		ret->body.FixedData.sval = NULL;
		break;
	  case	GL_TYPE_INT:
		ret->body.IntegerData = 0;
		break;
	  case	GL_TYPE_BOOL:
		ret->body.BoolData = FALSE;
		break;
	  case	GL_TYPE_OBJECT:
		ret->body.Object.apsid = 0;
		ret->body.Object.oid = 0;
		break;
	  case	GL_TYPE_RECORD:
		ret->body.RecordData.count = 0;
		ret->body.RecordData.members = NewNameHash();
		ret->body.RecordData.item = NULL;
		ret->body.RecordData.names = NULL;
#if	0
		if		(  HashTables  ==  NULL  ) {
			HashTables = NewChunk();
		}
		ChunkAppend(HashTables,ret->body.RecordData.members);
#endif
		break;
	  case	GL_TYPE_ARRAY:
		ret->body.ArrayData.count = 0;
		ret->body.ArrayData.item = NULL;
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
			for	( i = 0 ; i < val->body.ArrayData.count ; i ++ ) {
				FreeValueStruct(val->body.ArrayData.item[i]);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < val->body.RecordData.count ; i ++ ) {
				FreeValueStruct(val->body.RecordData.item[i]);
			}
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
			if		(  val->body.CharData.sval  !=  NULL  ) {
				xfree(val->body.CharData.sval);
			}
			break;
		  case	GL_TYPE_NUMBER:
			if		(  val->body.FixedData.sval  !=  NULL  ) {
				xfree(val->body.FixedData.sval);
			}
			break;
		  default:
			break;
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
		if		(  ( p = g_hash_table_lookup(value->body.RecordData.members,name) )
				   ==  NULL  ) {
			item = NULL;
		} else {
			item = value->body.RecordData.item[(int)p-1];
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
	
	if		(	(  i  >=  0                            )
			&&	(  i  <   value->body.ArrayData.count  ) )	{
		item = value->body.ArrayData.item[i];
	} else {
		item = NULL;
	}
	return	(item);
}

extern	int
ToInteger(
	ValueStruct	*val)
{
	int		ret;

	switch	(val->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		ret = atoi(ValueString(val));
		break;
	  case	GL_TYPE_NUMBER:
		ret = FixedToInt(ValueFixed(val));
		break;
	  case	GL_TYPE_INT:
		ret = ValueInteger(val);
		break;
	  case	GL_TYPE_FLOAT:
		ret = (int)ValueFloat(val);
		break;
	  case	GL_TYPE_BOOL:
		ret = ValueBool(val);
		break;
	  default:
		ret = 0;
	}
	return	(ret);
}

extern	double
ToFloat(
	ValueStruct	*val)
{
	double	ret;

	switch	(val->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		ret = atof(ValueString(val));
		break;
	  case	GL_TYPE_NUMBER:
		ret = FixedToFloat(ValueFixed(val));
		break;
	  case	GL_TYPE_INT:
		ret = (double)ValueInteger(val);
		break;
	  case	GL_TYPE_FLOAT:
		ret = ValueFloat(val);
		break;
	  case	GL_TYPE_BOOL:
		ret = (double)ValueBool(val);
		break;
	  default:
		ret = 0;
	}
	return	(ret);
}

extern	Fixed	*
ToFixed(
	ValueStruct	*val)
{
	Fixed	*ret;
	Fixed	*xval;

	switch	(val->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		ret = NewFixed(0,0);
		IntToFixed(ret,atoi(ValueString(val)));
		break;
	  case	GL_TYPE_NUMBER:
		xval = ValueFixed(val);
		ret = NewFixed(xval->flen,xval->slen);
		strcpy(ret->sval,xval->sval);
		break;
	  case	GL_TYPE_INT:
		ret = NewFixed(0,0);
		IntToFixed(ret,ValueInteger(val));
		break;
	  case	GL_TYPE_FLOAT:
		ret = NewFixed(SIZE_NUMBUF,(SIZE_NUMBUF / 2));
		FloatToFixed(ret,ValueFloat(val));
		break;
	  case	GL_TYPE_BOOL:
		ret = NewFixed(0,0);
		IntToFixed(ret,(int)ValueBool(val));
		break;
	  default:
		ret = NewFixed(0,0);
	}
	return	(ret);
}

extern	Bool
ToBool(
	ValueStruct	*val)
{
	Bool	ret;

	switch	(val->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		ret = ( *ValueString(val) == 'T' ) ? TRUE : FALSE;
		break;
	  case	GL_TYPE_NUMBER:
		ret = FixedToInt(ValueFixed(val)) ? TRUE : FALSE;
		break;
	  case	GL_TYPE_INT:
		ret = ValueInteger(val) ? TRUE : FALSE;
		break;
	  case	GL_TYPE_FLOAT:
		ret = (int)ValueFloat(val) ? TRUE : FALSE;
		break;
	  case	GL_TYPE_BOOL:
		ret = ValueBool(val);
		break;
	  default:
		ret = FALSE;
	}
	return	(ret);
}

extern	char	*
ToString(
	ValueStruct	*val)
{
	static	char	buff[SIZE_BUFF];
	char	*p
	,		*q;
	int		i;

	switch	(val->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		strcpy(buff,ValueString(val));
		break;
	  case	GL_TYPE_NUMBER:
		p = ValueFixed(val)->sval;
		q = buff;
		if		(  *p  >=  0x70  ) {
			*q ++ = '-';
			*p ^= 0x40;
		}
		strcpy(q,p);
		if		(  ValueFixed(val)->slen  >  0  ) {
			p = buff + strlen(buff);
			*(p + 1) = 0;
			q = p - 1;
			for	( i = 0 ; i < ValueFixed(val)->slen ; i ++ ) {
				*p -- = *q --;
			}
			*p = '.';
		}
		break;
	  case	GL_TYPE_INT:
		sprintf(buff,"%d",ValueInteger(val));
		break;
	  case	GL_TYPE_OBJECT:
		sprintf(buff,"[%d:%d]",ValueObject(val)->apsid,ValueObject(val)->oid);
		break;
	  case	GL_TYPE_FLOAT:
		sprintf(buff,"%g",ValueFloat(val));
		break;
	  case	GL_TYPE_BOOL:
		sprintf(buff,"%s",ValueBool(val) ? "TRUE" : "FALSE");
		break;
	  default:
		*buff = 0;
	}
	return	(buff);
}

static	void
FixedRescale(
	Fixed	*to,
	Fixed	*fr)
{
	char	*p
	,		*q;
	size_t	len;
	Bool	fMinus;

	if		(  ( *fr->sval & 0x40 )  ==  0x40  ) {
		fMinus = TRUE;
	} else {
		fMinus = FALSE;
	}
	memset(to->sval,'0',to->flen);
	to->sval[to->flen] = 0;
	p = fr->sval + ( fr->flen - fr->slen );
	q = to->sval + ( to->flen - to->slen );
	len = ( fr->slen > to->slen ) ? to->slen : fr->slen;
	for	( ; len > 0 ; len -- ) {
		*q ++ = ( *p & 0x3F );
		p ++;
	}
	p = fr->sval + ( fr->flen - fr->slen ) - 1;
	q = to->sval + ( to->flen - to->slen ) - 1;
	len = ( ( fr->flen - fr->slen ) > ( to->flen - to->slen ) )
		? ( to->flen - to->slen ) : ( fr->flen - fr->slen );
	for	( ; len > 0 ; len -- ) {
		*q -- = ( *p & 0x3F );
		p --;
	}
	if		(  fMinus  ) {
		*to->sval |= 0x40;
	}
}

extern	Bool
SetValueString(
	ValueStruct	*val,
	char		*str)
{
	Bool	rc
	,		fMinus
	,		fPoint;
	size_t	len;
	char	*p;
	char	buff[SIZE_BUFF];
	Fixed	from;

	if		(  val  ==  NULL  ) {
		fprintf(stderr,"no ValueStruct\n");
		return	(FALSE);
	}
	switch	(val->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
		memclear(val->body.CharData.sval,val->body.CharData.len + 1);
		if		(  str  !=  NULL  ) {
			strncpy(val->body.CharData.sval,str,val->body.CharData.len);
		}
		rc = TRUE;
		break;
	  case	GL_TYPE_NUMBER:
		p = buff;
		from.flen = 0;
		from.slen = 0;
		from.sval = buff;
		fPoint = FALSE;
		fMinus = FALSE;
		while	(  *str  !=  0  ) {
			if		(  fPoint  ) {
				from.slen ++;
			}
			if		(  *str  ==  '-'  ) {
				fMinus = TRUE;
			} else
			if	(  isdigit(*str)  ) {
				*p ++ = *str;
				from.flen ++;
			} else
			if		(  *str  ==  '.'  ) {
				fPoint = TRUE;
			}
			str ++;
		}
		*p = 0;
		if		(  fMinus  ) {
			*buff |= 0x40;
		}
		FixedRescale(&val->body.FixedData,&from);
		rc = TRUE;
		break;
	  case	GL_TYPE_TEXT:
		len = strlen(str) + 1;
		if		(  len  !=  val->body.CharData.len  ) {
			if		(  val->body.CharData.sval  !=  NULL  ) {
				xfree(val->body.CharData.sval);
			}
			val->body.CharData.sval = (char *)xmalloc(len + 1);
		}
		strcpy(val->body.CharData.sval,str);
		rc = TRUE;
		break;
	  case	GL_TYPE_INT:
		val->body.IntegerData = atoi(str);
		rc = TRUE;
		break;
	  case	GL_TYPE_FLOAT:
		val->body.FloatData = atof(str);
		rc = TRUE;
		break;
	  case	GL_TYPE_BOOL:
		val->body.BoolData = ( *str == 'T' ) ? TRUE : FALSE;
		rc = TRUE;
		break;
	  default:
		rc = FALSE;	  
	}
	return	(rc);
}

extern	Bool
SetValueInteger(
	ValueStruct	*val,
	int			ival)
{
	Bool	rc;
	char	str[SIZE_BUFF];
	Bool	fMinus;

	if		(  val  ==  NULL  ) {
		fprintf(stderr,"no ValueStruct\n");
		return	(FALSE);
	}
	switch	(val->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		sprintf(str,"%d",ival);
		rc = SetValueString(val,str);
		break;
	  case	GL_TYPE_NUMBER:
		if		(  ival  <  0  ) {
			ival = - ival;
			fMinus = TRUE;
		} else {
			fMinus = FALSE;
		}
		sprintf(str,"%0*d",ValueFixed(val)->flen,ival);
		if		(  fMinus  ) {
			*str |= 0x40;
		}
		rc = SetValueString(val,str);
		break;
	  case	GL_TYPE_INT:
		val->body.IntegerData = ival;
		rc = TRUE;
		break;
	  case	GL_TYPE_FLOAT:
		val->body.FloatData = ival;
		rc = TRUE;
		break;
	  case	GL_TYPE_BOOL:
		val->body.BoolData = ( ival == 0 ) ? FALSE : TRUE;
		rc = TRUE;
		break;
	  default:
		rc = FALSE;	  
	}
	return	(rc);
}

extern	Bool
SetValueBool(
	ValueStruct	*val,
	Bool		bval)
{
	Bool	rc;
	char	str[SIZE_BUFF];

	if		(  val  ==  NULL  ) {
		fprintf(stderr,"no ValueStruct\n");
		return	(FALSE);
	}
	switch	(val->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		sprintf(str,"%s",(bval ? "TRUE" : "FALSE"));
		rc = SetValueString(val,str);
		break;
	  case	GL_TYPE_NUMBER:
		sprintf(str,"%d",bval);
		rc = SetValueString(val,str);
		break;
	  case	GL_TYPE_INT:
		val->body.IntegerData = bval;
		rc = TRUE;
		break;
	  case	GL_TYPE_FLOAT:
		val->body.FloatData = bval;
		rc = TRUE;
		break;
	  case	GL_TYPE_BOOL:
		val->body.BoolData = bval;
		rc = TRUE;
		break;
	  default:
		rc = FALSE;	  
	}
	return	(rc);
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

extern	Bool
SetValueFloat(
	ValueStruct	*val,
	double		fval)
{
	Bool	rc;
	char	str[SIZE_BUFF];

	if		(  val  ==  NULL  ) {
		fprintf(stderr,"no ValueStruct\n");
		return	(FALSE);
	}
	switch	(val->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		sprintf(str,"%f",fval);
		rc = SetValueString(val,str);
		break;
	  case	GL_TYPE_NUMBER:
		FloatToFixed(ValueFixed(val),fval);
		rc = TRUE;
		break;
	  case	GL_TYPE_INT:
		val->body.IntegerData = fval;
		rc = TRUE;
		break;
	  case	GL_TYPE_FLOAT:
		val->body.FloatData = fval;
		rc = TRUE;
		break;
	  case	GL_TYPE_BOOL:
		val->body.BoolData = ( fval == 0 ) ? FALSE : TRUE;
		rc = TRUE;
		break;
	  default:
		rc = FALSE;	  
	}
	return	(rc);
}

extern	int
FixedToInt(
	Fixed	*xval)
{
	int		ival;
	int		i;

	ival = atoi(xval->sval);
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

extern	Bool
SetValueFixed(
	ValueStruct	*val,
	Fixed		*fval)
{
	Bool	rc;

	if		(  val  ==  NULL  ) {
		fprintf(stderr,"no ValueStruct\n");
		return	(FALSE);
	}
	switch	(val->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		rc = SetValueString(val,fval->sval);
		break;
	  case	GL_TYPE_NUMBER:
		FixedRescale(ValueFixed(val),fval);
		rc = TRUE;
		break;
	  case	GL_TYPE_INT:
		val->body.IntegerData = FixedToInt(fval);
		rc = TRUE;
		break;
	  case	GL_TYPE_FLOAT:
		val->body.FloatData = FixedToFloat(fval);
		rc = TRUE;
		break;
#if	0
	  case	GL_TYPE_BOOL:
		val->body.BoolData = ( *fval->sval == 0 ) ? FALSE : TRUE;
		rc = TRUE;
		break;
#endif
	  default:
		rc = FALSE;	  
	}
	return	(rc);
}

extern	ValueStruct	*
GetItemLongName(
	ValueStruct		*root,
	char			*longname)
{
	char	item[SIZE_BUFF+1]
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
			n = atoi(item);
			val = GetArrayItem(val,n);
		} else {
			while	(	(  *p  !=  0    )
					&&	(  *p  !=  '.'  )
					&&	(  *p  !=  '['  ) ) {
				*q ++ = *p ++;
			}
			*q = 0;
			val = GetRecordItem(val,item);
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
	printf("%s:",((value->attr&GL_ATTR_INPUT) == GL_ATTR_INPUT) ? "I" : "O");
	DumpValueStruct(value);
}

extern	void
DumpValueStruct(
	ValueStruct	*val)
{
	int		i;

	if		(  val  ==  NULL  )	{
		printf("null value\n");
	} else
	switch	(val->type) {
	  case	GL_TYPE_INT:
		printf("integer[%d]\n",val->body.IntegerData);
		fflush(stdout);
		break;
	  case	GL_TYPE_FLOAT:
		printf("float[%g]\n",val->body.FloatData);
		fflush(stdout);
		break;
	  case	GL_TYPE_BOOL:
		printf("Bool[%s]\n",(val->body.BoolData ? "T" : "F"));
		fflush(stdout);
		break;
	  case	GL_TYPE_CHAR:
		printf("char(%d) [",val->body.CharData.len);
		PrintFixString(val->body.CharData.sval,val->body.CharData.len);
		printf("]\n");
		fflush(stdout);
		break;
	  case	GL_TYPE_VARCHAR:
		printf("varchar(%d) [",val->body.CharData.len);
		PrintFixString(val->body.CharData.sval,val->body.CharData.len);
		printf("]\n");
		fflush(stdout);
		break;
	  case	GL_TYPE_DBCODE:
		printf("code(%d) [%s]\n",val->body.CharData.len,val->body.CharData.sval);
		fflush(stdout);
		break;
	  case	GL_TYPE_NUMBER:
		printf("number(%d,%d) [%s]\n",val->body.FixedData.flen,
			   val->body.FixedData.slen,
			   val->body.FixedData.sval);
		fflush(stdout);
		break;
	  case	GL_TYPE_TEXT:
		printf("text(%d) [",val->body.CharData.len);
		PrintFixString(val->body.CharData.sval,val->body.CharData.len);
		printf("]\n");
		fflush(stdout);
		break;
	  case	GL_TYPE_OBJECT:
		printf("object [%d:%d]\n",ValueObject(val)->apsid, ValueObject(val)->oid);
		fflush(stdout);
		break;
	  case	GL_TYPE_ARRAY:
		printf("array size = %d\n",val->body.ArrayData.count);
		fflush(stdout);
		for	( i = 0 ; i < val->body.ArrayData.count ; i ++ ) {
			DumpValueStruct(val->body.ArrayData.item[i]);
		}
		break;
	  case	GL_TYPE_RECORD:
		printf("record members = %d\n",val->body.RecordData.count);
		fflush(stdout);
		for	( i = 0 ; i < val->body.RecordData.count ; i ++ ) {
			DumpItem(val->body.RecordData.names[i],val->body.RecordData.item[i]);
		}
		printf("--\n");
		break;
	  default:
		break;
	}
}

extern	void
InitializeValue(
	ValueStruct	*value)
{
	int		i;

dbgmsg(">InitializeValue");
	if		(  value  ==  NULL  )	return;
	switch	(value->type) {
	  case	GL_TYPE_INT:
		value->body.IntegerData = 0;
		break;
	  case	GL_TYPE_FLOAT:
		value->body.FloatData = 0.0;
		break;
	  case	GL_TYPE_BOOL:
		value->body.BoolData = FALSE;
		break;
	  case	GL_TYPE_OBJECT:
		value->body.Object.apsid = 0;
		value->body.Object.oid = 0;
		break;
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
		memclear(value->body.CharData.sval,value->body.CharData.len+1);
		break;
	  case	GL_TYPE_TEXT:
		if		(  value->body.CharData.sval  !=  NULL  ) {
			xfree(value->body.CharData.sval);
		}
		value->body.CharData.sval = NULL;
		value->body.CharData.len = 0;
		break;
	  case	GL_TYPE_NUMBER:
		if		(  value->body.FixedData.flen  >  0  ) {
			memset(value->body.FixedData.sval,'0',value->body.FixedData.flen);
		}
		value->body.FixedData.sval[value->body.FixedData.flen] = 0;
		break;
	  case	GL_TYPE_ARRAY:
		for	( i = 0 ; i < value->body.ArrayData.count ; i ++ ) {
			InitializeValue(value->body.ArrayData.item[i]);
		}
		break;
	  case	GL_TYPE_RECORD:
		for	( i = 0 ; i < value->body.RecordData.count ; i ++ ) {
			InitializeValue(value->body.RecordData.item[i]);
		}
		break;
	  default:
		break;
	}
dbgmsg("<InitializeValue");
}

extern	void
MoveValue(
	ValueStruct	*to,
	ValueStruct	*from)
{
	Fixed	*xval;

	switch	(to->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		SetValueString(to,ToString(from));
		break;
	  case	GL_TYPE_NUMBER:
		xval = ToFixed(from);
		SetValueFixed(to,xval);
		FreeFixed(xval);
		break;
	  case	GL_TYPE_INT:
		SetValueInteger(to,ToInteger(from));
		break;
	  case	GL_TYPE_FLOAT:
		SetValueFloat(to,ToFloat(from));
		break;
	  case	GL_TYPE_BOOL:
		SetValueBool(to,ToBool(from));
		break;
	  default:
		break;
	}
}

extern	void
CopyValue(
	ValueStruct	*vd,
	ValueStruct	*vs)
{
	int		i;

dbgmsg(">CopyValue");
	if		(  vd  ==  NULL  )	return;
	if		(  vs  ==  NULL  )	return;
	switch	(vs->type) {
	  case	GL_TYPE_INT:
		vd->body.IntegerData = vs->body.IntegerData;
		break;
	  case	GL_TYPE_FLOAT:
		vd->body.FloatData = vs->body.FloatData;
		break;
	  case	GL_TYPE_BOOL:
		vd->body.BoolData = vs->body.BoolData;
		break;
	  case	GL_TYPE_OBJECT:
		vd->body.Object.apsid = vs->body.Object.apsid;
		vd->body.Object.oid = vs->body.Object.oid;
		break;
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
		memcpy(vd->body.CharData.sval,vs->body.CharData.sval,vs->body.CharData.len+1);
		vd->body.CharData.len = vs->body.CharData.len;
		break;
	  case	GL_TYPE_NUMBER:
		if		(  vd->body.FixedData.flen  >  0  ) {
			memcpy(vd->body.FixedData.sval,vs->body.FixedData.sval,vs->body.FixedData.flen);
		}
		vd->body.FixedData.sval[vd->body.FixedData.flen] = 0;
		vd->body.FixedData.slen = vs->body.FixedData.slen;
		break;
	  case	GL_TYPE_ARRAY:
		for	( i = 0 ; i < vs->body.ArrayData.count ; i ++ ) {
			CopyValue(vd->body.ArrayData.item[i],vs->body.ArrayData.item[i]);
		}
		break;
	  case	GL_TYPE_RECORD:
		for	( i = 0 ; i < vs->body.RecordData.count ; i ++ ) {
			CopyValue(vd->body.RecordData.item[i],vs->body.RecordData.item[i]);
		}
		vd->body.RecordData.members = vs->body.RecordData.members;
		vd->body.RecordData.names = vs->body.RecordData.names;
		break;
	  default:
		break;
	}
dbgmsg("<CopyValue");
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
	memcpy(p,template,sizeof(ValueStruct));
	switch	(template->type) {
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		if		(  template->body.CharData.len  >  0  ) {
			p->body.CharData.sval = (char *)xmalloc(template->body.CharData.len+1);
			memclear(p->body.CharData.sval,template->body.CharData.len+1);
		}
		p->body.CharData.len = template->body.CharData.len;
		break;
	  case	GL_TYPE_NUMBER:
		p->body.FixedData.sval = (char *)xmalloc(template->body.FixedData.flen+1);
		memcpy(p->body.FixedData.sval,
			   template->body.FixedData.sval,
			   template->body.FixedData.flen+1);
		p->body.FixedData.flen = template->body.FixedData.flen;
		p->body.FixedData.slen = template->body.FixedData.slen;
		break;
	  case	GL_TYPE_ARRAY:
		p->body.ArrayData.item = 
			MakeValueArray(template->body.ArrayData.item[0],
						   template->body.ArrayData.count);
		p->body.ArrayData.count = template->body.ArrayData.count;
		break;
	  case	GL_TYPE_INT:
		p->body.IntegerData = 0;
		break;
	  case	GL_TYPE_BOOL:
		p->body.BoolData = FALSE;
		break;
	  case	GL_TYPE_OBJECT:
		p->body.Object.apsid = 0;
		p->body.Object.oid = 0;
		break;
	  case	GL_TYPE_RECORD:
		/*	share name table		*/
		p->body.RecordData.members = template->body.RecordData.members;
		p->body.RecordData.names = template->body.RecordData.names;
		/*	duplicate data space	*/
		p->body.RecordData.item = 
			(ValueStruct **)xmalloc(sizeof(ValueStruct *) * template->body.RecordData.count);
		p->body.RecordData.count = template->body.RecordData.count;
		for	( i = 0 ; i < template->body.RecordData.count ; i ++ ) {
			p->body.RecordData.item[i] = 
				DuplicateValue(template->body.RecordData.item[i]);
		}
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
	ValueStruct	**item;
	char		**names;

dbgmsg(">ValueAddRecordItem");
#ifdef	TRACE
	printf("name = [%s]\n",name); 
#endif
	item = (ValueStruct **)
		xmalloc(sizeof(ValueStruct *) * ( upper->body.RecordData.count + 1 ) );
	names = (char **)
		xmalloc(sizeof(char *) * ( upper->body.RecordData.count + 1 ) );
	if		(  upper->body.RecordData.count  >  0  ) {
		memcpy(item, upper->body.RecordData.item, 
			   sizeof(ValueStruct *) * upper->body.RecordData.count );
		memcpy(names, upper->body.RecordData.names, 
			   sizeof(char *) * upper->body.RecordData.count );
		xfree(upper->body.RecordData.item);
		xfree(upper->body.RecordData.names);
	}
	upper->body.RecordData.item = item;
	upper->body.RecordData.names = names;
	item[upper->body.RecordData.count] = value;
	names[upper->body.RecordData.count] = StrDup(name);
	if		(  name  !=  NULL  ) {
		if		(  g_hash_table_lookup(upper->body.RecordData.members,name)  ==  NULL  ) {
			g_hash_table_insert(upper->body.RecordData.members,
								(gpointer)names[upper->body.RecordData.count],
								(gpointer)upper->body.RecordData.count+1);
		} else {
			printf("name = [%s]\t",name);
			Error("name duplicate");
		}
	}
	upper->body.RecordData.count ++;
dbgmsg("<ValueAddRecordItem");
}

