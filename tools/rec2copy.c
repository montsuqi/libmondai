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
#include	<unistd.h>
#include	<glib.h>
#include	"types.h"
#include	"libmondai.h"
#include	"RecParser.h"
#include	"option.h"
#include	"debug.h"

static	Bool	fNoConv;
static	Bool	fFiller;
static	Bool	fFull;
static	int		TextSize;
static	int		ArraySize;
static	char	*Prefix;
static	char	*RecName;
static	char	*Lang;

static	int		level;
static	char	namebuff[SIZE_BUFF];
static	int		Col;
static	int		is_return = FALSE;

static	CONVOPT	*Conv;

static	void	_COBOL(ValueStruct *val);

static	void
PutLevel(
	int		level,
	Bool	fNum)
{
	int		n;

	n = 8;
	if		(  level  >  1  ) {
		n += 4 + ( level - 2 ) * 2;
	}
	for	( ; n > 0 ; n -- ) {
		fputc(' ',stdout);
	}
	if		(  fNum  ) {
		printf("%02d  ",level);
	} else {
		printf("    ");
	}
	Col = 0;
}

static	void
PutTab(
	int		unit)
{
	int		n;

	n = unit - ( Col % unit );
	Col += n;
	for	( ; n > 0 ; n -- ) {
		fputc(' ',stdout);
	}
}

static	void
PutChar(
	int		c)
{
	fputc(c,stdout);
	Col ++;
}

static	void
PutString(
	char	*str)
{
	int		c;

	while	(  *str  !=  0  ) {
		if		(  fNoConv  ) {
			PutChar(*str);
		} else {
			c = toupper(*str);
			if		(  c  ==  '_'  ) {
				c = '-';
			}
			PutChar(c);
		}
		str ++;
	}
}

static	void
PutName(
	char	*name)
{
	char	buff[SIZE_BUFF];

	if		(	(  name           ==  NULL  )
			||	(  !stricmp(name,"filler")  ) ) {
		strcpy(buff,"filler");
	} else {
		int threshold;

		sprintf(buff,"%s%s",Prefix,name);
#define MAX_CHARACTER_IN_LINE 65

		threshold = MAX_CHARACTER_IN_LINE -
					(8 + level * 2 + 4 + 4 + 18 + 2 + 18 + 1);
		if		(	(  threshold <= 0          )
				||	(  strlen(buff) > threshold) ) {
			is_return = TRUE;
		}
	}
	PutString(buff);
}

static	char	PrevName[SIZE_BUFF];
static	int		PrevCount;
static	char	*PrevSuffix[] = { "X","Y","Z" };
static	void
_COBOL(
	ValueStruct	*val)
{
	int		i
	,		n;
	ValueStruct	*tmp
	,			*dummy;
	char	buff[SIZE_BUFF+1];
	char	*name;

	if		(  val  ==  NULL  )	return;

	switch	(ValueType(val)) {
	  case	GL_TYPE_INT:
		PutString("PIC S9(9)   BINARY");
		break;
	  case	GL_TYPE_FLOAT:
		PutString("PIC X(8)");
		break;
	  case	GL_TYPE_BOOL:
		PutString("PIC X");
		break;
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
		sprintf(buff,"PIC X(%d)",ValueStringLength(val));
		PutString(buff);
		break;
	  case	GL_TYPE_OBJECT:
		sprintf(buff,"PIC X(%d)",sizeof(int)+SIZE_OID);
		PutString(buff);
		break;
	  case	GL_TYPE_NUMBER:
		if		(  ValueFixedLength(val)  ==  0  ) {
			sprintf(buff,"PIC S9(%d)",ValueFixedLength(val));
		} else {
			sprintf(buff,"PIC S9(%d)V9(%d)",
					(ValueFixedLength(val) - ValueFixedSlen(val)),
					ValueFixedSlen(val));
		}
		PutString(buff);
		break;
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_BINARY:
		sprintf(buff,"PIC X(%d)",Conv->textsize);
		PutString(buff);
		break;
	  case	GL_TYPE_ARRAY:
		tmp = ValueArrayItem(val,0);
		n = ValueArraySize(val);
		if		(  n  ==  0  ) {
			n = Conv->arraysize;
		}
		switch	(ValueType(tmp)) {
		  case	GL_TYPE_RECORD:
			sprintf(buff,"OCCURS  %d TIMES",n);
			PutTab(8);
			PutString(buff);
			_COBOL(tmp);
			break;
		  case	GL_TYPE_ARRAY:
			sprintf(buff,"OCCURS  %d TIMES",n);
			PutTab(8);
			PutString(buff);
			dummy = NewValue(GL_TYPE_RECORD);
			sprintf(buff,"%s-%s",PrevName,PrevSuffix[PrevCount]);
			ValueAddRecordItem(dummy,buff,tmp);
			PrevCount ++;
			_COBOL(dummy);
			break;
		  default:
			_COBOL(tmp);
			sprintf(buff,"OCCURS  %d TIMES",n);
			PutTab(8);
			PutString(buff);
			break;
		}
		break;
	  case	GL_TYPE_RECORD:
		level ++;
		if		(  fFull  ) {
			name = namebuff + strlen(namebuff);
		} else {
			name = namebuff;
		}
		for	( i = 0 ; i < ValueRecordSize(val) ; i ++ ) {
			printf(".\n");
			PutLevel(level,TRUE);
			if		(  fFull  )	{
				sprintf(name,"%s%s",(( *namebuff == 0 ) ? "": "-"),
						ValueRecordName(val,i));
			} else {
				strcpy(namebuff,ValueRecordName(val,i));
			}
			strcpy(PrevName,ValueRecordName(val,i));
			PrevCount = 0;
			PutName(namebuff);
			tmp = ValueRecordItem(val,i);
			if		(  is_return  ) {
				/* new line if current ValueStruct children is item and it has
				   occurs */
				if		(	(  ValueType(tmp)  !=  GL_TYPE_RECORD  )
						&&	(  ValueType(tmp)  ==  GL_TYPE_ARRAY  )
						&&	(  ValueType(ValueArrayItem(tmp,0))  !=  GL_TYPE_RECORD  ) ) {
					printf("\n");
					PutLevel(level,FALSE);
				}
			  	is_return = FALSE;
			}
			if		(  ValueType(tmp)  !=  GL_TYPE_RECORD  ) {
				PutTab(4);
			}
			_COBOL(tmp);
			*name = 0;
		}
		level --;
		break;
	  default:
		break;
	}
}

static	void
COBOL(
	ValueStruct	*val)
{
	*namebuff = 0;
	_COBOL(val);
}

static	void
SIZE(
	ValueStruct	*val)
{
	char	buff[SIZE_BUFF+1];

	if		(  val  ==  NULL  )	return;
	level ++;
	PutLevel(level,TRUE);
	PutName("filler");
	PutTab(8);
	sprintf(buff,"PIC X(%d)",SizeValue(Conv,val));
	PutString(buff);
	level --;
}

static	void
MakeFromRecord(
	char	*name)
{
	ValueStruct	*value;
	char		*ValueName;

dbgmsg(">MakeFromRecord");
	level = 1;
	DD_ParserInit();
	if		(  ( value = DD_ParseValue(name,&ValueName) )  !=  NULL  ) {
		PutLevel(level,TRUE);
		if		(  *RecName  ==  0  ) {
			PutString(ValueName);
		} else {
			PutString(RecName);
		}
		if		(  fFiller  ) {
			printf(".\n");
			SIZE(value);
		} else {
			COBOL(value);
		}
		printf(".\n");
	}
dbgmsg("<MakeFromRecord");
}

static	ARG_TABLE	option[] = {
	{	"lang",		STRING,		TRUE,	(void*)&Lang	,
		"対象言語名"			 						},
	{	"textsize",	INTEGER,	TRUE,	(void*)&TextSize,
		"textの最大長"									},
	{	"arraysize",INTEGER,	TRUE,	(void*)&ArraySize,
		"可変要素配列の最大繰り返し数"					},
	{	"prefix",	STRING,		TRUE,	(void*)&Prefix,
		"項目名の前に付加する文字列"					},
	{	"name",		STRING,		TRUE,	(void*)&RecName,
		"レコードの名前"								},
	{	"filler",	BOOLEAN,	TRUE,	(void*)&fFiller,
		"レコードの中身をFILLERにする"					},
	{	"full",		BOOLEAN,	TRUE,	(void*)&fFull,
		"階層構造を名前に反映する"						},
	{	"noconv",	BOOLEAN,	TRUE,	(void*)&fNoConv,
		"項目名を大文字に加工しない"					},
	{	NULL,		0,			FALSE,	NULL,	NULL 	}
};

static	void
SetDefault(void)
{
	fNoConv = FALSE;
	fFiller = FALSE;
	fFull = FALSE;
	ArraySize = -1;
	TextSize = -1;
	Prefix = "";
	RecName = "";
	Lang = "OpenCOBOL";
}

extern	int
main(
	int		argc,
	char	**argv)
{
	FILE_LIST	*fl;
	char		*name
	,			*ext;

	SetDefault();
	fl = GetOption(option,argc,argv,"");
	ConvSetLanguage(Lang);
	Conv = NewConvOpt();
	ConvSetSize(Conv,TextSize,ArraySize);
	if		(  fl  !=  NULL  ) {
		name = fl->name;
		ext = GetExt(name);
		if		(	(  !stricmp(ext,".rec")  )
				||	(  !stricmp(ext,".db")  ) ) {
			MakeFromRecord(name);
		}
	}
	return	(0);
}
