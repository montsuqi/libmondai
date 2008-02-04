/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2003 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2004-2008 Ogochan.
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

#ifndef	_INC_HASH_H
#define	_INC_HASH_H
#include	<glib.h>

typedef	struct {
	size_t	count;
	void	**item;
}	Chunk;

extern	GHashTable	*NewNameHash(void);
extern	void		DestroySymbols(GHashTable *sym);
extern	GHashTable	*NewNameiHash(void);
extern	GHashTable	*NewIntHash(void);
extern	Chunk		*NewChunk(void);
extern	void		ChunkAppend(Chunk *chunk, void *body);
extern	void		ChunkDestroy(Chunk *chunk);

#define	g_int_hash_table_insert(h,i,d)		g_hash_table_insert((h),(void *)(i),(d))
#define	g_int_hash_table_foreach(h,p,a)		g_hash_table_foreach((h),(p),(a))
#define	g_int_hash_table_lookup(h,i)		g_hash_table_lookup((h),(void *)(i))
#define	g_int_hash_table_remove(h,i)		g_hash_table_remove((h),(void *)(i))

#endif
