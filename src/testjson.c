#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libmondai.h>
#include <RecParser.h>

const char *recdef = ""
"test2 {\n"
"  int0 int[10];\n"
"  int1 int;\n"
"  int2 int;\n"
"  double1 number(10,5);\n"
"  bool1 bool;\n"
"  command varchar(256);\n"
"  arg1 varchar(256);\n"
"  arg2 varchar(256);\n"
"  arg3 varchar(256);\n"
"  status int;\n"
"  record1 {\n"
"    col1 varchar(256);\n"
"    col2 varchar(256);\n"
"    col3 varchar(256);\n"
"    record2 {\n"
"      col21 varchar(256);\n"
"    };\n"
"  }[3];\n"
"  record3 {\n"
"    vc31 varchar(256);\n"
"    record4 {\n"
"      vc41 varchar(256);\n"
"      int41 int;\n"
"    }[2];\n"
"  }[2];\n"
"};";

const char *recdef2 = ""
"test2 {\n"
"  int1 int;\n"
"  arg1 varchar(256);\n"
"  arg2 varchar(256);\n"
"};";

int
main(int argc,char *argv[])
{
  ValueStruct *value,*v;
  size_t size;
  char *buf;

  RecParserInit();
  value = RecParseValueMem(recdef,NULL);

  InitializeValue(value);
  v = GetRecordItem(value,"command");
  SetValueString(v,"a\"a\\a/a\ba\fa\na\ra\ta",NULL);
  v = GetItemLongName(value,"record1[0].col1");
  SetValueString(v,"bbbb",NULL);
  v = GetItemLongName(value,"record1[0].record2.col21");
  SetValueString(v,"cccc",NULL);
  v = GetItemLongName(value,"int1");
  ValueInteger(v) = 10;
  v = GetItemLongName(value,"int2");
  ValueInteger(v) = 20;

  fprintf(stderr,"\n---- JSON_PackValue\n");
  size = JSON_SizeValue(NULL,value);
  fprintf(stderr,"size:%ld\n",size);
  buf = malloc(size+1);
  memset(buf,0,size+1);
  JSON_PackValue(NULL,buf,value);
  fprintf(stderr,"[%s]\nsize:%ld\n",buf,strlen(buf));

  fprintf(stderr,"\n---- JSON_UnPackValue 1\n");
  JSON_UnPackValue(NULL,buf,value);
  DumpValueStruct(value);
  free(buf);

  fprintf(stderr,"\n---- JSON_UnPackValue 2\n");
  JSON_UnPackValue(NULL,"{\"int1\":1000,\"int2\":2000}",value);
  DumpValueStruct(value);

  /* ommit */
  fprintf(stderr,"\n-------------------\n");
  fprintf(stderr,"ommit\n");
  fprintf(stderr,"-------------------\n\n");

  InitializeValue(value);
  v = GetRecordItem(value,"command");
  SetValueString(v,"a\"a\\a/a\ba\fa\na\ra\ta",NULL);
  v = GetItemLongName(value,"record1[0].col1");
  SetValueString(v,"bbbb",NULL);
  v = GetItemLongName(value,"record1[0].record2.col21");
  SetValueString(v,"cccc",NULL);
  v = GetItemLongName(value,"int1");
  ValueInteger(v) = 10;
  v = GetItemLongName(value,"int2");
  ValueInteger(v) = 20;
#if 1
  v = GetItemLongName(value,"record3[1].record4[1].vc41");
  SetValueString(v,"vc41",NULL);
#endif

  fprintf(stderr,"\n---- JSON_PackValueOmmit\n");
  size = JSON_SizeValueOmmit(NULL,value);
  fprintf(stderr,"size:%ld\n",size);
  buf = malloc(size+1);
  memset(buf,0,size+1);
  JSON_PackValueOmmit(NULL,buf,value);
  fprintf(stderr,"size:%ld [%s]\n",strlen(buf),buf);
  free(buf);

  fprintf(stderr,"\n---- JSON_UnPackValueOmmit\n");
  JSON_UnPackValueOmmit(NULL,"{\"int1\":1000,\"int2\":2000}",value);
  DumpValueStruct(value);

  fprintf(stderr,"\n---- JSON_UnPackValueOmmit 2\n");
  JSON_UnPackValueOmmit(NULL,"{\"int1\":1234,\"int2\":5678,\"bool1\":false,\"double1\":3.141592}",value);
  DumpValueStruct(value);

  fprintf(stderr,"\n---- JSON_UnPackValueOmmit 3\n");
  JSON_UnPackValueOmmit(NULL,"{\"int1\":1000,\"command\":\"moge\",\"record1\":[{\"col1\":\"muge\",\"record2\":{\"col21\":\"gage\"}},{},{\"col2\":\"nuge\"}],\"record3\":[{},{\"record4\":[{},{\"vc41\":\"vc41\"}]}]}",value);
  DumpValueStruct(value);

  InitializeValue(value);
  fprintf(stderr,"\n---- test\n");
  JSON_UnPackValueOmmit(NULL,"{\"int1\":10,\"int0\":[0],\"bool1\":false,\"arg1\":\"hogehoge\"}",value);
  size = JSON_SizeValueOmmit(NULL,value);
  fprintf(stderr,"size:%ld\n",size);
  buf = malloc(size+1);
  memset(buf,0,size+1);
  JSON_PackValueOmmit(NULL,buf,value);
  fprintf(stderr,"size:%ld %s\n",strlen(buf),buf);
  free(buf);

  return 0;
}
