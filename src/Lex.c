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

/*
#define	DEBUG
#define	TRACE
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<glib.h>
#include	"types.h"
#include	"misc.h"
#include	"memory.h"
#include	"monstring.h"
#include	"others.h"
#include	"hash.h"
#include	"value.h"
#define		_LEX
#include	"Lex.h"
#include	"DDparser.h"
#include	"debug.h"

extern	CURFILE	*
PushLexInfo(
	char	*name,
	char	*path,
	GHashTable	*res)
{
	CURFILE	*info;
	FILE	*fp;

	if		(  ( fp = fopen(name,"r") )  !=  NULL  ) {
		info = New(CURFILE);
		info->fn = StrDup(name);
		info->cLine = 1;
		info->fp = fp;
		info->ftop = NULL;
		info->Reserved = res;
		info->fError = FALSE;
		if		(  path  ==  NULL  ) {
			path = ".";
		}
		info->path = StrDup(path);
		info->Symbol = (char *)xmalloc(SIZE_SYMBOL+1);
		info->next = InfoRoot.curr;
		InfoRoot.curr = info;
	} else {
		printf("file not found [%s]\n",name);
		info = NULL;
	}
	return	(info);
}

extern	void
DropLexInfo(void)
{
	CURFILE	*info;

	info = InfoRoot.curr;
	xfree(info->fn);
	xfree(info->Symbol);
	xfree(info->path);
	fclose(info->fp);
	InfoRoot.curr = info->next;
	xfree(info);
}

static	void
ExitInclude(void)
{
	INCFILE	*back;
	CURFILE	*info = InfoRoot.curr;

dbgmsg(">ExitInclude");
	if		(  info->fp  !=  NULL  ) {
		fclose(info->fp);
	}
	back = info->ftop;
	info->fp = fopen(back->fn,"r");
	xfree(info->fn);
	info->fn = back->fn;
	fsetpos(info->fp,&back->pos);
	info->cLine = back->cLine;
	info->ftop = back->next;
	xfree(back);
dbgmsg("<ExitInclude");
}

static	void
DoInclude(
	char	*fn)
{
	INCFILE	*back;
	char	name[SIZE_LONGNAME+1];
	char	buff[SIZE_LONGNAME+1];
	char	*p
	,		*q;

dbgmsg(">DoInclude");
	back = New(INCFILE);
	back->next = CURR->ftop;
	back->fn =  CURR->fn;
	fgetpos(CURR->fp,&back->pos);
	back->cLine = CURR->cLine;
	CURR->ftop = back;
	fclose(CURR->fp);
	strcpy(buff,CURR->path);
	p = buff;
	do {
		if		(  ( q = strchr(p,':') )  !=  NULL  ) {
			*q = 0;
		}
		sprintf(name,"%s/%s",p,fn);
		if		(  ( CURR->fp = fopen(name,"r") )  !=  NULL  )	break;
		p = q + 1;
	}	while	(  q  !=  NULL  );
	CURR->cLine = 1;
	CURR->fn = StrDup(name);
	if		(  CURR->fp  ==  NULL  ) {
		fprintf(stderr,"include not found [%s]\n",name);
		ExitInclude();
	}
dbgmsg("<DoInclude");
}


extern	void
LexInit(void)
{
	InfoRoot.curr = NULL;
}

extern	GHashTable	*
MakeReservedTable(
	TokenTable	*table)
{
	int		i;
	GHashTable	*res;
	
	res = NewNameHash();
	for	( i = 0 ; table[i].token  !=  0 ; i ++ ) {
		g_hash_table_insert(res,StrDup(table[i].str),(gpointer)table[i].token);
	}
	return	(res);
}

extern	void
SetReserved(
	GHashTable	*res)
{
	InfoRoot.curr->Reserved = res;
}

static	int
CheckReserved(
	char	*str)
{
	gpointer	p;
	int		ret;

	if		(  ( p = g_hash_table_lookup(InfoRoot.curr->Reserved,str) ) !=  NULL  ) {
		ret = (int)p;
	} else {
		ret = T_SYMBOL;
	}
	return	(ret);
}

#define	GetChar()			fgetc(CURR->fp)
#define	UnGetChar(c)		ungetc((c),CURR->fp)
#define	SKIP_SPACE	while	(  isspace( c = GetChar() ) ) {	\
						if		(  c  ==  '\n'  ) {	\
							c = ' ';\
							CURR->cLine ++;\
						}	\
					}


static	void
ReadyDirective(void)
{
	char	buff[SIZE_LONGNAME+1];
	char	fn[SIZE_LONGNAME+1];
	char	*s;
	int		c;

ENTER_FUNC;
	SKIP_SPACE;
	s = buff;
	do {
		*s = c;
		s ++;
	}	while	(  !isspace( c = GetChar() )  );
	*s = 0;
	if		(  !stricmp(buff,"include")  ) {
		SKIP_SPACE;
		s = fn;
		switch	(c) {
		  case	'"':
			while	(  ( c = GetChar() )  !=  '"'  ) {
				*s ++ = c;
			}
			*s = 0;
			break;
		  case	'<':
			while	(  ( c = GetChar() )  !=  '>'  ) {
				*s ++ = c;
			}
			*s = 0;
			break;
		  default:
			*s = 0;
			break;
		}
		if		(  *fn  !=  0  ) {
			DoInclude(fn);
		}
	} else {
		UnGetChar(c);
		while	(  ( c = GetChar() )  !=  '\n'  );
		CURR->cLine ++;
	}
LEAVE_FUNC;
}

extern	int
Lex(
	Bool	fSymbol)
{	int		c
	,		len;
	int		token;
	char	*s;

ENTER_FUNC;
  retry:
	SKIP_SPACE; 
	if		(  c  ==  '#'  ) {
		ReadyDirective();
		goto	retry;
	}
	if		(  c  ==  '!'  ) {
		while	(  ( c = GetChar() )  !=  '\n'  );
		CURR->cLine ++;
		goto	retry;
	}
	if		(  c  ==  '"'  ) {
		s = ComSymbol;
		len = 0;
		while	(  ( c = GetChar() )  !=  '"'  ) {
			if		(  c  ==  '\\'  ) {
				c = GetChar();
			}
			*s = c;
			if		(  len  <  SIZE_SYMBOL  ) {
				s ++;
				len ++;
			}
		}
		*s = 0;
		token = T_SCONST;
	} else
	if		(  isalpha(c)  ) {
		s = ComSymbol;
		len = 0;
		do {
			*s = c;
			if		(  len  <  SIZE_SYMBOL  ) {
				s ++;
				len ++;
			}
			c = GetChar();
		}	while	(	(  isalpha(c) )
					 ||	(  isdigit(c) )
					 ||	(  c  ==  '_' ) );
		*s = 0;
		UnGetChar(c);
		if		(  fSymbol  ) {
			token = T_SYMBOL;
		} else {
			token = CheckReserved(ComSymbol);
		}
	} else
	if		(  isdigit(c)  )	{
		s = ComSymbol;
		len = 0;
		do {
			*s = c;
			if		(  len  <  SIZE_SYMBOL  ) {
				s ++;
				len ++;
			}
			c = GetChar();
		}	while	(  isdigit(c)  );
		*s = 0;
		UnGetChar(c);
		token = T_ICONST;
		ComInt = atoi(ComSymbol);
	} else {
		switch	(c) {
		  case	EOF:
			if		(  CURR->ftop  ==  NULL  )	{
				token = T_EOF;
			} else {
				ExitInclude();
				goto	retry;
			}
			break;
		  default:
			token = c;
			break;
		}
	}
#ifdef	DEBUG
	printf("token = ");
	switch	(token) {
	  case	T_SYMBOL:
		printf("symbol (%s)\n",ComSymbol);
		break;
	  case	T_ICONST:
		printf("iconst (%d)\n",ComInt);
		break;
	  case	T_EOF:
		printf("[EOF]\n");
		break;
	  default:
		if		(  token  <  128  ) {
			printf("(%c)\n",token);
		} else {
			printf("(%04X)\n",token);
		}
		break;
	}
#endif
LEAVE_FUNC;
	return	(token);
}

