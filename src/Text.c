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

static	size_t
_CSV_UnPackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value,
	char		*buff)
{
	int		i;
	char	*q;
	byte	*pp;

	pp = p;
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
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
		  case	GL_TYPE_BOOL:
		  case	GL_TYPE_FLOAT:
		  case	GL_TYPE_NUMBER:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_BYTE:
			SetValueString(value,buff,ConvCodeset(opt));
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p += _CSV_UnPackValue(opt,p,ValueArrayItem(value,i),buff);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p += _CSV_UnPackValue(opt,p,ValueRecordItem(value,i),buff);
			}
			break;
		  case	GL_TYPE_ALIAS:
		  default:
			printf("invalid flag [%d]\n",value->type);
			break;
		}
	}
	return	(p-pp);
}

extern	size_t
CSV_UnPackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF];

	return	(_CSV_UnPackValue(opt,p,value,buff));
}

static	void
CSV_Encode(
	char	*str,
	char	fSsep,
	char	*p)
{
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

static	size_t
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
	char	*pp;

	pp = p;
	if		(  value  !=  NULL  ) {
		switch	(value->type) {
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
		  case	GL_TYPE_TEXT:
		  case	GL_TYPE_BYTE:
		  case	GL_TYPE_BOOL:
			CSV_Encode(ValueToString(value,ConvCodeset(opt)),fSsep,buff);
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
				p += sprintf(p,"\"%s\",",ValueToString(value,ConvCodeset(opt)));
			} else {
				p += sprintf(p,"%s,",ValueToString(value,ConvCodeset(opt)));
			}
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p += __CSV_PackValue(opt,p,ValueArrayItem(value,i),fNsep,fSsep,fCesc,buff);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p += __CSV_PackValue(opt,p,ValueRecordItem(value,i),fNsep,fSsep,fCesc,buff);
			}
			break;
		  case	GL_TYPE_ALIAS:
		  default:
			break;
		}
	}
	return	(p-pp);
}

static	size_t
_CSV_PackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value,
	Bool		fNsep,
	Bool		fSsep,
	Bool		fCesc,
	char		*buff)
{
	size_t	ret;

	ret = __CSV_PackValue(opt,p,value,fNsep,fSsep,fCesc,buff);
	if		(  ret  >  0  ) {
		*(p+ret-1) = 0;
	}
	return	(ret);
}

extern	size_t
CSV1_PackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF+1];
	return	(_CSV_PackValue(opt,p,value,TRUE,TRUE,FALSE,buff));
}

extern	size_t
CSV2_PackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF+1];
	return	(_CSV_PackValue(opt,p,value,FALSE,FALSE,FALSE,buff));
}

extern	size_t
CSV3_PackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF+1];
	return	(_CSV_PackValue(opt,p,value,FALSE,TRUE,FALSE,buff));
}

extern	size_t
CSVE_PackValue(
	CONVOPT		*opt,
	byte		*p,
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
		str = ValueToString(value,ConvCodeset(opt));
		ret = strlen(str) + 1;
		if		(  fSsep  )	{
			ret += 2;
		} else {
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
			if		(  fComma  )	ret += 2;
		}
		break;
	  case	GL_TYPE_NUMBER:
	  case	GL_TYPE_INT:
	  case	GL_TYPE_FLOAT:
		ret = strlen(ValueToString(value,ConvCodeset(opt))) + 1;
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
	  case	GL_TYPE_ALIAS:
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

static	byte	*
RFC822_SkipNext(
	CONVOPT	*opt,
	char	*p)
{
	switch	(opt->encode) {
	  case	STRING_ENCODING_NULL:
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
	*rname = p;
	while	(	(  *p  !=  0     )
			&&	(  *p  !=  '.'   ) )	p ++;
	if		(  *p  !=  0  ) {
		*p = 0;
		p ++;
	}
	*vname = p;
	if		(  *p  !=  0  ) {
		while	(	(  *p  !=  0     )
				&&	(  !isspace(*p)  ) )	p ++;
	}
	*p = 0;
}

static	size_t
EncodeString(
	CONVOPT		*opt,
	byte		*out,
	char		*in)
{
	size_t	result;

	switch	(ConvEncoding(opt)) {
	  case	STRING_ENCODING_NULL:
		strcpy(out,in);
		result = strlen(out);
		break;
	  case	STRING_ENCODING_URL:
		result = EncodeStringURL(out,in);
		break;
	  case	STRING_ENCODING_BASE64:
		result = EncodeBase64(out,in,strlen(in));
		break;
	  default:
		result = 0;
		break;
	}
	return	(result);
}

static	size_t
EncodeLength(
	CONVOPT		*opt,
	char		*in)
{
	size_t	result;

	switch	(ConvEncoding(opt)) {
	  case	STRING_ENCODING_NULL:
		result = strlen(in);
		break;
	  case	STRING_ENCODING_URL:
		result = EncodeStringLengthURL(in);
		break;
	  case	STRING_ENCODING_BASE64:
		result = EncodeLengthBase64(in);
		break;
	  default:
		result = 0;
		break;
	}
	return	(result);
}

static	size_t
DecodeString(
	CONVOPT		*opt,
	byte		*out,
	char		*in)
{
	size_t	result;

	switch	(ConvEncoding(opt)) {
	  case	STRING_ENCODING_NULL:
		strcpy(out,in);
		result = strlen(out);
		break;
	  case	STRING_ENCODING_URL:
		result = DecodeStringURL(out,in);
		break;
	  case	STRING_ENCODING_BASE64:
		result = DecodeBase64(out,in,strlen(in));
		break;
	  default:
		result = 0;
		break;
	}
	return	(result);
}

static	size_t
_RFC822_UnPackValueNoNamed(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value,
	char		*buff)
{
	int		i;
	byte	*q
	,		*pp
	,		ch;
	size_t	len;

	pp = p;
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
			ch = *p;
			*p = 0;
			len = DecodeString(opt,buff,q);
			*p = ch;
			buff[len] = 0;
			SetValueString(value,buff,ConvCodeset(opt));
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
			SetValueString(value,q,ConvCodeset(opt));
			*p = ch;
			p ++;
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				p += _RFC822_UnPackValueNoNamed(opt,p,ValueArrayItem(value,i),buff);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p += _RFC822_UnPackValueNoNamed(opt,p,ValueRecordItem(value,i),buff);
			}
			break;
		  case	GL_TYPE_ALIAS:
		  default:
			break;
		}
	}
	return	(p-pp);
}

static	size_t
_RFC822_UnPackValueNamed(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value,
	char		*buff)
{
	char	str[SIZE_LONGNAME+1];
	char	*vname
	,		*rname;
	byte	*q
	,		*pp
	,		ch;
	ValueStruct	*e;
	size_t	len;

	pp = p;
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
						ch = *p;
						*p = 0;
						len = DecodeString(opt,buff,q);
						*p = ch;
						buff[len] = 0;
						SetValueString(e,buff,ConvCodeset(opt));
						break;
					  case	GL_TYPE_BOOL:
					  case	GL_TYPE_NUMBER:
					  case	GL_TYPE_INT:
					  case	GL_TYPE_FLOAT:
						ch = *p;
						*p = 0;
						SetValueString(e,q,ConvCodeset(opt));
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
	return	(p-pp);
}

extern	size_t
RFC822_UnPackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value)
{
	size_t	ret;
	char	buff[SIZE_BUFF];

	if		(  opt->fName  ) {
		ret = _RFC822_UnPackValueNamed(opt,p,value,buff);
	} else {
		ret = _RFC822_UnPackValueNoNamed(opt,p,value,buff);
	}
	return	(ret);
}

static	size_t
_RFC822_PackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value,
	char		*name,
	char		*longname,
	char		*buff)
{
	int		i;
	char	*str;
	byte	*pp;

	pp = p;
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
			str = ValueToString(value,ConvCodeset(opt));
			EncodeString(opt,buff,str);
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
			p += sprintf(p,"%s\n",ValueToString(value,ConvCodeset(opt)));
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				sprintf(name,"[%d]",i);
				p += _RFC822_PackValue(opt,p,ValueArrayItem(value,i),
									  name+strlen(name),longname,buff);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				sprintf(name,".%s",ValueRecordName(value,i));
				p += _RFC822_PackValue(opt,p,ValueRecordItem(value,i),
									  name+strlen(name),longname,buff);
			}
			break;
		  case	GL_TYPE_ALIAS:
		  default:
			break;
		}
	}
	*p = 0;
	return	(p-pp);
}

extern	size_t
RFC822_PackValue(
	CONVOPT	*opt,
	byte		*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF]
	,		longname[SIZE_LONGNAME+1];
	size_t	ret;

	memclear(longname,SIZE_LONGNAME);
	if		(  opt->recname  !=  NULL  ) {
		strcpy(longname,opt->recname);
	}
	ret = _RFC822_PackValue(opt,p,value,NULL,longname,buff);
	if		(  ret  >  0  ) {
		*(p+ret-1) = 0;
	}
	return	(ret);
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
		ret += EncodeLength(opt,ValueToString(value,ConvCodeset(opt)));
		break;
	  case	GL_TYPE_BOOL:
	  case	GL_TYPE_NUMBER:
	  case	GL_TYPE_INT:
	  case	GL_TYPE_FLOAT:
		ret = strlen(ValueToString(value,ConvCodeset(opt))) + 1;
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
	  case	GL_TYPE_ALIAS:
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

static	byte	*
CGI_SkipNext(
	CONVOPT	*opt,
	char	*p)
{
	while	(	(  *p  !=  0     )
			&&	(  *p  !=  '&'   )
			&&	(  *p  !=  '\n'  ) )	p ++;
	if		(  *p  !=  0  )	p ++;
	return	(p);
}

static	size_t
_CGI_UnPackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF+1];
	char	str[SIZE_LONGNAME+1];
	char	*vname
	,		*rname;
	char	*q
	,		ch;
	byte	*pp;
	ValueStruct	*e;

	pp = p;
	if		(  value  !=  NULL  ) {
		while	(  *p  !=  0  ) {
			q = str;
			while	(	(  *p  !=  0    )
					&&	(  *p  !=  '='  ) ) {
				*q ++ = *p ++;
			}
			*q = 0;
			if		(  *p  !=  0  )	p ++;
			DecodeName(&rname,&vname,str);
			if		(  strcmp(rname,opt->recname)  !=  0  ) {
				p = CGI_SkipNext(opt,p);
			} else {
				if		(  ( e = GetItemLongName(value,vname) )  !=  NULL  ) {
					q = p;
					while	(	(  *p  !=  0     )
							&&	(  *p  !=  '&'   )
							&&	(  *p  !=  '\n'  ) )	p ++;
					ch = *p;
					*p = 0;
					DecodeString(opt,buff,q);
					*p = ch;
					if		(  *p  !=  0  )	p ++;
					SetValueString(e,buff,ConvCodeset(opt));
				} else {
					p = CGI_SkipNext(opt,p);
				}
			}
		}
	}					
	return	(p-pp);
}

extern	size_t
CGI_UnPackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value)
{
	size_t	ret;

	ConvSetEncoding(opt,STRING_ENCODING_URL);
	ret = _CGI_UnPackValue(opt,p,value);
	return	(ret);
}

static	size_t
_CGI_PackValue(
	CONVOPT	*opt,
	byte		*p,
	ValueStruct	*value,
	char		*name,
	char		*longname)
{
	int		i;
	byte	*q
	,		*pp;

	pp = p;
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
			q = ValueToString(value,ConvCodeset(opt));
			p += sprintf(p,"%s=",longname);
#if	1
			p += EncodeString(opt,p,q);
#else
			while	(  *q  !=  0  ) {
				if		(  *q  ==  0x20  ) {
					*p ++ = '+';
				} else
				if		(  isalnum(*q)  ) {
					*p ++ = *q;
				} else {
					*p ++ = '%';
					p += sprintf(p,"%02X",(int)(*q & 0xFF));
				}
				q ++;
			}
#endif
			*p ++ = '&';
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				sprintf(name,"[%d]",i);
				p += _CGI_PackValue(opt,p,ValueArrayItem(value,i),
								   name+strlen(name),longname);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				sprintf(name,".%s",ValueRecordName(value,i));
				p += _CGI_PackValue(opt,p,ValueRecordItem(value,i),
								   name+strlen(name),longname);
			}
			break;
		  case	GL_TYPE_ALIAS:
		  default:
			break;
		}
	}
	return	(p-pp);
}

extern	size_t
CGI_PackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value)
{
	char	longname[SIZE_LONGNAME+1];
	size_t	ret;

	ConvSetEncoding(opt,STRING_ENCODING_URL);
	memclear(longname,SIZE_LONGNAME);
	if		(  opt->recname  !=  NULL  ) {
		strcpy(longname,opt->recname);
	}
	ret = _CGI_PackValue(opt,p,value,(longname + strlen(longname)),longname);
	if		(  ret  >  0  ) {
		*(p+ret-1) = 0;
	}
	return	(ret);
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
		ret = EncodeLength(opt,ValueToString(value,ConvCodeset(opt))) + strlen(longname) + 2;
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
	  case	GL_TYPE_ALIAS:
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

	ConvSetEncoding(opt,STRING_ENCODING_URL);
	strcpy(longname,opt->recname);
	return	(_CGI_SizeValue(opt,value,(longname + strlen(longname)),longname));
}
