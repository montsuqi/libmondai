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
/*
#define	TRACE
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#ifdef	TRACE
#include	<glib.h>
#include	"hash_v.h"
#endif
#include	"monstring.h"
#include	"libmondai.h"
#include	"memory_v.h"
#include	"lock.h"
#include	"debug.h"


typedef	struct	{
	LOCKOBJECT;
	GHashTable	*hash;
}	POOLMGR;

static	POOLMGR	*PoolTable = NULL;

#ifdef	TRACE
static	size_t	total = 0;
static	GHashTable	*PoolHash;
#endif

extern	void	*
_xmalloc(
	size_t	size,
	char	*fn,
	int		line)
{
	void	*ret;
#ifdef	TRACE
	unsigned char	*area;
	size_t	*sizea;

	total += size; 
	printf("xmalloc %s(%d),size = %d,",fn,line,size);fflush(stdout);
	if		(  ( area = (unsigned char *)malloc(size+sizeof(size_t)) )  ==  NULL  )	{
		fprintf(stderr,"no memory space!! %s(%d)\n",fn,line);
		exit(1);
	}
	sizea = (size_t *)(area + size);
	*sizea = size;
	ret = area;
	printf("allocate = %p, total = %d\n",ret,(int)total);fflush(stdout);
	if		(  PoolHash  ==  NULL  ) {
		PoolHash = NewIntHash();
	}
	g_int_hash_table_insert(PoolHash,ret,sizea);
#else
	if		(  size <= 0  ) {
		MonErrorPrintf("malloc size 0 %s(%d)",fn,line);
	}
	if		(  ( ret = malloc(size) )  ==  NULL  )	{
		MonErrorPrintf("no memory space!! %s(%d)",fn,line);
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

	if		(  p  ==  NULL  )	return;
	if		(  ( area = g_int_hash_table_lookup(PoolHash,p) )  ==  NULL  ) {
		fprintf(stderr, "%p free duplicate in %s(%d)\n",p,fn,line);
	}
	g_int_hash_table_remove(PoolHash,p);
	total -= *area;
	printf("xfree %p %d byte in %s(%d)\n",p,(int)area[0],fn,line);fflush(stdout);
	free(p);
#else
	if		(  p  ==  NULL  )	return;
	free(p);
#endif
}

extern	POOL	*
GetPool(
	char	*name)
{
	POOL	*ret;

	LockWrite(PoolTable);
	ret = g_hash_table_lookup(PoolTable->hash,name);
	UnLock(PoolTable);
	return	(ret);
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

	if		(  p  !=  NULL  ) {
		mp = &((MEMAREA *)p)[-1];
		mp->final = func;
		mp->data = data;
	}
}

extern	void
_ReleaseArea(
	POOL	*pool,
	void	*p)
{
	MEMAREA	*np
	,		*rp;

	if (!p) return;
ENTER_FUNC;
	if		(  pool  !=  NULL  ) {
		if		(  ( rp = pool->head )  !=  NULL  ) {
			while	(rp->next != NULL) {
				np = rp->next;
				if ((&np[1]) == p) {
					rp->next = np->next;
					if		(  np->final  !=  NULL  ) {
						(*np->final)(&np[1],np->data);
					}
					xfree(np);
					return;
				}
				rp = np;
			}
		}
	}
LEAVE_FUNC;
}

extern	void
_ReleasePool(
	POOL	*pool)
{
	MEMAREA	*np
	,		*rp;

ENTER_FUNC;
	if		(  pool  !=  NULL  ) {
		if		(  pool->name  !=  NULL  ) {
			LockWrite(PoolTable);
			g_hash_table_remove(PoolTable->hash,pool->name);
			xfree(pool->name);
			UnLock(PoolTable);
		}
		if		(  ( rp = pool->head )  !=  NULL  ) {
			while	(  rp  !=  NULL  )	{
				np = rp->next;
				if		(  rp->final  !=  NULL  ) {
					(*rp->final)((void *)&rp[1],rp->data);
				}
				xfree(rp);
				rp = np;
			}
		}
		xfree(pool);
	}
LEAVE_FUNC;
}

extern	POOL	*
NewPool(
	char	*name)
{
	POOL	*pool;

ENTER_FUNC;
	if		(  name  ==  NULL  ) {
		pool = (POOL *)xmalloc(sizeof(POOL));
		pool->name = NULL;
		pool->head = NULL;
	} else {
		LockWrite(PoolTable);
		if		(  g_hash_table_lookup(PoolTable->hash,name)  ==  NULL  ) {
			pool = (POOL *)xmalloc(sizeof(POOL));
			pool->name = StrDup(name);
			pool->head = NULL;
			g_hash_table_insert(PoolTable->hash,pool->name,pool);
		} else {
			pool = NULL;
		}
		UnLock(PoolTable);
	}
LEAVE_FUNC;
	return	(pool); 
}

extern	void
InitPool(void)
{
	if		(  PoolTable  ==  NULL  ) {
		PoolTable = New(POOLMGR);
		PoolTable->hash = NewNameHash();
		InitLock(PoolTable);
	}
}

static	void
_Release(
	char	*name,
	POOL	*pool,
	void	*dummy)
{
	_ReleasePool(pool);
}

extern	void
ReleaseAllPool(void)
{
	if		(  PoolTable  !=  NULL  ) {
		LockWrite(PoolTable);
		g_hash_table_foreach(PoolTable->hash,(GHFunc)_Release,NULL);
		g_hash_table_destroy(PoolTable->hash);
		UnLock(PoolTable);
		DestroyLock(PoolTable);
	}
	PoolTable = NULL;
}
