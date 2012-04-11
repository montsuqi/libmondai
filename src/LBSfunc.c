/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2002-2003 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2004-2008 Ogochan
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
#  include <config.h>
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<errno.h>
#ifdef	WITH_I18N
#include	<iconv.h>
#include	<wchar.h>
#endif
#include	<math.h>

#include	"types.h"
#include	"libmondai.h"
#include	"LBSfunc.h"
#include	"debug.h"

extern	LargeByteString	*
NewLBS(void)
{
	LargeByteString	*lbs;

	lbs = New(LargeByteString);
	lbs->ptr = 0;
	lbs->size = 0;
	lbs->asize = 0;
	lbs->body = NULL;

	return	(lbs);
}

extern	void
FreeLBS(
	LargeByteString	*lbs)
{
	if		(  lbs  !=  NULL  ) {
		if		(  lbs->body  !=  NULL  ) {
			xfree(lbs->body);
		}
		xfree(lbs);
	}
}

extern void
LBS_Glown(
	LargeByteString *lbs,
	size_t			size,
	Bool			fKeep)
{
	unsigned char   *body;
	
	if ( lbs->asize < size ) {
		body = (unsigned char *)xmalloc(size);
		if (  fKeep  ) {
			memcpy(body,lbs->body,lbs->size);
		} else {
			memclear(body,size);
		}
		if		(  lbs->body  !=  NULL  ) {
			xfree(lbs->body);
		}
		lbs->body = body;
		lbs->asize = size;
	}
}

extern	void
LBS_ReserveSize(
	LargeByteString	*lbs,
	size_t			size,
	Bool			fKeep)
{
	if		(  lbs  !=  NULL  ) {
		LBS_Glown(lbs, size, fKeep);
		lbs->size = size;
		lbs->ptr = size;
	}
}

extern	void
LBS_Seek(
	LargeByteString	*lbs,
	size_t			off,
	int				whence)
{
	size_t		newpos;

	if		(  lbs  !=  NULL  ) {
		switch	(whence) {
		  case	SEEK_SET:
			newpos = off;
			break;
		  case	SEEK_CUR:
			newpos = lbs->ptr + off;
			break;
		  case	SEEK_END:
			newpos = lbs->size - off;
			break;
		  default:
			newpos = lbs->ptr;
			break;
		}
		if		(  newpos  >  lbs->size  ) {
			LBS_ReserveSize(lbs,newpos,TRUE);
		}
		lbs->ptr = newpos;
	}
}

extern	int
LBS_FetchByte(
	LargeByteString	*lbs)
{
	int		ret;

	if		(  lbs  !=  NULL  ) {
		if		(  lbs->ptr  <  lbs->size  ) {
			ret = lbs->body[lbs->ptr];
			lbs->ptr ++;
		} else {
			ret = -1;
		}
	} else {
		ret = -1;
	}
	return	(ret);
}

extern	int
LBS_FetchChar(
	LargeByteString	*lbs)
{
	int		ret;

	if		(  ( ret = LBS_FetchByte(lbs) )  <  0  )	{
		ret = 0;
	}
	return	(ret);
}

extern	void	*
LBS_FetchPointer(
	LargeByteString	*lbs)
{
#ifdef	__LP64__
	uint64_t	ret;
#else
	uint32_t	ret;
#endif
	int		i;

	if		(  lbs  !=  NULL  ) {
		ret = 0;
		for	( i = 0 ; i < sizeof(ret) ; i ++ ) {
			ret <<= 8;
			ret |= LBS_FetchByte(lbs);
		}
	} else {
		ret = 0;
	}
	return	((void *)ret);
}

extern	int
LBS_FetchInt(
	LargeByteString	*lbs)
{
	int		ret;
	int		i;

	ret = 0;
	if		(  lbs  !=  NULL  ) {
		for	( i = 0 ; i < sizeof(int) ; i ++ ) {
			ret <<= 8;
			ret |= LBS_FetchByte(lbs);
		}
	}
	return	(ret);
}

extern	uint64_t
LBS_Fetch64(
	LargeByteString	*lbs)
{
	uint64_t	ret;
	int			i;

	ret = 0;
	if		(  lbs  !=  NULL  ) {
		for	( i = 0 ; i < sizeof(uint64_t) ; i ++ ) {
			ret <<= 8;
			ret |= LBS_FetchByte(lbs);
		}
	}
	return	(ret);
}

extern	void
LBS_EmitStart(
	LargeByteString	*lbs)
{
	if		(  lbs  !=  NULL  ) {
		lbs->ptr = 0;
		lbs->size = 0;
		if		(  lbs->asize  >  0  ) {
			memclear(lbs->body,lbs->asize);
		}
	}
}

extern	void
LBS_Emit(
	LargeByteString	*lbs,
	unsigned char			code)
{
	if		(  lbs  !=  NULL  ) {
		if		(  lbs->ptr  ==  lbs->asize  ) {
			LBS_Glown(lbs, lbs->asize + SIZE_GLOWN, TRUE);
		}
		lbs->body[lbs->ptr] = code;
		lbs->ptr ++;
		if		(  lbs->ptr  >  lbs->size  ) {
			lbs->size = lbs->ptr;
		}
	}
}

extern	void
LBS_EmitEnd(
	LargeByteString	*lbs)
{
	if ( (lbs->ptr == 0) || (lbs->body[lbs->ptr - 1] != '\0') ) {
		LBS_Glown(lbs, lbs->size + 1, TRUE);
		LBS_Emit(lbs,'\0');
	}
}

extern	void
LBS_Trim(
	LargeByteString	*lbs,
	size_t			size)
{
	if		(  lbs  !=  NULL  ) {
		if		(  lbs->ptr  >=  size  ) {
			memclear(&lbs->body[lbs->ptr - size],size);
			lbs->ptr -= size;
			lbs->size -= size;
		}
	}
}

extern	void
LBS_EmitString(
	LargeByteString	*lbs,
	char			*str)
{
	if		(  lbs  !=  NULL  ) {
		while	(  *str  !=  0  ) {
			LBS_EmitChar(lbs,*str);
			str ++;
		}
	}
}

extern	void
LBS_String(
	LargeByteString	*lbs,
	char			*str)
{
	LBS_EmitStart(lbs);
	LBS_EmitString(lbs, str);
	LBS_EmitEnd(lbs);
}

static GRegex *reg = NULL;

static gboolean
eval_cb1(
	const GMatchInfo *info,
	GString *res,
	gpointer data)
{
	gchar *match;

	match = g_match_info_fetch(info, 1);
	if (!strcmp(match,"\xEF\xBC\x8D")) {
		g_string_append (res, "\xE2\x88\x92");
	} else if (!strcmp(match,"\xEF\xBD\x9E")) {
		g_string_append (res, "\xE3\x80\x9C");
	} else if (!strcmp(match,"\xE2\x88\xA5")) {
		g_string_append (res, "\xE2\x80\x96");
	} else if (!strcmp(match,"\xEF\xBF\xA0")) {
		g_string_append (res, "\xC2\xA2");
	} else if (!strcmp(match,"\xEF\xBF\xA1")) {
		g_string_append (res, "\xC2\xA3");
	} else if (!strcmp(match,"\xEF\xBF\xA2")) {
		g_string_append (res, "\xC2\xAC");
	}
	g_free(match);
	return FALSE;
}

static gchar*
UTF8Normalize(
	gchar *str)
{
	gchar *ret;

	if (reg == NULL) {
		reg = g_regex_new("([" 
			"\xEF\xBC\x8D"
			"\xEF\xBD\x9E"
			"\xE2\x88\xA5"
			"\xEF\xBF\xA0"
			"\xEF\xBF\xA1"
			"\xEF\xBF\xA2"
			"])",0,0,NULL);
	}

	ret = g_regex_replace_eval(reg,str,-1,0,0,eval_cb1,NULL,NULL);
	if (ret == NULL) {
		ret = g_strdup(str);
	}
	return ret;
}

static int
ConvertForJISX0213(
    char *string)
{
    int i;
    const char *table[][2] = {
{"\xE2\x80\x95","\xE2\x80\x94"},//―、—
{"\xEF\xBC\x8D","\xE2\x88\x92"},//－,−
{"\xE4\xBB\xBC","\xE2\x96\xA0"},//仼
{"\xE5\x9D\x99","\xE2\x96\xA0"},//坙,■
{"\xE5\x9D\xA5","\xE2\x96\xA0"},//坥,■
{"\xE5\xA2\xB2","\xE2\x96\xA0"},//墲,■
{"\xE5\xA5\x93","\xE2\x96\xA0"},//奓,■
{"\xE5\xA5\xA3","\xE2\x96\xA0"},//奣,■
{"\xE5\xA6\xBA","\xE2\x96\xA0"},//妺,■
{"\xE5\xB3\xB5","\xE2\x96\xA0"},//峵,■
{"\xE5\xB7\x90","\xE2\x96\xA0"},//巐,■
{"\xE5\xBC\xA1","\xE2\x96\xA0"},//弡,■
{"\xE6\x81\x9D","\xE2\x96\xA0"},//恝,■
{"\xE6\x82\x85","\xE2\x96\xA0"},//悅,■
{"\xE6\x83\x9E","\xE2\x96\xA0"},//惞,■
{"\xE6\x84\xA0","\xE2\x96\xA0"},//愠,■
{"\xE6\x84\x91","\xE2\x96\xA0"},//愑,■
{"\xE6\x88\x93","\xE2\x96\xA0"},//戓,■
{"\xE6\x95\x8E","\xE2\x96\xA0"},//敎,■
{"\xE6\x98\xBB","\xE2\x96\xA0"},//昻,■
{"\xE6\x98\xAE","\xE2\x96\xA0"},//昮,■
{"\xEF\xA8\x92","\xE2\x96\xA0"},//晴,■
{"\xE6\x9C\x8E","\xE2\x96\xA0"},//朎,■
{"\xE6\xAB\xA2","\xE2\x96\xA0"},//櫢,■
{"\xE6\xB1\xAF","\xE2\x96\xA0"},//汯,■
{"\xE6\xB5\xAF","\xE2\x96\xA0"},//浯,■
{"\xE6\xB6\x96","\xE2\x96\xA0"},//涖,■
{"\xE6\xB7\xB8","\xE2\x96\xA0"},//淸,■
{"\xE6\xB7\xB2","\xE2\x96\xA0"},//淲,■
{"\xE6\xB8\xB9","\xE2\x96\xA0"},//渹,■
{"\xE7\x8C\xA4","\xE2\x96\xA0"},//猤,■
{"\xE7\x8E\xBD","\xE2\x96\xA0"},//玽,■
{"\xE7\x8F\x92","\xE2\x96\xA0"},//珒,■
{"\xE7\x8F\xB5","\xE2\x96\xA0"},//珵,■
{"\xE7\x90\xA9","\xE2\x96\xA0"},//琩,■
{"\xE7\x9A\x82","\xE2\x96\xA0"},//皂,■
{"\xEF\xA8\x97","\xE2\x96\xA0"},//益,■
{"\xE7\xA1\xBA","\xE2\x96\xA0"},//硺,■
{"\xEF\xA8\x98","\xE2\x96\xA0"},//礼,■
{"\xEF\xA8\x9C","\xE2\x96\xA0"},//靖,■
{"\xEF\xA8\x9D","\xE2\x96\xA0"},//精,■
{"\xE7\xBE\xA1","\xE2\x96\xA0"},//羡,■
{"\xEF\xA8\x9E","\xE2\x96\xA0"},//羽,■
{"\xE8\x8F\xB6","\xE2\x96\xA0"},//菶,■
{"\xE8\x95\xAB","\xE2\x96\xA0"},//蕫,■
{"\xE8\xA0\x87","\xE2\x96\xA0"},//蠇,■
{"\xE8\xAD\x93","\xE2\x96\xA0"},//譓,■
{"\xE8\xB5\xB6","\xE2\x96\xA0"},//赶,■
{"\xEF\xA8\xA3","\xE2\x96\xA0"},//﨣,■
{"\xE8\xBB\x8F","\xE2\x96\xA0"},//軏,■
{"\xEF\xA8\xA5","\xE2\x96\xA0"},//逸,■
{"\xE9\x81\xA7","\xE2\x96\xA0"},//遧,■
{"\xE9\x87\x9E","\xE2\x96\xA0"},//釞,■
{"\xE9\x88\x86","\xE2\x96\xA0"},//鈆,■
{"\xE9\x89\xB7","\xE2\x96\xA0"},//鉷,■
{"\xEF\xA8\xA7","\xE2\x96\xA0"},//﨧,■
{"\xE9\x8B\x95","\xE2\x96\xA0"},//鋕,■
{"\xEF\xA8\xA8","\xE2\x96\xA0"},//﨨,■
{"\xE9\x8E\xA4","\xE2\x96\xA0"},//鎤,■
{"\xE9\x8F\xB8","\xE2\x96\xA0"},//鏸,■
{"\xE9\x90\xB1","\xE2\x96\xA0"},//鐱,■
{"\xE9\x91\x88","\xE2\x96\xA0"},//鑈,■
{"\xE9\x96\x92","\xE2\x96\xA0"},//閒,■
{"\xEF\xA8\xA9","\xE2\x96\xA0"},//﨩,■
{"\xE9\x9D\x83","\xE2\x96\xA0"},//靃,■
{"\xE9\x9D\x91","\xE2\x96\xA0"},//靑,■
{"\xEF\xA8\xAA","\xE2\x96\xA0"},//飯,■
{"\xEF\xA8\xAB","\xE2\x96\xA0"},//飼,■
{"\xE9\xA4\xA7","\xE2\x96\xA0"},//餧,■
{"\xEF\xA8\xAC","\xE2\x96\xA0"},//館,■
{"\xE9\xAB\x99","\xE2\x96\xA0"},//髙,■
{"\xE9\xAE\xBB","\xE2\x96\xA0"},//鮻,■
{"\xEF\xA8\xAD","\xE2\x96\xA0"},//鶴,■
{"",""},
	};
	for(i=0; strlen(table[i][0]) != 0;i++) {
		if (strstr(string, table[i][0]) == string) {
			MonWarningPrintf("convert %s -> %s", table[i][0], table[i][1]);
			memcpy(string, table[i][1], strlen(table[i][1]));
			return 1;
		}
	}
	return 0;
}

#define	SIZE_CONV		10

extern	void
LBS_EmitStringCodeset(
	LargeByteString	*lbs,
	char			*str,
	size_t			isize,
	size_t			osize,
	char			*codeset)
{
#ifdef	WITH_I18N
	char		*oc
	,			*istr
	,			*buff
	,			*nstr;
	size_t		sib
		,		sob;
	iconv_t		cd;
	int			rc
	,			i;
	size_t		obsize
	,			ssize;
#endif

ENTER_FUNC;
 	if		(  lbs  !=  NULL  ) {
		if		( (str == NULL) || (strlen(str) == 0 ) ){
			return;
		}
#ifdef	WITH_I18N
		if		(  codeset  !=  NULL  ) {
			cd = iconv_open(codeset,"utf8");
			istr =nstr =  UTF8Normalize(str);
			sib = strlen(istr) + 1;
			obsize = isize * 2 + 1;
			LBS_ReserveSize(lbs, obsize, TRUE);
			oc = (char *)LBS_Body(lbs);
			sob = obsize;
			while	(TRUE) {
				if		(  ( rc = iconv(cd,&istr,&sib,&oc,&sob) )  ==  0  )	{
					break;
				}
				if (errno == EILSEQ) {
					if (!ConvertForJISX0213(istr)) {
						buff = xmalloc(sib * 4 + 1);
						for (i = 0;i < sib; i++) {
							sprintf(buff + i * 4, "\\x%02X", istr[i]);
						}
						MonWarningPrintf("iconv EILSEQ:%s", buff);
						xfree(buff);

						*istr = 0;
						sib = 1;
					}
				} else if (errno == E2BIG){
					MonWarningPrintf("iconv failure %s", strerror(errno));
					break;
				} else {
					buff = xmalloc(sib * 4 + 1);
					for (i = 0;i < sib; i++) {
						sprintf(buff + i * 4, "\\x%02X", istr[i]);
					}
					MonWarningPrintf("iconv failure %d str:%s", errno, buff);
					xfree(buff);
					break;
				}
			}
			*oc = 0;
			ssize = obsize - sob + 1;
			lbs->size = ssize;
			lbs->ptr = lbs->size;
			g_free(nstr);
			iconv_close(cd);
		} else {
#endif
			LBS_Glown(lbs, isize + 1, FALSE);
			memcpy(lbs->body, str, isize);
			lbs->size = isize;
			lbs->ptr = lbs->size;
#ifdef	WITH_I18N
		}
#endif
	}
LEAVE_FUNC;
}

extern	void
LBS_EmitPointer(
	LargeByteString	*lbs,
	void			*p)
{
 	if		(  lbs  !=  NULL  ) {
#ifdef	__LP64__
		LBS_Emit(lbs,(((uint64_t)p & 0xFF00000000000000) >> 56));
		LBS_Emit(lbs,(((uint64_t)p & 0x00FF000000000000) >> 48));
		LBS_Emit(lbs,(((uint64_t)p & 0x0000FF0000000000) >> 40));
		LBS_Emit(lbs,(((uint64_t)p & 0x000000FF00000000) >> 32));
		LBS_Emit(lbs,(((uint64_t)p & 0x00000000FF000000) >> 24));
		LBS_Emit(lbs,(((uint64_t)p & 0x0000000000FF0000) >> 16));
		LBS_Emit(lbs,(((uint64_t)p & 0x000000000000FF00) >>  8));
		LBS_Emit(lbs,(((uint64_t)p & 0x00000000000000FF)      ));
#else
		LBS_Emit(lbs,(((uint32_t)p & 0xFF000000) >> 24));
		LBS_Emit(lbs,(((uint32_t)p & 0x00FF0000) >> 16));
		LBS_Emit(lbs,(((uint32_t)p & 0x0000FF00) >>  8));
		LBS_Emit(lbs,(((uint32_t)p & 0x000000FF)      ));
#endif
	}
}

extern	void
LBS_EmitInt(
	LargeByteString	*lbs,
	int				i)
{
 	if		(  lbs  !=  NULL  ) {
		LBS_Emit(lbs,((i & 0xFF000000) >> 24));
		LBS_Emit(lbs,((i & 0x00FF0000) >> 16));
		LBS_Emit(lbs,((i & 0x0000FF00) >>  8));
		LBS_Emit(lbs,((i & 0x000000FF)      ));
	}
}

extern	void
LBS_Emit64(
	LargeByteString	*lbs,
	uint64_t		i)
{
 	if		(  lbs  !=  NULL  ) {
		LBS_Emit(lbs,((i & 0xFF00000000000000LL) >> 56));
		LBS_Emit(lbs,((i & 0x00FF000000000000LL) >> 48));
		LBS_Emit(lbs,((i & 0x0000FF0000000000LL) >> 40));
		LBS_Emit(lbs,((i & 0x000000FF00000000LL) >> 32));
		LBS_Emit(lbs,((i & 0x00000000FF000000LL) >> 24));
		LBS_Emit(lbs,((i & 0x0000000000FF0000LL) >> 16));
		LBS_Emit(lbs,((i & 0x000000000000FF00LL) >>  8));
		LBS_Emit(lbs,((i & 0x00000000000000FFLL)      ));
	}
}

extern	void
LBS_EmitFix(
	LargeByteString	*lbs)
{
	unsigned char	*body;

 	if		(  lbs  !=  NULL  ) {
		if ( lbs->asize != lbs->size ) {
			if		(  lbs->size  >  0  ) {
				body = (unsigned char *)xmalloc(lbs->size);
				memcpy(body,lbs->body,lbs->size);
				xfree(lbs->body);
				lbs->body = body;
			} else {
				xfree(lbs->body);
				lbs->body = NULL;
			}
			lbs->asize = lbs->size;
		}
		lbs->ptr = 0;
	}
}

extern	unsigned char	*
LBS_ToByte(
	LargeByteString	*lbs)
{
	unsigned char	*ret;

 	if		(  lbs  !=  NULL  ) {
		ret = (unsigned char *)xmalloc(LBS_Size(lbs));
		RewindLBS(lbs);
		memcpy(ret,LBS_Body(lbs),LBS_Size(lbs));
	} else {
		ret = NULL;
	}
	return	(ret);
}

extern	char	*
LBS_ToString(
	LargeByteString	*lbs)
{
	char	*ret;

 	if		(  lbs  !=  NULL  ) {
		ret = (char *)xmalloc(LBS_StringLength(lbs) + 1);
		RewindLBS(lbs);
		strncpy(ret,LBS_Body(lbs),LBS_Size(lbs));
		ret[LBS_Size(lbs)] = 0;
	} else {
		ret = NULL;
	}
	return	(ret);
}

extern	wchar_t	*
LBS_ToWcs(
	LargeByteString	*lbs)
{
	wchar_t	*ret;

 	if		(  lbs  !=  NULL  ) {
		ret = (wchar_t *)xmalloc(LBS_Size(lbs) + sizeof(wchar_t));
		wcscpy(ret,LBS_Body(lbs));
	} else {
		ret = NULL;
	}
	return	(ret);
}

extern	LargeByteString	*
LBS_Duplicate(
	LargeByteString	*lbs)
{
	LargeByteString	*ret;

 	if		(  lbs  !=  NULL  ) {
		ret = NewLBS();
		LBS_ReserveSize(ret,lbs->size,FALSE);
		memcpy(ret->body,lbs->body,lbs->size);
	} else {
		ret = NULL;
	}
	return	(ret);
}


