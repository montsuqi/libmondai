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

#define	CONV_TEST
/*
#define	TEST_VALUE
#define	XML_TEST
*/

#define	DEBUG
#define	TRACE
/*
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
#include	"OpenCOBOL_v.h"
#include	"dotCOBOL_v.h"
#include	"Native_v.h"
#include	"Text_v.h"
#include	"XML_v.h"
#include	"cobolvalue.h"
#include	"valueconv.h"
#include	"DDparser.h"
#include	"numerici.h"
#include	"hash.h"
#include	"monstring.h"
#include	"misc.h"
#include	"others.h"

#include	"debug.h"

static	char	*locale = "EUC-JP";
extern	int
main(
	int		argc,
	char	**argv)
{
#define	SIZE_DATA		128

	char	name[SIZE_DATA]
	,		str[SIZE_DATA];
	char	*p;
	int		i
	,		j;
	ValueStruct	*val
	,			*e;
	char	*buff;
	size_t	size
	,		size1;
	FILE	*fp;
	CONVOPT	*opt;

	RecordDir = ".";
	ConvSetLanguage(argv[1]);
	DD_ParserInit();
	if		( ( val = DD_ParseValue("testrec.rec") )  ==  NULL  )	{
		printf("file not found.\n");
		exit(1);
	}

	/*	set	*/
	SetValueString(GetItemLongName(val,"a"),"aaa",locale);
	SetValueString(GetItemLongName(val,"b"),"bb",locale);
	SetValueString(GetItemLongName(val,"g"),"NIL",locale);
	ValueAttribute(GetItemLongName(val,"g")) |= GL_ATTR_NIL;

	SetValueString(GetItemLongName(val,"l"),"abcde,fghijklmnop",locale);
	SetValueString(GetItemLongName(val,"m"),"´Á»ú¤òÆþ¤ì¤¿",locale);
	SetValueString(GetItemLongName(val,"n"),"200.003",NULL);
	SetValueString(GetItemLongName(val,"c.d"),"d",locale);
	SetValueInteger(GetItemLongName(val,"c.e"),100);
	for	( i = 0 ; i < ValueArraySize(GetItemLongName(val,"c.g")) ; i ++ ) {
		sprintf(name,"c.g[%d].h",i);
		SetValueFloat(GetItemLongName(val,name),(double)i);
		memset(str,0,SIZE_DATA);
		for	( j = 0 , p = str ; j < ( i + 1) ; j ++ , p ++ ) {
			*p = '0' + ( j % 10 );
		}
		sprintf(name,"c.g[%d].i",i);
		SetValueString(GetItemLongName(val,name),str,locale);
		sprintf(name,"c.g[%d].j",i);
		SetValueString(GetItemLongName(val,name),str,locale);
	}
	for	( i = 0 ; i < ValueArraySize(GetItemLongName(val,"f")) ; i ++ ) {
		sprintf(name,"f[%d].d",i);
		SetValueString(GetItemLongName(val,name),"d",locale);
		sprintf(name,"f[%d].e",i);
		SetValueString(GetItemLongName(val,name),"e",locale);
		sprintf(name,"f[%d].g",i);
		e = GetItemLongName(val,name);
		for	( j = 0 ; j < ValueArraySize(e) ; j ++ ) {
			sprintf(str,"%d%d",i,j);
			sprintf(name,"f[%d].g[%d].h",i,j);
			SetValueString(GetItemLongName(val,name),str,locale);
			sprintf(name,"f[%d].g[%d].i",i,j);
			SetValueString(GetItemLongName(val,name),str,locale);
			sprintf(name,"f[%d].g[%d].j",i,j);
			if		(  j % 2  ==  1  ) {
				SetValueBool(GetItemLongName(val,name),TRUE);
			} else {
				SetValueBool(GetItemLongName(val,name),FALSE);
			}
		}
	}

#ifdef	TEST_VALUE
	e = GetItemLongName(val,"q");
	for	( i = 0 ; i < ValueStringLength(e) ; i ++ ) {
		p = str;
		for	( j = 0 ; j < i ; j ++ ) {
			*p ++= j + '@';
		}
		*p = 0;
		SetValueString(e,str,NULL);
		printf("[%s]\n",ValueToString(e,NULL));
		printf("[%s]\n",ValueStringPointer(e));
	}
	e = GetItemLongName(val,"q");
	for	( i = 0 ; i < ValueStringLength(e) ; i ++ ) {
		strcpy(str,"£±£²£³£´£µ£¶£·£¸£¹£°£±£²£³£´£µ£¶£·£¸£¹£°");
		str[i*2] = 0;
		printf("[%s]\n",str);fflush(stdout);
		SetValueString(e,str,"euc-jp");
		printf("[%s]\n",ValueToString(e,"euc-jp"));
		//		printf("[%s]\n",ValueStringPointer(e));
	}
	e = GetItemLongName(val,"m");
	for	( i = 0 ; i < 20 ; i ++ ) {
		p = str;
		for	( j = 0 ; j < i ; j ++ ) {
			*p ++= j + '@';
		}
		*p = 0;
		SetValueString(e,str,NULL);
		printf("[%s]\n",ValueToString(e,NULL));
		printf("[%s]\n",ValueStringPointer(e));
	}
	e = GetItemLongName(val,"m");
	for	( i = 0 ; i < 20 ; i ++ ) {
		strcpy(str,"£±£²£³£´£µ£¶£·£¸£¹£°£±£²£³£´£µ£¶£·£¸£¹£°");
		str[i*2] = 0;
		printf("[%s]\n",str);fflush(stdout);
		SetValueString(e,str,"euc-jp");
		printf("[%s]\n",ValueToString(e,"euc-jp"));
		printf("[%s]\n",ValueStringPointer(e));
	}

	strcpy(str,"£±£²£³£´£µ£¶£·£¸£¹£°£±£²£³£´£µ£¶£·£¸£¹£°");
printf("*\n");fflush(stdout);
	SetValueString(GetItemLongName(val,"q"),str,"euc-jp");
	printf("[%s]\n",ValueToString(GetItemLongName(val,"q"),"euc-jp"));
printf("*\n");fflush(stdout);

	MoveValue(GetItemLongName(val,"p"),GetItemLongName(val,"q"));
	printf("[%s]\n",ValueToString(GetItemLongName(val,"p"),"euc-jp"));
printf("*\n");fflush(stdout);

#endif
	DumpValueStruct(val);

	opt = NewConvOpt();
	ConvSetLocale(opt,"shift-jis");
	ConvSetLocale(opt,"euc-jp");
#ifdef	XML_TEST
	buff = xmalloc(SIZE_BUFF);
	printf("***** XML Pack *****\n");
	ConvSetRecName(opt,"testrec");
	ConvSetIndent(opt,TRUE);
	//ConvSetIndent(opt,FALSE);
	ConvSetType(opt,FALSE);
	XML_PackValue(opt,buff,val);
	if		(  ( fp = fopen("test.xml","w") )  ==  NULL  ) 	exit(1);
	fprintf(fp,"%s\n",buff);
	fclose(fp);
	printf("size = %d\n",strlen(buff));
	printf("size = %d\n",XML_SizeValue(opt,val));
	printf("********************\n");
	InitializeValue(val);
	XML_UnPackValue(opt,buff,val);
	printf("********************\n");
	DumpValueStruct(val);
	printf("********************\n");
#endif

#ifdef	CONV_TEST

	printf("***** Native Pack(1) *****\n");
	ConvSetUseName(opt,FALSE);
	size =  NativeSizeValue(opt,val);
	printf("size = %d\n",size);
	buff = (char *)xmalloc(size);
	NativePackValue(opt,buff,val);
	if		(  ( fp = fopen("test.native1","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);
	xfree(buff);

	printf("***** Native UnPack(1) *****\n");
	if		(  ( fp = fopen("test.native1","r") )  ==  NULL  )	exit(1);
	buff = (char *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);
	fread(buff,SIZE_BUFF,1,fp);
	fclose(fp);
	NativeUnPackValue(opt,buff,val);
	DumpValueStruct(val);
	memset(buff,0,SIZE_BUFF);
	CSV3_PackValue(opt,buff,val);
	printf("%s\n",buff);

	printf("***** Native Pack(2) *****\n");
	ConvSetUseName(opt,TRUE);
	size =  NativeSizeValue(opt,val);
	printf("size = %d\n",size);
	buff = (char *)xmalloc(size);
	NativePackValue(opt,buff,val);
	if		(  ( fp = fopen("test.native2","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);
	xfree(buff);

	printf("***** Native UnPack(2) *****\n");
	if		(  ( fp = fopen("test.native2","r") )  ==  NULL  )	exit(1);
	buff = (char *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);
	fread(buff,SIZE_BUFF,1,fp);
	fclose(fp);
	NativeUnPackValue(opt,buff,val);
	DumpValueStruct(val);
	memset(buff,0,SIZE_BUFF);
	CSV3_PackValue(opt,buff,val);
	printf("%s\n",buff);

	printf("***** CSV Pack *****\n");
	size = CSV1_SizeValue(opt,val);
	printf("CSV1 size = %d\n",size);
	size = CSV2_SizeValue(opt,val);
	printf("CSV2 size = %d\n",size);
	size = CSV3_SizeValue(opt,val);
	printf("CSV3 size = %d\n",size);
	size = CSVE_SizeValue(opt,val);
	printf("CSVE size = %d\n",size);
	buff = (char *)xmalloc(SIZE_BUFF);

	if		(  ( fp = fopen("test.csv","w") )  ==  NULL  ) 	exit(1);
	CSV1_PackValue(opt,buff,val);
	fprintf(fp,"%s\n",buff);
	CSV2_PackValue(opt,buff,val);
	fprintf(fp,"%s\n",buff);
	CSV3_PackValue(opt,buff,val);
	fprintf(fp,"%s\n",buff);
	CSVE_PackValue(opt,buff,val);
	fprintf(fp,"%s\n",buff);
	fclose(fp);
	xfree(buff);

	printf("***** CSV UnPack *****\n");
	if		(  ( fp = fopen("test.csv","r") )  ==  NULL  )	exit(1);
	buff = (char *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	fgets(buff,SIZE_BUFF,fp);
	CSV_UnPackValue(opt,buff,val);
	CSV3_PackValue(opt,buff,val);
	printf("%s\n",buff);

	CSV_UnPackValue(opt,buff,val);
	CSV3_PackValue(opt,buff,val);
	printf("%s\n",buff);

	CSV_UnPackValue(opt,buff,val);
	CSV3_PackValue(opt,buff,val);
	printf("%s\n",buff);

	CSV_UnPackValue(opt,buff,val);
	CSV3_PackValue(opt,buff,val);
	printf("%s\n",buff);

	xfree(buff);
	fclose(fp);

	printf("***** RFC822 Pack *****\n");
	if		(  ( fp = fopen("test.822","w") )  ==  NULL  )	exit(1);
	ConvSetRecName(opt,"testrec");
	ConvSetUseName(opt,TRUE);
	size =  RFC822_SizeValue(opt,val);
	size1 = size;
	printf("size = %d\n",size);
	buff = (char *)xmalloc(size);
	RFC822_PackValue(opt,buff,val);
	printf("%s\n",buff);
	fwrite(buff,size,1,fp);

	ConvSetUseName(opt,FALSE);
	size =  RFC822_SizeValue(opt,val);
	printf("size = %d\n",size);
	buff = (char *)xmalloc(size);
	RFC822_PackValue(opt,buff,val);
	printf("%s\n",buff);
	fwrite(buff,size,1,fp);

	fclose(fp);
	xfree(buff);

	printf("***** RFC822 UnPack *****\n");
	ConvSetUseName(opt,TRUE);
	InitializeValue(val);
	if		(  ( fp = fopen("test.822","r") )  ==  NULL  )	exit(1);
	buff = (char *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	fread(buff,size1,1,fp);
	RFC822_UnPackValue(opt,buff,val);
	DumpValueStruct(val);

	ConvSetUseName(opt,FALSE);
	InitializeValue(val);
	fread(buff,size,1,fp);
	fclose(fp);
	RFC822_UnPackValue(opt,buff,val);
	DumpValueStruct(val);

	memset(buff,0,SIZE_BUFF);
	CSV3_PackValue(opt,buff,val);
	printf("%s\n",buff);

	printf("***** end *****\n");
	printf("***** CGI Pack *****\n");
	if		(  ( fp = fopen("test.CGI","w") )  ==  NULL  )	exit(1);
	ConvSetRecName(opt,"testrec");
	size =  CGI_SizeValue(opt,val);
	size1 = size;
	printf("size = %d\n",size);
	buff = (char *)xmalloc(size);
	CGI_PackValue(opt,buff,val);
	printf("%s\n",buff);
	fwrite(buff,size,1,fp);

	fclose(fp);
	xfree(buff);

	printf("***** CGI UnPack *****\n");
	InitializeValue(val);
	if		(  ( fp = fopen("test.CGI","r") )  ==  NULL  )	exit(1);
	buff = (char *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	fread(buff,size1,1,fp);
	CGI_UnPackValue(opt,buff,val);
	DumpValueStruct(val);

	memset(buff,0,SIZE_BUFF);
	CSV3_PackValue(opt,buff,val);
	printf("%s\n",buff);

	printf("***** end *****\n");

#endif
	return	(0);
}
