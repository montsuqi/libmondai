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
#include	"OpenCOBOL_v.h"
#include	"dotCOBOL_v.h"
#define	_VALUECONV
#include	"valueconv.h"
#include	"debug.h"


typedef	struct {
	char	*name;
	char	*(*Pack)(char *p, ValueStruct *value, size_t textsize);
	char	*(*UnPack)(char *p, ValueStruct *value, size_t textsize);
	size_t	(*Size)(ValueStruct *value, size_t arraysize, size_t textsize);
}	ConvFuncs;

extern	char	*
NativeUnPackValue(
	char	*p,
	ValueStruct	*value,
	size_t	textsize)
{
	int		i;
	size_t	size;

dbgmsg(">NativeUnPackValue");
	if		(  value  !=  NULL  ) {
		switch	(value->type) {
		  case	GL_TYPE_INT:
			memcpy(&value->body.IntegerData,p,sizeof(int));
			p += sizeof(int);
			break;
		  case	GL_TYPE_BOOL:
			value->body.BoolData = ( *(char *)p == 'T' ) ? TRUE : FALSE;
			p ++;
			break;
		  case	GL_TYPE_BYTE:
			memcpy(value->body.CharData.sval,p,value->body.CharData.len);
			p += value->body.CharData.len;
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			memcpy(value->body.CharData.sval,p,value->body.CharData.len);
			value->body.CharData.sval[value->body.CharData.len] = 0;
			p += value->body.CharData.len;
			break;
		  case	GL_TYPE_TEXT:
			memcpy(&size,p,sizeof(size_t));
			p += sizeof(size_t);
			if		(  size  !=  value->body.CharData.len  ) {
				xfree(value->body.CharData.sval);
				value->body.CharData.sval = (char *)xmalloc(size+1);
				value->body.CharData.len = size;
			}
			memcpy(value->body.CharData.sval,p,size);
			p += size;
			break;
		  case	GL_TYPE_NUMBER:
			memcpy(value->body.FixedData.sval,p,value->body.FixedData.flen);
			p += value->body.FixedData.flen;
			break;
		  case	GL_TYPE_OBJECT:
			memcpy(&value->body.Object.apsid,p,sizeof(int));
			p += sizeof(int);
			memcpy(&value->body.Object.oid,p,sizeof(int));
			p += sizeof(int);
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < value->body.ArrayData.count ; i ++ ) {
				p = NativeUnPackValue(p,value->body.ArrayData.item[i],textsize);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < value->body.RecordData.count ; i ++ ) {
				p = NativeUnPackValue(p,value->body.RecordData.item[i],textsize);
			}
			break;
		  default:
			break;
		}
	}
dbgmsg("<NativeUnPackValue");
	return	(p);
}

extern	char	*
NativePackValue(
	char	*p,
	ValueStruct	*value,
	size_t	textsize)
{
	int		i;

dbgmsg(">NativePackValue");
	if		(  value  !=  NULL  ) {
		switch	(value->type) {
		  case	GL_TYPE_INT:
			memcpy(p,&value->body.IntegerData,sizeof(int));
			p += sizeof(int);
			break;
		  case	GL_TYPE_BOOL:
			*(char *)p = value->body.BoolData ? 'T' : 'F';
			p ++;
			break;
		  case	GL_TYPE_BYTE:
			memcpy(p,value->body.CharData.sval,value->body.CharData.len);
			p += value->body.CharData.len;
			break;
		  case	GL_TYPE_CHAR:
		  case	GL_TYPE_VARCHAR:
		  case	GL_TYPE_DBCODE:
			memcpy(p,value->body.CharData.sval,value->body.CharData.len);
			p += value->body.CharData.len;
			break;
		  case	GL_TYPE_TEXT:
			memcpy(p,&value->body.CharData.len,sizeof(size_t));
			p += sizeof(size_t);
			memcpy(p,value->body.CharData.sval,value->body.CharData.len);
			p += value->body.CharData.len;
			break;
		  case	GL_TYPE_NUMBER:
			memcpy(p,ValueFixed(value)->sval,ValueFixed(value)->flen);
			p += value->body.FixedData.flen;
			break;
		  case	GL_TYPE_OBJECT:
			memcpy(p,&value->body.Object.apsid,sizeof(int));
			p += sizeof(int);
			memcpy(p,&value->body.Object.oid,sizeof(int));
			p += sizeof(int);
			break;
		  case	GL_TYPE_ARRAY:
			for	( i = 0 ; i < value->body.ArrayData.count ; i ++ ) {
				p = NativePackValue(p,value->body.ArrayData.item[i],textsize);
			}
			break;
		  case	GL_TYPE_RECORD:
			for	( i = 0 ; i < value->body.RecordData.count ; i ++ ) {
				p = NativePackValue(p,value->body.RecordData.item[i],textsize);
			}
			break;
		  default:
			break;
		}
	}
dbgmsg("<NativePackValue");
	return	(p);
}

extern	size_t
NativeSizeValue(
	ValueStruct	*val,
	size_t		arraysize,
	size_t		textsize)
{
	int		i
	,		n;
	size_t	ret;

	if		(  val  ==  NULL  )	return	(0);
dbgmsg(">NativeSizeValue");
	switch	(val->type) {
	  case	GL_TYPE_INT:
		ret = sizeof(int);
		break;
	  case	GL_TYPE_FLOAT:
		ret = sizeof(double);
		break;
	  case	GL_TYPE_BOOL:
		ret = 1;
		break;
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
		ret = val->body.CharData.len;
		break;
	  case	GL_TYPE_NUMBER:
		ret = val->body.FixedData.flen;
		break;
	  case	GL_TYPE_TEXT:
		if		(  textsize  >  0  ) {
			ret = textsize;
		} else {
			ret = val->body.CharData.len + sizeof(size_t);
		}
		break;
	  case	GL_TYPE_ARRAY:
		if		(  val->body.ArrayData.count  >  0  ) {
			n = val->body.ArrayData.count;
		} else {
			n = arraysize;
		}
		ret = NativeSizeValue(val->body.ArrayData.item[0],arraysize,textsize) * n;
		break;
	  case	GL_TYPE_RECORD:
		ret = 0;
		for	( i = 0 ; i < val->body.RecordData.count ; i ++ ) {
			ret += NativeSizeValue(val->body.RecordData.item[i],arraysize,textsize);
		}
		break;
	  default:
		ret = 0;
		break;
	}
dbgmsg("<NativeSizeValue");
	return	(ret);
}

static	ConvFuncs	funcs[] = {
	{	"OpenCOBOL",
		OpenCOBOL_PackValue,	OpenCOBOL_UnPackValue,	OpenCOBOL_SizeValue	},
	{	"dotCOBOL",
		dotCOBOL_PackValue,		dotCOBOL_UnPackValue,	dotCOBOL_SizeValue	},
	{	NULL,
		NativePackValue,		NativeUnPackValue,		NativeSizeValue		}
};

static	GHashTable	*FuncTable = NULL;

extern	void
SetLanguage(
	char	*name)
{
	int		i;
	ConvFuncs	*func;

dbgmsg(">SetLanguage");
	if		(  FuncTable  ==  NULL  ) {
		FuncTable = NewNameHash();
		for	( i = 0 ; funcs[i].name  !=  NULL ; i ++ ) {
			if		(  g_hash_table_lookup(FuncTable,funcs[i].name)  ==  NULL  ) {
				g_hash_table_insert(FuncTable,funcs[i].name,&funcs[i]);
			}
		}
	}
	if		(  ( func = (ConvFuncs *)g_hash_table_lookup(FuncTable,name) )  ==  NULL  ) {
		fprintf(stderr,"can not found %s convert rule\n",name);
		exit(1);
	}
	PackValue = func->Pack;
	UnPackValue = func->UnPack;
	SizeValue = func->Size;
dbgmsg("<SetLanguage");
}
