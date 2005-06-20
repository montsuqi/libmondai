/*	PANDA -- a simple transaction monitor

Copyright (C) 2000-2002 Ogochan & JMA (Japan Medical Association).

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

#define	DEBUG
#define	TRACE
/*
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<glib.h>
#include	"types.h"

#include <libxml/parser.h>
#include <libxml/tree.h>

#include	"value.h"
#include	"getset.h"
#include	"SOAP_v.h"
#include	"RecParser.h"
#include	"numerici.h"
#include	"memory_v.h"
#include	"monstring.h"
#include	"misc_v.h"
#include	"hash_v.h"
#include	"others.h"

#include	"debug.h"

#define	SRC_CODE		"euc-jp"
#define	DUMP_CODE		"utf-8"

#define	SIZE_FUNC		16
#define	SIZE_EVENT		64
#define	SIZE_STATUS		4
#define	SIZE_PUTTYPE	8
#define	SIZE_TERM		64
#define	SIZE_USER		16

#define	N_PATH			5

static	ValueStruct	*
BuildMcpArea(
	size_t	stacksize)
{
	ValueStruct	*value;
	char	buff[SIZE_BUFF];
	char	*p;

	p = buff;
	p += sprintf(p,	"mcparea	{");
	p += sprintf(p,		"func varchar(%d);",SIZE_FUNC);
	p += sprintf(p,		"obj object;");
	p += sprintf(p,		"bin binary;");
	p += sprintf(p,		"rc int;");
	p += sprintf(p,		"dc	{");
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
	p += sprintf(p,				"blocks	int;");
	p += sprintf(p,				"rname	int;");
	p += sprintf(p,				"pname	int;");
	p += sprintf(p,				"name	varchar(%d);",SIZE_NAME);
	p += sprintf(p,			"}[%d];",N_PATH);
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
		,		*val2;
	byte	*buff;
	char	method[SIZE_LONGNAME+1];
	FILE	*fp;

	RecordDir = ".";
	RecParserInit();
	val = BuildMcpArea(10);

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

	for	( i = 0 ; i < N_PATH ; i ++ ) {
		sprintf(name,"db.path[%d].blocks",i);
		SetValueInteger(GetItemLongName(val,name),i);
		sprintf(name,"db.path[%d].rname",i);
		SetValueInteger(GetItemLongName(val,name),i);
		sprintf(name,"db.path[%d].pname",i);
		SetValueInteger(GetItemLongName(val,name),i);
		sprintf(name,"db.path[%d].name",i);
		SetValueString(GetItemLongName(val,name),name,SRC_CODE);
	}

	SetValueString(GetItemLongName(val,"private.count"),"1",SRC_CODE);
	for	( i = 0 ; i < 100 ; i ++ ) {
		sprintf(name,"private.swindow[%d]",i);
		SetValueString(GetItemLongName(val,name),name,SRC_CODE);
	}
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


	printf("***** SOAP ****\n");
	memset(buff,0,SIZE_BUFF);

	SOAP_PackValue(buff,val,"Put","mcp","http://oreore",TRUE,FALSE);

	printf("%s\n",buff);
	fp = fopen("test.SOAP","w");
	fprintf(fp,"%s\n",buff);
	fclose(fp);

	val2 = DuplicateValue(val);

	SOAP_UnPackValue(val2,buff,method);

	printf("method = [%s]\n",method);
	{
		CONVOPT	*opt;

		opt = NewConvOpt();

		ConvSetCodeset(opt,DUMP_CODE);

		ConvSetXmlType(opt,XML_TYPE1);
		ConvSetIndent(opt,TRUE);
		ConvSetType(opt,FALSE);
		ConvSetRecName(opt,"mcparea");

		memset(buff,0,SIZE_BUFF);
		XML_PackValue(opt,buff,val2);
		printf("%s\n",buff);
	}

	return	(0);
}
