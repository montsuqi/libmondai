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

#ifndef	_INC_FIXED_H
#define	_INC_FIXED_H

typedef	struct {
	size_t		flen;
	size_t		slen;
	char		*sval;
}	Fixed;

extern	int			FixedToInt(Fixed *xval);
extern	void		FloatToFixed(Fixed *xval, double fval);
extern	void		IntToFixed(Fixed *xval, int ival);
extern	double		FixedToFloat(Fixed *xval);
extern	Fixed		*NewFixed(int flen, int slen);

extern	void		FreeFixed(Fixed *xval);
extern	void		FixedRescale(Fixed *to, Fixed *fr);

#endif
