#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libmondai.h>

int
main(int argc,char *argv[])
{
  Numeric num;
  char *out,buf[64],*format;

  format = "---,---";

  num = DoubleToNumeric(100);
  out = NumericOutput(num);
  NumericFormat(buf,format, num);
  fprintf(stderr,"%s %s\n",out,buf);
  xfree(out);

  num = DoubleToNumeric(-100);
  out = NumericOutput(num);
  NumericFormat(buf,format, num);
  fprintf(stderr,"%s %s\n",out,buf);
  xfree(out);

  num = DoubleToNumeric(-300);
  out = NumericOutput(num);
  NumericFormat(buf,format, num);
  fprintf(stderr,"%s %s\n",out,buf);
  xfree(out);
  
  return 0;
}
