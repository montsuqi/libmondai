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

#ifndef	_INC_VALUE_H
#define	_INC_VALUE_H

#define	SIZE_GLOWN		1024	/*	LBS glown unit	*/
//#define	SIZE_BUFF		1024
#define	SIZE_BUFF		65538
//#define	SIZE_SQL		16384
#define	SIZE_SQL		65538

#define	SIZE_NUMBUF	60

#include	<stdio.h>
#include	<glib.h>
#include	"types.h"
#include	"LBSfunc.h"

#ifndef	PacketClass
#define	PacketClass		unsigned char
#endif

typedef	unsigned char	PacketDataType;
typedef	unsigned char	ValueAttributeType;

typedef	struct {
	size_t		flen;
	size_t		slen;
	char		*sval;
}	Fixed;

typedef	struct {
	int		oid;
	char	*name;
	FILE	*fp;
}	Element;

typedef	struct _ValueStruct	{
	PacketDataType		type;
	Bool				fUpdate;
	ValueAttributeType	attr;
	union {
		struct {
			size_t					count;
			struct	_ValueStruct	**item;
		}	ArrayData;
		struct {
			size_t					count;
			struct	_ValueStruct	**item;
			char					**names;
			GHashTable				*members;
		}	RecordData;
		struct {
			size_t			len;
			unsigned char	*sval;
		}	CharData;
		struct {
			int		apsid;
			int		oid;
		}	Object;
		Fixed	FixedData;
		int		IntegerData;
		Bool	BoolData;
		double	FloatData;
	}	body;
}	ValueStruct;

typedef	struct {
	char	***item;
}	KeyStruct;

#define	DBOP_SELECT		0
#define	DBOP_FETCH		1
#define	DBOP_UPDATE		2
#define	DBOP_INSERT		3
#define	DBOP_DELETE		4

typedef	struct {
	char			*name;
	GHashTable		*opHash;
	int				ocount;
	LargeByteString	**ops;
}	PathStruct;


typedef	struct {
	KeyStruct	*pkey;
	PathStruct	**path;
	GHashTable	*paths;
	int			pcount;
	void		*dbg;
}	DB_Struct;

#define	RECORD_NULL		0
#define	RECORD_DB		1

typedef	struct _RecordStruct	{
	char		*name;
	ValueStruct	*rec;
	int		type;
	union {
		DB_Struct	*db;
	}	opt;
}	RecordStruct;

#define	GL_TYPE_NULL			(PacketDataType)0x00
#define	GL_TYPE_INT				(PacketDataType)0x10
#define	GL_TYPE_BOOL			(PacketDataType)0x11
#define	GL_TYPE_FLOAT			(PacketDataType)0x20
#define	GL_TYPE_CHAR			(PacketDataType)0x30
#define	GL_TYPE_TEXT			(PacketDataType)0x31
#define	GL_TYPE_VARCHAR			(PacketDataType)0x32
#define	GL_TYPE_BYTE			(PacketDataType)0x40
#define	GL_TYPE_NUMBER			(PacketDataType)0x50

#define	GL_TYPE_DBCODE			(PacketDataType)0x60
#define	GL_TYPE_OBJECT			(PacketDataType)0x61

#define	GL_TYPE_STRUCTURE		(PacketDataType)0x80
#define	GL_TYPE_ARRAY			(PacketDataType)0x90
#define	GL_TYPE_RECORD			(PacketDataType)0xA0

#define	GL_ATTR_NULL			(ValueAttributeType)0x00
#define	GL_ATTR_VIRTUAL			(ValueAttributeType)0x80
#define	GL_ATTR_INPUT			(ValueAttributeType)0x08
#define	GL_ATTR_OUTPUT			(ValueAttributeType)0x04

#define	GL_OBJ_NULL				0
#define	GL_OBJ_INACTIVE			-1

#define	IS_VALUE_RECORD(v)		((v)->type == GL_TYPE_RECORD)
#define	IS_VALUE_STRUCTURE(v)	(((v)->type & GL_TYPE_STRUCTURE) == GL_TYPE_STRUCTURE)

#define	ValueString(v)			((v)->body.CharData.sval)
#define	ValueStringLength(v)	((v)->body.CharData.len)

#define	ValueByte(v)			((v)->body.CharData.sval)
#define	ValueByteLength(v)		((v)->body.CharData.len)
#define	ValueInteger(v)			((v)->body.IntegerData)
#define	ValueBool(v)			((v)->body.BoolData)
#define	ValueFloat(v)			((v)->body.FloatData)
#define	ValueFixed(v)			(&(v)->body.FixedData)
#define	ValueFixedLength(v)		((v)->body.FixedData.flen)
#define	ValueFixedBody(v)		((v)->body.FixedData.sval)

#define	ValueArraySize(v)		((v)->body.ArrayData.count)
#define	ValueArrayItems(v)		((v)->body.ArrayData.item)
#define	ValueArrayItem(v,i)		((v)->body.ArrayData.item[(i)])

#define	ValueRecordSize(v)		((v)->body.RecordData.count)
#define	ValueRecordItems(v)		((v)->body.RecordData.item)
#define	ValueRecordItem(v,i)	((v)->body.RecordData.item[(i)])
#define	ValueRecordName(v,i)	((v)->body.RecordData.names[(i)])

#define	ValueObject(v)			(&(v)->body.Object)
#define	ValueObjectPlace(v)		((v)->body.Object.apsid)
#define	ValueObjectID(v)		((v)->body.Object.oid)

extern	ValueStruct	*NewValue(PacketDataType type);
extern	void		ValueAddRecordItem(ValueStruct *upper, char *name,
									   ValueStruct *value);
extern	ValueStruct	**MakeValueArray(ValueStruct *template, size_t count);
extern	ValueStruct	*DuplicateValue(ValueStruct *template);

extern	ValueStruct	*GetRecordItem(ValueStruct *value, char *name);
extern	ValueStruct	*GetArrayItem(ValueStruct *value, int i);
extern	ValueStruct	*GetItemLongName(ValueStruct *root, char *longname);
extern	Bool		SetValueString(ValueStruct *val, char *str);
extern	Bool		SetValueInteger(ValueStruct *val, int ival);
extern	Bool		SetValueBool(ValueStruct *val, Bool bval);
extern	Bool		SetValueFloat(ValueStruct *val, double bval);
extern	Bool		SetValueFixed(ValueStruct *val, Fixed *fval);

extern	int			FixedToInt(Fixed *xval);
extern	void		FloatToFixed(Fixed *xval, double fval);
extern	void		IntToFixed(Fixed *xval, int ival);
extern	double		FixedToFloat(Fixed *xval);
extern	Fixed		*NewFixed(int flen, int slen);
extern	void		FreeFixed(Fixed *xval);

extern	void		InitializeValue(ValueStruct *value);
extern	void		CopyValue(ValueStruct *vd, ValueStruct *vs);

extern	int			ToInteger(ValueStruct *val);
extern	double		ToFloat(ValueStruct *val);
extern	Fixed		*ToFixed(ValueStruct *val);
extern	Bool		ToBool(ValueStruct *val);
extern	char		*ToString(ValueStruct *value);
extern	void		MoveValue(ValueStruct *to, ValueStruct *from);

extern	void		FreeValueStruct(ValueStruct *val);
extern	void		DumpValueStruct(ValueStruct *val);

#endif
