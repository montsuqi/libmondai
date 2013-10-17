/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2002-2003 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2004-2008 Ogochan
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

#ifndef	_INC_DD_LEX_H
#define	_INC_DD_LEX_H
#include	<glib.h>

#define	YYBASE			256
#define	T_EOF			(YYBASE +1)
#define	T_SYMBOL		(YYBASE +2)
#define	T_SCONST		(YYBASE +3)
#define	T_ICONST		(YYBASE +4)
#define	T_NCONST		(YYBASE +5)
#define	T_RCONST		(YYBASE +6)

#define	T_LT			'<'
#define	T_LE			(YYBASE +10)
#define	T_EQ			'='
#define	T_GE			(YYBASE +12)
#define	T_GT			'>'
#define	T_NE			(YYBASE +14)

#define	T_YYBASE		(YYBASE +20)

typedef	struct	INCFILE_S	{
	struct	INCFILE_S	*next;
	size_t				pos;
	size_t				size;
	int					cLine;
	FILE				*fp;
	char				*fn;
	char				*body;
}	INCFILE;

typedef	struct	_CURFILE_S {
	struct	_CURFILE_S	*next;
	GHashTable			*Reserved;
	INCFILE	*ftop;
	char	*body;
	int		back;
	size_t	pos;
	size_t	size;
	FILE	*fp;
	char	*fn;
	char	*path;
	int		cLine;
	Bool	fError;
	char	*Symbol;
	int		Int;
	int		Token;
	char	*ValueName;
}	CURFILE;

typedef	struct	{
	char	*str;
	int		token;
}	TokenTable;

#undef	GLOBAL
#ifdef	_LEX
#define	GLOBAL	/*	*/
#else
#define	GLOBAL	extern
#endif
GLOBAL	Bool	fLexVerbose;
#undef	GLOBAL

extern	CURFILE		*PushLexInfo(CURFILE *in,const char *name,const char *path, GHashTable *res);
extern	CURFILE		*PushLexInfoMem(CURFILE *in,const char *mem,const char *path, GHashTable *res);
extern	CURFILE		*PushLexInfoStream(CURFILE *in, FILE *fp,const char *path, GHashTable *res);

extern	void			DropLexInfo(CURFILE **in);
extern	void			LexInit(void);
extern	GHashTable		*MakeReservedTable(TokenTable *table);
extern	void			SetReserved(CURFILE *in, GHashTable *res);

#define	LEX_GET_SYMBOL	1
#define	LEX_GET_NAME	2
#define	LEX_GET_STRING	3

extern	int				Lex(CURFILE *in, int type);
#define	GetSymbol		(ComToken = Lex(in,LEX_GET_SYMBOL))
#define	GetName			(ComToken = Lex(in,LEX_GET_NAME))
#define	GetString		(ComToken = Lex(in,LEX_GET_STRING))
#define	ComToken		(in->Token)
#define	ComInt			(in->Int)
#define	ComSymbol		(in->Symbol)

#define	ParError(msg)				{			\
	in->fError=TRUE;							\
	fprintf(stderr,"%s:%d:%s\n",in->fn,in->cLine,msg);	\
	GetSymbol;									\
}
#define	ParErrorPrintf(...)	{					\
	in->fError=TRUE;							\
	fprintf(stderr,"%s:%d:",in->fn,in->cLine);			\
	printf(__VA_ARGS__);						\
	GetSymbol;									\
}

#define	ERROR_BREAK		if (in->fError) break

#endif
