/*	PANDA -- a simple transaction monitor

Copyright (C) 2000-2002 Ogochan & JMA (Japan Medical Association).

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
