#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libmondai.h>
#include <RecParser.h>

const char *recdef = ""
                     "test_ {\n"
                     "  str varchar(128);\n"
                     "};";

/*euc-jisx0213範囲外の文字が?の列になること*/
int main(int argc, char *argv[]) {

  ValueStruct *value, *v;
  size_t size;
  char *buf;
  const char *src = "\xE3\x90\x83あいう\xF0\xA0\xAE\xB7えお\xE3\x90\x83かき\xE3\x90\x83﨑髙";
  FILE *fp;

  CONVOPT *opt = NewConvOpt();
  ConvSetSize(opt, 1024, 10);
  ConvSetCodeset(opt, "euc-jisx0213");
  buf = (unsigned char *)xmalloc(SIZE_BUFF);
  memset(buf, 0, SIZE_BUFF);

  RecParserInit();
  value = RecParseValueMem(recdef, NULL);

  /* valueにstr(utf-8)を設定*/
  InitializeValue(value);
  v = GetRecordItem(value, "str");
  fprintf(stderr,"src:%s\n",src);
  SetValueString(v, src, NULL);

  /* OpenCOBOL_PackValueでutf8からeuc-jisx0213に変換 */
  size = OpenCOBOL_PackValue(opt, buf, value);
  /* euc-jisx0213をファイル出力 */
  if ((fp = fopen("/tmp/opencobol.txt", "w")) == NULL)
    exit(1);
  fwrite(buf, size, 1, fp);
  fclose(fp);

  /* OpenCOBOL_UnPackValueでeuc-jisx0213からutf8に変換 */
  OpenCOBOL_UnPackValue(opt, buf, value);
  v = GetRecordItem(value, "str");
  fprintf(stderr,"dst:%s\n",ValueToString(v,NULL));
  xfree(buf);

/*
src:㐃あいう𠮷えお㐃かき㐃﨑髙
LBSfunc.c:382:convert 髙 -> ■
dst:???あいう????えお???かき???﨑■
*/

  return 0;
}
