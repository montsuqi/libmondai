/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 1989-2006 Ogochan.
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

#ifndef	_INC_MEMORY_H
#define	_INC_MEMORY_H

#include	<string.h>
#include	"types.h"

typedef	void	(*AreaFinalizerFunc)(void *area, void *data);

typedef	struct	MEMAREA_S	{
	struct	MEMAREA_S	*next;
	AreaFinalizerFunc	final;
	void	*data;
}	MEMAREA;

typedef	struct {
	char	*name;
	MEMAREA	*head;
}	POOL;

extern	void	*_xmalloc(size_t size, char *fn, int line);
extern	void	_xfree(void *p, char *fn, int line);

extern	POOL	*GetPool(char *name);
extern	void	*_GetArea(POOL *pool, size_t size, char *fn, int line);
extern	void	_ReleaseArea(POOL *pool, void *p);
extern	void	_ReleasePool(POOL *pool);
extern	POOL	*NewPool(char *name);
extern	void	InitPool(void);
extern	void	SetFinalizer(void *p, AreaFinalizerFunc func, void *data);

#define	GetAreaByPool(p,s)			_GetArea((p),(s),__FILE__,__LINE__)
#define	GetAreaByName(n,s)			_GetArea(GetPool(n),(s),__FILE__,__LINE__)
#define	ReleaseAreaByPool(p,a)		_ReleaseArea((p),(a))
#define	ReleaseAreaByName(n,a)		_ReleaseArea(GetPool(n),(a))
#define	ReleasePoolByPool(p)		_ReleasePool(p)
#define	ReleasePoolByName(n)		_ReleasePool(GetPool(n))
#define	New(s)						(s *)xmalloc(sizeof(s))

#ifndef	xmalloc
#define	xmalloc(s)					_xmalloc((s),__FILE__,__LINE__)
#endif
#ifndef	xfree
#define	xfree(p)					_xfree((p),__FILE__,__LINE__),(p) = NULL
#endif

#endif
