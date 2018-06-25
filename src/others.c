/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2003 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2004-2009 Ogochan.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <glib.h>
#include <math.h>

#include "types.h"
#include "misc_v.h"
#include "value.h"
#include "hash_v.h"
#include "monstring.h"
#include "memory_v.h"
#include "others.h"
#include "debug.h"

extern char **ParCommandLine(char *line) {
  int n;
  char buff[SIZE_LONGNAME + 1];
  char *p, *q;
  char **cmd;

  ENTER_FUNC;
  n = 0;
  p = line;
  while (*p != 0) {
    n++;
    while ((*p != 0) && (!isspace(*p))) {
      p++;
    }
    while ((*p != 0) && (isspace(*p))) {
      p++;
    }
  }
  cmd = (char **)xmalloc(sizeof(char *) * (n + 1));
  p = line;
  n = 0;
  while (*p != 0) {
    q = buff;
    while ((*p != 0) && (!isspace(*p))) {
      *q++ = *p++;
    }
    *q = 0;
    cmd[n] = StrDup(buff);
    n++;
    while ((*p != 0) && (isspace(*p))) {
      p++;
    }
  }
  cmd[n] = NULL;
  LEAVE_FUNC;

  return (cmd);
}

extern char *ExpandPath(char *org, char *base) {
  static char path[SIZE_LONGNAME + 1];
  char buff[SIZE_LONGNAME + 1];
  char *p, *pp, *q;
  FILE *fp;
  int c;

  ENTER_FUNC;
  p = path;
  while (*org != 0) {
    switch (*org) {
    case '$':
      org++;
      pp = buff;
      while ((isalpha(*org)) || (isdigit(*org)) || (*org == '_')) {
        *pp++ = *org++;
      }
      *pp = 0;
      if ((q = getenv(buff)) != NULL) {
        p += sprintf(p, "%s", q);
      }
      break;
    case '`':
      org++;
      pp = buff;
      while ((*org != 0) && (*org != '`')) {
        if (*org == '\\') {
          org++;
        }
        *pp++ = *org++;
      }
      *pp = 0;
      if ((fp = popen(buff, "r")) != NULL) {
        while ((c = fgetc(fp)) != EOF) {
          if ((c != '\r') && (c != '\n')) {
            if (isspace(c)) {
              c = ' ';
            }
            *p++ = c;
          }
        }
        pclose(fp);
      }
      org++;
      break;
    case '~':
      p += sprintf(p, "%s", getenv("HOME"));
      org++;
      break;
    case '=':
      if (base == NULL) {
        if ((q = getenv("BASE_DIR")) != NULL) {
          p += sprintf(p, "%s", q);
        } else {
          p += sprintf(p, ".");
        }
      } else {
        p += sprintf(p, "%s", base);
      }
      org++;
      break;
    case '\\':
      org++;
      *p++ = *org++;
      break;
    default:
      *p++ = *org++;
      break;
    }
  }
  *p = 0;
  LEAVE_FUNC;
  return (path);
}

extern size_t DecodeStringURL(unsigned char *q, char *p) {
  unsigned char *qq = q;

  while ((*p != 0) && (isspace(*p)))
    p++;
  while (*p != 0) {
    if (*p == '+') {
      *q++ = ' ';
    } else if (*p == '%') {
      *q++ = (unsigned char)HexToInt(p + 1, 2);
      p += 2;
    } else {
      *q++ = *p;
    }
    p++;
  }
  *q = 0;
  return (q - qq);
}

extern size_t EncodeStringURL(char *q, char *p) {
  char *qq;

  qq = q;
  while (*p != 0) {
    if (*p == 0x20) {
      *q++ = '+';
    } else if (isalnum(*p)) {
      *q++ = *p;
    } else {
      *q++ = '%';
      q += sprintf(q, "%02X", ((int)*p) & 0xFF);
    }
    p++;
  }
  *q = 0;
  return (q - qq);
}

extern size_t EncodeStringLengthURL(char *p) {
  size_t ret;

  ret = 0;
  while (*p != 0) {
    if (*p == 0x20) {
      ret++;
    } else if (isalnum(*p)) {
      ret++;
    } else {
      ret += 3;
    }
    p++;
  }
  return (ret);
}

/*
 *	base64
 */

static char base64char[64] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char base64val[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1,
    -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1};

#define BASE64VAL(c) (isascii(c) ? base64val[(int)(c)] : -1)

extern size_t EncodeBase64(char *out, int size, unsigned char *in, size_t len) {
  unsigned char *inp = in;
  char *outp = out;

  while (len >= 3) {
    if ((size >= 0) && ((outp - out) >= size))
      break;
    *outp++ = base64char[(inp[0] >> 2) & 0x3f];
    *outp++ = base64char[((inp[0] & 0x03) << 4) | ((inp[1] >> 4) & 0x0f)];
    *outp++ = base64char[((inp[1] & 0x0f) << 2) | ((inp[2] >> 6) & 0x03)];
    *outp++ = base64char[inp[2] & 0x3f];
    inp += 3;
    len -= 3;
  }

  if (len > 0) {
    if ((size >= 0) && ((outp - out) >= size)) {
    } else {
      *outp++ = base64char[(inp[0] >> 2) & 0x3f];
      if (len == 1) {
        *outp++ = base64char[(inp[0] & 0x03) << 4];
        *outp++ = '=';
      } else {
        *outp++ = base64char[((inp[0] & 0x03) << 4) | ((inp[1] >> 4) & 0x0f)];
        *outp++ = base64char[((inp[1] & 0x0f) << 2)];
      }
      *outp++ = '=';
    }
  }

  *outp = 0;

  return (outp - out);
}

extern size_t EncodeLengthBase64(char *str) {
  return (((strlen(str) + 2) / 3) * 4);
}

extern size_t DecodeBase64(unsigned char *out, int size, char *in, size_t len) {
  char *inp = in;
  unsigned char *outp = out;
  char buf[4];

  while ((len >= 4) && (*inp != 0)) {
    if ((size >= 0) && ((outp - out) >= size))
      break;
    buf[0] = *inp++;
    len--;
    if (BASE64VAL(buf[0]) == -1)
      break;

    buf[1] = *inp++;
    len--;
    if (BASE64VAL(buf[1]) == -1)
      break;

    buf[2] = *inp++;
    len--;
    if (buf[2] != '=' && BASE64VAL(buf[2]) == -1)
      break;

    buf[3] = *inp++;
    len--;
    if (buf[3] != '=' && BASE64VAL(buf[3]) == -1)
      break;

    *outp++ =
        ((BASE64VAL(buf[0]) << 2) & 0xfc) | ((BASE64VAL(buf[1]) >> 4) & 0x03);
    if (buf[2] != '=') {
      *outp++ =
          ((BASE64VAL(buf[1]) & 0x0f) << 4) | ((BASE64VAL(buf[2]) >> 2) & 0x0f);
      if (buf[3] != '=') {
        *outp++ =
            ((BASE64VAL(buf[2]) & 0x03) << 6) | (BASE64VAL(buf[3]) & 0x3f);
      }
    }
  }

  return outp - out;
}

/*
 *	backslash encoding
 */

extern size_t EncodeStringLengthBackslash(char *p) {
  size_t ret;

  ret = 0;
  while (*p != 0) {
    if ((*p == '"') || (*p == '\\')) {
      ret += 2;
    } else {
      ret++;
    }
    p++;
  }
  return (ret);
}

extern size_t EncodeStringBackslash(char *q, char *p) {
  char *qq;

  qq = q;
  while (*p != 0) {
    if ((*p == '"') || (*p == '\\')) {
      *q++ = '\\';
    }
    *q++ = *p;
    p++;
  }
  *q = 0;
  return (q - qq);
}

/*
 *	backslash and CR,LF encoding
 */

extern size_t EncodeStringLengthBackslashCRLF(char *p) {
  size_t ret;

  ret = 0;
  while (*p != 0) {
    if ((*p == '"') || (*p == '\\') || (*p == '\n') || (*p == '\r')) {
      ret += 2;
    } else {
      ret++;
    }
    p++;
  }
  return (ret);
}

extern size_t EncodeStringBackslashCRLF(char *q, char *p) {
  char *qq;

  qq = q;
  while (*p != 0) {
    switch (*p) {
    case '\n':
      *q++ = '\\';
      *q++ = 'n';
      break;
    case '\r':
      *q++ = '\\';
      *q++ = 'r';
      break;
    case '"':
    case '\\':
      *q++ = '\\';
      *q++ = *p;
      break;
    default:
      *q++ = *p;
      break;
    }
    p++;
  }
  *q = 0;
  return (q - qq);
}
