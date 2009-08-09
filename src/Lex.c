/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2009 Ogochan & JMA (Japan Medical Association).
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

static	CURFILE	*
NewCURFILE(
	CURFILE		*in,
	char		*path,
	GHashTable	*res)
{
	CURFILE	*info;

	info = New(CURFILE);
	info->fn = NULL;
	info->cLine = 1;
	info->body = NULL;
	info->back = -1;
	info->size = -1;
	info->pos = 0;
	info->fp = NULL;
	info->ftop = NULL;
	info->Reserved = res;
	info->fError = FALSE;
	info->ValueName = NULL;
	info->path = path;
	info->Symbol = NULL;
	info->ValueName = NULL;
	info->next = in;

	return	(info);
}

extern	CURFILE	*
PushLexInfo(
	CURFILE		*in,
	char		*name,
	char		*path,
	GHashTable	*res)
{
	CURFILE	*info;
	struct	stat	sb;
	FILE	*fp;

ENTER_FUNC;
	dbgprintf("fname = [%s]",name);
	if		(  ( fp = fopen(name,"r") )  !=  NULL  ) {
		fstat(fileno(fp),&sb);
		info = NewCURFILE(in,path,res);
		info->fn = StrDup(name);
		info->size = sb.st_size;
		info->fp = fp;
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
	info = NewCURFILE(in,path,res);
	info->body = mem;
	info->size = strlen(mem)+1;
LEAVE_FUNC;
	return	(info);
}

extern	CURFILE	*
PushLexInfoStream(
	CURFILE		*in,
	FILE		*fp,
	char		*path,
	GHashTable	*res)
{
	CURFILE	*info;

ENTER_FUNC;
	info = NewCURFILE(in,path,res);
	info->fp = fp;
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
	if		(	(  info->fp  !=  NULL   )
			&&	(  info->fp  !=  stdin  ) ) {
		fclose(info->fp);
	}
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

ENTER_FUNC;
	if		(  in->fp  !=  NULL  ) {
		fclose(in->fp);
	}
	if		(  in->body  !=  NULL  ) {
		xfree(in->body);
	}
	back = in->ftop;
	in->fp = back->fp;
	in->body = back->body;
	if		(  in->fn  !=  NULL  ) {
		xfree(in->fn);
	}
	in->fn = back->fn;
	in->pos = back->pos;
	in->size = back->size;
	in->cLine = back->cLine;
	in->ftop = back->next;
	xfree(back);
LEAVE_FUNC;

}

static	void
DoInclude(
	CURFILE	*in,
	char	*fn,
	Bool	fFull)
{
	INCFILE	*back;
	char	name[SIZE_LONGNAME+1];
	char	buff[SIZE_LONGNAME+1];
	char	*p
	,		*q;
	struct	stat	sb;

ENTER_FUNC;
	back = New(INCFILE);
	back->next = in->ftop;
	back->fn =  in->fn;
	back->pos = in->pos;
	back->cLine = in->cLine;
	back->body = in->body;
	back->size = in->size;
	in->ftop = back;
	back->fp = in->fp;
	if		(  in->path  !=  NULL  ) {
		if		(  fFull  ) {
			strcpy(buff,in->path);
		} else {
			strcpy(buff,ExpandPath(in->path,NULL));
		}
	} else {
		strcpy(buff,".");
	}
	p = buff;
	do {
		if		(  ( q = strchr(p,':') )  !=  NULL  ) {
			*q = 0;
		}
		if		(  fFull  ) {
			sprintf(name,"%s/%s",p,fn);
		} else {
			sprintf(name,"%s/%s",p,ExpandPath(fn,NULL));
		}
		dbgprintf("fname = [%s]",name);
		if		(  ( in->fp = fopen(name,"r") )  !=  NULL  )	break;
		p = q + 1;
	}	while	(  q  !=  NULL  );
	if		(  in->fp  !=  NULL  ) {
		fstat(fileno(in->fp),&sb);
		in->size = sb.st_size;
		in->cLine = 1;
		in->pos = 0;
		in->body = NULL;
		in->fn = StrDup(name);
	} else {
		fprintf(stderr,"include file %s not found.\n",fn);
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
	res = NewNameiHash();
	for	( i = 0 ; table[i].token  !=  0 ; i ++ ) {
		g_hash_table_insert(res,StrDup(table[i].str),(gpointer)(long)table[i].token);
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
		ret = (int)(long)p;
	} else {
		ret = T_SYMBOL;
	}
	return	(ret);
}

#define	SKIP_SPACE(in)		\
	while	(	(  ( c = GetChar(in) )  !=  0  )	\
			&&	(	(  isspace(c)              )	\
				||	(  iscntrl(c)              ) ) )	{	\
		if		(  c  ==  '\n'  ) {				\
			c = ' ';							\
			(in)->cLine ++;						\
		}										\
	}

static	void
UnGetChar(
	CURFILE	*in,
	int		c)
{
	in->pos --;
	in->back = c;
}

static	int
GetChar(
	CURFILE	*in)
{
	int		c;

	if		(  in->back  >=  0  ) {
		c = in->back;
		in->back = -1;
	} else {
		if		(  in->pos  ==  in->size  ) {
			c = 0;
		} else
		if		(  in->fp  !=  NULL  ) {
			if		(  ( c = fgetc(in->fp) )  <  0  ) {
				c = 0;
			}
		} else {
			if		(  in->body  ==  NULL  ) {
				fprintf(stderr,"nulpo!\n");
			}
			if		(  ( c = in->body[in->pos] )  ==  0  ) {
				c = 0;
			}
		}
	}
	in->pos ++;
	return	(c);
}

static	void
ReadyDirective(
	CURFILE	*in)
{
	char	*p;
	char	buff[SIZE_LONGNAME+1];
	int		c;
	Bool	fFull;

ENTER_FUNC;
	while	(	(  ( c = GetChar(in) )  !=  0  )
			&&	(  c  !=  '\n'  )
			&&	(  isspace(c)   ) );
	if		(  c  ==  '\n'  )	return;
	p = buff;
	*p ++ = c;
	while	(	( ( c = GetChar(in) )  !=  0  )
			&&	(  !isspace(c)                ) ) {
		*p ++ = c;
	}
	*p = 0;
	fFull = FALSE;
	if		(  !strlicmp(buff,"include")  ) {
		SKIP_SPACE(in);
		p = buff;
		switch	(c) {
		  case	'"':
			while	(  ( c = GetChar(in) )  !=  '"'  ) {
				*p ++ = c;
			}
			fFull = TRUE;
			break;
		  case	'<':
			while	(  ( c = GetChar(in) )  !=  '>'  ) {
				*p ++ = c;
			}
			fFull = FALSE;
			break;
		  default:
			break;
		}
		*p = 0;
		if		(  *buff  !=  0  ) {
			DoInclude(in,buff,fFull);
		}
	} else {
		UnGetChar(in,c);
		while	(	(  ( c = GetChar(in) )  !=  0  )
				&&	(  c  !=  '\n'  ) );
		UnGetChar(in,c);
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
	int		type)
{
	int		c;
	char	*p;
	char	buff[SIZE_BUFF];
	Bool	fDot;
	LargeByteString	*lbs;

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
		if		(  type  ==  LEX_GET_STRING  ) {
			lbs = NewLBS();
			while	(  ( c = GetChar(in) )  !=  '/'  ) {
				if		(  c  ==  '\\'  ) {
					c = GetChar(in);
				}
				LBS_EmitChar(lbs,c);
			}
			LBS_EmitEnd(lbs);
			in->Symbol = (char *)xmalloc(LBS_Size(lbs));
			strcpy(in->Symbol,LBS_Body(lbs));
			FreeLBS(lbs);
			in->Token = T_RCONST;
		} else {
			if		(  ( c = GetChar(in) )  !=  '*'  ) {
				UnGetChar(in,c);
				in->Token = '/';
			} else {
				do {
					while	(  ( c = GetChar(in) )  !=  '*'  );
					if		(  ( c = GetChar(in) )  ==  '/'  )	break;
					UnGetChar(in,c);
				}	while	(TRUE);
				goto	retry;
			}
		}
		break;
	  case	'"':
		lbs = NewLBS();
		while	(  ( c = GetChar(in) )  !=  '"'  ) {
			if		(  c  ==  '\\'  ) {
				c = GetChar(in);
			}
			LBS_EmitChar(lbs,c);
		}
		LBS_EmitEnd(lbs);
		in->Symbol = (char *)xmalloc(LBS_Size(lbs));
		strcpy(in->Symbol,LBS_Body(lbs));
		FreeLBS(lbs);
		in->Token = T_SCONST;
		break;
	  case	'\'':
		lbs = NewLBS();
		while	(  ( c = GetChar(in) )  !=  '\''  ) {
			if		(  c  ==  '\\'  ) {
				c = GetChar(in);
			}
			LBS_EmitChar(lbs,c);
		}
		LBS_EmitEnd(lbs);
		in->Symbol = (char *)xmalloc(LBS_Size(lbs));
		strcpy(in->Symbol,LBS_Body(lbs));
		FreeLBS(lbs);
		in->Token = T_SCONST;
		break;
	  case	'<':
		if		(  ( c = GetChar(in) )  ==  '='  ) {
			in->Token = T_LE;
		} else {
			in->Token = T_LT;
			UnGetChar(in,c);
		}
		break;
	  case	'>':
		if		(  ( c = GetChar(in) )  ==  '='  ) {
			in->Token = T_GE;
		} else {
			in->Token = T_GT;
			UnGetChar(in,c);
		}
		break;
	  case	'=':
		switch(c = GetChar(in))	{
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
			UnGetChar(in,c);
			break;
		}
		break;
	  case	'!':
		if		(  ( c = GetChar(in) )  ==  '='  ) {
			in->Token = T_NE;
		} else {
			in->Token = '!';
			UnGetChar(in,c);
		}
		break;
	  default:
		p = buff;
		if		(	(  isalpha(c)  )
				||	(  c  ==  '_'  ) ) {
			do {
				*p ++ = c;
				c = GetChar(in);
			}	while	(	(  isalpha(c)  )
						||	(  isdigit(c)  )
						||	(  c  ==  '_'  ) );
			UnGetChar(in,c);
			*p = 0;
			in->Symbol = (char *)xmalloc(strlen(buff)+1);
			strcpy(in->Symbol,buff);
			if		(  type  ==  LEX_GET_SYMBOL  ) {
				in->Token = CheckReserved(in,in->Symbol);
			} else {
				in->Token = T_SYMBOL;
			}
		} else
		if		(  isdigit(c) )	{
			fDot = FALSE;
			do {
				*p ++ = c;
				c = GetChar(in);
				if		(  c  ==  '.'  )
					fDot = TRUE;
			}	while	(	(  isalpha(c)  )
						||	(  isdigit(c)  )
						||	(  c  ==  '.'  ) );
			UnGetChar(in,c);
			*p = 0;
			in->Symbol = (char *)xmalloc(strlen(buff)+1);
			strcpy(in->Symbol,buff);
			if		(  fDot  ) {
				in->Token = T_NCONST;
			} else {
				in->Int = atol(in->Symbol);
				in->Token = T_ICONST;
			}
		} else {
			switch	(c) {
			  case	0:
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

