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
#define	_VALUECONV
#include	"types.h"
#include	"misc.h"
#include	"value.h"
#include	"hash.h"
#include	"OpenCOBOL_v.h"
#include	"dotCOBOL_v.h"
#include	"Native_v.h"
#include	"Text_v.h"
#include	"XML_v.h"
#include	"valueconv.h"
#include	"debug.h"

static	ConvFuncs	funcs[] = {

	{	"OpenCOBOL",			"",						"",
		OpenCOBOL_PackValue,	OpenCOBOL_UnPackValue,	OpenCOBOL_SizeValue	},

	{	"dotCOBOL",				"",						"",
		dotCOBOL_PackValue,		dotCOBOL_UnPackValue,	dotCOBOL_SizeValue	},

	{	"CSV1",					",",					"\n",
		CSV1_PackValue,			CSV_UnPackValue,		CSV1_SizeValue		},
	{	"CSV2",					",",					"\n",
		CSV2_PackValue,			CSV_UnPackValue,		CSV3_SizeValue		},
	{	"CSV3",					",",					"\n",
		CSV3_PackValue,			CSV_UnPackValue,		CSV3_SizeValue		},
	{	"CSVE",					",",					"\n",
		CSVE_PackValue,			CSV_UnPackValue,		CSVE_SizeValue		},
	{	"CSV",					",",					"\n",
		CSV_PackValue,			CSV_UnPackValue,		CSV_SizeValue		},
	{	"RFC822",				"\n",					"\n",
		RFC822_PackValue,		RFC822_UnPackValue,		RFC822_SizeValue	},

	{	"CGI",					"&",					"\n",
		CGI_PackValue,			CGI_UnPackValue,		CGI_SizeValue		},

	{	"XML",					"\n",					"\n",
		XML_PackValue,			XML_UnPackValue,		XML_SizeValue		},

	{	NULL,					"",						"",
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
	ret->encode = STRING_ENCODING_URL;
	ret->appendix = NULL;

	return	(ret);
}

extern	void
DestroyConvOpt(
	CONVOPT	*opt)
{
	xfree(opt);
}
