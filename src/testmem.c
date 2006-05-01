/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2002 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2003-2006 Ogochan.
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

#include	"value.h"
#include	"memory_v.h"
#include	"misc_v.h"
#include	"others.h"
#include	"debug.h"

#define	TEST_COUNT		100

static	void
final1(
	char	*s1,
	char	*s2)
{
	printf("[%s:%s]\n",s1,s2);
}

extern	int
main(
	int		argc,
	char	**argv)
{
	int		i;
	void	*p[TEST_COUNT];

	InitPool();

	NewPool("0");
	ReleasePoolByName("0");
	NewPool("1st");
	printf("***** get area *****\n");
	for	( i = 0 ; i < TEST_COUNT ; i ++ ) {
		p[i] = GetAreaByName("1st",10);
		sprintf(p[i],"%d",i);
		SetFinalizer(p[i],(AreaFinalizerFunc)final1,"1st");
	}
	printf("***** release area *****\n");
	for	( i = 0 ; i < TEST_COUNT ; i += 5 ) {
		ReleaseAreaByName("1st",p[i]);
	}

	NewPool("2nd");
	printf("***** get area *****\n");
	for	( i = 0 ; i < TEST_COUNT ; i ++ ) {
		p[i] = GetAreaByName("2nd",10);
		sprintf(p[i],"%d",i);
		SetFinalizer(p[i],(AreaFinalizerFunc)final1,"2nd");
	}
	printf("***** release area *****\n");
	for	( i = 0 ; i < TEST_COUNT ; i ++ ) {
		ReleaseAreaByName("2nd",p[i]);
	}

	printf("***** release pool *****\n");
	ReleasePoolByName("1st");
	ReleasePoolByName("2nd");
	return	(0);
}
