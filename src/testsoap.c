/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2005-2008 Ogochan.
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

#define	MAIN

#define	DEBUG
#define	TRACE
/*
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#ifdef	USE_SOAP

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
	p += sprintf(p,			"state		char(1)[%d];",(int)stacksize);
	p += sprintf(p,			"index		int[%d];",(int)stacksize);
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

static	void
DumpByXML(
	ValueStruct	*val,
	char		*name)
{
	CONVOPT	*opt;
	unsigned char	buff[SIZE_BUFF];

	if		(  val  !=  NULL  ) {
		opt = NewConvOpt();

		ConvSetCodeset(opt,DUMP_CODE);
		ConvSetXmlType(opt,XML_TYPE1);
		ConvSetIndent(opt,TRUE);
		ConvSetType(opt,FALSE);
		ConvSetRecName(opt,name);

		memset(buff,0,SIZE_BUFF);
		XML_PackValue(opt,buff,val);
		printf("%s\n",buff);
	}
}

extern	int
main(
	int		argc,
	char	**argv)
{
#define	SIZE_DATA		128

	char	name[SIZE_DATA];
	unsigned char	*p;
	int		i;
	ValueStruct	*val
		,		*val2;
	unsigned char	*buff;
	char	method[SIZE_LONGNAME+1];
	FILE	*fp;

	RecordDir = ".";
	RecParserInit();
	val = BuildMcpArea(10);

	/*	set	*/
	printf("***** Value setting *****\n");
	SetValueString(GetItemLongName(val,"func"),"aaa",SRC_CODE);
	SetValueString(GetItemLongName(val,"rc"),"0",SRC_CODE);

	SetValueString(GetItemLongName(val,"dc.window"),"1 Ë",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.widget"),"widget",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.event"),"1002 ",SRC_CODE);
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
		*p = (unsigned char)i;
	}
	SetValueBinary(GetItemLongName(val,"bin"),buff,256);


	printf("***** SOAP ****\n");
	memset(buff,0,SIZE_BUFF);

	SOAP_PackValue(buff,val,"Put","mcp","http://oreore",TRUE,FALSE);

	printf("%s\n",buff);
	fp = fopen("test.SOAP","w");
	fprintf(fp,"%s\n",buff);
	fclose(fp);

	val2 = DuplicateValue(val,FALSE);

	SOAP_UnPackValue(val2,(char *)buff,method);

	printf("method = [%s]\n",method);
	DumpByXML(val2,"mcparea");

	SOAP_PackValue(buff,val,"Put","mcp","http://oreore",TRUE,FALSE);
	val2 = SOAP_LoadValue((char *)buff,method);
	printf("method = [%s]\n",method);
	DumpByXML(val2,"mcparea");
	//DumpValueStruct(val2);

	return	(0);
}
#endif
