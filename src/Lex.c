/*
libmondai -- MONTSUQI data access library
Copyright (C) 2000-2003 Ogochan & JMA (Japan Medical Association).

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
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
#include	<unistd.h>
#include	<glib.h>
#include	<sys/mman.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	"types.h"
#include	"misc_v.h"
#include	"memory_v.h"
#include	"monstring.h"
#include	"others.h"
#include	"hash_v.h"
#include	"value.h"
#define		_LEX
#include	"Lex.h"
#include	"RecParser.h"
#include	"debug.h"

static	char	*
GetFile(
	char	*name,
	size_t	*size)
{
	struct	stat	sb;
	int		fd;
	char	*p
		,	*ret;

ENTER_FUNC;
	dbgprintf("name = [%s]\n",name);
	ret = NULL;
	if		(  ( fd = open(name,O_RDONLY ) )  >=  0  ) {
		fstat(fd,&sb);
		if		(  ( p = mmap(NULL,sb.st_size,PROT_READ,MAP_PRIVATE,fd,0) )
				   !=  NULL  ) {
			ret = (char *)xmalloc(sb.st_size+1);
			memcpy(ret,p,sb.st_size);
			munmap(p,sb.st_size);
			ret[sb.st_size] = 0;
			*size = sb.st_size;
		}
		close(fd);
	}
LEAVE_FUNC;
	return	(ret);
}

extern	CURFILE	*
PushLexInfo(
	CURFILE		*in,
	char		*name,
	char		*path,
	GHashTable	*res)
{
	CURFILE	*info;
	char	*str;
	size_t	size;

ENTER_FUNC;
	if		(  ( str = GetFile(name,&size) )  !=  NULL  ) {
		info = New(CURFILE);
		info->fn = StrDup(name);
		info->cLine = 1;
		info->body = str;
		info->size = size;
		info->pos = 0;
		info->ftop = NULL;
		info->Reserved = res;
		info->fError = FALSE;
		info->ValueName = NULL;
		info->path = path;
		info->Symbol = NULL;
		info->ValueName = NULL;
		info->next = in;
	} else {
		if		(  fLexVerbose  ) {
			fprintf(stderr,"file not found [%s]\n",name);
		}
		info = NULL;
	}
LEAVE_FUNC;
	return	(info);
}

extern	CURFILE	*
PushLexInfoMem(
	CURFILE		*in,
	char		*mem,
	char		*path,
	GHashTable	*res)
{
	CURFILE	*info;

ENTER_FUNC;
	info = New(CURFILE);
	info->fn = NULL;
	info->cLine = 1;
	info->body = mem;
	info->size = strlen(mem)+1;
	info->pos = 0;
	info->ftop = NULL;
	info->Reserved = res;
	info->fError = FALSE;
	info->path = path;
	info->Symbol = NULL;
	info->ValueName = NULL;
	info->next = in;
LEAVE_FUNC;
	return	(info);
}

extern	void
DropLexInfo(
	CURFILE		**in)
{
	CURFILE	*info;

ENTER_FUNC;
	info = (*in);
	if		(  info->fn  !=  NULL  ) {
		xfree(info->fn);
		xfree(info->body);
	}
	if		(  info->Symbol  !=  NULL  ) {
		xfree(info->Symbol);
	}
	if		(  info->ValueName  !=  NULL  ) {
		xfree(info->ValueName);
	}
	(*in) = info->next;
	xfree(info);
LEAVE_FUNC;
}

static	void
ExitInclude(
	CURFILE	*in)
{
	INCFILE	*back;
	size_t	size;

ENTER_FUNC;
	if		(  in->body  !=  NULL  ) {
		xfree(in->body);
	}
	back = in->ftop;
	in->body = GetFile(back->fn,&size);
	if		(  in->fn  !=  NULL  ) {
		xfree(in->fn);
	}
	in->fn = back->fn;
	in->pos = back->pos;
	in->size = size;
	in->cLine = back->cLine;
	in->ftop = back->next;
	xfree(back);
LEAVE_FUNC;

}

static	void
DoInclude(
	CURFILE	*in,
	char	*fn)
{
	INCFILE	*back;
	char	name[SIZE_LONGNAME+1];
	char	buff[SIZE_LONGNAME+1];
	char	*p
	,		*q;
	size_t	size;

ENTER_FUNC;
	back = New(INCFILE);
	back->next = in->ftop;
	back->fn =  in->fn;
	back->pos = in->pos;
	back->cLine = in->cLine;
	in->ftop = back;
	if		(  in->body  !=  NULL  ) {
		xfree(in->body);
	}
	if		(  in->path  !=  NULL  ) {
		strcpy(buff,in->path);
	}
	p = buff;
	do {
		if		(  ( q = strchr(p,':') )  !=  NULL  ) {
			*q = 0;
		}
		sprintf(name,"%s/%s",p,fn);
		if		(  ( in->body = GetFile(name,&size) )  !=  NULL  )	break;
		p = q + 1;
	}	while	(  q  !=  NULL  );
	in->size = size;
	in->cLine = 1;
	in->pos = 0;
	in->fn = StrDup(name);
	if		(  in->body  ==  NULL  ) {
		ExitInclude(in);
	}
LEAVE_FUNC;
}

extern	void
LexInit(void)
{
	fLexVerbose = FALSE;
}

extern	GHashTable	*
MakeReservedTable(
	TokenTable	*table)
{
	int		i;
	GHashTable	*res;
	
ENTER_FUNC;
	res = NewNameHash();
	for	( i = 0 ; table[i].token  !=  0 ; i ++ ) {
		g_hash_table_insert(res,StrDup(table[i].str),(gpointer)table[i].token);
	}
LEAVE_FUNC;
	return	(res);
}

extern	void
SetReserved(
	CURFILE		*in,
	GHashTable	*res)
{
	in->Reserved = res;
}

static	int
CheckReserved(
	CURFILE	*in,
	char	*str)
{
	gpointer	p;
	int		ret;

	if		(  ( p = g_hash_table_lookup(in->Reserved,str) ) !=  NULL  ) {
		ret = (int)p;
	} else {
		ret = T_SYMBOL;
	}
	return	(ret);
}

#define	UnGetChar(in)		(in)->pos --
#define	GetPos(in)			&(in)->body[(in)->pos]
#define	SKIP_SPACE(in)		\
	while	(  isspace( c = GetChar(in) ) ) {	\
		if		(  c  ==  '\n'  ) {				\
			c = ' ';							\
			(in)->cLine ++;						\
		}										\
	}

static	int
GetChar(
	CURFILE	*in)
{
	int		c;

	if		(  in->body  ==  NULL  ) {
		fprintf(stderr,"nulpo!\n");
	}
	if		(  in->pos  ==  in->size  ) {
		c = EOF;
	} else
	if		(  ( c = in->body[in->pos ++] )  ==  0  ) {
		c = EOF;
	}
	return	(c);
}

static	void
ReadyDirective(
	CURFILE	*in)
{
	char	*p
		,	*q;
	char	fn[SIZE_LONGNAME+1];
	int		c;

ENTER_FUNC;
	SKIP_SPACE(in);
	p = GetPos(in)-1;
	while	(  !isspace(c = GetChar(in))  );
	q = GetPos(in)-1;
	if		(  !strlicmp(p,"include")  ) {
		SKIP_SPACE(in);
		p = GetPos(in);
		switch	(c) {
		  case	'"':
			while	(  GetChar(in)  !=  '"'  );
			break;
		  case	'<':
			while	(  GetChar(in)  !=  '>'  );
			break;
		  default:
			break;
		}
		q = GetPos(in)-1;
		memclear(fn,SIZE_LONGNAME+1);
		memcpy(fn,p,q-p);
		if		(  *fn  !=  0  ) {
			DoInclude(in,fn);
		}
	} else {
		UnGetChar(in);
		while	(  ( c = GetChar(in) )  !=  '\n'  );
		in->cLine ++;
	}
LEAVE_FUNC;
}

#ifdef	DEBUG
static	void
DumpCURFILE(
	CURFILE	*in)
{
	printf("token = ");
	switch	(in->Token) {
	  case	T_SYMBOL:
		printf("symbol (%s)\n",in->Symbol);
		break;
	  case	T_ICONST:
		printf("iconst (%d)\n",in->Int);
		break;
	  case	T_EOF:
		printf("[EOF]\n");
		break;
	  default:
		if		(  in->Token  <  128  ) {
			printf("(%c)\n",in->Token);
		} else {
			printf("(%04X)\n",in->Token);
		}
		break;
	}
}
#endif

extern	int
Lex(
	CURFILE	*in,
	Bool	fName)
{
	int		c;
	char	*p
		,	*q;
	Bool	fDot;

ENTER_FUNC;
  retry:
	if		(  in->Symbol  !=  NULL  ) {
		xfree(in->Symbol);
		in->Symbol = NULL;
	}
	SKIP_SPACE(in);
	switch	(c) {
	  case	'#':
		ReadyDirective(in);
		goto	retry;
		break;
	  case	'/':
		if		(  GetChar(in)  !=  '*'  ) {
			UnGetChar(in);
			in->Token = '/';
		} else {
			do {
				while	(  ( c = GetChar(in) )  !=  '*'  );
				if		(  ( c = GetChar(in) )  ==  '/'  )	break;
				UnGetChar(in);
			}	while	(TRUE);
			goto	retry;
		}
		break;
	  case	'"':
		p = GetPos(in);
		while	(  ( c = GetChar(in) )  !=  '"'  ) {
			if		(  c  ==  '\\'  ) {
				GetChar(in);
			}
		}
		q = GetPos(in)-1;
		in->Symbol = (char *)xmalloc(q-p+1);
		memcpy(in->Symbol,p,q-p);
		in->Symbol[q-p] = 0;
		in->Token = T_SCONST;
		break;
	  case	'\'':
		p = GetPos(in);
		while	(  ( c = GetChar(in) )  !=  '\''  ) {
			if		(  c  ==  '\\'  ) {
				GetChar(in);
			}
		}
		q = GetPos(in)-1;
		in->Symbol = (char *)xmalloc(q-p+1);
		memcpy(in->Symbol,p,q-p);
		in->Symbol[q-p] = 0;
		in->Token = T_SCONST;
		break;
	  case	'<':
		if		(  GetChar(in) ==  '='  ) {
			in->Token = T_LE;
		} else {
			in->Token = T_LT;
			UnGetChar(in);
		}
		break;
	  case	'>':
		if		(  GetChar(in) ==  '='  ) {
			in->Token = T_GE;
		} else {
			in->Token = T_GT;
			UnGetChar(in);
		}
		break;
	  case	'=':
		switch(GetChar(in))	{
		  case	'=':
			in->Token = T_EQ;
			break;
		  case	'>':
			in->Token = T_GE;
			break;
		  case	'<':
			in->Token = T_LE;
			break;
			break;
		  default:
			in->Token = T_EQ;
			UnGetChar(in);
			break;
		}
		break;
	  case	'!':
		if		(  GetChar(in) ==  '='  ) {
			in->Token = T_NE;
		} else {
			in->Token = '!';
			UnGetChar(in);
		}
		break;
	  default:
		if		(	(  isalpha(c)  )
				||	(  c  ==  '_'  ) ) {
			p = GetPos(in)-1;
			do {
				c = GetChar(in);
			}	while	(	(  isalpha(c)  )
						||	(  isdigit(c)  )
						||	(  c  ==  '_'  ) );
			UnGetChar(in);
			q = GetPos(in);
			in->Symbol = (char *)xmalloc(q-p+1);
			memcpy(in->Symbol,p,q-p);
			in->Symbol[q-p] = 0;
			if		(  fName  ) {
				in->Token = T_SYMBOL;
			} else {
				in->Token = CheckReserved(in,in->Symbol);
			}
		} else
		if		(  isdigit(c) )	{
			p = GetPos(in)-1;
			fDot = FALSE;
			do {
				c = GetChar(in);
				if		(  c  ==  '.'  )
					fDot = TRUE;
			}	while	(	(  isalpha(c)  )
						||	(  isdigit(c)  )
						||	(  c  ==  '.'  ) );
			UnGetChar(in);
			q = GetPos(in);
			in->Symbol = (char *)xmalloc(q-p+1);
			memcpy(in->Symbol,p,q-p);
			in->Symbol[q-p] = 0;
			if		(  fDot  ) {
				in->Token = T_NCONST;
			} else {
				in->Int = atol(in->Symbol);
				in->Token = T_ICONST;
			}
		} else {
			switch	(c) {
			  case	EOF:
				if		(  in->ftop  ==  NULL  )	{
					in->Token = T_EOF;
				} else {
					ExitInclude(in);
					goto	retry;
				}
				break;
			  default:
				in->Token = c;
				break;
			}
		}
		break;
	}
dbgmsg("*");
#ifdef	DEBUG
	DumpCURFILE(in);
#endif
LEAVE_FUNC;
	return	(in->Token);
}

