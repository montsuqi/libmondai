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

#ifndef	_INC_DD_LEX_H
#define	_INC_DD_LEX_H
#include	<glib.h>

#define	YYBASE			256
#define	T_EOF			(YYBASE +1)
#define	T_SYMBOL		(YYBASE +2)
#define	T_SCONST		(YYBASE +3)
#define	T_ICONST		(YYBASE +4)
#define	T_LT			'<'
#define	T_LE			(YYBASE +6)
#define	T_EQ			'='
#define	T_GE			(YYBASE +8)
#define	T_GT			'>'
#define	T_NE			(YYBASE +10)

#define	T_YYBASE		(YYBASE +20)

typedef	struct	INCFILE_S	{
	struct	INCFILE_S	*next;
	size_t				pos;
	int					cLine;
	char				*fn;
}	INCFILE;
typedef	struct	_CURFILE_S {
	struct	_CURFILE_S	*next;
	GHashTable			*Reserved;
	INCFILE	*ftop;
	char	*body;
	size_t	pos;
	size_t	size;
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

extern	CURFILE			*PushLexInfo(CURFILE *in, char *name, char *path, GHashTable *res);
extern	CURFILE			*PushLexInfoMem(CURFILE *in, char *mem, char *path, GHashTable *res);
extern	void			DropLexInfo(CURFILE **in);
extern	void			LexInit(void);
extern	GHashTable		*MakeReservedTable(TokenTable *table);
extern	int				Lex(CURFILE *in, Bool fSymbol);
extern	void			SetReserved(CURFILE *in, GHashTable *res);

#define	GetSymbol		(ComToken = Lex(in,FALSE))
#define	GetName			(ComToken = Lex(in,TRUE))
#define	ComToken		(in->Token)
#define	ComInt			(in->Int)
#define	ComSymbol		(in->Symbol)
#define	MoveSymbol(to)	((to) = in->Symbol, in->Symbol = NULL)

#endif
