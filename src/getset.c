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
#include	"monstring.h"
#include	"value.h"
#include	"getset.h"
#include	"debug.h"

extern	int
ValueToInteger(
	ValueStruct	*val)
{
	int		ret;

	switch	(val->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		ret = StrToInt(ValueString(val),strlen(ValueString(val)));
		break;
	  case	GL_TYPE_NUMBER:
		ret = FixedToInt(&ValueFixed(val));
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
ValueToFloat(
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
		ret = FixedToFloat(&ValueFixed(val));
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
ValueToFixed(
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
		IntToFixed(ret,StrToInt(ValueString(val),strlen(ValueString(val))));
		break;
	  case	GL_TYPE_NUMBER:
		xval = &ValueFixed(val);
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
ValueToBool(
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
		ret = FixedToInt(&ValueFixed(val)) ? TRUE : FALSE;
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
ValueToString(
	ValueStruct	*val)
{
	static	char	buff[SIZE_BUFF];
	char	*p
	,		*q;
	int		i;

	memset(buff,0,SIZE_BUFF);
	switch	(val->type) {
	  case	GL_TYPE_CHAR:
		memcpy(buff,ValueString(val),ValueStringLength(val));
		break;
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
		if		(  ValueString(val)  !=  NULL  ) {
			strcpy(buff,ValueString(val));
		}
		break;
	  case	GL_TYPE_BYTE:
		p = ValueByte(val);
		q = buff;
		for	( i = 0 ; i < ValueByteLength(val) ; i ++ , p ++ ) {
			if		(  *p  ==  '%'  ) {
				*q ++ = '%';
				*q ++ = '%';
			} else
			if		(  isprint(*p)  ) {
				*q ++ = *p;
			} else {
				q += sprintf(q,"%02X",(int)*p);
			}
		}
		*q = 0;
		break;
	  case	GL_TYPE_NUMBER:
		p = ValueFixedBody(val);
		q = buff;
		if		(  *p  >=  0x70  ) {
			*q ++ = '-';
			*p ^= 0x40;
		}
		strcpy(q,p);
		if		(  ValueFixedBody(val)  >  0  ) {
			p = buff + strlen(buff);
			*(p + 1) = 0;
			q = p - 1;
			for	( i = 0 ; i < ValueFixedLength(val) ; i ++ ) {
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
	char	buff[SIZE_NUMBUF+1];
	Fixed	from;

	if		(  val  ==  NULL  ) {
		fprintf(stderr,"no ValueStruct\n");
		return	(FALSE);
	}
	switch	(ValueType(val)) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
		memclear(ValueString(val),ValueStringLength(val) + 1);
		if		(  str  !=  NULL  ) {
			strncpy(ValueString(val),str,ValueStringLength(val));
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
		FixedRescale(&ValueFixed(val),&from);
		rc = TRUE;
		break;
	  case	GL_TYPE_TEXT:
		len = strlen(str) + 1;
		if		(  len  >  ValueStringLength(val)  ) {
			if		(  ValueString(val)  !=  NULL  ) {
				xfree(ValueString(val));
			}
			ValueString(val) = (char *)xmalloc(len + 1);
			ValueStringLength(val) = len;
		}
		memset(ValueString(val),0,ValueStringLength(val)+1);
		strcpy(ValueString(val),str);
		rc = TRUE;
		break;
	  case	GL_TYPE_INT:
		ValueInteger(val) = StrToInt(str,strlen(str));
		rc = TRUE;
		break;
	  case	GL_TYPE_FLOAT:
		ValueFloat(val) = atof(str);
		rc = TRUE;
		break;
	  case	GL_TYPE_BOOL:
		ValueBool(val) = ( *str == 'T' ) ? TRUE : FALSE;
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
	char	str[SIZE_NUMBUF+1];
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
		sprintf(str,"%0*d",ValueFixedLength(val),ival);
		if		(  fMinus  ) {
			*str |= 0x40;
		}
		rc = SetValueString(val,str);
		break;
	  case	GL_TYPE_INT:
		ValueInteger(val) = ival;
		rc = TRUE;
		break;
	  case	GL_TYPE_FLOAT:
		ValueFloat(val) = ival;
		rc = TRUE;
		break;
	  case	GL_TYPE_BOOL:
		ValueBool(val) = ( ival == 0 ) ? FALSE : TRUE;
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
	char	str[SIZE_NUMBUF+1];

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
		ValueInteger(val) = bval;
		rc = TRUE;
		break;
	  case	GL_TYPE_FLOAT:
		ValueFloat(val) = bval;
		rc = TRUE;
		break;
	  case	GL_TYPE_BOOL:
		ValueBool(val) = bval;
		rc = TRUE;
		break;
	  default:
		rc = FALSE;	  
	}
	return	(rc);
}

extern	Bool
SetValueFloat(
	ValueStruct	*val,
	double		fval)
{
	Bool	rc;
	char	str[SIZE_NUMBUF+1];

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
		FloatToFixed(&ValueFixed(val),fval);
		rc = TRUE;
		break;
	  case	GL_TYPE_INT:
		ValueInteger(val) = (int)fval;
		rc = TRUE;
		break;
	  case	GL_TYPE_FLOAT:
		ValueFloat(val) = fval;
		rc = TRUE;
		break;
	  case	GL_TYPE_BOOL:
		ValueBool(val) = ( fval == 0 ) ? FALSE : TRUE;
		rc = TRUE;
		break;
	  default:
		rc = FALSE;	  
	}
	return	(rc);
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
		FixedRescale(&ValueFixed(val),fval);
		rc = TRUE;
		break;
	  case	GL_TYPE_INT:
		ValueInteger(val) = FixedToInt(fval);
		rc = TRUE;
		break;
	  case	GL_TYPE_FLOAT:
		ValueFloat(val) = FixedToFloat(fval);
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

