#define	DEBUG
#define	TRACE

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
#include	"OpenCOBOL_v.h"
#include	"valueconv.h"
#include	"RecParser.h"
#include	"numerici.h"
#include	"memory_v.h"
#include	"misc_v.h"
#include	"LBSfunc.h"
#include	"others.h"

#include	"debug.h"

extern	int
main(
	int		argc,
	char	**argv)
{
	ValueStruct *val;
	ValueStruct *val2;
	ValueStruct *val3;
	ValueStruct *v;
	LargeByteString *lbs;
	size_t size;
	int i;
	char *p;
	const char *str = ""
"dummy	{"
	"id		char(8);"
	"num	int;"
	"query	{"
		"key	char(8);"
		"value	char(8);"
	"}[2];"
"};";

	RecParserInit();

fprintf(stderr, "\n\n---- normal\n");
	lbs = NewLBS();
	val = RecParseValueMem(str, NULL);
	val3 = RecParseValueMem(str, NULL);
	InitializeValue(val);
	v = GetItemLongName(val,"query[0].key");
	SetValueStringWithLength(v, "key0", strlen("key0"), NULL);
	v = GetItemLongName(val,"query[1].key");
	SetValueStringWithLength(v, "key1", strlen("key1"), NULL);
	v = GetItemLongName(val,"query[0].value");
	//SetValueStringWithLength(v, "value0", strlen("value0"), NULL);
	SetValueStringWithLength(v, "valvalvalvalvalvalvalue0", strlen("valvalvalvalvalvalue0"), NULL);
	v = GetItemLongName(val,"query[1].value");
	SetValueStringWithLength(v, "value1", strlen("value1"), NULL);

fprintf(stderr, "#dump before\n");
	DumpValueStruct(val);
fprintf(stderr, "#pack\n");
	size = NativeSizeValue(lbs,val);
fprintf(stderr, "#size = %ld\n",(long)size);
	LBS_ReserveSize(lbs, size, FALSE);
	NativePackValue(NULL, LBS_Body(lbs), val);

fprintf(stderr, "####\n");
p = LBS_Body(lbs);
for(i=0;i<size;i++) {
fprintf(stderr, "\\x%X",(unsigned int)*p);
p++;
}
fprintf(stderr, "####\n");

#if 1
fprintf(stderr, "#unpacknew\n");
	NativeUnPackValueNew(NULL, LBS_Body(lbs), &val2);
fprintf(stderr, "#dump after val2=%p\n", val2);
	DumpValueStruct(val2);
#endif

fprintf(stderr, "#nativeunpack\n");
	NativeUnPackValue(NULL, LBS_Body(lbs), val3);
fprintf(stderr, "#dump val3\n");
	DumpValueStruct(val3);
fprintf(stderr, "#end\n");

fprintf(stderr, "\n\n---- memclear\n");
	memset(LBS_Body(lbs), 0, size);
fprintf(stderr, "#unpack\n");
	NativeUnPackValueNew(NULL, LBS_Body(lbs), &val2);
fprintf(stderr, "#dump\n");
	DumpValueStruct(val2);

fprintf(stderr, "\n\n---- no elevate\n");
char *q1 = "\x82\x0\x0\x3\x0\x0\x0\x69\x64\x0\x21\x0\x80\x9\x0\x0\x0\x8\x0\x0\x0\x0\x6E\x75\x6D\x0\x11\x0\x80\x0\x0\x0\x0\x71\x75\x65\x72\x79\x0\x81\x0\x0\x2\x0\x0\x0\x82\x0\x0\x2\x0\x0\x0\x6B\x65\x79\x0\x21\x0\x0\x9\x0\x0\x0\x8\x0\x0\x0\x6B\x65\x79\x30\x0\x76\x61\x6C\x75\x65\x0\x21\x0\x0\x16\x0\x0\x0\x8\x0\x0\x0\x76\x61\x6C\x76\x61\x6C\x76\x61\x6C\x76\x61\x6C\x76\x61\x6C\x76\x61\x6C\x76\x61\x6C\x0\x82\x0\x0\x2\x0\x0\x0\x6B\x65\x79\x0\x21\x0\x0\x9\x0\x0\x0\x8\x0\x0\x0\x6B\x65\x79\x31\x0\x76\x61\x6C\x75\x65\x0\x21\x0\x0\x9\x0\x0\x0\x8\x0\x0\x0\x76\x61\x6C\x75\x65\x31\x0";
fprintf(stderr, "#unpack\n");
	NativeUnPackValueNew(NULL, q1, &val2);
fprintf(stderr, "#dump\n");
	DumpValueStruct(val2);

fprintf(stderr, "\n\n---- elevate 4byte\n");
char *q2 = "\x82\x32\x44\x82\x82\x0\x0\x3\x0\x0\x0\x69\x64\x0\x21\x0\x80\x9\x0\x0\x0\x8\x0\x0\x0\x0\x6E\x75\x6D\x0\x11\x0\x80\x0\x0\x0\x0\x71\x75\x65\x72\x79\x0\x81\x0\x0\x2\x0\x0\x0\x82\x0\x0\x2\x0\x0\x0\x6B\x65\x79\x0\x21\x0\x0\x9\x0\x0\x0\x8\x0\x0\x0\x6B\x65\x79\x30\x0\x76\x61\x6C\x75\x65\x0\x21\x0\x0\x16\x0\x0\x0\x8\x0\x0\x0\x76\x61\x6C\x76\x61\x6C\x76\x61\x6C\x76\x61\x6C\x76\x61\x6C\x76\x61\x6C\x76\x61\x6C\x0\x82\x0\x0\x2\x0\x0\x0\x6B\x65\x79\x0\x21\x0\x0\x9\x0\x0\x0\x8\x0\x0\x0\x6B\x65\x79\x31\x0\x76\x61\x6C\x75\x65\x0\x21\x0\x0\x9\x0\x0\x0\x8\x0\x0\x0\x76\x61\x6C\x75\x65\x31\x0";
fprintf(stderr, "#unpack\n");
	NativeUnPackValueNew(NULL, q2, &val2);
fprintf(stderr, "#dump\n");
	DumpValueStruct(val2);

	return 0;
}
