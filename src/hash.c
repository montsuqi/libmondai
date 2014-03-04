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
#include	<glib.h>
#include	<math.h>

#include	"types.h"
#include	"monstring.h"
#include	"memory_v.h"
#include	"hash_v.h"
#include	"debug.h"

static gboolean need_init_lock = TRUE;
static pthread_mutex_t lock;

static	guint
NameHash(
	gconstpointer	key)
{
	char	*p;
	guint	ret;

	ret = 0;
	if		(  key  !=  NULL  ) {
		for	( p = (char *)key ; *p != 0 ; p ++ ) {
			ret = ( ret << 4 ) | *p;
		}
	}
	return	(ret);
}

static	gint
NameCompare(
	gconstpointer	s1,
	gconstpointer	s2)
{
	if (s1 == NULL) {
		return -1;
	}
	if (s2 == NULL) {
		return 1;
	}
	return	(!strcmp(s1,s2));
}

extern	GHashTable	*
NewNameHash(void)
{
	GHashTable	*ret;

	if (need_init_lock) {
		pthread_mutex_init(&lock,NULL);
		need_init_lock = FALSE;
	}

	pthread_mutex_lock(&lock);
	ret = g_hash_table_new((GHashFunc)NameHash,(GCompareFunc)NameCompare);
	pthread_mutex_unlock(&lock);

	return	(ret);
}

static	guint
NameiHash(
	gconstpointer	key)
{
	char	*p;
	guint	ret;

	ret = 0;
	if		(  key  !=  NULL  ) {
		for	( p = (char *)key ; *p != 0 ; p ++ ) {
			ret = ( ret << 4 ) | toupper(*p);
		}
	}
	return	(ret);
}

static	gint
NameiCompare(
	gconstpointer	s1,
	gconstpointer	s2)
{
	return	(!stricmp((char *)s1,(char *)s2));
}

extern	GHashTable	*
NewNameiHash(void)
{
	GHashTable	*ret;

	if (need_init_lock) {
		pthread_mutex_init(&lock,NULL);
		need_init_lock = FALSE;
	}

	pthread_mutex_lock(&lock);
	ret = g_hash_table_new((GHashFunc)NameiHash,(GCompareFunc)NameiCompare);
	pthread_mutex_unlock(&lock);

	return	(ret);
}

static	void
_ClearNames(
	gpointer	key,
	gpointer	value,
	gpointer	user_data)
{
	xfree(key);
}

extern	void
DestroySymbols(
	GHashTable		*sym)
{
	if		(  sym  !=  NULL  ) {
		g_hash_table_foreach(sym,(GHFunc)_ClearNames,NULL);
		DestroyHashTable(sym);
	}
}

static	guint
IntHash(
	gconstpointer	key)
{
	return	((guint)(long)key);
}

static	gint
IntCompare(
	gconstpointer	s1,
	gconstpointer	s2)
{
	return	( (int)(long)s1 == (int)(long)s2 );
}

extern	GHashTable	*
NewIntHash(void)
{
	GHashTable	*ret;

	if (need_init_lock) {
		pthread_mutex_init(&lock,NULL);
		need_init_lock = FALSE;
	}

	pthread_mutex_lock(&lock);
	ret = g_hash_table_new((GHashFunc)IntHash,(GCompareFunc)IntCompare);
	pthread_mutex_unlock(&lock);

	return	(ret);
}

extern	void
DestroyHashTable(
	GHashTable *hash)
{
	if (hash == NULL) {
		return;
	}
MonWarning("==================== destoryhashtable");

	pthread_mutex_lock(&lock);
	g_hash_table_destroy(hash);
	pthread_mutex_unlock(&lock);
}

extern	Chunk	*
NewChunk(void)
{
	Chunk	*ret;

	ret = New(Chunk);
	ret->count = 0;
	ret->item = NULL;
	return	(ret);
}

extern	void
ChunkAppend(
	Chunk	*chunk,
	void	*body)
{
	void	**temp;

	temp = (void **)xmalloc(sizeof(void*) * (chunk->count + 1));
	if		(  chunk->item  !=  NULL  ) {
		memcpy(temp,chunk->item,(sizeof(void*) * chunk->count));
		xfree(chunk->item);
	}
	chunk->item = temp;
	chunk->item[chunk->count] = body;
	chunk->count ++;
}

extern	void
ChunkDestroy(
	Chunk	*chunk)
{
	xfree(chunk->item);
	xfree(chunk);
}
