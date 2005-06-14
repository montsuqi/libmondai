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

#include	<stdint.h>

#define	SIZE_OID		4

#define	SIZE_GLOWN		1024	/*	LBS glown unit	*/
#define	SIZE_BUFF		65536

#define	SIZE_SYMBOL			255
#define	SIZE_NUMBUF			60

#ifndef	SIZE_NAME
#define	SIZE_NAME			64
#endif
#ifndef	SIZE_LONGNAME
#define	SIZE_LONGNAME		1024
#endif

#include	<stdio.h>
#include	<glib.h>
#include	"types.h"
#include	"LBSfunc.h"
#include	"fixed_v.h"

#ifndef	PacketClass
#define	PacketClass		unsigned char
#endif

typedef	unsigned char	PacketDataType;
typedef	unsigned char	ValueAttributeType;
typedef	uint64_t		MonObjectType;

#define	IS_OBJECT_NULL(obj)	((obj) ==  0)

typedef	struct _ValueStruct	{
	PacketDataType		type;
	ValueAttributeType	attr;
	LargeByteString		*str;
	union {
		struct {
			size_t					count;
			Bool					fExpandable;
			struct	_ValueStruct	*prototype
			,						**item;
		}	ArrayData;
		struct {
			size_t					count;
			struct	_ValueStruct	**item;
			char					**names;
			GHashTable				*members;
		}	RecordData;
		struct {
			size_t			length
			,				asize;
			unsigned char	*sval;
		}	CharData;
		struct {
			size_t					count;
			struct	_ValueStruct	**item;
		}	ValuesData;
		MonObjectType	Object;
		char	*AliasName;
		Fixed	FixedData;
		int		IntegerData;
		Bool	BoolData;
		double	FloatData;
		struct	_ValueStruct	*Pointer;
	}	body;
}	ValueStruct;

#define	GL_TYPE_CLASS			(PacketDataType)0xF0
#define	GL_TYPE_NULL			(PacketDataType)0x00

#define	GL_TYPE_NUMERIC			(PacketDataType)0x10
#define	GL_TYPE_INT				(PacketDataType)0x11
#define	GL_TYPE_BOOL			(PacketDataType)0x12
#define	GL_TYPE_FLOAT			(PacketDataType)0x13
#define	GL_TYPE_NUMBER			(PacketDataType)0x14

#define	GL_TYPE_STRING			(PacketDataType)0x20
#define	GL_TYPE_CHAR			(PacketDataType)0x21
#define	GL_TYPE_TEXT			(PacketDataType)0x22
#define	GL_TYPE_VARCHAR			(PacketDataType)0x23
#define	GL_TYPE_BYTE			(PacketDataType)0x24
#define	GL_TYPE_DBCODE			(PacketDataType)0x25
#define	GL_TYPE_BINARY			(PacketDataType)0x26
#define	GL_TYPE_POINTER			(PacketDataType)0x27

#define	GL_TYPE_OBJECT			(PacketDataType)0x40

#define	GL_TYPE_STRUCTURE		(PacketDataType)0x80
#define	GL_TYPE_ARRAY			(PacketDataType)0x81
#define	GL_TYPE_RECORD			(PacketDataType)0x82
#define	GL_TYPE_ALIAS			(PacketDataType)0x83
#define	GL_TYPE_VALUES			(PacketDataType)0x84

#define	GL_ATTR_NIL				(ValueAttributeType)0x80

#define	GL_ATTR_UNIQ			(ValueAttributeType)0x40
#define	GL_ATTR_DESC			(ValueAttributeType)0x20

#define	GL_ATTR_ALIAS			(ValueAttributeType)0x10
#define	GL_ATTR_VIRTUAL			(ValueAttributeType)0x08
#define	GL_ATTR_INPUT			(ValueAttributeType)0x04
#define	GL_ATTR_OUTPUT			(ValueAttributeType)0x02
#define	GL_ATTR_UPDATE			(ValueAttributeType)0x01
#define	GL_ATTR_NULL			(ValueAttributeType)0x00

#define	CHAR_NIL				0x01

#define	IS_VALUE_NIL(v)			(((v)->attr & GL_ATTR_NIL) == GL_ATTR_NIL)
#define	IS_VALUE_VIRTUAL(v)		(((v)->attr & GL_ATTR_VIRTUAL) == GL_ATTR_VIRTUAL)
#define	IS_VALUE_ALIAS(v)		(((v)->attr & GL_ATTR_ALIAS) == GL_ATTR_ALIAS)
#define	IS_VALUE_UPDATE(v)		(((v)->attr & GL_ATTR_UPDATE) == GL_ATTR_UPDATE)
#define	IS_VALUE_DESC(v)		(((v)->attr & GL_ATTR_DESC) == GL_ATTR_DESC)
#define	IS_VALUE_UNIQ(v)		(((v)->attr & GL_ATTR_UNIQ) == GL_ATTR_UNIQ)

#define	GL_OBJ_NULL				0

#define	IS_VALUE_RECORD(v)		((v)->type == GL_TYPE_RECORD)
#define	IS_VALUE_NUMERIC(v)		(((v)->type & GL_TYPE_CLASS) == GL_TYPE_NUMERIC)
#define	IS_VALUE_STRING(v)		(((v)->type & GL_TYPE_CLASS) == GL_TYPE_STRING)
#define	IS_VALUE_STRUCTURE(v)	(((v)->type & GL_TYPE_CLASS) == GL_TYPE_STRUCTURE)

#define	ValueType(v)			((v)->type)
#define	ValueSize(v)			LBS_StringLength((v)->str)
#define	ValueStr(v)				((v)->str)
#define	ValueStrBody(v)			(char *)LBS_Body(((v)->str))

#define	ValueAttribute(v)		((v)->attr)
#define	ValueIsNil(v)			((v)->attr |= GL_ATTR_NIL)
#define	ValueIsNonNil(v)		((v)->attr &= ~GL_ATTR_NIL)
#define	ValueIsUpdate(v)		((v)->attr |= GL_ATTR_UPDATE)
#define	ValueIsNotUpdate(v)		((v)->attr &= ~GL_ATTR_UPDATE)
#define	ValueOrderIsDesc(v)		((v)->attr |= GL_ATTR_DESC)
#define	ValueOrderIsAsc(v)		((v)->attr &= ~GL_ATTR_DESC)

#ifdef	__VALUE_DIRECT
#define	ValueString(v)			((v)->body.CharData.sval)
#define	ValueStringSize(v)		((v)->body.CharData.asize)
#endif
#define	ValueStringPointer(v)	((v)->body.CharData.sval)
#define	ValueStringLength(v)	((v)->body.CharData.length)

#define	ValueByte(v)			((v)->body.CharData.sval)
#define	ValueByteLength(v)		((v)->body.CharData.length)
#define	ValueByteSize(v)		((v)->body.CharData.asize)

#define	ValueInteger(v)			((v)->body.IntegerData)
#define	ValueBool(v)			((v)->body.BoolData)
#define	ValueFloat(v)			((v)->body.FloatData)

#define	ValueFixed(v)			((v)->body.FixedData)
#define	ValueFixedLength(v)		((v)->body.FixedData.flen)
#define	ValueFixedSlen(v)		((v)->body.FixedData.slen)
#define	ValueFixedBody(v)		((v)->body.FixedData.sval)

#define	ValueArraySize(v)		((v)->body.ArrayData.count)
#define	ValueArrayExpandable(v)	((v)->body.ArrayData.fExpandable)
#define	ValueArrayPrototype(v)	((v)->body.ArrayData.prototype)
#define	ValueArrayItems(v)		((v)->body.ArrayData.item)
#define	ValueArrayItem(v,i)		((v)->body.ArrayData.item[(i)])

#define	ValueRecordSize(v)		((v)->body.RecordData.count)
#define	ValueRecordItems(v)		((v)->body.RecordData.item)
#define	ValueRecordItem(v,i)	((v)->body.RecordData.item[(i)])
#define	ValueRecordNames(v)		((v)->body.RecordData.names)
#define	ValueRecordName(v,i)	((v)->body.RecordData.names[(i)])
#define	ValueRecordMembers(v)	((v)->body.RecordData.members)

#define	ValueObject(v)			((v)->body.Object)

#define	ValueAliasName(v)		((v)->body.AliasName)

#define	ValuePointer(v)			((v)->body.Pointer)

#define	ValueValuesSize(v)		((v)->body.ValuesData.count)
#define	ValueValuesItems(v)		((v)->body.ValuesData.item)
#define	ValueValuesItem(v,i)	((v)->body.ValuesData.item[(i)])

extern	ValueStruct	*NewValue(PacketDataType type);
extern	void		ValueAddRecordItem(ValueStruct *upper, char *name,
									   ValueStruct *value);
extern	ValueStruct	**MakeValueArray(ValueStruct *template, size_t count);
extern	ValueStruct	*DuplicateValue(ValueStruct *template);

extern	ValueStruct	*GetRecordItem(ValueStruct *value, char *name);
extern	ValueStruct	*GetArrayItem(ValueStruct *value, int i);
extern	ValueStruct	*GetValuesItem(ValueStruct *value, int i);

extern	ValueStruct	*GetItemLongName(ValueStruct *root, char *longname);

extern	void		InitializeValue(ValueStruct *value);
extern	void		CopyValue(ValueStruct *vd, ValueStruct *vs);
extern	int			CompareValue(ValueStruct *vl, ValueStruct *vr);
extern	Bool		EqualValue(ValueStruct *vl, ValueStruct *vr);

extern	void		AssignValue(ValueStruct *to, ValueStruct *from);
extern	void		MoveValue(ValueStruct *to, ValueStruct *from);

extern	void		FreeValueStruct(ValueStruct *val);
extern	void		DumpValueStruct(ValueStruct *val);

#endif
