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

extern	void
LBS_ReserveSize(
	LargeByteString	*lbs,
	size_t			size,
	Bool			fKeep)
{
	byte	*body;

	if		(  lbs  !=  NULL  ) {
		if		(  lbs->asize  <  size  ) {
			body = (byte *)xmalloc(size);
			if		(  fKeep  ) {
				memcpy(body,lbs->body,lbs->size);
			}
			if		(  lbs->body  !=  NULL  ) {
				xfree(lbs->body);
			}
			lbs->body = body;
			lbs->asize = size;
		}
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
	byte			code)
{
	byte	*body;

	if		(  lbs  !=  NULL  ) {
		if		(  lbs->ptr  ==  lbs->asize  ) {
			lbs->asize += SIZE_GLOWN;
			body = (byte *)xmalloc(lbs->asize);
			if		(  lbs->body  !=  NULL  ) {
				memcpy(body,lbs->body,lbs->ptr);
				xfree(lbs->body);
			}
			lbs->body = body;
		}
		lbs->body[lbs->ptr] = code;
		lbs->ptr ++;
		if		(  lbs->ptr  >  lbs->size  ) {
			lbs->size = lbs->ptr;
		}
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

static int
ConvertForIconv(
	char *string)
{
	int i;
	const char *table[][2] = {
		{"\357\274\215","\342\210\222"}, // －,−
		{"\344\273\274","\342\226\240"}, // 仼,■
		{"\344\274\271","\342\226\240"}, // 伹,■
		{"\344\277\215","\342\226\240"}, // 俍,■
		{"\344\277\277","\342\226\240"}, // 俿,■
		{"\345\203\264","\342\226\240"}, // 僴,■
		{"\345\203\230","\342\226\240"}, // 僘,■
		{"\345\205\244","\342\226\240"}, // 兤,■
		{"\345\206\276","\342\226\240"}, // 冾,■
		{"\345\207\254","\342\226\240"}, // 凬,■
		{"\345\212\234","\342\226\240"}, // 劜,■
		{"\345\213\200","\342\226\240"}, // 勀,■
		{"\345\215\262","\342\226\240"}, // 卲,■
		{"\345\217\235","\342\226\240"}, // 叝,■
		{"\357\250\216","\342\226\240"}, // 﨎,■
		{"\345\235\231","\342\226\240"}, // 坙,■
		{"\345\235\245","\342\226\240"}, // 坥,■
		{"\345\242\262","\342\226\240"}, // 墲,■
		{"\345\245\223","\342\226\240"}, // 奓,■
		{"\345\245\243","\342\226\240"}, // 奣,■
		{"\345\246\272","\342\226\240"}, // 妺,■
		{"\345\263\265","\342\226\240"}, // 峵,■
		{"\345\267\220","\342\226\240"}, // 巐,■
		{"\345\274\241","\342\226\240"}, // 弡,■
		{"\346\201\235","\342\226\240"}, // 恝,■
		{"\346\202\205","\342\226\240"}, // 悅,■
		{"\346\203\236","\342\226\240"}, // 惞,■
		{"\346\204\240","\342\226\240"}, // 愠,■
		{"\346\204\221","\342\226\240"}, // 愑,■
		{"\346\210\223","\342\226\240"}, // 戓,■
		{"\346\225\216","\342\226\240"}, // 敎,■
		{"\346\230\273","\342\226\240"}, // 昻,■
		{"\346\230\256","\342\226\240"}, // 昮,■
		{"\357\250\222","\342\226\240"}, // 晴,■
		{"\346\234\216","\342\226\240"}, // 朎,■
		{"\346\253\242","\342\226\240"}, // 櫢,■
		{"\346\261\257","\342\226\240"}, // 汯,■
		{"\346\265\257","\342\226\240"}, // 浯,■
		{"\346\266\226","\342\226\240"}, // 涖,■
		{"\346\267\270","\342\226\240"}, // 淸,■
		{"\346\267\262","\342\226\240"}, // 淲,■
		{"\346\270\271","\342\226\240"}, // 渹,■
		{"\347\214\244","\342\226\240"}, // 猤,■
		{"\347\216\275","\342\226\240"}, // 玽,■
		{"\347\217\222","\342\226\240"}, // 珒,■
		{"\347\217\265","\342\226\240"}, // 珵,■
		{"\347\220\251","\342\226\240"}, // 琩,■
		{"\347\232\202","\342\226\240"}, // 皂,■
		{"\357\250\227","\342\226\240"}, // 益,■
		{"\347\241\272","\342\226\240"}, // 硺,■
		{"\357\250\230","\342\226\240"}, // 礼,■
		{"\357\250\234","\342\226\240"}, // 靖,■
		{"\357\250\235","\342\226\240"}, // 精,■
		{"\347\276\241","\342\226\240"}, // 羡,■
		{"\357\250\236","\342\226\240"}, // 羽,■
		{"\350\217\266","\342\226\240"}, // 菶,■
		{"\350\225\253","\342\226\240"}, // 蕫,■
		{"\350\240\207","\342\226\240"}, // 蠇,■
		{"\350\255\223","\342\226\240"}, // 譓,■
		{"\350\265\266","\342\226\240"}, // 赶,■
		{"\357\250\243","\342\226\240"}, // 﨣,■
		{"\350\273\217","\342\226\240"}, // 軏,■
		{"\357\250\245","\342\226\240"}, // 逸,■
		{"\351\201\247","\342\226\240"}, // 遧,■
		{"\351\207\236","\342\226\240"}, // 釞,■
		{"\351\210\206","\342\226\240"}, // 鈆,■
		{"\351\211\267","\342\226\240"}, // 鉷,■
		{"\357\250\247","\342\226\240"}, // 﨧,■
		{"\351\213\225","\342\226\240"}, // 鋕,■
		{"\357\250\250","\342\226\240"}, // 﨨,■
		{"\351\216\244","\342\226\240"}, // 鎤,■
		{"\351\217\270","\342\226\240"}, // 鏸,■
		{"\351\220\261","\342\226\240"}, // 鐱,■
		{"\351\221\210","\342\226\240"}, // 鑈,■
		{"\351\226\222","\342\226\240"}, // 閒,■
		{"\357\250\251","\342\226\240"}, // 﨩,■
		{"\351\235\203","\342\226\240"}, // 靃,■
		{"\351\235\221","\342\226\240"}, // 靑,■
		{"\357\250\252","\342\226\240"}, // 飯,■
		{"\357\250\253","\342\226\240"}, // 飼,■
		{"\351\244\247","\342\226\240"}, // 餧,■
		{"\357\250\254","\342\226\240"}, // 館,■
		{"\351\253\231","\342\226\240"}, // 髙,■
		{"\351\256\273","\342\226\240"}, // 鮻,■
		{"\357\250\255","\342\226\240"}, // 鶴,■
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
	char	*oc
	,		*istr
	,		*buff;
	size_t	sib
		,	sob;
	iconv_t	cd;
	int		rc
	,		i;
	char	*obuff;
	size_t	obsize
		,	ssize;
#endif

ENTER_FUNC;
 	if		(  lbs  !=  NULL  ) {
#ifdef	WITH_I18N
		if		(  codeset  !=  NULL  ) {
			cd = iconv_open(codeset,"utf8");
			obsize = isize * 3 + 1;
			while	(TRUE) {
				istr = str;
				sib = isize;
				obuff = (char *)xmalloc(obsize);
				oc = obuff;
				sob = obsize;
				if		(  ( rc = iconv(cd,&istr,&sib,&oc,&sob) )  ==  0  )	{
					break;
				}
				if		(  errno  ==  E2BIG  ) {
					xfree(obuff);
					obsize *= 2;
				} else {
					if (!ConvertForIconv(istr)) {
						MonWarningPrintf("iconv failure %s", strerror(errno));
						buff = xmalloc(sib * 3 + 1);
						for (i = 0;i < sib; i++) {
							sprintf(buff + i * 3, "%02X,", istr[i]);
						}
						MonWarningPrintf("str:%s", buff);
						xfree(buff);
						break;
					}
				}
			}
			*oc = 0;
			// A2B1(jisx0213) -> A1DD(jisx0208) for ghostscript
			while((oc = strstr(obuff, "\242\261")) != NULL) {
				*oc     = 0xA1;
				*(oc+1) = 0xDD;
			}
			ssize = obsize - sob + sizeof(wchar_t);
			LBS_ReserveSize(lbs,ssize,FALSE);
			memclear(LBS_Body(lbs),ssize);
			memcpy(LBS_Body(lbs),obuff,ssize);
			xfree(obuff);
			iconv_close(cd);
		} else {
#endif
			while	(  isize  >  0  )	{
				LBS_Emit(lbs,*str);
				str ++;
				isize --;
				osize --;
				if		(  osize  ==  0  )	break;
			}
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
	byte	*body;

 	if		(  lbs  !=  NULL  ) {
		if		(  lbs->size  >  0  ) {
			body = (byte *)xmalloc(lbs->size);
			memcpy(body,lbs->body,lbs->size);
			xfree(lbs->body);
			lbs->body = body;
		} else {
			xfree(lbs->body);
			lbs->body = NULL;
		}
		lbs->asize = lbs->size;
		lbs->ptr = 0;
	}
}

extern	byte	*
LBS_ToByte(
	LargeByteString	*lbs)
{
	byte	*ret;

 	if		(  lbs  !=  NULL  ) {
		ret = (byte *)xmalloc(LBS_Size(lbs));
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

