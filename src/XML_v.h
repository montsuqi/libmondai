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
#ifndef	_INC_XML_VALUE_H
#define	_INC_XML_VALUE_H
#include	"valueconv.h"

#define	NS_URI		"http://panda.montsuqui.org/libmondai"

typedef	struct {
	Bool	fIndent;
	Bool	fType;
	int		type;
}	XMLOPT;

#define	XML_TYPE1		1
#define	XML_TYPE2		2

#define	ConvIndent(opt)		\
		((opt)->appendix != NULL) && (((XMLOPT *)(opt)->appendix)->fIndent)
#define	ConvType(opt)		\
		((opt)->appendix != NULL) && (((XMLOPT *)(opt)->appendix)->fType)
#define	ConvXmlType(opt)	\
		(((opt)->appendix == NULL) ? XML_TYPE1 : (((XMLOPT *)(opt)->appendix)->type))

extern	void	ConvSetIndent(CONVOPT *opt, Bool v);
extern	void	ConvSetType(CONVOPT *opt, Bool v);
extern	void	ConvSetXmlType(CONVOPT *opt, int type);

extern	byte	*XML_PackValue(CONVOPT *opt, byte *p, ValueStruct *value);
extern	byte	*XML_UnPackValue(CONVOPT *opt, byte *p, ValueStruct *value);
extern	size_t	XML_SizeValue(CONVOPT *opt, ValueStruct *value);
#endif
