/*	PANDA -- a simple transaction monitor

Copyright (C) 2003 Ogochan & JMA (Japan Medical Association).

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
#include	<math.h>

#include	"types.h"
#include	"misc.h"
#include	"value.h"
#include	"monstring.h"
#include	"others.h"
#include	"debug.h"

extern	char	**
ParCommandLine(
	char	*line)
{
	int			n;
	char		buff[SIZE_LONGNAME+1];
	char		*p
	,			*q;
	char		**cmd;

dbgmsg(">ParCommandLine");
	n = 0;
	p = line;
	while	(  *p  !=  0  ) {
		n ++;
		while	(	(  *p  !=  0  )
				&&	(  !isspace(*p)  ) ) {
			p ++;
		}
		while	(	(  *p  !=  0  )
				&&	(  isspace(*p)  ) ) {
			p ++;
		}
	}
	cmd = (char **)xmalloc(sizeof(char *) * ( n + 1));
	p = line;
	n = 0;
	while	(  *p  !=  0  ) {
		q = buff;
		while	(	(  *p  !=  0  )
				&&	(  !isspace(*p)  ) ) {
			*q ++ = *p ++;
		}
		*q = 0;
		cmd[n] = StrDup(buff);
		n ++;
		while	(	(  *p  !=  0  )
				&&	(  isspace(*p)  ) ) {
			p ++;
		}
	}
	cmd[n] = NULL;
dbgmsg("<ParCommandLine");

	return	(cmd); 
}

extern	void
DestroyPort(
	Port	*port)
{
	if		(  port->port  !=  NULL  ) {
		xfree(port->port);
	}
	if		(  port->host  !=  NULL  ) {
		xfree(port->host);
	}
	xfree(port);
}

extern	Port	*
ParPort(
	char	*str,
	int		def)
{
	Port	*ret;
	char	*p;
	char	dup[SIZE_LONGNAME+1];

	strncpy(dup,str,SIZE_LONGNAME);
	ret = New(Port);
	if		(  dup[0]  ==  '['  ) {
		if		(  ( p = strchr(dup,']') )  !=  NULL  ) {
			*p = 0;
			ret->host = StrDup(&dup[1]);
			p ++;
			if		(  *p  ==  ':'  ) {
				ret->port = StrDup(p+1);
			} else {
				if		(  def  <  0  ) {
					ret->port = NULL;
				} else {
					ret->port = IntStrDup(def);
				}
			}
		}
	} else {
		if		(  ( p = strchr(dup,':') )  !=  NULL  ) {
			*p = 0;
			ret->host = StrDup(dup);
			ret->port = StrDup(p+1);
		} else {
			ret->host = StrDup(dup);
			if		(  def  <  0  ) {
				ret->port = NULL;
			} else {
				ret->port = IntStrDup(def);
			}
		}
	}
	if		(  *ret->host  ==  0  ) {
		ret->host = "localhost";
	}
	return	(ret);
}

extern	void
ParseURL(
	URL		*url,
	char	*instr)
{
	char	*p
	,		*str;
	char	buff[256];
	Port	*port;

	strcpy(buff,instr);
	str = buff;
	if		(  ( p = strchr(str,':') )  ==  NULL  ) {
		url->protocol = StrDup("http");
	} else {
		*p = 0;
		url->protocol = StrDup(str);
		str = p + 1;
	}
	if		(  *str  ==  '/'  ) {
		str ++;
	}
	if		(  *str  ==  '/'  ) {
		str ++;
	}
	if		(  ( p = strchr(str,'/') )  !=  NULL  ) {
		*p = 0;
	}
	port = ParPort(str,-1);
	url->host = StrDup(port->host);
	url->port = StrDup(port->port);
	DestroyPort(port);
	if		(  p  !=  NULL  ) {
		url->file = StrDup(p+1);
	} else {
		url->file = "";
	}
}

extern	char	*
ExpandPath(
	char	*org,
	char	*base)
{
	static	char	path[SIZE_LONGNAME+1];
	char	*p
	,		*q;

	p = path;
	while	(  *org  !=  0  ) {
		if		(  *org  ==  '~'  ) {
			p += sprintf(p,"%s",getenv("HOME"));
		} else
		if		(  *org  ==  '='  ) {
			if		(  base  ==  NULL  ) {
				if		(  ( q = getenv("BASE_DIR") )  !=  NULL  ) {
					p += sprintf(p,"%s",q);
				} else {
					p += sprintf(p,".");
				}
			} else {
				p += sprintf(p,"%s",base);
			}
		} else {
			*p ++ = *org;
		}
		org ++;
	}
	*p = 0;
	return	(path);
}

extern	void
DecodeStringURL(
	char	*q,
	char	*p)
{
	while	(	(  *p  !=  0    )
			&&	(  isspace(*p)  ) )	p ++;
	while	(  *p  !=  0  ) {
		if		(  *p  ==  '+'  ) {
			*q ++ = ' ';
		} else
		if		(  *p  ==  '%'  ) {
			*q ++ = (char)HexToInt(p+1,2);
			p += 2;
		} else {
			*q ++ = *p;
		}
		p ++;
	}
	*q = 0;			
}

extern	void
EncodeStringURL(
	char	*q,
	char	*p)
{
	while	(  *p  !=  0  ) {
		if		(  *p  ==  0x20  ) {
			*q ++ = '+';
		} else
		if		(  isalnum(*p)  ) {
			*q ++ = *p;
		} else {
			*q ++ = '%';
			q += sprintf(q,"%02X",((int)*p)&0xFF);
		}
		p ++;
	}
	*q = 0;			
}

extern	size_t
EncodeStringLengthURL(
	char	*p)
{
	size_t	ret;

	ret = 0;
	while	(  *p  !=  0  ) {
		if		(  *p  ==  0x20  ) {
		} else
		if		(  isalnum(*p)  ) {
		} else {
			ret += 3;
		}
		ret ++;
		p ++;
	}
	return	(ret);
}

/*
 *	base64
 */

static	char	base64char[64] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static	char	base64val[128]	=	{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

#define BASE64VAL(c)	(isascii(c) ? base64val[(int)(c)] : -1)

extern	void
EncodeBase64(
	char	*out,
	byte	*in,
	size_t	len)
{
	byte	*inp = in;
	char	*outp = out;

	while	(  len  >=  3  )	{
		*outp++ = base64char[(inp[0] >> 2) & 0x3f];
		*outp++ = base64char[((inp[0] & 0x03) << 4) |
							 ((inp[1] >> 4) & 0x0f)];
		*outp++ = base64char[((inp[1] & 0x0f) << 2) |
							 ((inp[2] >> 6) & 0x03)];
		*outp++ = base64char[inp[2] & 0x3f];
		inp += 3;
		len -= 3;
	}

	if		( len  >  0  )	{
		*outp++ = base64char[(inp[0] >> 2) & 0x3f];
		if (  len  ==  1  )	{
			*outp++ = base64char[(inp[0] & 0x03) << 4];
			*outp++ = '=';
		} else {
			*outp++ = base64char[((inp[0] & 0x03) << 4) |
								 ((inp[1] >> 4) & 0x0f)];
			*outp++ = base64char[((inp[1] & 0x0f) << 2)];
		}
		*outp++ = '=';
	}

	*outp = 0;
}

extern	size_t
EncodeLengthBase64(
	char	*str)
{
	return	(( ( strlen(str) + 2 ) / 3 ) * 4);
}

extern	size_t
DecodeBase64(
	byte	*out,
	char	*in,
	size_t	len)
{
	char	*inp = in;
	byte	*outp = out;
	char	buf[4];

	if		(  len  <  0  ) {
		len = G_MAXINT;
	}

	while	(	(  len  >=  4  )
			&&	(  *inp   !=  0  ) )	{
		buf[0] = *inp++;
		len--;
		if (BASE64VAL(buf[0]) == -1) break;

		buf[1] = *inp++;
		len--;
		if (BASE64VAL(buf[1]) == -1) break;

		buf[2] = *inp++;
		len--;
		if (buf[2] != '=' && BASE64VAL(buf[2]) == -1) break;

		buf[3] = *inp++;
		len--;
		if (buf[3] != '=' && BASE64VAL(buf[3]) == -1) break;

		*outp++ = ((BASE64VAL(buf[0]) << 2) & 0xfc) |
			  ((BASE64VAL(buf[1]) >> 4) & 0x03);
		if (buf[2] != '=') {
			*outp++ = ((BASE64VAL(buf[1]) & 0x0f) << 4) |
				  ((BASE64VAL(buf[2]) >> 2) & 0x0f);
			if (buf[3] != '=') {
				*outp++ = ((BASE64VAL(buf[2]) & 0x03) << 6) |
					   (BASE64VAL(buf[3]) & 0x3f);
			}
		}
	}

	return outp - out;
}
