/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2007-2009 Ogochan.
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

#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include    <sys/types.h>
#include	<json.h>

#include	"types.h"
#include	"misc_v.h"
#include	"monstring.h"
#include	"memory_v.h"
#include	"others.h"
#include	"value.h"
#include	"getset.h"
#include	"json_v.h"
#include	"debug.h"

static json_object *JSONOBJ = NULL;
static size_t JSONSIZE = 0;

static const char*
str_json_object_type(
	json_type type)
{
	switch(type) {
	case json_type_null:
		return "null";
	case json_type_boolean:
		return "boolean";
	case json_type_double:
		return "double";
	case json_type_int:
		return "int";
	case json_type_object:
		return "object";
	case json_type_array:
		return "array";
	case json_type_string:
		return "string";
	default:
		return "UNDEF";
	}
}

static Bool
check_json_object_type(
	ValueStruct *value,
	json_object *obj,
	json_type type)
{
	if (json_object_is_type(obj,json_type_string)) {
		return TRUE;
	}
	MonWarningPrintf("Invalid json type [%s] for [%s];expected type [%s]",
		str_json_object_type(json_object_get_type(obj)),
		str_json_object_type(type),
		GetValueLongName(value)
		);
	return FALSE;
}

static	void
_JSON_UnPackValue(
	CONVOPT *opt,
	json_object *obj,
	ValueStruct *value)
{
	int i,length;
	json_object *child;

ENTER_FUNC;
	if (value == NULL || obj == NULL) {
		return;
	}
	ValueIsNonNil(value);
	switch	(value->type) {
	case GL_TYPE_CHAR:
	case GL_TYPE_VARCHAR:
	case GL_TYPE_DBCODE:
	case GL_TYPE_TEXT:
	case GL_TYPE_SYMBOL:
	case GL_TYPE_ALIAS:
	case GL_TYPE_OBJECT:
	case GL_TYPE_BYTE:
	case GL_TYPE_BINARY:
		if (check_json_object_type(value,obj,json_type_string)) {
			SetValueString(value,json_object_get_string(obj),NULL);
		}
		break;
	case GL_TYPE_BOOL:
		if (check_json_object_type(value,obj,json_type_boolean)) {
			ValueBool(value) = json_object_get_boolean(obj);
		}
		break;
	case GL_TYPE_INT:
		if (check_json_object_type(value,obj,json_type_int)) {
			ValueInteger(value) = json_object_get_int(obj);
		}
		break;
	case GL_TYPE_NUMBER:
	case GL_TYPE_FLOAT:
		if (check_json_object_type(value,obj,json_type_double)) {
			SetValueFloat(value,json_object_get_double(obj));
		}
		break;
	case GL_TYPE_ARRAY:
		if (check_json_object_type(value,obj,json_type_array)) {
			length = json_object_array_length(obj);
			for	(i = 0 ; i < ValueArraySize(value) && i < length ; i ++ ) {
				_JSON_UnPackValue(opt,
					json_object_array_get_idx(obj,i),
					ValueArrayItem(value,i));
			}
		}
		break;
	case GL_TYPE_RECORD:
		if (check_json_object_type(value,obj,json_type_object)) {
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				child = json_object_object_get(obj,ValueRecordName(value,i));
				if (child != NULL) {
					_JSON_UnPackValue(opt,child,ValueRecordItem(value,i));
				}
			}
		}
		break;
	}
LEAVE_FUNC;
}

extern	size_t
JSON_UnPackValue(
	CONVOPT		*opt,
	unsigned char		*p,
	ValueStruct	*value)
{
	json_object *obj;
ENTER_FUNC;
	obj = json_tokener_parse(p);
	_JSON_UnPackValue(opt,obj,value);
	json_object_put(obj);
LEAVE_FUNC;
	return	0;
}

static	json_object*
_JSON_PackValue(
	CONVOPT	*opt,
	ValueStruct	*value)
{
	int i;
	json_object *obj,*child;

ENTER_FUNC;
	if (value == NULL) {
		return json_object_new_string("");
	} 
	switch	(value->type) {
	case GL_TYPE_CHAR:
	case GL_TYPE_VARCHAR:
	case GL_TYPE_DBCODE:
	case GL_TYPE_TEXT:
	case GL_TYPE_SYMBOL:
	case GL_TYPE_ALIAS:
	case GL_TYPE_OBJECT:
	case GL_TYPE_BYTE:
	case GL_TYPE_BINARY:
		return json_object_new_string(ValueToString(value,NULL));
	case GL_TYPE_BOOL:
		return json_object_new_boolean(ValueBool(value));
	case GL_TYPE_INT:
		return json_object_new_int(ValueInteger(value));
	case GL_TYPE_NUMBER:
	case GL_TYPE_FLOAT:
		return json_object_new_double(ValueToFloat(value));
	case GL_TYPE_ARRAY:
		obj = json_object_new_array();
		for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
			child = _JSON_PackValue(opt,ValueArrayItem(value,i));
			json_object_array_add(obj,child);
		}
		return obj;
	case GL_TYPE_RECORD:
		obj = json_object_new_object();
		for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
			child = _JSON_PackValue(opt,ValueRecordItem(value,i));
			json_object_object_add(obj,ValueRecordName(value,i),child);
		}
		return obj;
	}
LEAVE_FUNC;
	return json_object_new_string("");
}

/* SizeValueでPackしたものを使い、実際にはPackしない  */
extern	size_t
JSON_PackValue(
	CONVOPT		*opt,
	unsigned char		*p,
	ValueStruct	*value)
{
ENTER_FUNC;
	if (JSONOBJ != NULL) {
		memcpy(p,json_object_to_json_string(JSONOBJ),JSONSIZE);
		json_object_put(JSONOBJ);
		JSONOBJ = NULL;
	}
LEAVE_FUNC;
	return	JSONSIZE;
}

/* SizeValueでPackし、その結果を保存してPackValueで使う  */
/* SizeValueした後PackValueしないとリークする  */
extern	size_t
JSON_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value)
{
	JSONOBJ = _JSON_PackValue(opt,value);
	JSONSIZE = strlen(json_object_to_json_string(JSONOBJ)) + 1;
	
	return	JSONSIZE;
}
