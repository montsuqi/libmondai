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
#define	T_YYBASE		(YYBASE+4)

typedef	struct	INCFILE_S	{
	struct	INCFILE_S	*next;
	fpos_t				pos;
	int					cLine;
	char				*fn;
}	INCFILE;
typedef	struct	_CURFILE_S {
	struct	_CURFILE_S	*next;
	GHashTable			*Reserved;
	INCFILE	*ftop;
	FILE	*fp;
	char	*fn;
	char	*path;
	int		cLine;
	Bool	fError;
	char	*Symbol;
	int		Int;
	int		Token;
}	CURFILE;

typedef	struct	{
	char	*str;
	int		token;
}	TokenTable;

typedef	struct _LexInfo	{
	CURFILE		*curr;
}	LexInfo;

#undef	GLOBAL
#ifdef	_LEX
#define	GLOBAL	/*	*/
#else
#define	GLOBAL	extern
#endif
GLOBAL	LexInfo	InfoRoot;
GLOBAL	Bool	fLexVerbose;
#undef	GLOBAL

extern	CURFILE			*PushLexInfo(char *name, char *path, GHashTable *res);
extern	void			DropLexInfo();
extern	void			LexInit(void);
extern	GHashTable		*MakeReservedTable(TokenTable *table);
extern	int				Lex(Bool fSymbol);
extern	void			SetReserved(GHashTable *res);

#define	CURR			(InfoRoot.curr)
#define	GetSymbol		(ComToken = Lex(FALSE))
#define	GetName			(ComToken = Lex(TRUE))
#define	ComToken		(InfoRoot.curr->Token)
#define	ComInt			(InfoRoot.curr->Int)
#define	ComSymbol		(InfoRoot.curr->Symbol)

#endif
