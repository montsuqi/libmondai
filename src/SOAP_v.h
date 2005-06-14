/*	PANDA -- a simple transaction monitor

Copyright (C) 2005 Ogochan.

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
#ifndef	_INC_SOAP_VALUE_H
#define	_INC_SOAP_VALUE_H

#include	"XML_v.h"

extern	size_t	SOAP_PackValue(byte *p, ValueStruct *value, char *method,
							   char *prefix, char *uri, Bool fIndent, Bool fBodyOnly);
extern	void	SOAP_UnPackValue(ValueStruct *val, char *data, char *method);

#endif
