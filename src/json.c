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

static size_t _JSON_SizeValueOmmit(CONVOPT *,ValueStruct *,PacketDataType p) ;

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

static void
print_type_error(
	json_type type,
	json_type expected)
{
	MonWarningPrintf("Invalid json type [%s];expected type [%s]",
		str_json_object_type(type),
		str_json_object_type(expected));
}

static	void
_JSON_UnPackValue(
	CONVOPT *opt,
	json_object *obj,
	ValueStruct *value)
{
	const char *str;
	char buf[256];
	int i,length;
	json_object *child;
	json_type type;

ENTER_FUNC;
	if (value == NULL || obj == NULL) {
#if 0
		MonWarningPrintf("invalid value[%p] or obj[%p]",value,obj);
#endif
		return;
	}
	ValueIsNonNil(value);
	type = json_object_get_type(obj);
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
	case GL_TYPE_TIMESTAMP:
	case GL_TYPE_DATE:
	case GL_TYPE_TIME:
		switch (type) {
		case json_type_boolean:
			if (json_object_get_boolean(obj)) {
				SetValueString(value,"True",NULL);
			} else {
				SetValueString(value,"False",NULL);
			}
			break;
		case json_type_double:
			snprintf(buf,sizeof(buf),"%lf",json_object_get_double(obj));
			SetValueString(value,buf,NULL);
			break;
		case json_type_int:
			snprintf(buf,sizeof(buf),"%d",json_object_get_int(obj));
			SetValueString(value,buf,NULL);
			break;
		case json_type_object:
			SetValueString(value,json_object_to_json_string(obj),NULL);
			break;
		case json_type_array:
			SetValueString(value,json_object_to_json_string(obj),NULL);
			break;
		case json_type_string:
			SetValueString(value,json_object_get_string(obj),NULL);
			break;
		case json_type_null:
			print_type_error(type,json_type_string);
			break;
		default:
			MonWarning("does not reach here");
			break;
		}
		break;
	case GL_TYPE_BOOL:
		switch (json_object_get_type(obj)) {
		case json_type_string:
			str = json_object_get_string(obj);
			if (str != NULL && (str[0] == 'T' || str[0] == 't')) {
				ValueBool(value) = TRUE;
			} else {
				ValueBool(value) = FALSE;
			}
			break;
		case json_type_boolean:
			ValueBool(value) = json_object_get_boolean(obj);
			break;
		case json_type_int:
			if (json_object_get_int(obj) != 0) {
				ValueBool(value) = TRUE;
			} else {
				ValueBool(value) = FALSE;
			}
			break;
		case json_type_double:
		case json_type_null:
		case json_type_object:
		case json_type_array:
			print_type_error(type,json_type_boolean);
			break;
		default:
			MonWarning("does not reach here");
			break;
		}
		break;
	case GL_TYPE_INT:
		switch (type) {
		case json_type_boolean:
			if (json_object_get_boolean(obj)) {
				ValueInteger(value) = 1;
			} else {
				ValueInteger(value) = 0;
			}
			break;
		case json_type_double:
			ValueInteger(value) = (int)(json_object_get_double(obj));
			break;
		case json_type_int:
			ValueInteger(value) = json_object_get_int(obj);
			break;
		case json_type_string:
			ValueInteger(value) = atoi(json_object_get_string(obj));
			break;
		case json_type_null:
		case json_type_object:
		case json_type_array:
			print_type_error(type,json_type_int);
			break;
		default:
			MonWarning("does not reach here");
			break;
		}
		break;
	case GL_TYPE_NUMBER:
	case GL_TYPE_FLOAT:
		switch (type) {
		case json_type_double:
			SetValueFloat(value,json_object_get_double(obj));
			break;
		case json_type_int:
			SetValueFloat(value,(double)(json_object_get_int(obj)));
			break;
		case json_type_string:
			SetValueFloat(value,atof(json_object_get_string(obj)));
			break;
		case json_type_boolean:
		case json_type_null:
		case json_type_object:
		case json_type_array:
			print_type_error(type,json_type_double);
			break;
		default:
			MonWarning("does not reach here");
			break;
		}
		break;
	case GL_TYPE_ARRAY:
		if (type == json_type_array) {
			length = json_object_array_length(obj);
			for	(i = 0 ; i < ValueArraySize(value) && i < length ; i ++ ) {
				_JSON_UnPackValue(opt,
					json_object_array_get_idx(obj,i),
					ValueArrayItem(value,i));
			}
		} else {
			print_type_error(type,json_type_array);
		}
		break;
	case GL_TYPE_RECORD:
		if (type == json_type_object) {
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				child = json_object_object_get(obj,ValueRecordName(value,i));
				if (child != NULL) {
					_JSON_UnPackValue(opt,child,ValueRecordItem(value,i));
				}
			}
		} else {
			print_type_error(type,json_type_array);
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
	if (is_error(obj)) {
		MonWarning("invalid json");
	} else {
		_JSON_UnPackValue(opt,obj,value);
	}
	json_object_put(obj);
LEAVE_FUNC;
	return	0;
}

static	void
emit(
	unsigned char **p,
	const char *str,
	size_t size)
{
	memcpy(*p,str,size);
	*p += size;
}

static	size_t
EscapeStr(
	const char *str,
	unsigned char *p)
{
	size_t size;
	int i;

	size = 0;

	*p = '\"';
	p++;
	size++;

	for (i=0;i<strlen(str);i++) {
		switch(*(str+i)) {
		case '\"':
			emit(&p,"\\\"",2);
			size += 2;
			break;
		case '\\':
			emit(&p,"\\\\",2);
			size += 2;
			break;
		case '/':
			emit(&p,"\\/",2);
			size += 2;
			break;
		case '\b':
			emit(&p,"\\b",2);
			size += 2;
			break;
		case '\f':
			emit(&p,"\\f",2);
			size += 2;
			break;
		case '\n':
			emit(&p,"\\n",2);
			size += 2;
			break;
		case '\r':
			emit(&p,"\\r",2);
			size += 2;
			break;
		case '\t':
			emit(&p,"\\t",2);
			size += 2;
			break;
		default:
			*p = *(str+i);
			p++;
			size++;
			break;
		}
	}

	*p = '\"';
	p++;
	size++;

	return size;
}

static	size_t
EscapeStrLength(
	const char *str)
{
	size_t size;
	int i;

	size = 1;/* " */

	for (i=0;i<strlen(str);i++) {
		switch(*(str+i)) {
		case '\"':
			size += 2;
			break;
		case '\\':
			size += 2;
			break;
		case '/':
			size += 2;
			break;
		case '\b':
			size += 2;
			break;
		case '\f':
			size += 2;
			break;
		case '\n':
			size += 2;
			break;
		case '\r':
			size += 2;
			break;
		case '\t':
			size += 2;
			break;
		default:
			size++;
			break;
		}
	}
	size++;/* " */

	return size;
}

/* ommit pack */
static	size_t
_JSON_PackValueOmmit(
	CONVOPT *opt,
	unsigned char *p,
	ValueStruct *value,
	PacketDataType parent_type)
{
	size_t size;
	int i,j;
	unsigned char *pp;
	char buf[256],*str,*key;
ENTER_FUNC;
	if (value == NULL) {
		return 0;
	}

	pp = p;

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
	case GL_TYPE_TIMESTAMP:
	case GL_TYPE_DATE:
	case GL_TYPE_TIME:
		str = ValueToString(value,NULL);
		if (strlen(str)) {
			size = EscapeStr(str,p);
			p += size;
		} else {
			if (parent_type != GL_TYPE_RECORD) {
				emit(&p,"\"\"",2);
			} else {
				/* ommit "" */
			}
		}
		break;
	case GL_TYPE_BOOL:
		if (ValueBool(value) && parent_type == GL_TYPE_RECORD) {
			/*ommit TRUE*/
		} else {
			if (ValueBool(value)) {
				emit(&p,"true",4);
			} else {
				emit(&p,"false",5);
			}
		}
		break;
	case GL_TYPE_INT:
		if (ValueInteger(value) == 0 && parent_type == GL_TYPE_RECORD) {
			/*ommit 0*/
		} else {
			snprintf(buf,sizeof(buf),"%d",ValueInteger(value));
			size = strlen(buf);
			emit(&p,buf,size);
		}
		break;
	case GL_TYPE_NUMBER:
	case GL_TYPE_FLOAT:
		if (ValueToFloat(value) == 0.0 && parent_type == GL_TYPE_RECORD) {
			/*ommit 0.0*/
		} else {
			snprintf(buf,sizeof(buf),"%lf",ValueToFloat(value));
			size = strlen(buf);
			emit(&p,buf,size);
		}
		break;
	case GL_TYPE_ARRAY:
		emit(&p,"[",1);
		for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
			if (i > 0) {
				emit(&p,",",1);
			}
			p += _JSON_PackValueOmmit(opt,p,ValueArrayItem(value,i),value->type);
		}
		emit(&p,"]",1);
		break;
	case GL_TYPE_RECORD:
		size = _JSON_SizeValueOmmit(opt,value,parent_type);
		if (size > 0) {
			emit(&p,"{",1);
			for	( i = j = 0 ; i < ValueRecordSize(value) ; i ++ ) {
			    size = _JSON_SizeValueOmmit(opt,ValueRecordItem(value,i),GL_TYPE_RECORD);
				if (size > 0) {
					if (j > 0) {
						emit(&p,",",1);
					}
					key = ValueRecordName(value,i);
					emit(&p,"\"",1);
					emit(&p,key,strlen(key));
					emit(&p,"\":",2);
			    	size = _JSON_PackValueOmmit(opt,p,ValueRecordItem(value,i),GL_TYPE_RECORD);
					p += size;
					j++;
				}
			}
			emit(&p,"}",1);
		}
		break;
	}
LEAVE_FUNC;
	return	(p-pp);
}

extern	size_t
JSON_PackValueOmmit(
	CONVOPT *opt,
	unsigned char *p,
	ValueStruct *value)
{
	size_t size;
ENTER_FUNC;
	size = _JSON_PackValueOmmit(opt,p,value,GL_TYPE_RECORD);
	p += size;
	/*null terminate*/
	*p = 0;
	size += 1;
LEAVE_FUNC;
	return	size;
}

static	size_t
_JSON_SizeValueOmmit(
	CONVOPT *opt,
	ValueStruct *value,
	PacketDataType parent_type)
{
	size_t size,inc,inc_total,inc_child;
	int i,j;
	char buf[256],*str;
ENTER_FUNC;
	if (value == NULL) {
		return 0;
	}

	size = 0;

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
	case GL_TYPE_TIMESTAMP:
	case GL_TYPE_DATE:
	case GL_TYPE_TIME:
		str = ValueToString(value,NULL);
		if (strlen(str)) {
			size = EscapeStrLength(str);
		} else {
			if (parent_type == GL_TYPE_RECORD) {
				/* ommit "" */
			} else {
				size = 2; /* "" */
			}
		}
		break;
	case GL_TYPE_BOOL:
		if (ValueBool(value) && parent_type == GL_TYPE_RECORD) {
			/*ommit TRUE*/
		} else {
			if (ValueBool(value)) {
				size = 4; /*true*/
			} else {
				size = 5 /*false*/;
			}
		}
		break;
	case GL_TYPE_INT:
		if (ValueInteger(value) == 0 && parent_type == GL_TYPE_RECORD) {
			/*ommit 0*/
		} else {
			snprintf(buf,sizeof(buf),"%d",ValueInteger(value));
			size = strlen(buf);
		}
		break;
	case GL_TYPE_NUMBER:
	case GL_TYPE_FLOAT:
		if (ValueToFloat(value) == 0.0 && parent_type == GL_TYPE_RECORD) {
			/* ommit 0.0 */
		} else {
			snprintf(buf,sizeof(buf),"%lf",ValueToFloat(value));
			size = strlen(buf);
		}
		break;
	case GL_TYPE_ARRAY:
		size ++; /*[*/
		for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
			if (i > 0) {
				size ++; /*,*/
			}
			size += _JSON_SizeValueOmmit(opt,ValueArrayItem(value,i),value->type);
		}
		size ++; /*]*/
		break;
	case GL_TYPE_RECORD:
		for	( i = j = inc_total = 0 ; i < ValueRecordSize(value) ; i ++ ) {
			inc = 0;
			if (j > 0) {
				inc ++; /* , */
			}
			inc += strlen(ValueRecordName(value,i));
			inc += 3; /* "<key>": */
			inc_child = _JSON_SizeValueOmmit(opt,ValueRecordItem(value,i),value->type);
			inc += inc_child;
			if (inc_child > 0) {
				inc_total += inc;
				j++;
			}
		}
		size += inc_total;
		if (parent_type == GL_TYPE_RECORD && inc_total == 0) {
			/* ommit */
		} else {
			size += 2; /*{}*/
		}
		break;
	}
LEAVE_FUNC;
	return	size;
}

extern	size_t
JSON_SizeValueOmmit(
	CONVOPT *opt,
	ValueStruct *value)
{
	size_t size;
ENTER_FUNC;
	size = _JSON_SizeValueOmmit(opt,value,GL_TYPE_RECORD);
	/*null terminate*/
	size += 1;
LEAVE_FUNC;
	return	size;
}

/* normal pack */
static	size_t
_JSON_PackValue(
	CONVOPT *opt,
	unsigned char *p,
	ValueStruct *value)
{
	int i;
	unsigned char *pp;
	char buf[256],*str,*key;
ENTER_FUNC;
	if (value == NULL) {
		return 0;
	}

	pp = p;

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
	case GL_TYPE_TIMESTAMP:
	case GL_TYPE_DATE:
	case GL_TYPE_TIME:
		str = ValueToString(value,NULL);
		if (strlen(str)) {
			p += EscapeStr(str,p);
		} else {
			emit(&p,"\"\"",2);
		}
		break;
	case GL_TYPE_BOOL:
		if (ValueBool(value)) {
			emit(&p,"true",4);
		} else {
			emit(&p,"false",5);
		}
		break;
	case GL_TYPE_INT:
		snprintf(buf,sizeof(buf),"%d",ValueInteger(value));
		emit(&p,buf,strlen(buf));
		break;
	case GL_TYPE_NUMBER:
	case GL_TYPE_FLOAT:
		snprintf(buf,sizeof(buf),"%lf",ValueToFloat(value));
		emit(&p,buf,strlen(buf));
		break;
	case GL_TYPE_ARRAY:
		emit(&p,"[",1);
		for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
			if (i > 0) {
				emit(&p,",",1);
			}
			p += _JSON_PackValue(opt,p,ValueArrayItem(value,i));
		}
		emit(&p,"]",1);
		break;
	case GL_TYPE_RECORD:
		emit(&p,"{",1);
		for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
			if (i > 0) {
				emit(&p,",",1);
			}
			key = ValueRecordName(value,i);
			emit(&p,"\"",1);
			emit(&p,key,strlen(key));
			emit(&p,"\":",2);
			p += _JSON_PackValue(opt,p,ValueRecordItem(value,i));
		}
		emit(&p,"}",1);
		break;
	}
LEAVE_FUNC;
	return	(p-pp);
}

extern	size_t
JSON_PackValue(
	CONVOPT *opt,
	unsigned char *p,
	ValueStruct *value)
{
	size_t size;
ENTER_FUNC;
	size = _JSON_PackValue(opt,p,value);
	p += size;
	/*null terminate*/
	*p = 0;
	size += 1;
LEAVE_FUNC;
	return	size;
}

extern	size_t
_JSON_SizeValue(
	CONVOPT *opt,
	ValueStruct *value)
{
	size_t size,name_size,inc;
	int i;
	char buf[256],*str;
ENTER_FUNC;
	if (value == NULL) {
		return 0;
	}

	size = 0;

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
	case GL_TYPE_TIMESTAMP:
	case GL_TYPE_DATE:
	case GL_TYPE_TIME:
		str = ValueToString(value,NULL);
		if (strlen(str)) {
			size = EscapeStrLength(str);
		} else {
			size = 2; /* "" */
		}
		break;
	case GL_TYPE_BOOL:
		if (ValueBool(value)) {
			size = 4; /*true*/
		} else {
			size = 5 /*false*/;
		}
		break;
	case GL_TYPE_INT:
		snprintf(buf,sizeof(buf),"%d",ValueInteger(value));
		size = strlen(buf);
		break;
	case GL_TYPE_NUMBER:
	case GL_TYPE_FLOAT:
		snprintf(buf,sizeof(buf),"%lf",ValueToFloat(value));
		size = strlen(buf);
		break;
	case GL_TYPE_ARRAY:
		size ++; /*[*/
		for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
			if (i > 0) {
				size ++; /*,*/
			}
			size += _JSON_SizeValue(opt,ValueArrayItem(value,i));
		}
		size ++; /*]*/
		break;
	case GL_TYPE_RECORD:
		size ++; /*{*/
		for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
			inc = 0;
			if (i > 0) {
				size ++; inc ++; /* , */
			}
			name_size = strlen(ValueRecordName(value,i));
			size += name_size; inc += name_size;
			size += 3; inc += 3; /* "<key>": */
			name_size = _JSON_SizeValue(opt,ValueRecordItem(value,i));
			size += name_size;
			if (name_size == 0) {
				size -= inc;
			}
		}
		size ++; /*}*/
		break;
	}
LEAVE_FUNC;
	return	size;
}

extern	size_t
JSON_SizeValue(
	CONVOPT *opt,
	ValueStruct *value)
{
	size_t size;
ENTER_FUNC;
	size = _JSON_SizeValue(opt,value);
	/*null terminate*/
	size += 1;
LEAVE_FUNC;
	return	size;
}

Bool
CheckJSONObject(
	json_object *obj,
	enum json_type type)
{
	if (obj == NULL) {
		return FALSE;
	}
	if (is_error(obj)) {
		return FALSE;
	}
	if (!json_object_is_type(obj,type)) {
		return FALSE;
	}
	return TRUE;
}
