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

#define	TEST_CODE		"euc-jp"
#define	SRC_CODE		"euc-jp"

#define	TEST_VALUE

/*
#define	TEST_CODE		"shift-jis"
*/

/*
#define	DEBUG
#define	TRACE
*/
#define	MAIN
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

#include	"value.h"
#include	"getset.h"
#include	"Native_v.h"
#include	"Text_v.h"
#include	"XML_v.h"
#include	"valueconv.h"
#include	"DDparser.h"
#include	"numerici.h"
#include	"misc.h"
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
	FILE	*fp;
	char	name[SIZE_LONGNAME+1];
	ValueStruct	*value;

	sprintf(name,"/tmp/mcparea%d.rec",(int)getpid());
	if		(  ( fp = fopen(name,"w") )  ==  NULL  ) {
		fprintf(stderr,"tempfile can not make.\n");
		exit(1);
	}
	fprintf(fp,	"mcparea	{");
	fprintf(fp,		"func varchar(%d);",SIZE_FUNC);
	fprintf(fp,		"rc int;");
	fprintf(fp,		"dc	{");
	fprintf(fp,			"window	 varchar(%d);",SIZE_NAME);
	fprintf(fp,			"widget	 varchar(%d);",SIZE_NAME);
	fprintf(fp,			"event	 varchar(%d);",SIZE_EVENT);
	fprintf(fp,			"module	 varchar(%d);",SIZE_NAME);
	fprintf(fp,			"fromwin varchar(%d);",SIZE_NAME);
	fprintf(fp,			"status	 varchar(%d);",SIZE_STATUS);
	fprintf(fp,			"puttype varchar(%d);",SIZE_PUTTYPE);
	fprintf(fp,			"term	 varchar(%d);",SIZE_TERM);
	fprintf(fp,			"user	 varchar(%d);",SIZE_USER);
	fprintf(fp,		"};");
	fprintf(fp,		"db	{");
	fprintf(fp,			"path	{");
	fprintf(fp,				"blocks	int;");
	fprintf(fp,				"rname	int;");
	fprintf(fp,				"pname	int;");
	fprintf(fp,			"};");
	fprintf(fp,		"};");
	fprintf(fp,		"private	{");
	fprintf(fp,			"count	int;");
	fprintf(fp,			"swindow	char(%d)[%d];",SIZE_NAME,stacksize);
	fprintf(fp,			"state		char(1)[%d];",stacksize);
	fprintf(fp,			"index		int[%d];",stacksize);
	fprintf(fp,			"pstatus	char(1);");
	fprintf(fp,			"pputtype 	char(1);");
	fprintf(fp,			"prc		char(1);");
	fprintf(fp,		"};");
	fprintf(fp,	"};");
	fclose(fp);

	value = DD_ParseValue(name);
	remove(name);

	return	(value);
}

extern	int
main(
	int		argc,
	char	**argv)
{
#define	SIZE_DATA		128

	char	name[SIZE_DATA];
	int		i;
	ValueStruct	*val;
	FILE	*fp;
	CONVOPT	*opt;
	byte	*buff;

	printf("***** libmondai test start *****\n");
	RecordDir = ".";
	printf("***** DD_ParserInit *****\n");
	DD_ParserInit();
	printf("***** DD_ParseValue *****\n");
	val = BuildMcpArea(10);

	/*	set	*/
	printf("***** Value setting *****\n");
	SetValueString(GetItemLongName(val,"func"),"aaa",SRC_CODE);
	SetValueString(GetItemLongName(val,"rc"),"0",SRC_CODE);

	SetValueString(GetItemLongName(val,"dc.window"),"window",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.widget"),"widget",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.event"),"event",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.fromwin"),"fromwin",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.status"),"status",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.puttype"),"puttype",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.term"),"term",SRC_CODE);
	SetValueString(GetItemLongName(val,"dc.user"),"user",SRC_CODE);

	SetValueString(GetItemLongName(val,"db.path.blocks"),"1",SRC_CODE);
	SetValueString(GetItemLongName(val,"db.path.rname"),"2",SRC_CODE);
	SetValueString(GetItemLongName(val,"db.path.pname"),"3",SRC_CODE);

	SetValueString(GetItemLongName(val,"private.count"),"1",SRC_CODE);
	for	( i = 0 ; i < ValueArraySize(GetItemLongName(val,"private.swindow")) ; i ++ ) {
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

	opt = NewConvOpt();
	ConvSetCoding(opt,TEST_CODE);

	buff = xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);
	ConvSetXmlType(opt,XML_TYPE2);
	ConvSetIndent(opt,TRUE);
	ConvSetType(opt,FALSE);
	ConvSetRecName(opt,"mcparea");

	XML_PackValue(opt,buff,val);

	if		(  ( fp = fopen("test.value","w") )  ==  NULL  ) 	exit(1);
	fprintf(fp,"%s\n",buff);
	fclose(fp);
	xfree(buff);

	return	(0);
}
