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

#ifndef	_INC_LBSFUNC_H
#define	_INC_LBSFUNC_H

#include	"types.h"
#include	<stdint.h>
#include	<wchar.h>

typedef	struct {
	size_t	ptr
	,		size
	,		asize;
	unsigned char	*body;
}	LargeByteString;

extern	LargeByteString	*NewLBS(void);
extern	void			FreeLBS(LargeByteString *lbs);
extern	void			LBS_ReserveSize(LargeByteString *lbs, size_t size, Bool fKeep);
extern	void			LBS_Seek(LargeByteString *lbs,size_t off, int whence);
extern	void			LBS_EmitStart(LargeByteString *lbs);
extern	void			LBS_Glown(LargeByteString *lbs, size_t size);
extern	void			LBS_Emit(LargeByteString *lbs, unsigned char code);
extern	void			LBS_EmitEnd(LargeByteString *lbs);
extern	void			LBS_EmitString(LargeByteString *lbs, char *str);
extern	void			LBS_String(LargeByteString *lbs, char *str);
extern	void			LBS_EmitStringCodeset(LargeByteString *lbs, char *str,
											  size_t isize, size_t osize, char *codeset);
extern	void			LBS_EmitPointer(LargeByteString *lbs, void *p);
extern	void			LBS_EmitInt(LargeByteString *lbs, int i);
extern	void			LBS_Emit64(LargeByteString *lbs, uint64_t i);
extern	void			LBS_EmitFix(LargeByteString *lbs);
extern	void			LBS_Trim(LargeByteString *lbs, size_t size);
extern	int				LBS_FetchByte(LargeByteString *lbs);
extern	int				LBS_FetchChar(LargeByteString *lbs);
extern	void			*LBS_FetchPointer(LargeByteString *lbs);
extern	int				LBS_FetchInt(LargeByteString *lbs);
extern	uint64_t		LBS_Fetch64(LargeByteString *lbs);
extern	size_t			LBS_StringLength(LargeByteString *lbs);
extern	char			*LBS_ToString(LargeByteString *lbs);
extern	wchar_t			*LBS_ToWcs(LargeByteString *lbs);
extern	unsigned char			*LBS_ToByte(LargeByteString *lbs);
extern	LargeByteString	*LBS_Duplicate(LargeByteString *lbs);

#define	RewindLBS(lbs)			(((LargeByteString *)(lbs))->ptr = 0)
#define	LBS_UnFetchChar(lbs)	LBS_Seek((lbs),-1,SEEK_CUR)
#define	LBS_Peek(lbs)			(((LargeByteString *)(lbs))->body[((LargeByteString *)(lbs))->ptr])
#define	LBS_EmitSpace(lbs)		LBS_EmitChar((lbs),' ')
#define	LBS_EmitByte(lbs,c)		LBS_Emit((lbs),(c))
#define	LBS_EmitChar(lbs,c)		LBS_Emit((lbs),(c))
#define	LBS_Clear(lbs)			LBS_EmitStart(lbs)
#define	LBS_Size(lbs)			(((LargeByteString *)(lbs))->size)
#define	LBS_StringLength(lbs)	LBS_Size(lbs)
#define	LBS_Body(lbs)			((void *)((LargeByteString *)(lbs))->body)

#define	LBS_Ptr(lbs)			(&((LargeByteString *)(lbs))->body[((LargeByteString *)(lbs))->ptr])
#define	LBS_GetPos(lbs)			(((LargeByteString *)(lbs))->ptr)
#define	LBS_SetPos(lbs,pos)		(((LargeByteString *)(lbs))->ptr = (pos))
#define	LBS_Eof(lbs)			(((LargeByteString *)(lbs))->ptr >= ((LargeByteString *)(lbs))->size)
#endif
