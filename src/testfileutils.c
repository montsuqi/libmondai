/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2002 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2003-2007 Ogochan.
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

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"fileutils.h"

extern	int
main(
	int		argc,
	char	**argv)
{
	int i,mode;

	mode = 0;
	if (argc < 3) {
		fprintf(stderr,"%% testfileutils <MakeDir|mkdir_p|rm_r> dir1 dir2 ...\n");
		exit(1);
	}
	if (!strcmp(argv[1],"MakeDir")) {
		mode = 1;
	} else if (!strcmp(argv[1],"mkdir_p")) {
		mode = 2;
	} else if (!strcmp(argv[1],"rm_r")) {
		mode = 3;
	} else {
		fprintf(stderr,"%% testfileutils <MakeDir|mkdir_p|rm_r> dir1 dir2 ...\n");
		exit(1);
	}

	for (i = 2; i < argc; i++) {
		switch(mode){
		case 1:
			fprintf(stderr,"MakeDir[%s] [%d]\n",argv[i],MakeDir(argv[i],0700));
			break;
		case 2:
			fprintf(stderr,"mkdir_p[%s] [%d]\n",argv[i],mkdir_p(argv[i],0700));
			break;
		case 3:
			fprintf(stderr,"rm_r[%s] [%d]\n",argv[i],rm_r(argv[i]));
			break;
		}
	}
	return	0;
}

