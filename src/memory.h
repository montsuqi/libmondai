/*	PANDA -- a simple transaction monitor

Copyright (C) 1989-2003 Ogochan.

This module is part of PANDA.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the 
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA  02111-1307, USA.
*/

#ifndef	_INC_MEMORY_H
#define	_INC_MEMORY_H

#include	<string.h>
#include	"types.h"

typedef	struct	MEMAREA_S	{
	struct	MEMAREA_S	*next;
#if defined(sparc)
	long dummy;					/* for 8byte alignment */
#endif
}	MEMAREA;

typedef	struct {
	char	*name;
	MEMAREA	*head;
}	POOL;

extern	void	*_xmalloc(size_t size, char *fn, int line);
extern	void	_xfree(void *p, char *fn, int line);

extern	POOL	*GetPool(char *name);
extern	void	*__GetArea(POOL *pool, size_t size, char *fn, int line);
extern	void	__ReleaseArea(POOL *pool, void *p);
extern	void	ReleasePool(char *name);
extern	void	InitPool(void);

#define	GetArea(s)					_GetArea((s),__FILE__,__LINE__)
#define	ReleaseArea(p)				_ReleaseArea(p)
#define	_GetArea(name,size,fn,line)	__GetArea(GetPool(name),(size),(fn),(line))
#define	_ReleaseArea(name,p)		__ReleaseArea(GetPool(name),(p));

#ifndef	New
#define	New(s)				(s *)xmalloc(sizeof(s))
#endif
#ifndef	xmalloc
#define	xmalloc(s)			_xmalloc((s),__FILE__,__LINE__)
#endif
#ifndef	xfree
#define	xfree(p)			_xfree((p),__FILE__,__LINE__)
#endif

#endif
