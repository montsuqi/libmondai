/*	PANDA -- a simple transaction monitor

Copyright (C) 2000-2003 Ogochan & JMA (Japan Medical Association).

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
#include	<glib.h>
#include	<math.h>

#include	"types.h"
#include	"misc.h"
#include	"monstring.h"
#include	"hash.h"
#include	"debug.h"

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
	return	(!strcmp((char *)s1,(char *)s2));
}

extern	GHashTable	*
NewNameHash(void)
{
	GHashTable	*ret;

	ret = g_hash_table_new((GHashFunc)NameHash,(GCompareFunc)NameCompare);

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

	ret = g_hash_table_new((GHashFunc)NameiHash,(GCompareFunc)NameiCompare);

	return	(ret);
}

extern	void
DestroySymbols(
	GHashTable		*sym)
{
	void	ClearNames(
		gpointer	key,
		gpointer	value,
		gpointer	user_data)
		{
			xfree(key);
		}

	if		(  sym  !=  NULL  ) {
		g_hash_table_foreach(sym,(GHFunc)ClearNames,NULL);
		g_hash_table_destroy(sym);
	}
}

static	guint
IntHash(
	gconstpointer	key)
{
	return	((guint)key);
}

static	gint
IntCompare(
	gconstpointer	s1,
	gconstpointer	s2)
{
	return	( (int)s1 == (int)s2 );
}

extern	GHashTable	*
NewIntHash(void)
{
	GHashTable	*ret;

	ret = g_hash_table_new((GHashFunc)IntHash,(GCompareFunc)IntCompare);

	return	(ret);
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
