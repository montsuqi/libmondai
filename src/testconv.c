/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2002 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2003-2009 Ogochan.
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
#define	MAIN
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define	TEST_CODE		"euc-jp"
#define	SRC_CODE		"utf-8"

#define	TEST_NATIVE1
#define	TEST_NATIVE2
#define	TEST_NATIVE3
#define	TEST_NATIVE4
/*
#define	TEST_VALUE
#define	TEST_XML1
#define	TEST_XML2
#define	TEST_OPENCOBOL
#define	TEST_DOTCOBOL
#define	TEST_CSV1
#define	TEST_CSV2
#define	TEST_CSV3
#define	TEST_CSVE
#define	TEST_RFC822_1
#define	TEST_RFC822_2
#define	TEST_CGI
#define	TEST_SQL
#define	TEST_JSON
*/
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
#include	"OpenCOBOL_v.h"
#include	"dotCOBOL_v.h"
#include	"Native_v.h"
#include	"Text_v.h"
#ifdef	USE_XML
#include	"XML_v.h"
#endif
#include	"cobolvalue.h"
#include	"valueconv.h"
#include	"json_v.h"

#include	"RecParser.h"
#include	"numerici.h"
#include	"hash_v.h"
#include	"monstring.h"
#include	"memory_v.h"
#include	"misc_v.h"
#include	"others.h"

#include	"debug.h"

extern	int
main(
	int		argc,
	char	**argv)
{
#define	SIZE_DATA		128

	char	name[SIZE_DATA]
	,		str[SIZE_DATA];
	byte	*p;
	int		i
	,		j;
	ValueStruct	*val
	,			*e;
	size_t	size
	,		size1;
	FILE	*fp;
	CONVOPT	*opt;
	byte	*buff;

	printf("***** libmondai test start *****\n");
	RecordDir = ".";
	//	ConvSetLanguage(argv[1]);
	printf("***** DD_ParserInit *****\n");
	RecParserInit();
	printf("***** RecParseValue *****\n");
	if		( ( val = RecParseValue("testrec.rec",NULL) )  ==  NULL  )	{
		fprintf(stderr,"file not found.\n");
		exit(1);
	}

	/*	set	*/
	printf("***** Value setting *****\n");
	InitializeValue(val);
	SetValueString(GetItemLongName(val,"a"),"aaa",SRC_CODE);
	SetValueString(GetItemLongName(val,"b"),"bb",SRC_CODE);
	SetValueString(GetItemLongName(val,"g"),"NIL",SRC_CODE);
	ValueAttribute(GetItemLongName(val,"g")) |= GL_ATTR_NIL;

	SetValueString(GetItemLongName(val,"l"),"abcde,fghijklmnop",SRC_CODE);
	SetValueString(GetItemLongName(val,"m"),"漢字を入れた",SRC_CODE);
	SetValueString(GetItemLongName(val,"n"),"200.003",NULL);
	SetValueString(GetItemLongName(val,"c.d"),"d",SRC_CODE);
	SetValueInteger(GetItemLongName(val,"c.e"),100);
	for	( i = 0 ; i < ValueArraySize(GetItemLongName(val,"c.g")) ; i ++ ) {
		sprintf(name,"c.g[%d].h",i);
		SetValueFloat(GetItemLongName(val,name),(double)i);
		memset(str,0,SIZE_DATA);
		for	( j = 0 , p = (byte *)str ; j < ( i + 1) ; j ++ , p ++ ) {
			*p = '0' + ( j % 10 );
		}
		sprintf(name,"c.g[%d].i",i);
		SetValueString(GetItemLongName(val,name),str,SRC_CODE);
		sprintf(name,"c.g[%d].j",i);
		SetValueString(GetItemLongName(val,name),str,SRC_CODE);
	}
	for	( i = 0 ; i < ValueArraySize(GetItemLongName(val,"f")) ; i ++ ) {
		sprintf(name,"f[%d].d",i);
		SetValueString(GetItemLongName(val,name),"d",SRC_CODE);
		sprintf(name,"f[%d].e",i);
		SetValueString(GetItemLongName(val,name),"e",SRC_CODE);
		sprintf(name,"f[%d].g",i);
		e = GetItemLongName(val,name);
		for	( j = 0 ; j < ValueArraySize(e) ; j ++ ) {
			sprintf(str,"%d%d",i,j);
			sprintf(name,"f[%d].g[%d].h",i,j);
			SetValueString(GetItemLongName(val,name),str,SRC_CODE);
			sprintf(name,"f[%d].g[%d].i",i,j);
			SetValueString(GetItemLongName(val,name),str,SRC_CODE);
			sprintf(name,"f[%d].g[%d].j",i,j);
			if		(  j % 2  ==  1  ) {
				SetValueBool(GetItemLongName(val,name),TRUE);
			} else {
				SetValueBool(GetItemLongName(val,name),FALSE);
			}
		}
	}

#ifdef	TEST_VALUE
	printf("***** test Value *****\n");
	printf("***** varchar(40) *****\n");
	e = GetItemLongName(val,"q");
	for	( i = 0 ; i < ValueStringLength(e) ; i ++ ) {
		p = (byte *)str;
		for	( j = 0 ; j < i ; j ++ ) {
			*p ++= j + '@';
		}
		*p = 0;
		printf("in   [%s]\n",str);fflush(stdout);
		SetValueString(e,str,NULL);
		printf("buff [%s]\n",ValueStringPointer(e));
		printf("out  [%s]\n",ValueToString(e,TEST_CODE));
	}
	printf("***** varchar(40) *****\n");
	e = GetItemLongName(val,"q");
	for	( i = 0 ; i < ValueStringLength(e) ; i ++ ) {
		strcpy(str,"１２３４５６７８９０１２３４５６７８９０");
		str[i*3] = 0;
		printf("in   [%s]\n",str);fflush(stdout);
		SetValueString(e,str,SRC_CODE);
		printf("buff [%s]\n",ValueStringPointer(e));
		printf("out  [%s]\n",ValueToString(e,TEST_CODE));
	}
	printf("***** text(with coding) *****\n");
	e = GetItemLongName(val,"m");
	for	( i = 0 ; i < 20 ; i ++ ) {
		p = (byte *)str;
		for	( j = 0 ; j < i ; j ++ ) {
			*p ++= j + '@';
		}
		*p = 0;
		printf("in   [%s]\n",str);fflush(stdout);
		SetValueString(e,str,SRC_CODE);
		printf("out  [%s]\n",ValueToString(e,TEST_CODE));
		printf("buff [%s]\n",ValueStringPointer(e));
	}
	printf("***** text *****\n");
	e = GetItemLongName(val,"m");
	for	( i = 0 ; i < 20 ; i ++ ) {
		p = (byte *)str;
		for	( j = 0 ; j < i ; j ++ ) {
			*p ++= j + '@';
		}
		*p = 0;
		printf("in   [%s]\n",str);fflush(stdout);
		SetValueString(e,str,NULL);
		printf("out  [%s]\n",ValueToString(e,NULL));
		printf("buff [%s]\n",ValueStringPointer(e));
	}
	printf("***** text *****\n");
	e = GetItemLongName(val,"m");
	for	( i = 0 ; i < 20 ; i ++ ) {
		strcpy(str,"１２３４５６７８９０１２３４５６７８９０");
		str[i*3] = 0;
		printf("in   [%s]\n",str);fflush(stdout);
		SetValueString(e,str,SRC_CODE);
		printf("buff [%s]\n",ValueStringPointer(e));
		printf("out  [%s]\n",ValueToString(e,TEST_CODE));
	}

	strcpy(str,"１２３４５６７８９０１２３４５６７８９０");
	printf("***** str -> varchar(40) *****\n");
	e = GetItemLongName(val,"q");
	printf("in   [%s]\n",str);
	SetValueString(GetItemLongName(val,"q"),str,SRC_CODE);
	printf("out  [%s]\n",ValueToString(e,TEST_CODE));
	printf("buff [%s]\n",ValueStringPointer(e));

	printf("***** varchar(40) -> varchar(20) *****\n");
	e = GetItemLongName(val,"p");
	MoveValue(e,GetItemLongName(val,"q"));
	printf("buff [%s]\n",ValueStringPointer(e));
	printf("out  [%s]\n",ValueToString(e,TEST_CODE));
	printf("********************\n");
	printf("***** binary *****\n");
	buff = xmalloc(256);
	memset(buff,0,256);
	for	( p = buff, i = 0 ; i < 256 ; i ++ , p ++) {
		*p = (byte)i;
	}
	e = GetItemLongName(val,"j");
	for	( i = 0 ; i < 256 ; i ++ ) {
		SetValueBinary(e,buff,i);
		printf("size   = %d\n",(int)ValueByteSize(e));
		printf("length = %d\n",(int)ValueByteLength(e));
	}
	SetValueBinary(e,buff,256);
	xfree(buff);
	printf("********************\n");

#endif
	DumpValueStruct(val);

	opt = NewConvOpt();
	ConvSetCodeset(opt,TEST_CODE);
#ifdef	USE_XML
#ifdef	TEST_XML1
	buff = xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** XML(1) *****\n");
	ConvSetRecName(opt,"testrec");
	ConvSetIndent(opt,TRUE);
	//ConvSetIndent(opt,FALSE);
	ConvSetType(opt,FALSE);
	ConvSetXmlType(opt,XML_TYPE1);

	ConvSetOutput(opt,(XML_OUT_HEADER|XML_OUT_BODY|XML_OUT_TAILER));
	//	ConvSetOutput(opt,(XML_OUT_HEADER|XML_OUT_TAILER));
	//	ConvSetOutput(opt,XML_OUT_BODY);
	//	ConvSetOutput(opt,(XML_OUT_HEADER|XML_OUT_BODY));

	printf("***** XML Size(1) *****\n");
	size = XML_SizeValue(opt,val);
	size1 = XML_PackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.xml1","w") )  ==  NULL  ) 	exit(1);
	fprintf(fp,"%s\n",buff);
	fclose(fp);

	printf("***** XML UnPack(1) *****\n");
	InitializeValue(val);
	XML_UnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** XML(1) end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_XML2
	buff = xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** XML(2) *****\n");
	ConvSetXmlType(opt,XML_TYPE2);
	ConvSetIndent(opt,TRUE);
	ConvSetType(opt,FALSE);

	printf("***** XML Size(2) *****\n");
	size = XML_SizeValue(opt,val);
	size1 = XML_PackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.xml2","w") )  ==  NULL  ) 	exit(1);
	fprintf(fp,"%s\n",buff);
	fclose(fp);

	printf("***** XML UnPack(2) *****\n");
	InitializeValue(val);
	XML_UnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** XML(2) end *****\n");
	xfree(buff);
#endif
#endif

#ifdef	TEST_NATIVE1
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** Native(1) *****\n");
	ConvSetCodeset(opt,TEST_CODE);
	ConvSetUseName(opt,FALSE);

	printf("***** Native Size(1) *****\n");
	size = NativeSizeValue(opt,val);
	size1 = NativePackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.native1","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** Native UnPack(1) *****\n");
	InitializeValue(val);
	NativeUnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** Native(1) end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_NATIVE2
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** Native(2) *****\n");
	ConvSetUseName(opt,TRUE);

	printf("***** Native Size(2) *****\n");
	size = NativeSizeValue(opt,val);
	size1 = NativePackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.native2","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** Native UnPack(2) *****\n");
	InitializeValue(val);
	NativeUnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** Native(2) end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_NATIVE3
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** Native(3) *****\n");
	ConvSetUseName(opt,TRUE);
	ConvSetType(opt,TRUE);

	printf("***** Native Size(3) *****\n");
	size = NativeSaveSize(val,TRUE);
	size1 = NativeSaveValue(buff,val,TRUE);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.native3","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** Native UnPack(3) *****\n");

	val = NativeRestoreValue(buff,TRUE);
	DumpValueStruct(val);
	printf("***** Native(3) end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_NATIVE4
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** Native(4) *****\n");
	ConvSetUseName(opt,TRUE);
	ConvSetType(opt,TRUE);

	printf("***** Native Size(4) *****\n");
	size = NativeSizeValue(opt,val);
	size1 = NativePackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.native4","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** Native UnPack(4) *****\n");

	val = NativeRestoreValue(buff,TRUE);
	DumpValueStruct(val);
	printf("***** Native(4) end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_OPENCOBOL
	ConvSetSize(opt,100,100);
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** OpenCOBOL *****\n");

	printf("***** OpenCOBOL Size *****\n");
	size = OpenCOBOL_SizeValue(opt,val);
	size1 = OpenCOBOL_PackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.oc","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** OpenCOBOL UnPack *****\n");
	InitializeValue(val);
	OpenCOBOL_UnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** OpenCOBOL end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_DOTCOBOL
	ConvSetSize(opt,100,100);
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** dotCOBOL *****\n");

	printf("***** dotCOBOL Size *****\n");
	size = dotCOBOL_SizeValue(opt,val);
	size1 = dotCOBOL_PackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.dc","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** dotCOBOL UnPack *****\n");
	InitializeValue(val);
	dotCOBOL_UnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** dotCOBOL end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_CSV1
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** CSV(1) *****\n");
	ConvSetCodeset(opt,TEST_CODE);

	printf("***** CSV Size(1) *****\n");
	size = CSV1_SizeValue(opt,val);
	size1 = CSV1_PackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.csv1","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** CSV UnPack(1) *****\n");
	InitializeValue(val);
	CSV_UnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** CSV(1) end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_CSV2
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** CSV(2) *****\n");
	ConvSetCodeset(opt,TEST_CODE);

	printf("***** CSV Size(2) *****\n");
	size = CSV2_SizeValue(opt,val);
	size1 = CSV2_PackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.csv2","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** CSV UnPack(2) *****\n");
	InitializeValue(val);
	CSV_UnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** CSV(2) end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_CSV3
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** CSV(3) *****\n");
	ConvSetCodeset(opt,TEST_CODE);

	printf("***** CSV Size(3) *****\n");
	size = CSV3_SizeValue(opt,val);
	size1 = CSV3_PackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.csv3","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** CSV UnPack(3) *****\n");
	InitializeValue(val);
	CSV_UnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** CSV(3) end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_CSVE
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** CSV(E) *****\n");
	ConvSetCodeset(opt,TEST_CODE);

	printf("***** CSV Size(E) *****\n");
	size = CSVE_SizeValue(opt,val);
	size1 = CSVE_PackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.csve","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** CSV UnPack(E) *****\n");
	InitializeValue(val);
	CSV_UnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** CSV(E) end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_RFC822_1
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** RFC822(with name) *****\n");
	ConvSetEncoding(opt,STRING_ENCODING_URL);
	//ConvSetEncoding(opt,STRING_ENCODING_BASE64);
	ConvSetCodeset(opt,TEST_CODE);
	ConvSetRecName(opt,"testrec");
	ConvSetUseName(opt,TRUE);

	printf("***** RFC822 Size(with name) *****\n");
	size = RFC822_SizeValue(opt,val);
	size1 = RFC822_PackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.822n","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** RFC822 UnPack(with name) *****\n");
	InitializeValue(val);
	RFC822_UnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** RFC822(with name) end *****\n");
	printf("********************\n");
	xfree(buff);
#endif

#ifdef	TEST_RFC822_2
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** RFC822(without name) *****\n");
	ConvSetCodeset(opt,TEST_CODE);
	ConvSetRecName(opt,"testrec");
	ConvSetUseName(opt,FALSE);

	printf("***** RFC822 Size(without name) *****\n");
	size = RFC822_SizeValue(opt,val);
	size1 = RFC822_PackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.822o","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** RFC822 UnPack(without name) *****\n");
	InitializeValue(val);
	RFC822_UnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** RFC822(without name) end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_CGI
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** CGI *****\n");
	ConvSetEncoding(opt,STRING_ENCODING_URL);
	ConvSetCodeset(opt,TEST_CODE);
	ConvSetRecName(opt,"testrec");
	ConvSetUseName(opt,FALSE);

	printf("***** CGI Size *****\n");
	size = CGI_SizeValue(opt,val);
	size1 = CGI_PackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.CGI","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** CGI UnPack *****\n");
	InitializeValue(val);
	CGI_UnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** CGI end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_SQL
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** SQL *****\n");
	ConvSetEncoding(opt,STRING_ENCODING_URL);
	ConvSetCodeset(opt,TEST_CODE);
	ConvSetRecName(opt,"testrec");
	ConvSetUseName(opt,FALSE);

	printf("***** SQL Size *****\n");
	size = SQL_SizeValue(opt,val);
	size1 = SQL_PackValue(opt,buff,val);
	printf("size = %d\n",(int)size);
	printf("size = %d\n",(int)size1);

	if		(  ( fp = fopen("test.SQL","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);

	printf("***** SQL UnPack *****\n");
	InitializeValue(val);
	SQL_UnPackValue(opt,buff,val);
	DumpValueStruct(val);
	printf("***** SQL end *****\n");
	xfree(buff);
#endif

#ifdef	TEST_JSON
	buff = (byte *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	printf("***** JSON *****\n");
	ConvSetIndent(opt,TRUE);
	ConvSetRecName(opt,"testrec");
	ConvSetUseName(opt,FALSE);

	printf("***** JSON Size *****\n");
	size = JSON_SizeValue(opt,val);
	printf("size = %d\n",(int)size);fflush(stdout);
	size1 = JSON_PackValue(opt,buff,val);
	printf("size = %d\n",(int)size1);fflush(stdout);

	if		(  ( fp = fopen("test.json","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);
	printf("***** JSON UnPack *****\n");
	InitializeValue(val);
	JSON_UnPackValue(opt,buff,val);
	DumpValueStruct(val);

	printf("***** JSON end *****\n");
	xfree(buff);
#endif

	return	(0);
}
