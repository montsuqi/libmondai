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
#include	<ctype.h>
#include    <sys/types.h>

#include	"types.h"
#include	"misc.h"
#include	"monstring.h"
#include	"others.h"
#include	"value.h"
#include	"getset.h"
#include	"Text_v.h"
#include	"debug.h"

static	char	*
_CSV_UnPackValue(
	CONVOPT		*opt,
	char		*p,
	ValueStruct	*value,
	char		*buff)
{
	int		i;
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
				p = _CSV_UnPackValue(opt,p,ValueArrayItem(value,i),buff);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p = _CSV_UnPackValue(opt,p,ValueRecordItem(value,i),buff);
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
	CONVOPT		*opt,
	char		*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF];

	return	(_CSV_UnPackValue(opt,p,value,buff));
}

static	void
CSV_Encode(
	char	*str,
	char	fSsep,
	char	*buff)
{
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
__CSV_PackValue(
	CONVOPT		*opt,
	char		*p,
	ValueStruct	*value,
	Bool		fNsep,
	Bool		fSsep,
	Bool		fCesc,
	char		*buff)
{
	int		i;

	if		(  value  !=  NULL  ) {
		switch	(value->type) {
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BOOL:
			CSV_Encode(ValueToString(value),fSsep,buff);
			if		(  fSsep  ) {
				p += sprintf(p,"\"%s\",",buff);
			} else {
				if		(  IsComma(buff)  ) {
					p += sprintf(p,"\"%s\",",buff);
				} else {
					p += sprintf(p,"%s,",buff);
				}
			}
			break;
		  case	GL_TYPE_NUMBER:
		  case	GL_TYPE_INT:
		  case	GL_TYPE_FLOAT:
			if		(  fNsep  ) {
				p += sprintf(p,"\"%s\",",ValueToString(value));
			} else {
				p += sprintf(p,"%s,",ValueToString(value));
			}
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p = __CSV_PackValue(opt,p,ValueArrayItem(value,i),fNsep,fSsep,fCesc,buff);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p = __CSV_PackValue(opt,p,ValueRecordItem(value,i),fNsep,fSsep,fCesc,buff);
			}
			break;
		  default:
			break;
		}
	}
	return	(p);
}

static	char	*
_CSV_PackValue(
	CONVOPT		*opt,
	char	*p,
	ValueStruct	*value,
	Bool		fNsep,
	Bool		fSsep,
	Bool		fCesc,
	char		*buff)
{
	char	*ret;

	ret = __CSV_PackValue(opt,p,value,fNsep,fSsep,fCesc,buff);
	ret --;
	*ret = 0;
	return	(ret);
}

extern	char	*
CSV1_PackValue(
	CONVOPT		*opt,
	char	*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF+1];
	return	(_CSV_PackValue(opt,p,value,TRUE,TRUE,FALSE,buff));
}

extern	char	*
CSV2_PackValue(
	CONVOPT		*opt,
	char	*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF+1];
	return	(_CSV_PackValue(opt,p,value,FALSE,FALSE,FALSE,buff));
}

extern	char	*
CSV3_PackValue(
	CONVOPT		*opt,
	char	*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF+1];
	return	(_CSV_PackValue(opt,p,value,FALSE,TRUE,FALSE,buff));
}

extern	char	*
CSVE_PackValue(
	CONVOPT		*opt,
	char	*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF+1];
	return	(_CSV_PackValue(opt,p,value,FALSE,FALSE,FALSE,buff));
}

static	size_t
_CSV_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value,
	Bool		fNsep,
	Bool		fSsep,
	Bool		fCesc)
{
	int		i;
	size_t	ret;
	char	*str;
	Bool	fComma;

dbgmsg(">_CSV_SizeValue");
	if		(  value  ==  NULL  )	return	(0);
	switch	(value->type) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_BOOL:
		str = ValueToString(value);
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
		ret = strlen(ValueToString(value)) + 1;
		if		(  fNsep  )	ret += 2;
		break;
	  case	GL_TYPE_ARRAY:
		ret = 0;
		for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
			ret += _CSV_SizeValue(opt,ValueArrayItem(value,i),fNsep,fSsep,fCesc);
		}
		break;
	  case	GL_TYPE_RECORD:
		ret = 0;
		for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
			ret += _CSV_SizeValue(opt,ValueRecordItem(value,i),fNsep,fSsep,fCesc);
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
	CONVOPT		*opt,
	ValueStruct	*value)
{
	return	(_CSV_SizeValue(opt,value,TRUE,TRUE,FALSE));
}

extern	size_t
CSV2_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value)
{
	return	(_CSV_SizeValue(opt,value,FALSE,FALSE,FALSE));
}

extern	size_t
CSV3_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value)
{
	return	(_CSV_SizeValue(opt,value,FALSE,TRUE,FALSE));
}

extern	size_t
CSVE_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value)
{
	return	(_CSV_SizeValue(opt,value,FALSE,FALSE,TRUE));
}

/*
 *	RFC822 type conversion
 */

static	char	*
RFC822_SkipNext(
	CONVOPT	*opt,
	char	*p)
{
	switch	(opt->encode) {
	  case	STRING_ENCODING_URL:
		while	(	(  *p  !=  0     )
				&&	(  *p  !=  '\n'  ) )	p ++;
		break;
	  default:
		break;
	}
	return	(p);
}

static	void
DecodeName(
	char	**rname,
	char	**vname,
	char	*p)
{
	while	(  isspace(*p)  )	p ++;
	*rname = p;
	while	(	(  *p  !=  0     )
			&&	(  *p  !=  '.'   ) )	p ++;
	*p = 0;
	p ++;
	*vname = p;
	if		(  *p  !=  0  ) {
		while	(	(  *p  !=  0     )
				&&	(  !isspace(*p)  ) )	p ++;
	}
	*p = 0;
}

static	char	*
_RFC822_UnPackValueNoNamed(
	CONVOPT		*opt,
	char		*p,
	ValueStruct	*value,
	char		*buff)
{
	int		i;
	char	*q
	,		ch;
	size_t	len;

	if		(  value  !=  NULL  ) {
		switch	(ValueType(value)) {
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_BYTE:
			q = p;
			while	(	(  *p  !=  0     )
					&&	(  *p  !=  '\n'  ) )	p ++;
			len = DecodeBase64(buff,q,p-q);
			buff[len] = 0;
			SetValueString(value,buff);
			p ++;
			break;
		  case	GL_TYPE_BOOL:
		  case	GL_TYPE_NUMBER:
		  case	GL_TYPE_INT:
		  case	GL_TYPE_FLOAT:
			q = p;
			while	(	(  *p  !=  0     )
					&&	(  *p  !=  '\n'  ) )	p ++;
			ch = *p;
			*p = 0;
			SetValueString(value,q);
			*p = ch;
			p ++;
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p = _RFC822_UnPackValueNoNamed(opt,p,ValueArrayItem(value,i),buff);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p = _RFC822_UnPackValueNoNamed(opt,p,ValueRecordItem(value,i),buff);
			}
			break;
		  default:
			break;
		}
	}
	return	(p);
}

static	char	*
_RFC822_UnPackValueNamed(
	CONVOPT		*opt,
	char		*p,
	ValueStruct	*value,
	char		*buff)
{
	char	str[SIZE_LONGNAME+1];
	char	*vname
	,		*rname;
	char	*q
	,		ch;
	ValueStruct	*e;
	size_t	len;

	if		(  value  !=  NULL  ) {
		while	(  *p  !=  0  ) {
			q = str;
			while	(	(  *p  !=  0    )
					&&	(  *p  !=  ':'  ) ) {
				*q ++ = *p ++;
			}
			*q = 0;
			p ++;
			while	(	(  *p  !=  0    )
					&&	(  isspace(*p)  ) )	p ++;
			DecodeName(&rname,&vname,str);
			if		(  strcmp(rname,opt->recname)  ) {
				p = RFC822_SkipNext(opt,p);
			} else {
				if		(  ( e = GetItemLongName(value,vname) )  !=  NULL  ) {
					q = p;
					while	(	(  *p  !=  0     )
							&&	(  *p  !=  '\n'  ) )	p ++;
					switch	(ValueType(e)) {
					  case	GL_TYPE_CHAR:
					  case	GL_TYPE_VARCHAR:
					  case	GL_TYPE_DBCODE:
					  case	GL_TYPE_TEXT:
					  case	GL_TYPE_BYTE:
						len = DecodeBase64(buff,q,p-q);
						buff[len] = 0;
						SetValueString(e,buff);
						break;
					  case	GL_TYPE_BOOL:
					  case	GL_TYPE_NUMBER:
					  case	GL_TYPE_INT:
					  case	GL_TYPE_FLOAT:
						ch = *p;
						*p = 0;
						SetValueString(e,q);
						*p = ch;
						break;
					  default:
						break;
					}
					p ++;
				} else {
					p = RFC822_SkipNext(opt,p);
				}
			}
		}
	}					
	return	(p);
}

extern	char	*
RFC822_UnPackValue(
	CONVOPT		*opt,
	char	*p,
	ValueStruct	*value)
{
	char	*ret;
	char	buff[SIZE_BUFF];

	if		(  opt->fName  ) {
		ret = _RFC822_UnPackValueNamed(opt,p,value,buff);
	} else {
		ret = _RFC822_UnPackValueNoNamed(opt,p,value,buff);
	}
	return	(ret);
}

static	char	*
_RFC822_PackValue(
	CONVOPT		*opt,
	char		*p,
	ValueStruct	*value,
	char		*name,
	char		*longname,
	char		*buff)
{
	int		i;
	char	*str;

	if		(  value  !=  NULL  ) {
		if		(  name  ==  NULL  ) { 
			name = longname + strlen(longname);
		}
		switch	(ValueType(value)) {
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_BYTE:
			str = ValueToString(value);
			EncodeBase64(buff,str,strlen(str));
			if		(  opt->fName  ) {
				p+= sprintf(p,"%s: ",longname);
			}
			p += sprintf(p,"%s\n",buff);
			break;
		  case	GL_TYPE_BOOL:
		  case	GL_TYPE_NUMBER:
		  case	GL_TYPE_INT:
		  case	GL_TYPE_FLOAT:
			if		(  opt->fName  ) {
				p+= sprintf(p,"%s: ",longname);
			}
			p += sprintf(p,"%s\n",ValueToString(value));
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				sprintf(name,"[%d]",i);
				p = _RFC822_PackValue(opt,p,ValueArrayItem(value,i),
									  name+strlen(name),longname,buff);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				sprintf(name,".%s",ValueRecordName(value,i));
				p = _RFC822_PackValue(opt,p,ValueRecordItem(value,i),
									  name+strlen(name),longname,buff);
			}
			break;
		  default:
			break;
		}
	}
	*p = 0;
	return	(p);
}

extern	char	*
RFC822_PackValue(
	CONVOPT	*opt,
	char	*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF]
	,		longname[SIZE_LONGNAME+1];

	memclear(longname,SIZE_LONGNAME);
	if		(  opt->recname  !=  NULL  ) {
		strcpy(longname,opt->recname);
	}
	return	(_RFC822_PackValue(opt,p,value,NULL,longname,buff));
}

static	size_t
_RFC822_SizeValue(
	CONVOPT	*opt,
	ValueStruct	*value,
	char		*name,
	char		*longname)
{
	int		i;
	size_t	ret;

dbgmsg(">_RFC822_SizeValue");
	if		(  value  ==  NULL  )	return	(0);
	if		(  name  ==  NULL  ) { 
		name = longname + strlen(longname);
	}
	switch	(ValueType(value)) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_BYTE:
		ret = 1;
		if		(  opt->fName  ) {
			ret += strlen(longname) + 2;
		}
		ret += EncodeLengthBase64(ValueToString(value));
		break;
	  case	GL_TYPE_BOOL:
	  case	GL_TYPE_NUMBER:
	  case	GL_TYPE_INT:
	  case	GL_TYPE_FLOAT:
		ret = strlen(ValueToString(value)) + 1;
		if		(  opt->fName  ) {
			ret += strlen(longname) + 2;
		}
		break;
	  case	GL_TYPE_ARRAY:
		ret = 0;
		for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
			sprintf(name,"[%d]",i);
			ret += _RFC822_SizeValue(opt,ValueArrayItem(value,i),
									 name+strlen(name),longname);
		}
		break;
	  case	GL_TYPE_RECORD:
		ret = 0;
		for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
			sprintf(name,".%s",ValueRecordName(value,i));
			ret += _RFC822_SizeValue(opt,ValueRecordItem(value,i),
									 name+strlen(name),longname);
		}
		break;
	  case	GL_TYPE_OBJECT:
	  default:
		ret = 0;
		break;
	}
dbgmsg("<_RFC822_SizeValue");
	return	(ret);
}

extern	size_t
RFC822_SizeValue(
	CONVOPT	*opt,
	ValueStruct	*value)
{
	char	longname[SIZE_LONGNAME+1];

	memclear(longname,SIZE_LONGNAME);
	if		(  opt->recname  !=  NULL  ) {
		strcpy(longname,opt->recname);
	}
	return	(_RFC822_SizeValue(opt,value,NULL,longname));
}

/*
 *	CGI format
 */

static	char	*
CGI_SkipNext(
	CONVOPT	*opt,
	char	*p)
{
	while	(	(  *p  !=  0     )
			&&	(  *p  !=  '&'   )
			&&	(  *p  !=  '\n'  ) )	p ++;
	p ++;
	return	(p);
}

static	char	*
_CGI_UnPackValue(
	CONVOPT		*opt,
	char		*p,
	ValueStruct	*value,
	char		*buff)
{
	char	str[SIZE_LONGNAME+1];
	char	*vname
	,		*rname;
	char	*q
	,		ch;
	ValueStruct	*e;

	if		(  value  !=  NULL  ) {
		while	(  *p  !=  0  ) {
			q = str;
			while	(	(  *p  !=  0    )
					&&	(  *p  !=  '='  ) ) {
				*q ++ = *p ++;
			}
			*q = 0;
			p ++;
			DecodeName(&rname,&vname,str);
			if		(  strcmp(rname,opt->recname)  ) {
				p = CGI_SkipNext(opt,p);
			} else {
				if		(  ( e = GetItemLongName(value,vname) )  !=  NULL  ) {
					q = p;
					while	(	(  *p  !=  0     )
							&&	(  *p  !=  '&'   )
							&&	(  *p  !=  '\n'  ) )	p ++;
					ch = *p;
					*p = 0;
					DecodeStringURL(buff,q);
					*p = ch;
					p ++;
					SetValueString(e,buff);
				} else {
					p = CGI_SkipNext(opt,p);
				}
			}
		}
	}					
	return	(p);
}

extern	char	*
CGI_UnPackValue(
	CONVOPT		*opt,
	char	*p,
	ValueStruct	*value)
{
	char	*ret;
	char	buff[SIZE_BUFF];

	ret = _CGI_UnPackValue(opt,p,value,buff);
	return	(ret);
}

static	char	*
_CGI_PackValue(
	CONVOPT	*opt,
	char		*p,
	ValueStruct	*value,
	char		*name,
	char		*longname,
	char		*buff)
{
	int		i;

	if		(  value  !=  NULL  ) {
		switch	(value->type) {
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BOOL:
		  case	GL_TYPE_NUMBER:
		  case	GL_TYPE_INT:
		  case	GL_TYPE_FLOAT:
			EncodeStringURL(buff,ValueToString(value));
			p += sprintf(p,"%s=%s&",longname,buff);
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				sprintf(name,"[%d]",i);
				p = _CGI_PackValue(opt,p,ValueArrayItem(value,i),
								   name+strlen(name),longname,buff);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				sprintf(name,".%s",ValueRecordName(value,i));
				p = _CGI_PackValue(opt,p,ValueRecordItem(value,i),
								   name+strlen(name),longname,buff);
			}
			break;
		  default:
			break;
		}
	}
	return	(p);
}

extern	char	*
CGI_PackValue(
	CONVOPT	*opt,
	char	*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF]
	,		longname[SIZE_LONGNAME+1];
	char	*q;

	memclear(longname,SIZE_LONGNAME);
	if		(  opt->recname  !=  NULL  ) {
		strcpy(longname,opt->recname);
	}
	q = _CGI_PackValue(opt,p,value,(longname + strlen(longname)),longname,buff);
	if		(  q  >  p  ) {
		*(q-1) = 0;
	}
	return	(q);
}

static	size_t
_CGI_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value,
	char		*name,
	char		*longname)
{
	int		i;
	size_t	ret;

dbgmsg(">_CGI_SizeValue");
	if		(  value  ==  NULL  )	return	(0);
	switch	(ValueType(value)) {
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_BOOL:
	  case	GL_TYPE_NUMBER:
	  case	GL_TYPE_INT:
	  case	GL_TYPE_FLOAT:
		ret = EncodeStringLengthURL(ValueToString(value)) + strlen(longname) + 2;
		break;
	  case	GL_TYPE_ARRAY:
		ret = 0;
		for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
			sprintf(name,"[%d]",i);
			ret += _CGI_SizeValue(opt,ValueArrayItem(value,i),name+strlen(name),longname);
		}
		break;
	  case	GL_TYPE_RECORD:
		ret = 0;
		for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
			sprintf(name,".%s",ValueRecordName(value,i));
			ret += _CGI_SizeValue(opt,ValueRecordItem(value,i),name+strlen(name),longname);
		}
		break;
	  case	GL_TYPE_OBJECT:
	  default:
		ret = 0;
		break;
	}
dbgmsg("<_CGI_SizeValue");
	return	(ret);
}

extern	size_t
CGI_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value)
{
	char	longname[SIZE_LONGNAME+1];

	strcpy(longname,opt->recname);
	return	(_CGI_SizeValue(opt,value,(longname + strlen(longname)),longname));
}