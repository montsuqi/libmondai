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
	ValueStruct	*rec
	,			*e;
	char	*buff;
	size_t	size
	,		size1;
	FILE	*fp;

	RecordDir = ".";
	SetLanguage(argv[1]);
	DD_ParserInit();
	if		( ( rec = ReadRecordDefine("testrec") )  ==  NULL  )	{
		printf("file not found.\n");
		exit(1);
	}

	/*	set	*/
	SetValueString(GetItemLongName(rec,"a"),"aaa");
	SetValueString(GetItemLongName(rec,"b"),"bb");
	SetValueString(GetItemLongName(rec,"l"),"abcde,fghijklmnop");
	SetValueString(GetItemLongName(rec,"c.d"),"d");
	SetValueInteger(GetItemLongName(rec,"c.e"),100);
	for	( i = 0 ; i < ValueArraySize(GetItemLongName(rec,"c.g")) ; i ++ ) {
		sprintf(name,"c.g[%d].h",i);
		SetValueFloat(GetItemLongName(rec,name),(double)i);
		memset(str,0,SIZE_DATA);
		for	( j = 0 , p = str ; j < ( i + 1) ; j ++ , p ++ ) {
			*p = '0' + ( j % 10 );
		}
		sprintf(name,"c.g[%d].i",i);
		SetValueString(GetItemLongName(rec,name),str);
		sprintf(name,"c.g[%d].j",i);
		SetValueString(GetItemLongName(rec,name),str);
	}
	for	( i = 0 ; i < ValueArraySize(GetItemLongName(rec,"f")) ; i ++ ) {
		sprintf(name,"f[%d].d",i);
		SetValueString(GetItemLongName(rec,name),"d");
		sprintf(name,"f[%d].e",i);
		SetValueString(GetItemLongName(rec,name),"e");
		sprintf(name,"f[%d].g",i);
		e = GetItemLongName(rec,name);
		for	( j = 0 ; j < ValueArraySize(e) ; j ++ ) {
			sprintf(str,"%d%d",i,j);
			sprintf(name,"f[%d].g[%d].h",i,j);
			SetValueString(GetItemLongName(rec,name),str);
			sprintf(name,"f[%d].g[%d].i",i,j);
			SetValueString(GetItemLongName(rec,name),str);
			sprintf(name,"f[%d].g[%d].j",i,j);
			if		(  j % 2  ==  1  ) {
				SetValueBool(GetItemLongName(rec,name),TRUE);
			} else {
				SetValueBool(GetItemLongName(rec,name),FALSE);
			}
		}
	}
	DumpValueStruct(rec);

	printf("***** CSV Pack *****\n");
	size = CSV1_SizeValue(rec,0,0);
	printf("CSV1 size = %d\n",size);
	size = CSV2_SizeValue(rec,0,0);
	printf("CSV2 size = %d\n",size);
	size = CSV3_SizeValue(rec,0,0);
	printf("CSV3 size = %d\n",size);
	size = CSVE_SizeValue(rec,0,0);
	printf("CSVE size = %d\n",size);
	buff = (char *)xmalloc(SIZE_BUFF);

	if		(  ( fp = fopen("test.csv","w") )  ==  NULL  ) 	exit(1);
	CSV1_PackValue(buff,rec,0);
	fprintf(fp,"%s\n",buff);
	CSV2_PackValue(buff,rec,0);
	fprintf(fp,"%s\n",buff);
	CSV3_PackValue(buff,rec,0);
	fprintf(fp,"%s\n",buff);
	CSVE_PackValue(buff,rec,0);
	fprintf(fp,"%s\n",buff);
	fclose(fp);
	xfree(buff);

	printf("***** CSV UnPack *****\n");
	if		(  ( fp = fopen("test.csv","r") )  ==  NULL  )	exit(1);
	buff = (char *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	fgets(buff,SIZE_BUFF,fp);
	CSV_UnPackValue(buff,rec,0);
	CSV3_PackValue(buff,rec,0);
	printf("%s\n",buff);

	CSV_UnPackValue(buff,rec,0);
	CSV3_PackValue(buff,rec,0);
	printf("%s\n",buff);

	CSV_UnPackValue(buff,rec,0);
	CSV3_PackValue(buff,rec,0);
	printf("%s\n",buff);

	CSV_UnPackValue(buff,rec,0);
	CSV3_PackValue(buff,rec,0);
	printf("%s\n",buff);

	xfree(buff);
	fclose(fp);

	printf("***** Native Pack *****\n");
	size =  NativeSizeValue(rec,0,0);
	printf("size = %d\n",size);
	buff = (char *)xmalloc(size);
	NativePackValue(buff,rec,0);
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
	NativeUnPackValue(buff,rec,0);
	DumpValueStruct(rec);
	memset(buff,0,SIZE_BUFF);
	CSV3_PackValue(buff,rec,0);
	printf("%s\n",buff);

	printf("***** RFC822 Pack *****\n");
	if		(  ( fp = fopen("test.822","w") )  ==  NULL  )	exit(1);
	RFC822_SetOptions("testrec",NULL,STRING_ENCODING_NULL,TRUE);
	size =  RFC822_SizeValue(rec,0,0);
	size1 = size;
	printf("size = %d\n",size);
	buff = (char *)xmalloc(size);
	RFC822_PackValue(buff,rec,0);
	printf("%s\n",buff);
	fwrite(buff,size,1,fp);

	RFC822_SetOptions("testrec",NULL,STRING_ENCODING_NULL,FALSE);
	size =  RFC822_SizeValue(rec,0,0);
	printf("size = %d\n",size);
	buff = (char *)xmalloc(size);
	RFC822_PackValue(buff,rec,0);
	printf("%s\n",buff);
	fwrite(buff,size,1,fp);

	fclose(fp);
	xfree(buff);

	printf("***** RFC822 UnPack *****\n");
	RFC822_SetOptions("testrec",NULL,STRING_ENCODING_NULL,TRUE);
	InitializeValue(rec);
	if		(  ( fp = fopen("test.822","r") )  ==  NULL  )	exit(1);
	buff = (char *)xmalloc(SIZE_BUFF);
	memset(buff,0,SIZE_BUFF);

	fread(buff,size1,1,fp);
	RFC822_UnPackValue(buff,rec,0);
	DumpValueStruct(rec);

	RFC822_SetOptions("testrec",NULL,STRING_ENCODING_NULL,FALSE);
	InitializeValue(rec);
	fread(buff,size,1,fp);
	fclose(fp);
	RFC822_UnPackValue(buff,rec,0);
	DumpValueStruct(rec);

	memset(buff,0,SIZE_BUFF);
	CSV3_PackValue(buff,rec,0);
	printf("%s\n",buff);

	printf("***** end *****\n");
	return	(0);
}
