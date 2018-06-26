/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 1989-2008 Ogochan.
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

#ifndef _INC_MONDAI_STRING_H
#define _INC_MONDAI_STRING_H

#include <string.h>
#include "types.h"

#define memclear(b, s) memset((b), 0, (s))
#define strlcmp(s1, s2) strncmp((s1), (s2), strlen(s2))
#define strlicmp(s1, s2) strnicmp((s1), (s2), strlen(s2))

#ifdef __GNUC__
extern int stricmp(const char *s1, const char *s2);
extern int strnicmp(const char *s1, const char *s2, size_t l);
#endif

extern char *StrDup(const char *s);
extern char *StrnDup(const char *s, size_t len);
extern char *IntStrDup(int val);
extern char *StringChop(char *str);
extern long StrToInt(const char *str, size_t len);
extern long HexToInt(const char *str, size_t len);
extern char *IntToStr(char *str, long val, size_t len);
extern size_t CharLength(unsigned char c);

#endif
