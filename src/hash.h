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

#ifndef	_INC_HASH_H
#define	_INC_HASH_H
#include	<glib.h>

extern	GHashTable	*NewNameHash(void);
extern	void		DestroySymbols(GHashTable *sym);
extern	GHashTable	*NewNameiHash(void);
extern	GHashTable	*NewIntHash(void);

#define	g_int_hash_table_insert(h,i,d)		g_hash_table_insert((h),(void *)(i),(d))
#define	g_int_hash_table_foreach(h,p,a)		g_hash_table_foreach((h),(p),(a))
#define	g_int_hash_table_lookup(h,i)		g_hash_table_lookup((h),(void *)(i))
#define	g_int_hash_table_remove(h,i)		g_hash_table_remove((h),(void *)(i))
#endif
