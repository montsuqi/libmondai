/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2008 Ogochan.
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

#ifdef HAVE_RUBY
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<glib.h>

#include    <ruby.h>
#include    <env.h>

#include	"libmondai.h"
#include	"Lex.h"
#include	"RecParser.h"
#include	"debug.h"

static char *codeset;

static VALUE aryval_new(ValueStruct *val, int need_free);
static VALUE recval_new(ValueStruct *val, int need_free);
static VALUE recval_get_field(VALUE self);
static VALUE recval_set_field(VALUE self, VALUE obj);

static VALUE mPanda;
static VALUE cArrayValue;
static VALUE cRecordValue;

static VALUE
bigdecimal_new(ValueStruct *val)
{
    VALUE cBigDecimal;
    char *s;

    rb_require("bigdecimal");
    cBigDecimal = rb_const_get(rb_cObject, rb_intern("BigDecimal"));
    s = ValueToString(val, NULL);
    return rb_funcall(cBigDecimal, rb_intern("new"), 2, rb_str_new2(s),
                      INT2NUM((ValueFixedLength(val))));
}


static VALUE
timestamp_new(ValueStruct *val)
{
    VALUE cTime
		, ret;

ENTER_FUNC;
    cTime = rb_const_get(rb_cObject, rb_intern("Time"));
    ret = rb_funcall(cTime, rb_intern("local"), 6,
					 INT2NUM((ValueDateTimeYear(val))),
					 INT2NUM((ValueDateTimeMon(val)+1)),
					 INT2NUM((ValueDateTimeMDay(val))),
					 INT2NUM((ValueDateTimeHour(val))),
					 INT2NUM((ValueDateTimeMin(val))),
					 INT2NUM((ValueDateTimeSec(val))));
LEAVE_FUNC;
	return	ret;
}

static VALUE
date_new(ValueStruct *val)
{
    VALUE cTime
		, ret;

ENTER_FUNC;
    cTime = rb_const_get(rb_cObject, rb_intern("Time"));
    ret = rb_funcall(cTime, rb_intern("local"), 3,
					 INT2NUM((ValueDateTimeYear(val))),
					 INT2NUM((ValueDateTimeMon(val)+1)),
					 INT2NUM((ValueDateTimeMDay(val))));
LEAVE_FUNC;
	return	ret;
}

static VALUE
get_value(ValueStruct *val)
{
    if (val == NULL)
        return Qnil;
    if (IS_VALUE_NIL(val))
        return Qnil;
    switch (ValueType(val)) {
    case GL_TYPE_BOOL:
        return ValueBool(val) ? Qtrue : Qfalse;
    case GL_TYPE_INT:
        return INT2NUM(ValueInteger(val));
    case GL_TYPE_FLOAT:
        return rb_float_new(ValueFloat(val));
    case GL_TYPE_NUMBER:
        return bigdecimal_new(val);
    case GL_TYPE_CHAR:
    case GL_TYPE_VARCHAR:
    case GL_TYPE_DBCODE:
    case GL_TYPE_TEXT:
        return rb_str_new2(ValueToString(val, codeset));
    case GL_TYPE_BYTE:
    case GL_TYPE_BINARY:
        if (ValueByte(val) == NULL) {
            return Qnil;
        }
        else {
            return rb_str_new(ValueByte(val), ValueByteLength(val));
        }
    case GL_TYPE_OBJECT:
        return INT2NUM(ValueObjectId(val));
    case GL_TYPE_ARRAY:
        return aryval_new(val, 0);
    case GL_TYPE_RECORD:
        return recval_new(val, 0);
	case GL_TYPE_TIMESTAMP:
		return timestamp_new(val);
	case GL_TYPE_DATE:
		return date_new(val);
    default:
        rb_raise(rb_eArgError, "unsupported ValueStruct type");
        break;
    }
    return Qnil;                /* not reached */
}

typedef struct _value_struct_data {
    ValueStruct *value;
    VALUE cache;
} value_struct_data;

static void
set_value(ValueStruct *value, VALUE obj)
{
    VALUE class_path, str;

    if (NIL_P(obj)) {
        ValueIsNil(value);
    }
    else {
        ValueIsNonNil(value);
        switch (TYPE(obj)) {
        case T_TRUE:
        case T_FALSE:
            SetValueBool(value, RTEST(obj) ? TRUE : FALSE);
            break;
        case T_FIXNUM:
            SetValueInteger(value, FIX2INT(obj));
            break;
        case T_BIGNUM:
            SetValueInteger(value, NUM2INT(obj));
            break;
        case T_FLOAT:
            SetValueFloat(value, RFLOAT(obj)->value);
            break;
        case T_STRING:
            switch (ValueType(value)) {
            case GL_TYPE_BYTE:
            case GL_TYPE_BINARY:
                SetValueBinary(value, RSTRING(obj)->ptr, RSTRING(obj)->len);
                break;
            default:
                SetValueStringWithLength(value,
                                         RSTRING(obj)->ptr,
                                         RSTRING(obj)->len,
                                         codeset);
                break;
            }
            break;
        default:
            class_path = rb_class_path(CLASS_OF(obj));
            if (strcasecmp(StringValuePtr(class_path), "BigDecimal") == 0) {
                str = rb_funcall(obj, rb_intern("to_s"), 1, rb_str_new2("F"));
            } else
            if (strcasecmp(StringValuePtr(class_path), "Time") == 0) {
                str = rb_funcall(obj, rb_intern("strftime"), 1, rb_str_new2("%Y%m%d%H%M%S"));
dbgprintf("strftime [%s]",StringValuePtr(str));
            }
            else {
                str = rb_funcall(obj, rb_intern("to_s"), 0);
            }
            SetValueString(value, StringValuePtr(str), codeset);
            break;
        }
    }
}

static int
value_equal(ValueStruct *val, VALUE obj)
{
    if (val == NULL)
        return 0;
    if (IS_VALUE_NIL(val))
        return NIL_P(obj);
    switch (ValueType(val)) {
    case GL_TYPE_BOOL:
        if (ValueBool(val)) {
            return obj == Qtrue;
        }
        else {
            return obj == Qfalse;
        }
    case GL_TYPE_INT:
        return ValueInteger(val) == NUM2INT(obj);
    case GL_TYPE_FLOAT:
        return ValueFloat(val) == NUM2DBL(obj);
    case GL_TYPE_NUMBER:
    {
        VALUE bd = bigdecimal_new(val);
        return RTEST(rb_funcall(bd, rb_intern("=="), 1, obj));
    }
    case GL_TYPE_CHAR:
    case GL_TYPE_VARCHAR:
    case GL_TYPE_DBCODE:
    case GL_TYPE_TEXT:
        return strcmp(ValueToString(val, codeset), StringValuePtr(obj)) == 0;
    case GL_TYPE_BYTE:
    case GL_TYPE_BINARY:
        return memcmp(ValueByte(val), StringValuePtr(obj),
                      ValueByteLength(val)) == 0;
    case GL_TYPE_OBJECT:
        return ValueInteger(val) == NUM2INT(obj);
    default:
        return 0;
    }
}

#define CACHEABLE(val) (val != NULL && \
                        (ValueType(val) == GL_TYPE_ARRAY || \
                         ValueType(val) == GL_TYPE_RECORD))

static void
value_struct_mark(value_struct_data *data)
{
    rb_gc_mark(data->cache);
}

static void
value_struct_free(value_struct_data *data)
{
ENTER_FUNC;
	FreeValueStruct(data->value);
	free(data);
LEAVE_FUNC;
}

static VALUE
aryval_new(ValueStruct *val, int need_free)
{
    VALUE obj;
    value_struct_data *data;

    obj = Data_Make_Struct(cArrayValue, value_struct_data,
                           value_struct_mark,
                           (need_free ?
							(RUBY_DATA_FUNC) value_struct_free :
							(RUBY_DATA_FUNC) free),
                           data);
    data->value = val;
    data->cache = rb_ary_new2(ValueArraySize(val));
    return obj;
}

static VALUE
aryval_length(VALUE self)
{
    value_struct_data *data;

    Data_Get_Struct(self, value_struct_data, data);
    return INT2NUM(ValueArraySize(data->value));
}

static VALUE
aryval_aref(VALUE self, VALUE index)
{
    value_struct_data *data;
    int i = NUM2INT(index);
    VALUE obj;
    ValueStruct *val;

    Data_Get_Struct(self, value_struct_data, data);
    if (i >= 0 && i < RARRAY(data->cache)->len &&
        !NIL_P(RARRAY(data->cache)->ptr[i]))
        return RARRAY(data->cache)->ptr[i];
    val = GetArrayItem(data->value, i);
    if (val == NULL)
        return Qnil;
    obj = get_value(val);
    if (CACHEABLE(val))
        rb_ary_store(data->cache, i, obj);
    return obj;
}

static VALUE
aryval_aset(VALUE self, VALUE index, VALUE obj)
{
    value_struct_data *data;
    int i = NUM2INT(index);
    ValueStruct *val;

    Data_Get_Struct(self, value_struct_data, data);
    val = GetArrayItem(data->value, i);
    if (val == NULL)
        rb_raise(rb_eIndexError, "index out of range: %d", i);
    set_value(val, obj);
    return obj;
}

static VALUE
aryval_index(VALUE self, VALUE obj)
{
    value_struct_data *data;
    int i;

    Data_Get_Struct(self, value_struct_data, data);
    for (i = 0; i < ValueArraySize(data->value); i++) {
        if (value_equal(ValueArrayItem(data->value, i), obj))
            return INT2NUM(i);
    }
    return Qnil;
}

static	void
recval_set_method(VALUE obj, ValueStruct *val)
{
    int i;
    VALUE name;

    for (i = 0; i < ValueRecordSize(val); i++) {
        rb_define_singleton_method(obj, ValueRecordName(val, i),
                                   recval_get_field, 0);
        name = rb_str_new2(ValueRecordName(val, i));
        rb_str_cat2(name, "=");
        rb_define_singleton_method(obj, StringValuePtr(name),
                                   recval_set_field, 1);
    }
}


static VALUE
recval_new(ValueStruct *val, int need_free)
{
    VALUE obj;
    value_struct_data *data;

ENTER_FUNC;
    obj = Data_Make_Struct(cRecordValue, value_struct_data,
                           value_struct_mark,
                           (need_free ?
							(RUBY_DATA_FUNC) value_struct_free :
							(RUBY_DATA_FUNC) free),
                           data);
    data->value = val;
    data->cache = rb_hash_new();
	recval_set_method(obj,data->value);
LEAVE_FUNC;
    return obj;
}

static VALUE
recval_alloc(VALUE klass)
{
    VALUE obj;
    value_struct_data *data;

ENTER_FUNC;
    obj = Data_Make_Struct(cRecordValue, value_struct_data,
                           value_struct_mark,
						   (RUBY_DATA_FUNC) value_struct_free,
                           data);
LEAVE_FUNC;
    return obj;
}

static VALUE
recval_initialize(VALUE self, VALUE defs)
{
	ValueStruct	*val;
    value_struct_data *data;

ENTER_FUNC;
	Data_Get_Struct(self, value_struct_data, data);
	val = RecParseValueMem(StringValuePtr(defs),NULL);
	data->value = val;
    data->cache = rb_hash_new();
	recval_set_method(self,data->value);
LEAVE_FUNC;
    return Qnil;
}

static VALUE
recval_length(VALUE self)
{
    value_struct_data *data;

    Data_Get_Struct(self, value_struct_data, data);
    return INT2NUM(ValueRecordSize(data->value));
}

static VALUE
recval_clear(VALUE self)
{
    value_struct_data *data;

    Data_Get_Struct(self, value_struct_data, data);
	InitializeValue(data->value);
    return Qnil;
}

static VALUE
recval_aref(VALUE self, VALUE name)
{
    VALUE obj;
    value_struct_data *data;
    ValueStruct *val;

    Data_Get_Struct(self, value_struct_data, data);

    if (!NIL_P(obj = rb_hash_aref(data->cache, name)))
        return obj;

    val = GetItemLongName(data->value, StringValuePtr(name));
    if (val == NULL)
        rb_raise(rb_eArgError, "no such field: %s", StringValuePtr(name));
    obj = get_value(val);
    if (CACHEABLE(val))
        rb_hash_aset(data->cache, name, obj);
    return obj;
}

static VALUE
recval_aset(VALUE self, VALUE name, VALUE obj)
{
    value_struct_data *data;
    ValueStruct *val;

    Data_Get_Struct(self, value_struct_data, data);
    val = GetItemLongName(data->value, StringValuePtr(name));
    if (val == NULL)
        rb_raise(rb_eArgError, "no such field: %s", StringValuePtr(name));
    set_value(val, obj);
    return obj;
}

static VALUE
recval_get_field(VALUE self)
{
    VALUE obj;
    value_struct_data *data;
    ValueStruct *val;
    char *name = (char*)rb_id2name(ruby_frame->last_func);

    Data_Get_Struct(self, value_struct_data, data);

    if (!NIL_P(obj = rb_hash_aref(data->cache, rb_str_new2(name))))
        return obj;

    val = GetRecordItem(data->value, name);
    obj = get_value(val);
    if (CACHEABLE(val))
        rb_hash_aset(data->cache, rb_str_new2(name), obj);
    return obj;
}

static VALUE
recval_set_field(VALUE self, VALUE obj)
{
    value_struct_data *data;
    ValueStruct *val;
    char *s = (char*)rb_id2name(ruby_frame->last_func);
    VALUE name;

    name = rb_str_new(s, strlen(s) - 1);

    Data_Get_Struct(self, value_struct_data, data);
    val = GetRecordItem(data->value, StringValuePtr(name));
    if (val == NULL)
        rb_raise(rb_eArgError, "no such field: %s", StringValuePtr(name));
    set_value(val, obj);
    return obj;
}

static VALUE
recval_native_pack(VALUE self)
{
    value_struct_data *data;
    LargeByteString *lbs;
    size_t size;
    VALUE packed;

    Data_Get_Struct(self, value_struct_data, data);

    lbs = NewLBS();
    size = NativeSizeValue(NULL,data->value);
	LBS_ReserveSize(lbs,size,FALSE);
    NativePackValue(NULL,LBS_Body(lbs),data->value);
    packed = rb_str_new(LBS_Body(lbs),size);
    FreeLBS(lbs);

    return packed;
}

static VALUE
recval_native_unpack(VALUE self,VALUE packed)
{
    value_struct_data *data;

    Data_Get_Struct(self, value_struct_data, data);
	NativeUnPackValue(NULL,RSTRING(packed)->ptr,data->value);
    return self;
}

static VALUE
recval_children_longnames(VALUE self)
{
    value_struct_data *data;
    GList *list;
    VALUE ret;
    VALUE str;

    ret = rb_ary_new();

    Data_Get_Struct(self, value_struct_data, data);
	list = GetChildrenLongNames(NULL,data->value);
    for(;list != NULL;list=list->next) {
       str = rb_str_new2((char*)list->data);
       rb_ary_push(ret,str);
    }
    return ret;
}

extern	void
Init_rubymondai(void)
{
ENTER_FUNC;
    mPanda = rb_define_module("Rubymondai");
    cArrayValue = rb_define_class_under(mPanda, "ArrayValue", rb_cObject);
    rb_define_method(cArrayValue, "length", aryval_length, 0);
    rb_define_method(cArrayValue, "size", aryval_length, 0);
    rb_define_method(cArrayValue, "[]", aryval_aref, 1);
    rb_define_method(cArrayValue, "[]=", aryval_aset, 2);
    rb_define_method(cArrayValue, "index", aryval_index, 1);
    cRecordValue = rb_define_class_under(mPanda, "RecordValue", rb_cObject);
	rb_define_alloc_func(cRecordValue, recval_alloc);
	rb_define_private_method(cRecordValue, "initialize", recval_initialize, 1);
    rb_define_method(cRecordValue, "length", recval_length, 0);
    rb_define_method(cRecordValue, "size", recval_length, 0);
    rb_define_method(cRecordValue, "clear", recval_clear, 0);
    rb_define_method(cRecordValue, "[]", recval_aref, 1);
    rb_define_method(cRecordValue, "[]=", recval_aset, 2);
    rb_define_method(cRecordValue, "native_pack",recval_native_pack,0);
    rb_define_method(cRecordValue, "native_unpack",recval_native_unpack,1);
	rb_define_method(cRecordValue, "children_longnames", recval_children_longnames,0);

	RecParserInit();
    codeset = "utf-8";
LEAVE_FUNC;
}
#endif
