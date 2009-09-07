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
	,		*istr;
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
					fprintf(stderr, "[%s:%d] iconv failure %s\n", __FILE__, __LINE__, strerror(errno));
					fprintf(stderr, "[");
					for (i = 0;i < strlen(str); i++) {
						fprintf(stderr, "%02X,", str[i]);
					}
					fprintf(stderr, "][%s]\n",str);
					break;
				}
			}
			*oc = 0;
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

