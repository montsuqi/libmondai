/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2004-2008 Ogochan.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#define MAIN

/*
#define	DEBUG
#define	TRACE
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <glib.h>
#include "types.h"
#include "libmondai.h"
#include "RecParser.h"
#include "option.h"
#include "debug.h"

static size_t InArraySize;
static size_t OutArraySize;
static size_t InTextSize;
static size_t OutTextSize;
static char *InLang;
static char *OutLang;
static char *InCode;
static char *OutCode;
static char *InEncode;
static char *OutEncode;
static size_t SizeBuff;
static Bool fInBigEndian;
static Bool fOutBigEndian;

static ARG_TABLE option[] = {
    {"inlang", STRING, TRUE, (void *)&InLang, "input language type name"},
    {"outlang", STRING, TRUE, (void *)&OutLang, "output language type name"},

    {"incode", STRING, TRUE, (void *)&InCode, "input character encoding"},
    {"outcode", STRING, TRUE, (void *)&OutCode, "output character encoding"},

    {"inencode", STRING, TRUE, (void *)&InEncode, "input string encoding"},
    {"outencode", STRING, TRUE, (void *)&OutEncode, "output string encoding"},

    {"intextsize", INTEGER, TRUE, (void *)&InTextSize,
     "input max length of text(for COBOL)"},
    {"inarraysize", INTEGER, TRUE, (void *)&InArraySize,
     "input max items of array(for COBOL)"},

    {"outtextsize", INTEGER, TRUE, (void *)&OutTextSize,
     "output max length of text(for COBOL)"},
    {"outarraysize", INTEGER, TRUE, (void *)&OutArraySize,
     "output max items of array(for COBOL)"},

    {"inbigendian", BOOLEAN, TRUE, (void *)&fInBigEndian,
     "input integer is bigendian"},
    {"outbigendian", BOOLEAN, TRUE, (void *)&fOutBigEndian,
     "output integer is bigendian"},

    {NULL, 0, FALSE, NULL, NULL}};

static void SetDefault(void) {
  InArraySize = 10;
  InTextSize = 100;
  OutArraySize = 10;
  OutTextSize = 100;

  InLang = "CSVE";
  OutLang = "CSV3";
  InCode = "euc-jp";
  OutCode = "euc-jp";

  InEncode = "null";
  OutEncode = "null";

  fInBigEndian = FALSE;
  fOutBigEndian = FALSE;

  SizeBuff = 1024 * 1024;
}

extern int main(int argc, char **argv) {
  FILE_LIST *fl;
  struct stat sb;
  char *out, *p;
  ConvFuncs *infunc, *outfunc;
  CONVOPT *inopt, *outopt;
  ValueStruct *rec;
  size_t isize, osize;
  ssize_t left;
  int type;
  char *ValueName;

  SetDefault();
  fl = GetOption(option, argc, argv,
                 "\tLang can accept\n"
                 "\tOpenCOBOL\n"
                 "\tdotCOBOL\n"
                 "\tCSV1(number and string delimited by \"\")\n"
                 "\tCSV2(number and string *not* delimitd by \"\")\n"
                 "\tCSV3(string delimitd by \"\")\n"
                 "\tCSVE(Excel type CSV)\n"
                 "\tCSV(same as CSV3)\n"
                 "\tSQL(SQL type)\n"
                 "\tXML1(tag is data class)\n"
                 "\tXML2(tag is data name)\n"
                 "\tCGI('name=value&...)\n"
                 "\tPHP(PHP serialize)\n"
                 "\tJSON(JSON serialize)\n"
                 "\tRFC822('name: value\\n')\n");

  RecParserInit();
  if ((fl->name == NULL) ||
      ((rec = RecParseValue(fl->name, &ValueName)) == NULL)) {
    fprintf(stderr, "can not found %s input record define.\n", fl->name);
    exit(1);
  }
  type = 0;
  if (!strlicmp(InLang, "xml")) {
    if (!strlicmp(InLang, "xml2")) {
      type = XML_TYPE2;
    } else {
      type = XML_TYPE1;
    }
    InLang = "XML";
  }
  if ((infunc = GetConvFunc(InLang)) == NULL) {
    fprintf(stderr, "can not found %s convert rule\n", InLang);
    exit(1);
  } else {
    inopt = NewConvOpt();
    ConvSetCodeset(inopt, InCode);
    ConvSetSize(inopt, InTextSize, InArraySize);
    if (!stricmp(InEncode, "url")) {
      ConvSetEncoding(inopt, STRING_ENCODING_URL);
    } else if (!stricmp(InEncode, "cgi")) {
      ConvSetEncoding(inopt, STRING_ENCODING_BASE64);
    } else {
      ConvSetEncoding(inopt, STRING_ENCODING_NULL);
    }
    if (!stricmp(InLang, "xml")) {
      ConvSetXmlType(inopt, type);
      ConvSetIndent(inopt, TRUE);
      ConvSetType(inopt, TRUE);
    }
    ConvSetRecName(inopt, StrDup(ValueName));
    ConvSetUseName(inopt, TRUE);
    inopt->fBigEndian = fInBigEndian;
  }
  if (!strlicmp(OutLang, "xml")) {
    if (!stricmp(OutLang, "xml2")) {
      type = XML_TYPE2;
    } else {
      type = XML_TYPE1;
    }
    OutLang = "XML";
  }
  if ((outfunc = GetConvFunc(OutLang)) == NULL) {
    fprintf(stderr, "can not found %s convert rule\n", OutLang);
    exit(1);
  } else {
    outopt = NewConvOpt();
    ConvSetCodeset(outopt, OutCode);
    ConvSetSize(outopt, OutTextSize, OutArraySize);
    if (!stricmp(OutEncode, "url")) {
      ConvSetEncoding(outopt, STRING_ENCODING_URL);
    } else if (!stricmp(OutEncode, "cgi")) {
      ConvSetEncoding(outopt, STRING_ENCODING_BASE64);
    } else {
      ConvSetEncoding(outopt, STRING_ENCODING_NULL);
    }
    if (!stricmp(OutLang, "xml")) {
      ConvSetXmlType(outopt, type);
      ConvSetIndent(outopt, TRUE);
      ConvSetType(outopt, TRUE);
    }
    ConvSetRecName(outopt, StrDup(ValueName));
    ConvSetUseName(outopt, TRUE);
    outopt->fBigEndian = fOutBigEndian;
  }

  fstat(fileno(stdin), &sb);
  if ((p = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fileno(stdin), 0)) !=
      NULL) {
    left = sb.st_size;
    out = (char *)xmalloc(SizeBuff);
    while (left > 0) {
      isize = infunc->UnPackValue(inopt, p, rec);
      left -= isize + strlen(infunc->bsep);
      p += isize + strlen(infunc->bsep);
      osize = outfunc->PackValue(outopt, out, rec);
      if (strlen(outfunc->bsep) > 0) {
        fwrite(out, osize, 1, stdout);
        fwrite(outfunc->bsep, strlen(outfunc->bsep), 1, stdout);
      } else {
        fwrite(out, osize, 1, stdout);
      }
    }
    munmap(p, sb.st_size);
  }

  return (0);
}
