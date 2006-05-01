/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2002 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2003-2006 Ogochan.
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
*/
#define	DEBUG
#define	TRACE

#define	MAIN
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define	TEST_CODE		"euc-jp"
#define	SRC_CODE		"euc-jp"

#define	TEST_VALUE

/*
#define	TEST_CODE		"shift-jis"
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<glib.h>
#include	"types.h"

#include	"value.h"
#include	"getset.h"
#include	"Native_v.h"
#include	"Text_v.h"
#include	"XML_v.h"
#include	"OpenCOBOL_v.h"
#include	"valueconv.h"
#include	"RecParser.h"
#include	"numerici.h"
#include	"memory_v.h"
#include	"misc_v.h"
#include	"others.h"

#include	"debug.h"

#define	SIZE_FUNC		16
#define	SIZE_EVENT		64
#define	SIZE_STATUS		4
#define	SIZE_PUTTYPE	8
#define	SIZE_TERM		64
#define	SIZE_USER		16

static	ValueStruct	*
BuildMcpArea(
	size_t	stacksize)
{
	ValueStruct	*value;
	char	buff[SIZE_BUFF];
	char	*p;

ENTER_FUNC;
	p = buff;
	p += sprintf(p,
"/*\n"
"libmondai -- MONTSUQI data access library\n"
"Copyright (C) 2000-2002 Ogochan & JMA (Japan Medical Association).\n"
"Copyright (C) 2003-2005 Ogochan.\n"
"\n"
"This library is free software; you can redistribute it and/or\n"
"modify it under the terms of the GNU Lesser General Public\n"
"License as published by the Free Software Foundation; either\n"
"version 2 of the License, or (at your option) any later version.\n"
"\n"
"This library is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
"Lesser General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU Lesser General Public\n"
"License along with this library; if not, write to the\n"
"Free Software Foundation, Inc., 59 Temple Place - Suite 330,\n"
"Boston, MA 02111-1307, USA.\n"
"*/\n");
	p += sprintf(p,	"mcparea	{");
	p += sprintf(p,		"func varchar(%d);",SIZE_FUNC);
	p += sprintf(p,		"obj object;");
	p += sprintf(p,		"bin binary;");
	p += sprintf(p,		"rc int;");
	p += sprintf(p,		"dc	{	/* .... */ ");
	p += sprintf(p,			"window	 varchar(%d);",4);
	p += sprintf(p,			"widget	 varchar(%d);",SIZE_NAME);
	p += sprintf(p,			"event	 varchar(%d);",SIZE_EVENT);
	p += sprintf(p,			"module	 varchar(%d);",SIZE_NAME);
	p += sprintf(p,			"fromwin varchar(%d);",SIZE_NAME);
	p += sprintf(p,			"status	 varchar(%d);",SIZE_STATUS);
	p += sprintf(p,			"puttype varchar(%d);",SIZE_PUTTYPE);
	p += sprintf(p,			"term	 varchar(%d);",SIZE_TERM);
	p += sprintf(p,			"user	 varchar(%d);",SIZE_USER);
	p += sprintf(p,		"};");
	p += sprintf(p,		"db	{");
	p += sprintf(p,			"path	{");
	p += sprintf(p,				"blocks	integer;");
	p += sprintf(p,				"rname	int;");
	p += sprintf(p,				"pname	int;");
	p += sprintf(p,				"name	varchar(%d);",SIZE_NAME);
	p += sprintf(p,			"};");
	p += sprintf(p,		"};");
	p += sprintf(p,		"private	{");
	p += sprintf(p,			"count	int;");
	p += sprintf(p,			"swindow	char(%d)[];",SIZE_NAME);
	p += sprintf(p,			"state		char(1)[%d];",stacksize);
	p += sprintf(p,			"index		int[%d];",stacksize);
	p += sprintf(p,			"pstatus	char(1);");
	p += sprintf(p,			"pputtype 	char(1);");
	p += sprintf(p,			"prc		char(1);");
	p += sprintf(p,		"};");
	p += sprintf(p,     "aliases    {");
	p += sprintf(p,     "  foo = db.path.blocks;");
	p += sprintf(p,     "  baa = private.count;");
	p += sprintf(p,		"};");
	p += sprintf(p,	"};");

	value = RecParseValueMem(buff,NULL);
LEAVE_FUNC;
	return	(value);
}

extern	int
main(
	int		argc,
	char	**argv)
{
#define	SIZE_DATA		128

	char	name[SIZE_DATA];
	byte	*p;
	int		i;
	ValueStruct	*val
		,		*val2
		,		*val3;
	CONVOPT	*opt;
	byte	*buff;
	size_t	size;

	printf("***** libmondai test start *****\n");
	RecordDir = ".";
	printf("***** RecParserInit *****\n");
	RecParserInit();
	printf("***** RecParseValue *****\n");
	val = BuildMcpArea(10);

	DumpValueStruct(val);
	/*	set	*/
	printf("***** Value setting *****\n");
	SetValueString(GetItemLongName(val,"func"),"aaa",SRC_CODE);
	SetValueString(GetItemLongName(val,"rc"),"0",SRC_CODE);

	SetValueString(GetItemLongName(val,"dc.window"),"1 男",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.widget"),"widget",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.event"),"1002 医療機関情報ー所在地、連絡先",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.fromwin"),"fromwin",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.status"),"status",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.puttype"),"puttype",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.term"),"term",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.user"),"user",SRC_CODE);

	SetValueString(GetItemLongName(val,"db.path.blocks"),"1",SRC_CODE);
	SetValueString(GetItemLongName(val,"db.path.rname"),"2",SRC_CODE);
	SetValueString(GetItemLongName(val,"db.path.pname"),"3",SRC_CODE);

	SetValueString(GetItemLongName(val,"private.count"),"1",SRC_CODE);
printf("** variable size **\n");fflush(stdout);
	for	( i = 0 ; i < 100 ; i ++ ) {
		sprintf(name,"private.swindow[%d]",i);
printf("private.swindow[%d]\n",i);fflush(stdout);
		SetValueString(GetItemLongName(val,name),name,SRC_CODE);
	}
printf("** variable size (end)**\n");fflush(stdout);
	for	( i = 0 ; i < ValueArraySize(GetItemLongName(val,"private.state")) ; i ++ ) {
		sprintf(name,"private.state[%d]",i);
		SetValueString(GetItemLongName(val,name),name,SRC_CODE);
	}
	for	( i = 0 ; i < ValueArraySize(GetItemLongName(val,"private.index")) ; i ++ ) {
		sprintf(name,"private.index[%d]",i);
		SetValueString(GetItemLongName(val,name),name,SRC_CODE);
	}
	SetValueString(GetItemLongName(val,"private.pstatus"),"1",SRC_CODE);
	SetValueString(GetItemLongName(val,"private.pputtype"),"2",SRC_CODE);
	SetValueString(GetItemLongName(val,"private.prc"),"3",SRC_CODE);
	buff = xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);
	for	( p = buff, i = 0 ; i < 256 ; i ++ , p ++) {
		*p = (byte)i;
	}
	SetValueBinary(GetItemLongName(val,"bin"),buff,256);
	printf("***** Value setting (end)*****\n");
		

	buff = xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);
	printf("***** Value Save (structure only)*****\n");
	size = NativeSaveValue(buff,val,FALSE);
	printf("***** Value Save (end)*****\n");
	printf("***** Value Restore *****\n");
	val2 = NativeRestoreValue(buff,FALSE);
	printf("***** Value Restore (end)*****\n");

	opt = NewConvOpt();
	ConvSetCodeset(opt,TEST_CODE);

	ConvSetXmlType(opt,XML_TYPE1);
	ConvSetIndent(opt,TRUE);
	ConvSetType(opt,FALSE);
	ConvSetRecName(opt,"mcparea");

	printf("***** before pack ****\n");
	XML_PackValue(opt,buff,val);
	printf("%s\n",buff);
	printf("***** after pack ****\n");
	XML_PackValue(opt,buff,val2);
	printf("%s\n",buff);

	memset(buff,0,SIZE_BUFF);
	printf("***** Value Save (with data)*****\n");
	size = NativeSaveValue(buff,val,TRUE);
	printf("***** Value Save (end)*****\n");
	printf("***** Value Restore *****\n");
	val2 = NativeRestoreValue(buff,TRUE);
	printf("***** Value Restore (end)*****\n");

	opt = NewConvOpt();
	ConvSetCodeset(opt,TEST_CODE);

	ConvSetXmlType(opt,XML_TYPE1);
	ConvSetIndent(opt,TRUE);
	ConvSetType(opt,FALSE);
	ConvSetRecName(opt,"mcparea");

	printf("***** after pack ****\n");
	XML_PackValue(opt,buff,val2);
	printf("%s\n",buff);


	printf("***** Value duplicate *****\n");
	val3 = DuplicateValue(val);
	printf("***** Value duplicate (end)*****\n");

	printf("***** after duplicate ****\n");
	memset(buff,0,SIZE_BUFF);
	XML_PackValue(opt,buff,val3);
	printf("%s\n",buff);

	return	(0);
}
