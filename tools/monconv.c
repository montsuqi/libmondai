/*	PANDA -- a simple transaction monitor

Copyright (C) 2004 Ogochan.

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

#define	MAIN

/*
#define	DEBUG
#define	TRACE
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include	<stdio.h>
#include	<sys/mman.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<glib.h>
#include	"types.h"
#include	"libmondai.h"
#include	"RecParser.h"
#include	"option.h"
#include	"debug.h"

static	size_t	InArraySize;
static	size_t	OutArraySize;
static	size_t	InTextSize;
static	size_t	OutTextSize;
static	char	*InLang;
static	char	*OutLang;
static	char	*InCode;
static	char	*OutCode;
static	char	*InEncode;
static	char	*OutEncode;
static	size_t	SizeBuff;

static	ARG_TABLE	option[] = {
	{	"inlang",	STRING,		TRUE,	(void*)&InLang	,
		"入力形式名"			 						},
	{	"outlang",	STRING,		TRUE,	(void*)&OutLang	,
		"出力形式名"			 						},
	{	"incode",	STRING,		TRUE,	(void*)&InCode	,
		"入力文字コード" 								},
	{	"outcode",	STRING,		TRUE,	(void*)&OutCode	,
		"出力文字コード" 								},

	{	"inencode",	STRING,		TRUE,	(void*)&InEncode,
		"入力文字列encoding" 							},
	{	"outencode",STRING,		TRUE,	(void*)&OutEncode,
		"出力文字列encoding" 							},

	{	"intextsize",INTEGER,	TRUE,	(void*)&InTextSize,
		"textの最大長"									},
	{	"inarraysize",INTEGER,	TRUE,	(void*)&InArraySize,
		"可変要素配列の最大繰り返し数"					},

	{	"outtextsize",INTEGER,	TRUE,	(void*)&OutTextSize,
		"textの最大長"									},
	{	"outarraysize",INTEGER,	TRUE,	(void*)&OutArraySize,
		"可変要素配列の最大繰り返し数"					},

	{	NULL,		0,			FALSE,	NULL,	NULL 	}
};

static	void
SetDefault(void)
{
	InArraySize = 10;
	InTextSize = 100;
	OutArraySize = 10;
	OutTextSize = 100;

	InLang = "CSVE";
	OutLang = "CSV3";
	InCode = "euc-jp";
	OutCode = "euc-jp";

	InEncode = "null";
	OutEncode = "null";

	SizeBuff = 1024 * 1024;
}

extern	int
main(
	int		argc,
	char	**argv)
{
	FILE_LIST	*fl;
	struct	stat	sb;
	char		*out
		,		*p;
	ConvFuncs	*infunc
		,		*outfunc;
	CONVOPT		*inopt
		,		*outopt;
	ValueStruct	*rec;
	size_t		isize
		,		osize;
	ssize_t		left;
	int			type;
	char		*ValueName;

	SetDefault();
	fl = GetOption(option,argc,argv,
				   "\tLang can accept\n"
				   "\tOpenCOBOL\n"
				   "\tdotCOBOL\n"
				   "\tCSV1(number and string delimited by \"\")\n"
				   "\tCSV2(number and string *not* delimitd by \"\")\n"
				   "\tCSV3(string delimitd by \"\")\n"
				   "\tCSVE(Excel type CSV)\n"
				   "\tCSV(same as CSV3)\n"
				   "\tSQL(SQL type)\n"
				   "\tXML1(tag is data class)\n"
				   "\tXML2(tag is data name)\n"
				   "\tCGI('name=value&...)\n"
				   "\tRFC822('name: value\\n')\n"
		);

	RecParserInit();
	if		(	(  fl->name  ==  NULL  )
			||	(  ( rec = RecParseValue(fl->name,&ValueName) )  ==  NULL  ) ) {
		fprintf(stderr,"can not found %s input record define.\n",fl->name);
		exit(1);
	}
	if		(  !strlicmp(InLang,"xml")  ) {
		if		(  !strlicmp(InLang,"xml2")  ) {
			type = XML_TYPE2;
		} else {
			type = XML_TYPE1;
		}
		InLang = "XML";
	}
	if		(  ( infunc = GetConvFunc(InLang) )  ==  NULL  ) {
		fprintf(stderr,"can not found %s convert rule\n",InLang);
		exit(1);
	} else {
		inopt = NewConvOpt();
		ConvSetCodeset(inopt,InCode);
		ConvSetSize(inopt,InTextSize,InArraySize);
		if		(  !stricmp(InEncode,"url")  ) {
			ConvSetEncoding(inopt,STRING_ENCODING_URL);
		} else
		if		(  !stricmp(InEncode,"cgi")  ) {
			ConvSetEncoding(inopt,STRING_ENCODING_BASE64);
		} else {
			ConvSetEncoding(inopt,STRING_ENCODING_NULL);
		}
		if		(  !stricmp(InLang,"xml")  ) {
			ConvSetXmlType(inopt,type);
			ConvSetIndent(inopt,TRUE);
			ConvSetType(inopt,TRUE);
		}
		ConvSetRecName(inopt,StrDup(ValueName));
		ConvSetUseName(inopt,TRUE);
	}
	if		(  !strlicmp(OutLang,"xml")  ) {
		if		(  !stricmp(OutLang,"xml2")  ) {
			type = XML_TYPE2;
		} else {
			type = XML_TYPE1;
		}
		OutLang = "XML";
	}
	if		(  ( outfunc = GetConvFunc(OutLang) )  ==  NULL  ) {
		fprintf(stderr,"can not found %s convert rule\n",OutLang);
		exit(1);
	} else {
		outopt = NewConvOpt();
		ConvSetCodeset(outopt,OutCode);
		ConvSetSize(outopt,OutTextSize,OutArraySize);
		if		(  !stricmp(OutEncode,"url")  ) {
			ConvSetEncoding(outopt,STRING_ENCODING_URL);
		} else
		if		(  !stricmp(OutEncode,"cgi")  ) {
			ConvSetEncoding(outopt,STRING_ENCODING_BASE64);
		} else {
			ConvSetEncoding(outopt,STRING_ENCODING_NULL);
		}
		if		(  !stricmp(OutLang,"xml")  ) {
			ConvSetXmlType(outopt,type);
			ConvSetIndent(outopt,TRUE);
			ConvSetType(outopt,TRUE);
		}
		ConvSetRecName(outopt,StrDup(ValueName));
		ConvSetUseName(outopt,TRUE);
	}

	fstat(fileno(stdin),&sb);
	if		(  ( p = mmap(NULL,sb.st_size,PROT_READ,MAP_PRIVATE,fileno(stdin),0) )
			   !=  NULL  ) {
		left = sb.st_size;
		out = (char *)xmalloc(SizeBuff);
		while	(  left  >  0  ) {
			isize = infunc->UnPackValue(inopt,p,rec);
			left -= isize + strlen(infunc->bsep);
			p += isize + strlen(infunc->bsep);
			osize = outfunc->PackValue(outopt,out,rec);
			if		(  strlen(outfunc->bsep)  >  0  ) {
				fwrite(out,osize-strlen(outfunc->bsep),1,stdout);
				fwrite(outfunc->bsep,strlen(outfunc->bsep),1,stdout);
			} else {
				fwrite(out,osize,1,stdout);
			}
		}
		munmap(p,sb.st_size);
	}

	return	(0);
}
