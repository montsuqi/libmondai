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
#include	"numeric.h"

#define	dump(v)		_dump((#v),(v))

static	void
_dump(
	char	*name,
	Numeric	v)
{
	printf("%s->varlen   = %d\n",name,(int)v->varlen);
	printf("%s->n_weight = %d\n",name,v->n_weight);
	printf("%s->n_rscale = %d\n",name,v->n_rscale);
	printf("%s->n_sign_dscale = %d\n",name,v->n_sign_dscale);
}

extern	int
main(
	int		argc,
	char	**argv)
{
	Numeric	v1, v2, v22, v3, v4;
	char	buff[128];
	if (argc != 5){
		printf("Error: testfixed num num num format\n");
		exit(1);
	}
	v1 = NumericInput(argv[1],10,5);
	v2 = NumericInput(argv[2],10,4);
	dump(v1);
	v22 = NumericShift(v2,-2);
	dump(v22);
	v3 = NumericADD(v1,v22);
	printf("[%s]\n",NumericOutput(v1));
	printf("[%s]\n",NumericOutput(v22));
	printf("[%s]\n",NumericOutput(v3));
	dump(v3);

	v3 = NumericSUB(v1,v22);
	printf("[%s]\n",NumericOutput(v3));
	dump(v3);
	v3 = NumericMUL(v1,v22);
	printf("[%s]\n",NumericOutput(v3));
	dump(v3);
	v3 = NumericDIV(v1,v22);
	printf("[%s]\n",NumericOutput(v3));
	dump(v3);

	v3 = NumericUMinus(v1);
	printf("*[%s]\n",NumericOutput(v3));
	dump(v3);

	v4 = NumericADD(v3,v22);
	printf("[%s]\n",NumericOutput(v4));
	dump(v4);

	NumericFormat(buff,argv[4],v2);
	printf("[%s]\n",buff);
	NumericFree(v2);

	v4 = NumericInputChar(*argv[3],10,5);
	printf("[%s]\n",NumericOutput(v4));
	v4->n_weight ++;
	printf("[%s]\n",NumericOutput(v4));

	{
		int		precision
		,		scale;
		NumericFormatToPrecision(argv[4],&precision,&scale);
		printf("[%s] %d.%d\n",argv[4],precision,scale);
	}

	return	0;
}

