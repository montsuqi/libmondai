#include	<stdio.h>
#include	"numeric.h"

#define	dump(v)		_dump((#v),(v))

static	void
_dump(
	char	*name,
	Numeric	v)
{
	printf("%s->varlen   = %d\n",name,v->varlen);
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

