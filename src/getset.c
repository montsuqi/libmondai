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
#include	<errno.h>
#include	<iconv.h>
#include	<glib.h>
#include	<math.h>

#define		__VALUE_DIRECT
#include	"types.h"
#include	"misc.h"
#include	"monstring.h"
#include	"value.h"
#include	"memory.h"
#include	"getset.h"
#include	"debug.h"

extern	int
ValueToInteger(
	ValueStruct	*val)
{
	int		ret;

	switch	(ValueType(val)) {
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
	ValueStruct	*val,
	char		*codeset)
{
	char	work[SIZE_NUMBUF+1];
	char	work2[SIZE_NUMBUF+2];
	char	*p
	,		*q;
	int		i;
	int		size;

ENTER_FUNC;
	dbgprintf("type = %X\n",(int)ValueType(val));
	if		(  ValueStr(val)  ==  NULL  ) {
		ValueStr(val) = NewLBS();
	}
	LBS_EmitStart(ValueStr(val));
	if		(  IS_VALUE_NIL(val)  ) {
		LBS_EmitChar(ValueStr(val),CHAR_NIL);
	} else
	switch	(ValueType(val)) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
#if	0
		if		(  codeset  ==  NULL  ) {
			LBS_ReserveSize(ValueStr(val),ValueStringSize(val),FALSE);
			memcpy(ValueStrBody(val),ValueString(val),ValueStringSize(val));
		} else {
			LBS_EmitStringCodeset(ValueStr(val),ValueString(val),
								  ValueStringSize(val),
								  ValueStringLength(val),codeset);
		}
#else
		LBS_EmitStringCodeset(ValueStr(val),ValueString(val),
							  ValueStringSize(val),
							  ValueStringLength(val),codeset);
#endif
		if		(  ( size = ValueStringLength(val) - ValueSize(val) )  >  0  ) {
			for	(  ; size > 0 ; size -- ) {
				LBS_EmitChar(ValueStr(val),0);
			}
		}
		break;
	  case	GL_TYPE_TEXT:
		if		(  ValueString(val)  !=  NULL  ) {
			if		(  codeset  ==  NULL  ) {
				LBS_ReserveSize(ValueStr(val),strlen(ValueString(val))+1,FALSE);
				strcpy(ValueStrBody(val),ValueString(val));
			} else {
				LBS_EmitStringCodeset(ValueStr(val),ValueString(val),
									 ValueStringSize(val),
									 0,codeset);
			}
		} else {
			LBS_EmitChar(ValueStr(val),CHAR_NIL);
		}
		break;
	  case	GL_TYPE_BYTE:
		p = ValueByte(val);
		for	( i = 0 ; i < ValueByteLength(val) ; i ++ , p ++ ) {
			if		(  *p  ==  '%'  ) {
				LBS_EmitChar(ValueStr(val),'%');
				LBS_EmitChar(ValueStr(val),'%');
			} else
			if		(  isprint(*p)  ) {
				LBS_EmitChar(ValueStr(val),*p);
			} else {
				sprintf(work,"%02X",(int)*p);
				LBS_EmitString(ValueStr(val),work);
			}
		}
		if		(  ( size = ValueStringLength(val) - ValueSize(val) )  >  0  ) {
			for	(  ; size > 0 ; size -- ) {
				LBS_EmitByte(ValueStr(val),0);
			}
		}
		break;
	  case	GL_TYPE_NUMBER:
		strcpy(work,ValueFixedBody(val));
		p = work;
		q = work2;
		if		(  *p  >=  0x70  ) {
			*q ++ = '-';
			*p ^= 0x40;
		}
		strcpy(q,p);
		if		(  ValueFixedSlen(val)  >  0  ) {
			p = work2 + strlen(work2);
			*(p + 1) = 0;
			q = p - 1;
			for	( i = 0 ; i < ValueFixedSlen(val) ; i ++ ) {
				*p -- = *q --;
			}
			*p = '.';
		}
		LBS_EmitString(ValueStr(val),work2);
		break;
	  case	GL_TYPE_INT:
		sprintf(work,"%d",ValueInteger(val));
		LBS_EmitString(ValueStr(val),work);
		break;
	  case	GL_TYPE_OBJECT:
		sprintf(work,"[%d:%d]",ValueObject(val)->apsid,ValueObject(val)->oid);
		LBS_EmitString(ValueStr(val),work);
		break;
	  case	GL_TYPE_FLOAT:
		sprintf(work,"%g",ValueFloat(val));
		LBS_EmitString(ValueStr(val),work);
		break;
	  case	GL_TYPE_BOOL:
		sprintf(work,"%s",ValueBool(val) ? "TRUE" : "FALSE");
		LBS_EmitString(ValueStr(val),work);
		break;
	  default:
		break;
	}
	LBS_EmitEnd(ValueStr(val));
LEAVE_FUNC;
	return	(ValueStrBody(val));
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
	char		*str,
	char		*codeset)
{
	Bool	rc
	,		fMinus
	,		fPoint;
	size_t	len;
	int		i;
	char	*p
	,		*q
	,		*istr;
	char	buff[SIZE_NUMBUF+1]
	,		sbuff[SIZE_LONGNAME+1];
	Fixed	from;
	size_t	size;
	iconv_t	cd;
	size_t	sob
	,		sib;

	if		(  val  ==  NULL  ) {
		fprintf(stderr,"no ValueStruct\n");
		return	(FALSE);
	}
ENTER_FUNC;
	if		(	(  str   ==  NULL      )
			||	(  *str  ==  CHAR_NIL  ) ) {
		ValueIsNil(val);
		rc = TRUE;
	} else {
		ValueIsNonNil(val);
		if		(  codeset  !=  NULL  ) {
			cd = iconv_open("utf8",codeset);
		} else {
			cd = (iconv_t)0;
		}
		switch	(ValueType(val)) {
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			if		(  codeset  !=  NULL  ) {
				len = ValueStringLength(val) < strlen(str) ?
					ValueStringLength(val) : strlen(str);
				while	(TRUE) {
					istr = str;
					sib = len;
					sob = ValueStringSize(val);
					if		(  ( q = ValueString(val) )  !=  NULL  ) {
						*q = 0;
						if		(  iconv(cd,&istr,&sib,&q,&sob)  ==  0  )	break;
#if	1
						if		(	(  errno  ==  E2BIG  )
								||	(  errno  ==  EINVAL  ) ) {
#else
						if		(  errno  ==  E2BIG ) {
#endif
							xfree(ValueString(val));
							ValueStringSize(val) *= 2;
						} else
							break;
					} else {
						ValueStringSize(val) = 1;
					}
					ValueString(val) = (char *)xmalloc(ValueStringSize(val));
				};
				*q = 0;
			} else {
				size = strlen(str) + 1;
				if		(  size  >  ValueStringSize(val)  ) {
					if		(  ValueString(val)  !=  NULL  ) {
						xfree(ValueString(val));
					}
					ValueStringSize(val) = size;
					ValueString(val) = (char *)xmalloc(size);
				}
				memclear(ValueString(val),ValueStringSize(val));
				strcpy(ValueString(val),str);
			}
			rc = TRUE;
			break;
		  case	GL_TYPE_TEXT:
			len = strlen(str);
			if		(  codeset  !=  NULL  ) {
				while	(TRUE) {
					istr = str;
					sib = len;
					sob = ValueStringSize(val);
					if		(  ( q = ValueString(val) )  !=  NULL  ) {
						*q = 0;
						if		(  iconv(cd,&istr,&sib,&q,&sob)  ==  0  )	break;
#if	1
						if		(	(  errno  ==  E2BIG  )
								||	(  errno  ==  EINVAL  ) ) {
#else
						if		(  errno  ==  E2BIG ) {
#endif
							xfree(ValueString(val));
							ValueStringSize(val) *= 2;
						} else
							break;
					} else {
						ValueStringSize(val) = 1;
					}
					ValueString(val) = (char *)xmalloc(ValueStringSize(val));
					memclear(ValueString(val),ValueStringSize(val));
				};
				*q = 0;
			} else {
				size = len + 1;
				if		(  size  >  ValueStringSize(val)  ) {
					if		(  ValueString(val)  !=  NULL  ) {
						xfree(ValueString(val));
					}
					ValueStringSize(val) = size;
					ValueString(val) = (char *)xmalloc(size);
				}
				memclear(ValueString(val),ValueStringSize(val));
				strcpy(ValueString(val),str);
			}
			ValueStringLength(val) = len;
			rc = TRUE;
			break;
		  case	GL_TYPE_NUMBER:
		  case	GL_TYPE_INT:
		  case	GL_TYPE_FLOAT:
		  case	GL_TYPE_BOOL:
			if		(  codeset  !=  NULL  ) {
				istr = str;
				sib = strlen(str);
				sob = SIZE_NUMBUF;
				p = sbuff;
				iconv(cd,&istr,&sib,&p,&sob);
				*p = 0;
				str = sbuff;
			} else {
				strncpy(sbuff,str,SIZE_NUMBUF);
				str = sbuff;
			}
			switch	(ValueType(val)) {
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
				break;
			}
			break;
		  case	GL_TYPE_BYTE:
			p = ValueByte(val);
			for	( i = 0 ; i < ValueByteLength(val) ; i ++ , p ++ ) {
				if		(  *str  ==  '%'  ) {
					str ++;
					if		(  *str  ==  '%'  ) {
						*p = '%';
						str ++;
					} else {
						*p = (unsigned char)HexToInt(str,2);
						str += 2;
					}
				} else {
					*p = *str;
					str ++;
				}
			}
			rc = TRUE;
			break;
		  default:
			rc = FALSE;	  
		}
		if		(  codeset  !=  NULL  ) {
			iconv_close(cd);
		}
	}
LEAVE_FUNC;
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
		rc = SetValueString(val,str,NULL);
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
		rc = SetValueString(val,str,NULL);
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
	ValueIsNonNil(val);
	return	(rc);
}

extern	Bool
SetValueChar(
	ValueStruct	*val,
	char		cval)
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
		sprintf(str,"%c",cval);
		rc = SetValueString(val,str,NULL);
		break;
	  case	GL_TYPE_NUMBER:
		if		(  cval  <  0  ) {
			cval = - cval;
			fMinus = TRUE;
		} else {
			fMinus = FALSE;
		}
		sprintf(str,"%0*d",ValueFixedLength(val),(int)cval);
		if		(  fMinus  ) {
			*str |= 0x40;
		}
		rc = SetValueString(val,str,NULL);
		break;
	  case	GL_TYPE_INT:
		ValueInteger(val) = (int)cval;
		rc = TRUE;
		break;
	  case	GL_TYPE_FLOAT:
		ValueFloat(val) = (double)cval;
		rc = TRUE;
		break;
	  case	GL_TYPE_BOOL:
		ValueBool(val) = ( cval == 0 ) ? FALSE : TRUE;
		rc = TRUE;
		break;
	  default:
		rc = FALSE;	  
	}
	ValueIsNonNil(val);
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
		rc = SetValueString(val,str,NULL);
		break;
	  case	GL_TYPE_NUMBER:
		sprintf(str,"%d",bval);
		rc = SetValueString(val,str,NULL);
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
	ValueIsNonNil(val);
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
		rc = SetValueString(val,str,NULL);
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
	ValueIsNonNil(val);
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
		rc = SetValueString(val,fval->sval,NULL);
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
	ValueIsNonNil(val);
	return	(rc);
}

