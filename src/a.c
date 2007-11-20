#include	<stdio.h>

int	main(void)
{
	char	foo[sizeof(long)];
	char	foo2[sizeof(long long)];
	int		i;
	long	mask
		,	val;
	long long	val2
		,		mask2;

	printf("sizeof(int)        = %d\n",sizeof(int));
	printf("sizeof(long)       = %d\n",sizeof(long));
	printf("sizeof(long long)  = %d\n",sizeof(long long));
	printf("sizeof(void*)      = %d\n",sizeof(void*));
#ifdef	__LP64__
	printf("LP64!!\n");
#endif

	for	( i = 0 ; i < sizeof(long) ; i ++ ) {
		foo[i] = i;
	}
	for	( i = 0 ; i < sizeof(long long) ; i ++ ) {
		foo2[i] = i;
	}
	val = *(long *)foo;
	val2 = *(long long *)foo2;

	printf("long          = ");
	mask = 0xFF;
	for	( i = 0 ; i < sizeof(long) ; i ++ ) {
		printf("%d ",(int)((val & mask) >> ( i * 8 )));
		mask <<= 8;
	}
	printf("\n");
	printf("long long     = ");
	mask2 = 0xFF;
	for	( i = 0 ; i < sizeof(long long) ; i ++ ) {
		printf("%d ",(int)((val2 & mask2) >> ( i * 8 )));
		mask2 <<= 8;
	}
	printf("\n");
}
