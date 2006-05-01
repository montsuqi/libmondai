/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2003 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2004-2006 Ogochan.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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
#define	_VALUECONV
#include	"types.h"
#include	"misc_v.h"
#include	"memory_v.h"
#include	"value.h"
#include	"hash_v.h"
#include	"OpenCOBOL_v.h"
#include	"dotCOBOL_v.h"
#include	"Native_v.h"
#include	"Text_v.h"
#include	"XML_v.h"
#include	"valueconv.h"
#include	"debug.h"

static	ConvFuncs	funcs[] = {

	{	"OpenCOBOL",			TRUE,	"",						"",
		OpenCOBOL_PackValue,	OpenCOBOL_UnPackValue,	OpenCOBOL_SizeValue	},

	{	"dotCOBOL",				TRUE,	"",						"",
		dotCOBOL_PackValue,		dotCOBOL_UnPackValue,	dotCOBOL_SizeValue	},

	{	"CSV1",					FALSE,	",",					"\n",
		CSV1_PackValue,			CSV_UnPackValue,		CSV1_SizeValue		},
	{	"CSV2",					FALSE,	",",					"\n",
		CSV2_PackValue,			CSV_UnPackValue,		CSV3_SizeValue		},
	{	"CSV3",					FALSE,	",",					"\n",
		CSV3_PackValue,			CSV_UnPackValue,		CSV3_SizeValue		},
	{	"CSVE",					FALSE,	",",					"\n",
		CSVE_PackValue,			CSV_UnPackValue,		CSVE_SizeValue		},
	{	"CSV",					FALSE,	",",					"\n",
		CSV_PackValue,			CSV_UnPackValue,		CSV_SizeValue		},
	{	"RFC822",				FALSE,	"\n",					"\n",
		RFC822_PackValue,		RFC822_UnPackValue,		RFC822_SizeValue	},

	{	"SQL",					FALSE,	",",					"\n",
		SQL_PackValue,			SQL_UnPackValue,		SQL_SizeValue		},

	{	"CGI",					FALSE,	"&",					"\n",
		CGI_PackValue,			CGI_UnPackValue,		CGI_SizeValue		},

#ifdef	USE_XML
	{	"XML",					FALSE,	"\n",					"\n",
		XML_PackValue,			XML_UnPackValue,		XML_SizeValue		},
	{	"XML1",					FALSE,	"\n",					"\n",
		XML1_PackValue,			XML2_UnPackValue,		XML2_SizeValue		},
	{	"XML2",					FALSE,	"\n",					"\n",
		XML2_PackValue,			XML2_UnPackValue,		XML2_SizeValue		},
#endif

	{	"Native",				TRUE,	"",						"",
		NativePackValue,		NativeUnPackValue,		NativeSizeValue		},

	{	NULL,					TRUE,	"",						"",
		NativePackValue,		NativeUnPackValue,		NativeSizeValue		}
};

static	GHashTable	*FuncTable = NULL;

extern	ConvFuncs	*
GetConvFunc(
	char	*name)
{
	ConvFuncs	*func;
	int			i;

	if		(  name  !=  NULL  ) {
		if		(  FuncTable  ==  NULL  ) {
			FuncTable = NewNameHash();
			for	( i = 0 ; funcs[i].name  !=  NULL ; i ++ ) {
				if		(  g_hash_table_lookup(FuncTable,funcs[i].name)  ==  NULL  ) {
					g_hash_table_insert(FuncTable,funcs[i].name,&funcs[i]);
				}
			}
		}
		func = (ConvFuncs *)g_hash_table_lookup(FuncTable,name);
	} else {
		func = NULL;
	}
	return	(func);
}

extern	void
ConvSetLanguage(
	char	*name)
{
	ConvFuncs	*func;

dbgmsg(">SetLanguage");
	if		(  name  !=  NULL  ) {
		if		(  ( func = GetConvFunc(name) )  ==  NULL  ) {
			fprintf(stderr,"can not found %s convert rule\n",name);
			exit(1);
		}
		PackValue = func->PackValue;
		UnPackValue = func->UnPackValue;
		SizeValue = func->SizeValue;
	} else {
		PackValue = NULL;
		UnPackValue = NULL;
		SizeValue = NULL;
	}
dbgmsg("<SetLanguage");
}

extern	CONVOPT	*
NewConvOpt(void)
{
	CONVOPT	*ret;

	ret = New(CONVOPT);
	ret->codeset = NULL;
	ret->recname = NULL;
	ret->textsize = 100;
	ret->arraysize = 10;
	ret->encode = STRING_ENCODING_NULL;
	ret->appendix = NULL;

	return	(ret);
}

extern	void
DestroyConvOpt(
	CONVOPT	*opt)
{
	xfree(opt);
}
