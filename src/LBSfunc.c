/*	PANDA -- a simple transaction monitor

Copyright (C) 2002 Ogochan & JMA (Japan Medical Association).

This module is part of PANDA.

	PANDA is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves
any particular purpose or works at all, unless he says so in writing.
Refer to the GNU General Public License for full details. 

	Everyone is granted permission to copy, modify and redistribute
PANDA, but only under the conditions described in the GNU General
Public License.  A copy of this license is supposed to have been given
to you along with PANDA so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies. 
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
#include	<math.h>

#include	"types.h"
#include	"value.h"
#include	"misc.h"
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

extern	void
LBS_Seek(
	LargeByteString	*lbs,
	size_t			off,
	int				whence)
{
	size_t		newpos;

	switch	(whence) {
	  case	SEEK_SET:
		newpos = off;
		break;
	  case	SEEK_CUR:
		newpos = lbs->ptr + off;
		break;
	  case	SEEK_END:
		newpos = lbs->size + off;
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

extern	int
LBS_FetchByte(
	LargeByteString	*lbs)
{
	int		ret;

	if		(  lbs->ptr  <  lbs->size  ) {
		ret = lbs->body[lbs->ptr];
		lbs->ptr ++;
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
	int		ret;
	int		i;

	ret = 0;
	for	( i = 0 ; i < sizeof(void *) ; i ++ ) {
		ret <<= 8;
		ret |= LBS_FetchByte(lbs);
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
	for	( i = 0 ; i < sizeof(void *) ; i ++ ) {
		ret <<= 8;
		ret |= LBS_FetchByte(lbs);
	}
	return	(ret);
}

extern	void
LBS_EmitStart(
	LargeByteString	*lbs)
{
	lbs->ptr = 0;
	lbs->size = 0;
	if		(  lbs->asize  >  0  ) {
		memclear(lbs->body,lbs->asize);
	}
}

extern	void
LBS_Emit(
	LargeByteString	*lbs,
	byte			code)
{
	byte	*body;

	if		(  lbs->ptr  ==  lbs->asize  ) {
		lbs->asize += SIZE_GLOWN;
		body = (byte *)xmalloc(lbs->asize);
		if		(  lbs->body  !=  NULL  ) {
			memcpy(body,lbs->body,lbs->size);
			xfree(lbs->body);
		}
		lbs->body = body;
	}
	lbs->body[lbs->ptr] = code;
	lbs->ptr ++;
	if		(  lbs->ptr  >  lbs->size  ) {
		lbs->size ++;
	}
}

extern	void
LBS_Trim(
	LargeByteString	*lbs,
	size_t			size)
{
	if		(  lbs->ptr  >=  size  ) {
		memclear(&lbs->body[lbs->ptr - size],size);
		lbs->ptr -= size;
		lbs->size -= size;
	}
}

extern	void
LBS_EmitString(
	LargeByteString	*lbs,
	char			*str)
{
	while	(  *str  !=  0  ) {
		LBS_EmitChar(lbs,*str);
		str ++;
	}
}

extern	void
LBS_EmitPointer(
	LargeByteString	*lbs,
	void			*p)
{
	LBS_Emit(lbs,(((int)p & 0xFF000000) >> 24));
	LBS_Emit(lbs,(((int)p & 0x00FF0000) >> 16));
	LBS_Emit(lbs,(((int)p & 0x0000FF00) >>  8));
	LBS_Emit(lbs,(((int)p & 0x000000FF)      ));
}

extern	void
LBS_EmitInt(
	LargeByteString	*lbs,
	int				i)
{
	LBS_Emit(lbs,((i & 0xFF000000) >> 24));
	LBS_Emit(lbs,((i & 0x00FF0000) >> 16));
	LBS_Emit(lbs,((i & 0x0000FF00) >>  8));
	LBS_Emit(lbs,((i & 0x000000FF)      ));
}

extern	void
LBS_EmitFix(
	LargeByteString	*lbs)
{
	byte	*body;

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

extern	void	*
LBS_Body(
	LargeByteString	*lbs)
{
	return	((void *)lbs->body);
}

extern	char	*
LBS_ToString(
	LargeByteString	*lbs)
{
	char	*ret;

	ret = (char *)xmalloc(LBS_StringLength(lbs) + 1);
	RewindLBS(lbs);
	strncpy(ret,LBS_Body(lbs),LBS_Size(lbs));
	ret[LBS_Size(lbs)] = 0;
	return	(ret);
}

extern	LargeByteString	*
LBS_Duplicate(
	LargeByteString	*lbs)
{
	LargeByteString	*ret;

	ret = NewLBS();
	LBS_ReserveSize(ret,lbs->size,FALSE);
	memcpy(ret->body,lbs->body,lbs->size);
	return	(ret);
}

