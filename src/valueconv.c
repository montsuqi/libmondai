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
#include	"Native_v.h"
#define	_VALUECONV
#include	"valueconv.h"
#include	"debug.h"


typedef	struct {
	char	*name;
	char	*(*Pack)(char *p, ValueStruct *value, size_t textsize);
	char	*(*UnPack)(char *p, ValueStruct *value, size_t textsize);
	size_t	(*Size)(ValueStruct *value, size_t arraysize, size_t textsize);
}	ConvFuncs;

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
