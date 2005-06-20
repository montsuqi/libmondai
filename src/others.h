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

#ifndef	_INC_OTHERS_H
#define	_INC_OTHERS_H
#include	<glib.h>
#include	"types.h"

extern	char		**ParCommandLine(char *line);
extern	char		*ExpandPath(char *org,char *base);

extern	size_t		DecodeStringURL(byte *q, char *p);
extern	size_t		EncodeStringURL(char *q, byte *p);
extern	size_t		EncodeStringLengthURL(byte *q);
extern	size_t		EncodeBase64(char *out, int size, byte *in, size_t len);
extern	size_t		DecodeBase64(byte *out, int size, char *in, size_t len);
extern	size_t		EncodeLengthBase64(char *str);

#define	BASE64SIZE(s)	((((s)+2)/2)*3)

#endif
