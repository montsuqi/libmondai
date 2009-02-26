/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2002 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2003-2008 Ogochan.
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

#define	MAIN
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define	TEST_MEM

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<glib.h>
#include	"types.h"

#include	"others.h"
#include	"debug.h"

static	void
TEST(
	char	*orig,
	char	*base)
{
	printf("[%s] = [%s]\n",orig,ExpandPath(orig,base));
}
		   
extern	int
main(
	int		argc,
	char	**argv)
{
	TEST("=","BASE");
	TEST("=:~","BASE");
	TEST("=:$PATH","BASE");
	TEST("=:$PATH/$HOME","BASE");
	TEST("=:`date`$HOME","BASE");
	TEST("=:\\$PATH/$HOME","BASE");
	TEST("=:`echo $PATH`/$HOME","BASE");
	TEST("=:`date +%Y%m%d`/$HOME","BASE");
	TEST("`pwd`/$HOME","");

	return	(0);
}
