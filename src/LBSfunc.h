/*	PANDA -- a simple transaction monitor

Copyright (C) 2002-2003 Ogochan & JMA (Japan Medical Association).

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

#ifndef	_INC_LBSFUNC_H
#define	_INC_LBSFUNC_H

#include	"types.h"

typedef	struct {
	size_t	ptr
	,		size
	,		asize;
	byte	*body;
}	LargeByteString;

extern	LargeByteString	*NewLBS(void);
extern	void			FreeLBS(LargeByteString *lbs);
extern	void			LBS_ReserveSize(LargeByteString *lbs, size_t size, Bool fKeep);
extern	void			LBS_Seek(LargeByteString *lbs,size_t off, int whence);
extern	void			LBS_EmitStart(LargeByteString *lbs);
extern	void			LBS_Emit(LargeByteString *lbs, byte code);
extern	void			LBS_EmitString(LargeByteString *lbs, char *str);
extern	void			LBS_EmitStringCodeset(LargeByteString *lbs, char *str,
											  size_t isize, size_t osize, char *codeset);
extern	void			LBS_EmitPointer(LargeByteString *lbs, void *p);
extern	void			LBS_EmitInt(LargeByteString *lbs, int i);
extern	void			LBS_EmitFix(LargeByteString *lbs);
extern	void			LBS_Trim(LargeByteString *lbs, size_t size);
extern	int				LBS_FetchByte(LargeByteString *lbs);
extern	int				LBS_FetchChar(LargeByteString *lbs);
extern	void			*LBS_FetchPointer(LargeByteString *lbs);
extern	int				LBS_FetchInt(LargeByteString *lbs);
extern	size_t			LBS_StringLength(LargeByteString *lbs);
extern	char			*LBS_ToString(LargeByteString *lbs);
extern	LargeByteString	*LBS_Duplicate(LargeByteString *lbs);

#define	RewindLBS(lbs)			((lbs)->ptr = 0)
#define	LBS_Peek(lbs)			((lbs)->body[(lbs)->ptr])
#define	LBS_EmitSpace(lbs)		LBS_EmitChar((lbs),' ')
#define	LBS_EmitByte(lbs,c)		LBS_Emit((lbs),(c))
#define	LBS_EmitChar(lbs,c)		LBS_Emit((lbs),(c))
#define	LBS_EmitEnd(lbs)		LBS_Emit((lbs),0);
#define	LBS_Clear(lbs)			LBS_EmitStart(lbs)
#define	LBS_Size(lbs)			((lbs)->size)
#define	LBS_StringLength(lbs)	LBS_Size(lbs)
#define	LBS_Body(lbs)			((void *)((LargeByteString *)lbs)->body)

#define	LBS_Ptr(lbs)			(&(lbs)->body[(lbs)->ptr])
#define	LBS_GetPos(lbs)			((lbs)->ptr)
#define	LBS_SetPos(lbs,pos)		((lbs)->ptr = (pos))
#define	LBS_Eof(lbs)			((lbs)->ptr >= (lbs)->size)
#endif
