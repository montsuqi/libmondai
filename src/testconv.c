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
#include	"debug.h"

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
	RecordStruct	*rec;
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
	if		( ( rec = ReadRecordDefine("testrec") )  ==  NULL  )	{
		printf("file not found.\n");
		exit(1);
	}
	val = rec->value;

	/*	set	*/
	SetValueString(GetItemLongName(val,"a"),"aaa");
	SetValueString(GetItemLongName(val,"b"),"bb");
	SetValueString(GetItemLongName(val,"l"),"abcde,fghijklmnop");
	SetValueString(GetItemLongName(val,"m"),"漢字を入れた");
	SetValueString(GetItemLongName(val,"c.d"),"d");
	SetValueInteger(GetItemLongName(val,"c.e"),100);
	for	( i = 0 ; i < ValueArraySize(GetItemLongName(val,"c.g")) ; i ++ ) {
		sprintf(name,"c.g[%d].h",i);
		SetValueFloat(GetItemLongName(val,name),(double)i);
		memset(str,0,SIZE_DATA);
		for	( j = 0 , p = str ; j < ( i + 1) ; j ++ , p ++ ) {
			*p = '0' + ( j % 10 );
		}
		sprintf(name,"c.g[%d].i",i);
		SetValueString(GetItemLongName(val,name),str);
		sprintf(name,"c.g[%d].j",i);
		SetValueString(GetItemLongName(val,name),str);
	}
	for	( i = 0 ; i < ValueArraySize(GetItemLongName(val,"f")) ; i ++ ) {
		sprintf(name,"f[%d].d",i);
		SetValueString(GetItemLongName(val,name),"d");
		sprintf(name,"f[%d].e",i);
		SetValueString(GetItemLongName(val,name),"e");
		sprintf(name,"f[%d].g",i);
		e = GetItemLongName(val,name);
		for	( j = 0 ; j < ValueArraySize(e) ; j ++ ) {
			sprintf(str,"%d%d",i,j);
			sprintf(name,"f[%d].g[%d].h",i,j);
			SetValueString(GetItemLongName(val,name),str);
			sprintf(name,"f[%d].g[%d].i",i,j);
			SetValueString(GetItemLongName(val,name),str);
			sprintf(name,"f[%d].g[%d].j",i,j);
			if		(  j % 2  ==  1  ) {
				SetValueBool(GetItemLongName(val,name),TRUE);
			} else {
				SetValueBool(GetItemLongName(val,name),FALSE);
			}
		}
	}
	DumpValueStruct(val);

	opt = NewConvOpt();
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

	printf("***** Native Pack *****\n");
	size =  NativeSizeValue(opt,val);
	printf("size = %d\n",size);
	buff = (char *)xmalloc(size);
	NativePackValue(opt,buff,val);
	if		(  ( fp = fopen("test.native","w") )  ==  NULL  )	exit(1);
	fwrite(buff,size,1,fp);
	fclose(fp);
	xfree(buff);

	printf("***** Native UnPack *****\n");
	if		(  ( fp = fopen("test.native","r") )  ==  NULL  )	exit(1);
	buff = (char *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);
	fgets(buff,SIZE_BUFF,fp);
	fclose(fp);
	NativeUnPackValue(opt,buff,val);
	DumpValueStruct(val);
	memset(buff,0,SIZE_BUFF);
	CSV3_PackValue(opt,buff,val);
	printf("%s\n",buff);

	printf("***** RFC822 Pack *****\n");
	if		(  ( fp = fopen("test.822","w") )  ==  NULL  )	exit(1);
	ConvSetRecName(opt,"testrec");
	ConvSetUseFileName(opt,TRUE);
	size =  RFC822_SizeValue(opt,val);
	size1 = size;
	printf("size = %d\n",size);
	buff = (char *)xmalloc(size);
	RFC822_PackValue(opt,buff,val);
	printf("%s\n",buff);
	fwrite(buff,size,1,fp);

	ConvSetUseFileName(opt,FALSE);
	size =  RFC822_SizeValue(opt,val);
	printf("size = %d\n",size);
	buff = (char *)xmalloc(size);
	RFC822_PackValue(opt,buff,val);
	printf("%s\n",buff);
	fwrite(buff,size,1,fp);

	fclose(fp);
	xfree(buff);

	printf("***** RFC822 UnPack *****\n");
	ConvSetUseFileName(opt,TRUE);
	InitializeValue(val);
	if		(  ( fp = fopen("test.822","r") )  ==  NULL  )	exit(1);
	buff = (char *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	fread(buff,size1,1,fp);
	RFC822_UnPackValue(opt,buff,val);
	DumpValueStruct(val);

	ConvSetUseFileName(opt,FALSE);
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

	return	(0);
}
