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

#define	XML_TEST
/*
#define	CONV_TEST
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
#include	"libmondai.h"
#include	"debug.h"

typedef	struct {
	Bool	fIndent;
}	XMLOPT;

#define	ConvIndent(opt)			(((XMLOPT *)(opt)->appendix)->fIndent)
#define	ConvSetIndent(opt,v)	(((XMLOPT *)(opt)->appendix)->fIndent = (v))

static	char	*
XML_Encode(
	char	*str,
	char	*buff)
{
	char	*p;

	p = buff;
	for	( ; *str != 0 ; str ++ ) {
		if		(  *str  <  0x20  ) {
			p += sprintf(p,"\\%02X",*str);
		} else {
			*p ++ = *str;
		}
	}
	*p = 0;
	return	(buff);
}

static	byte	*
_XML_PackValue(
	CONVOPT		*opt,
	 byte		*p,
	char		*name,
	ValueStruct	*value,
	char		*buff)
{
	char	num[SIZE_NAME+1];
	int		i;

	if		(  value  !=  NULL  ) {
		switch	(ValueType(value)) {
		  case	GL_TYPE_INT:
			p += sprintf(p,"<VALUE name=\"%s\" type=\"int\"/>%s\n",name,ValueToString(value));
			break;
		  case	GL_TYPE_BOOL:
			p += sprintf(p,"<VALUE name=\"%s\" type=\"bool\"/>%s\n",name,ValueToString(value));
			break;
		  case	GL_TYPE_BYTE:
			p += sprintf(p,"<VALUE name=\"%s\" type=\"byte\"/>%s\n"
						 ,name,XML_Encode(ValueToString(value),buff));
			break;
		  case	GL_TYPE_CHAR:
			p += sprintf(p,"<VALUE name=\"%s\" type=\"char\" size=\"%d\" />%s\n"
						 ,name,ValueStringLength(value),XML_Encode(ValueString(value),buff));
			break;
		  case	GL_TYPE_VARCHAR:
			p += sprintf(p,"<VALUE name=\"%s\" type=\"varchar\" size=\"%d\" />%s\n"
						 ,name,ValueStringLength(value),XML_Encode(ValueString(value),buff));
			break;
		  case	GL_TYPE_TEXT:
			p += sprintf(p,"<VALUE name=\"%s\" type=\"text\" size=\"%d\" />%s\n"
						 ,name,ValueStringLength(value),XML_Encode(ValueString(value),buff));
			break;
		  case	GL_TYPE_DBCODE:
			p += sprintf(p,"<VALUE name=\"%s\" type=\"dbcode\" size=\"%d\" />%s\n"
						 ,name,ValueStringLength(value),XML_Encode(ValueString(value),buff));
			break;
		  case	GL_TYPE_NUMBER:
			p += sprintf(p,"<VALUE name=\"%s\" type=\"number\"/>%s\n"
						 ,name,XML_Encode(ValueToString(value),buff));
			break;
		  case	GL_TYPE_FLOAT:
			p += sprintf(p,"<VALUE name=\"%s\" type=\"float\"/>%s\n"
						 ,name,XML_Encode(ValueToString(value),buff));
			break;
		  case	GL_TYPE_ARRAY:
			p += sprintf(p,"<ARRAY name=\"%s\" count=\"%d\">\n"
						 ,name,ValueArraySize(value));
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				sprintf(num,"%d",i);
				p = _XML_PackValue(opt,p,num,ValueArrayItem(value,i),buff);
			}
			p += sprintf(p,"</ARRAY>\n");
			break;
		  case	GL_TYPE_RECORD:
			p += sprintf(p,"<RECORD name=\"%s\" size=\"%d\">\n"
						 ,name,ValueRecordSize(value));
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p = _XML_PackValue(opt,p,ValueRecordName(value,i),ValueRecordItem(value,i),buff);
			}
			p += sprintf(p,"</RECORD>\n");
			break;
		  case	GL_TYPE_ALIAS:
		  default:
			break;
		}
	}
	return	(p);
}

extern	byte	*
XML_PackValue(
	CONVOPT		*opt,
	 byte		*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF+1];

	_XML_PackValue(opt,p,opt->recname,value,buff);
	return	(p);
}

extern	byte	*
XML_UnPackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value)
{
}

extern	size_t
XML_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value)
{
}



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
	SetValueString(GetItemLongName(val,"a"),"aaa");
	SetValueString(GetItemLongName(val,"b"),"bb");
	SetValueString(GetItemLongName(val,"g"),"NIL");
	ValueAttribute(GetItemLongName(val,"g")) |= GL_ATTR_NIL;

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
	//	DumpValueStruct(val);
#ifdef	XML_TEST
	opt = NewConvOpt();
	buff = xmalloc(SIZE_BUFF);
	printf("***** XML Pack *****\n");
	ConvSetRecName(opt,"testrec");
	//	ConvSetUseName(opt,TRUE);
	ConvSetIndent(opt,TRUE);
	XML_PackValue(opt,buff,val);
	printf("%s\n",buff);
	printf("********************\n");
#endif

#ifdef	CONV_TEST
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
	fgets(buff,SIZE_BUFF,fp);
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
