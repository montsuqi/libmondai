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

/*
#define	TRACE
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include	<malloc.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<strings.h>
#include	"libmondai.h"
#include	"memory.h"
#include	"debug.h"


static	GHashTable	*PoolTable;

#ifdef	TRACE
static	size_t	total = 0;
#endif

extern	void	*
_xmalloc(
	size_t	size,
	char	*fn,
	int		line)
{
	void	*ret;
#ifdef	TRACE
	size_t	*area;

	total += size; 
	printf("xmalloc %s(%d),size = %d,",fn,line,size);fflush(stdout);
	if		(  ( area = (size_t *)malloc(size+sizeof(size_t)) )  ==  NULL  )	{
		printf("no memory space!! %s(%d)\n",fn,line);
		exit(1);
	}
	*area = size;
	ret = (void *)&area[1];
	printf("allocate = %p, total = %d\n",ret,(int)total);fflush(stdout);
#else
	if		(  ( ret = malloc(size) )  ==  NULL  )	{
		printf("no memory space!! %s(%d)\n",fn,line);
		exit(1);
	}
#endif
	return	(ret);
}

extern	void
_xfree(
	void	*p,
	char	*fn,
	int		line)
{
#ifdef	TRACE
	size_t	*area;

	area = (size_t *)p;
	total -= area[-1];
	printf("xfree %d byte in %s(%d)\n",(int)area[-1],fn,line);
	free(&area[-1]);
#else
	free(p);
#endif
}

extern	POOL	*
GetPool(
	char	*name)
{
	return	(g_hash_table_lookup(PoolTable,name));
}

extern	void	*
_GetArea(
	POOL	*pool,
	size_t	size,
	char	*fn,
	int		line)
{
	MEMAREA	*area;
	void	*p;

#ifdef	TRACE
	printf("GetArea %s(%d)\t",fn,line);
#endif
	if		(  pool  !=  NULL  ) {
		if		(  ( area = (MEMAREA *)xmalloc(size + sizeof(MEMAREA)) )  ==  NULL  )	{
#ifdef DEBUG
			printf("no memory space!! %s(%d) %d\n",fn,line,size);
#endif
			abort();
		}
		area->next = pool->head;
		area->final = NULL;
		area->data = NULL;
		pool->head = area;
		p = (void *)&area[1];

		memset(p,0,size);
#ifdef	TRACE
		printf("allocate = %p , size = %d\n",area,size);
#endif
	} else {
		p = NULL;
	}
	return	(p);
}

extern	void
SetFinalizer(
	void	*p,
	AreaFinalizerFunc	func,
	void	*data)
{
	MEMAREA	*mp;

	mp = &((MEMAREA *)p)[-1];
	mp->final = func;
	mp->data = data;
}

extern	void
_ReleaseArea(
	POOL	*pool,
	void	*p)
{
	MEMAREA	*np
	,		*rp;

	if (!p) return;
dbgmsg(">_ReleaseArea");
	if		(  pool  !=  NULL  ) {
		rp = pool->head;
		while	(rp->next != NULL) {
			np = rp->next;
			if ((&np[1]) == p) {
				rp->next = np->next;
				if		(  np->final  !=  NULL  ) {
					(*np->final)(&np[1],np->data);
				}
				free(np);
				return;
			}
			rp = np;
		}
	}
dbgmsg("<_ReleaseArea");
}

extern	void
_ReleasePool(
	POOL	*pool)
{
	MEMAREA	*np
	,		*rp;

dbgmsg(">ReleasePool");
	if		(  pool  !=  NULL  ) {
		g_hash_table_remove(PoolTable,pool->name);
		np = pool->head;
		rp = np->next;
		np->next = NULL;
		while	(  rp  !=  NULL  )	{
			np = rp->next;
			if		(  rp->final  !=  NULL  ) {
				(*rp->final)((void *)&rp[1],rp->data);
			}
			free(rp);
			rp = np;
		}
		xfree(pool->name);
		xfree(pool);
	}
dbgmsg("<ReleasePool");
}

extern	POOL	*
NewPool(
	char	*name)
{
	POOL	*pool;

dbgmsg(">NewPool");
	if		(  g_hash_table_lookup(PoolTable,name)  ==  NULL  ) {
		pool = (POOL *)xmalloc(sizeof(POOL));
		pool->name = StrDup(name);
		pool->head = NULL;
		g_hash_table_insert(PoolTable,pool->name,pool);
	} else {
		pool = NULL;
	}
dbgmsg("<NewPool");
	return	(pool); 
}

extern	void
InitPool(void)
{
	PoolTable = NewNameHash();
}
