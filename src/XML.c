/*	PANDA -- a simple transaction monitor

Copyright (C) 2001-2003 Ogochan & JMA (Japan Medical Association).

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
#ifdef	USE_XML

#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include    <sys/types.h>
#ifdef	USE_XML2
#include	<libxml/parser.h>
#else
#include	<parser.h>
#endif

#include	"types.h"
#include	"misc.h"
#include	"value.h"
#include	"getset.h"
#include	"monstring.h"
#include	"XML_v.h"
#include	"debug.h"

typedef struct	{
	ValueStruct	*root
	,			*value;
	char		longname[SIZE_LONGNAME+1];
	xmlChar		*buff;
	size_t		size;
	Bool		fStart;
	CONVOPT		*opt;
}	ValueContext;
#define	NextName(ctx)	(((ctx)->longname)+strlen((ctx)->longname))

static	XMLOPT	*
NewXMLOPT(void)
{
	XMLOPT	*ret;

	ret = New(XMLOPT);
	ret->fIndent = FALSE;
	ret->fType = FALSE;
	ret->type = XML_TYPE1;

	return	(ret);
}

extern	void
ConvSetIndent(
	CONVOPT	*opt,
	Bool	v)
{
	if		(  opt->appendix  ==  NULL  ) {
		opt->appendix = NewXMLOPT();
	}
	((XMLOPT *)opt->appendix)->fIndent = v;
}

extern	void
ConvSetType(
	CONVOPT	*opt,
	Bool	v)
{
	if		(  opt->appendix  ==  NULL  ) {
		opt->appendix = NewXMLOPT();
	}
	((XMLOPT *)opt->appendix)->fType = v;
}

extern	void
ConvSetXmlType(
	CONVOPT	*opt,
	int		type)
{
	if		(  opt->appendix  ==  NULL  ) {
		opt->appendix = NewXMLOPT();
	}
	((XMLOPT *)opt->appendix)->type = type;
}

static	int		nIndent;

static	char	*
XML_Encode(
	char	*str,
	char	*buff)
{
#if	0
	char	*p;

	p = buff;
	for	( ; *str != 0 ; str ++ ) {
		if		(  *str  <  0x20  ) {
			switch	(*str) {
			  case	' ':
			  case	0x1B:
				*p ++ = *str;
				break;
			  default:
				p += sprintf(p,"\\%02X",*str);
				break;
			}
		} else {
			*p ++ = *str;
		}
	}
	*p = 0;
	return	(buff);
#else
	return	(str);
#endif
}

static	size_t
PutCR(
	CONVOPT		*opt,
	char		*p)
{
	size_t	size;

	if		(  ConvIndent(opt)  ) {
		*p = '\n';
		size = 1;
	} else {
		size = 0;
	}
	return	(size);
}

static	byte	*
_XML_PackValue1(
	CONVOPT		*opt,
	byte		*p,
	char		*name,
	ValueStruct	*value,
	char		*buff)
{
	char	num[SIZE_NAME+1];
	int		i;

	if		(  IS_VALUE_NIL(value)  )	return	(p);
	if		(  value  !=  NULL  ) {
		nIndent ++;
		if		(  ConvIndent(opt)  ) {
			for	( i = 0 ; i < nIndent ; i ++ )	*p ++ = '\t';
		}
		switch	(ValueType(value)) {
		  case	GL_TYPE_ARRAY:
			p += sprintf(p,"<lm:array name=\"%s\" count=\"%d\">"
						 ,name,ValueArraySize(value));
			p += PutCR(opt,p);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				sprintf(num,"%s[%d]",name,i);
				p = _XML_PackValue1(opt,p,num,ValueArrayItem(value,i),buff);
			}
			if		(  ConvIndent(opt)  ) {
				for	( i = 0 ; i < nIndent ; i ++ )	*p ++ = '\t';
			}
			p += sprintf(p,"</lm:array>");
			break;
		  case	GL_TYPE_RECORD:
			p += sprintf(p,"<lm:record name=\"%s\" size=\"%d\">"
						 ,name,ValueRecordSize(value));
			p += PutCR(opt,p);
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p = _XML_PackValue1(opt,p,ValueRecordName(value,i),ValueRecordItem(value,i),buff);
			}
			if		(  ConvIndent(opt)  ) {
				for	( i = 0 ; i < nIndent ; i ++ )	*p ++ = '\t';
			}
			p += sprintf(p,"</lm:record>");
			break;
		  default:
			p += sprintf(p,"<lm:item name=\"%s\"",name);
			if		(  ConvType(opt)  ) {
				p += sprintf(p," type=");
				switch	(ValueType(value)) {
				  case	GL_TYPE_INT:
					p += sprintf(p,"\"int\"");
					break;
				  case	GL_TYPE_BOOL:
					p += sprintf(p,"\"bool\"");
					break;
				  case	GL_TYPE_BYTE:
					p += sprintf(p,"\"byte\" size=\"%d\"",ValueStringLength(value));
					break;
				  case	GL_TYPE_CHAR:
					p += sprintf(p,"\"char\" size=\"%d\"",ValueStringLength(value));
					break;
				  case	GL_TYPE_VARCHAR:
					p += sprintf(p,"\"varchar\" size=\"%d\"",ValueStringLength(value));
					break;
				  case	GL_TYPE_TEXT:
					p += sprintf(p,"\"text\" size=\"%d\"",ValueStringLength(value));
					break;
				  case	GL_TYPE_DBCODE:
					p += sprintf(p,"\"dbcode\" size=\"%d\"",ValueStringLength(value));
					break;
				  case	GL_TYPE_NUMBER:
					p += sprintf(p,"\"number\" size=\"%d\" ssize=\"%d\"",
								 ValueFixedLength(value),ValueFixedSlen(value));
					break;
				  case	GL_TYPE_FLOAT:
					p += sprintf(p,"\"float\"");
					break;
				  case	GL_TYPE_ALIAS:
				  default:
					break;
				}
			}
			if		(  ConvIndent(opt)  ) {
				p += sprintf(p,">");
			} else {
				p += sprintf(p," />");
			}
			if		(  !IS_VALUE_NIL(value)  ) {
#ifdef	USE_XML2
				p += sprintf(p,"%s",XML_Encode(ValueToString(value,opt->coding),buff));
#else
				p += sprintf(p,"%s",XML_Encode(ValueToString(value,LIBXML_CODE),buff));
#endif
			}
			if		(  ConvIndent(opt)  ) {
				p += sprintf(p,"</lm:item>");
			}
			break;
		}
		p += PutCR(opt,p);
		nIndent --;
	}
	return	(p);
}

static	byte	*
_XML_PackValue2(
	CONVOPT		*opt,
	byte		*p,
	char		*name,
	ValueStruct	*value,
	char		*buff)
{
	char	num[SIZE_NAME+1];
	int		i;

	if		(  IS_VALUE_NIL(value)  )	return	(p);
	if		(  value  !=  NULL  ) {
		nIndent ++;
		if		(  ConvIndent(opt)  ) {
			for	( i = 0 ; i < nIndent ; i ++ )	*p ++ = '\t';
		}
		switch	(ValueType(value)) {
		  case	GL_TYPE_ARRAY:
			p += sprintf(p,"<%s:%s type=\"array\"",opt->recname,name);
			p += sprintf(p," count=\"%d\">",ValueArraySize(value));
			p += PutCR(opt,p);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				sprintf(num,"%s:%d",name,i);
				p = _XML_PackValue2(opt,p,num,ValueArrayItem(value,i),buff);
			}
			if		(  ConvIndent(opt)  ) {
				for	( i = 0 ; i < nIndent ; i ++ )	*p ++ = '\t';
			}
			p += sprintf(p,"</%s:%s>",opt->recname,name);
			break;
		  case	GL_TYPE_RECORD:
			p += sprintf(p,"<%s:%s type=\"record\"",opt->recname,name);
			p += sprintf(p," size=\"%d\">",ValueRecordSize(value));
			p += PutCR(opt,p);
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				p = _XML_PackValue2(opt,p,ValueRecordName(value,i),ValueRecordItem(value,i),buff);
			}
			if		(  ConvIndent(opt)  ) {
				for	( i = 0 ; i < nIndent ; i ++ )	*p ++ = '\t';
			}
			p += sprintf(p,"</%s:%s>",opt->recname,name);
			break;
		  default:
			p += sprintf(p,"<%s:%s",opt->recname,name);
			p += sprintf(p," type=");
			switch	(ValueType(value)) {
			  case	GL_TYPE_INT:
				p += sprintf(p,"\"int\"");
				break;
			  case	GL_TYPE_BOOL:
				p += sprintf(p,"\"bool\"");
				break;
			  case	GL_TYPE_BYTE:
				p += sprintf(p,"\"byte\" size=\"%d\"",ValueStringLength(value));
				break;
			  case	GL_TYPE_CHAR:
				p += sprintf(p,"\"char\" size=\"%d\"",ValueStringLength(value));
				break;
			  case	GL_TYPE_VARCHAR:
				p += sprintf(p,"\"varchar\" size=\"%d\"",ValueStringLength(value));
				break;
			  case	GL_TYPE_TEXT:
				p += sprintf(p,"\"text\" size=\"%d\"",ValueStringLength(value));
				break;
			  case	GL_TYPE_DBCODE:
				p += sprintf(p,"\"dbcode\" size=\"%d\"",ValueStringLength(value));
				break;
			  case	GL_TYPE_NUMBER:
				p += sprintf(p,"\"number\" size=\"%d\" ssize=\"%d\"",
							 ValueFixedLength(value),ValueFixedSlen(value));
				break;
			  case	GL_TYPE_FLOAT:
				p += sprintf(p,"\"float\"");
				break;
			  case	GL_TYPE_ALIAS:
			  default:
				break;
			}
			if		(  ConvIndent(opt)  ) {
				p += sprintf(p,">");
			} else {
				p += sprintf(p," />");
			}
			if		(  !IS_VALUE_NIL(value)  ) {
#ifdef	USE_XML2
				p += sprintf(p,"%s",XML_Encode(ValueToString(value,opt->coding),buff));
#else
				p += sprintf(p,"%s",XML_Encode(ValueToString(value,LIBXML_CODE),buff));
#endif
			}
			if		(  ConvIndent(opt)  ) {
				p += sprintf(p,"</%s:%s>",opt->recname,name);
			}
			break;
		}
		p += PutCR(opt,p);
		nIndent --;
	}
	return	(p);
}

extern	byte	*
XML_PackValue(
	CONVOPT		*opt,
	 byte		*p,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF+1];

	p += sprintf(p,"<?xml version=\"1.0\"");
#ifdef	USE_XML2
	if		(  opt->coding  !=  NULL  ) {
		p += sprintf(p," encoding=\"%s\"",opt->coding);
	}
#else
	p += sprintf(p," encoding=\"%s\"",LIBXML_CODE);
#endif
	p += sprintf(p,"?>");
	p += PutCR(opt,p);
	nIndent = 0;
	switch	(ConvXmlType(opt)) {
	  case	XML_TYPE1:
		p += sprintf(p,"<lm:block xmlns:lm=\"%s\">",NS_URI);
		p += PutCR(opt,p);
		p =_XML_PackValue1(opt,p,opt->recname,value,buff);
		p += sprintf(p,"</lm:block>");
		break;
	  case	XML_TYPE2:
		p += sprintf(p,"<%s:data xmlns:%s=\"%s/%s.rec\">"
					 ,opt->recname,opt->recname,NS_URI,opt->recname);
		p += PutCR(opt,p);
		p =_XML_PackValue2(opt,p,opt->recname,value,buff);
		p += sprintf(p,"</%s:data>",opt->recname);
		break;
	}
	*p = 0;
	return	(p);
}

static	int
isStandalone_(
	ValueContext	*value)
{
	fprintf(stderr, "SAX.isStandalone()\n");
	return(0);
}

static	int
hasInternalSubset_(
	ValueContext	*value)
{
	fprintf(stderr, "SAX.hasInternalSubset()\n");
	return(0);
}

static	int
hasExternalSubset_(
	ValueContext	*value)
{
	fprintf(stderr, "SAX.hasExternalSubset()\n");
	return(0);
}

static	void
internalSubset_(
	ValueContext	*value,
	xmlChar		*name,
	xmlChar		*ExternalID,
	xmlChar		*SystemID)
{
	fprintf(stderr, "SAX.internalSubset(%s,", name);
	if (ExternalID == NULL)
		fprintf(stderr, " ,");
	else
		fprintf(stderr, " %s,", ExternalID);
	if (SystemID == NULL)
		fprintf(stderr, " )\n");
	else
		fprintf(stderr, " %s)\n", SystemID);
}

#ifdef	USE_XML2
static	void
externalSubset_(
	ValueContext	*value,
	xmlChar		*name,
	xmlChar		*ExternalID,
	xmlChar		*SystemID)
{
	fprintf(stderr, "SAX.externalSubset(%s,", name);
	if (ExternalID == NULL)
		fprintf(stderr, " ,");
	else
		fprintf(stderr, " %s,", ExternalID);
	if (SystemID == NULL)
		fprintf(stderr, " )\n");
	else
		fprintf(stderr, " %s)\n", SystemID);
}
#endif

static	xmlParserInputPtr
resolveEntity_(
	ValueContext	*value,
	xmlChar		*publicId,
	xmlChar		*systemId)
{
	fprintf(stderr, "SAX.resolveEntity(");
	if (publicId != NULL)
		fprintf(stderr, "%s", (char *)publicId);
	else
		fprintf(stderr, " ");
	if (systemId != NULL)
		fprintf(stderr, ", %s)\n", (char *)systemId);
	else
		fprintf(stderr, ", )\n");
	return	(NULL);
}

static	xmlEntityPtr
getEntity_(
	ValueContext	*value,
	xmlChar		*name)
{
	fprintf(stderr, "SAX.getEntity(%s)\n", name);
	return(NULL);
}

static	xmlEntityPtr
getParameterEntity_(
	ValueContext	*value,
	xmlChar		*name)
{
	fprintf(stderr, "SAX.getParameterEntity(%s)\n", name);
    return(NULL);
}


static	void
entityDecl_(
	ValueContext	*value,
	xmlChar	*name,
	int		type,
	xmlChar	*publicId,
	xmlChar	*systemId,
	xmlChar *content)
{
	fprintf(stderr, "SAX.entityDecl(%s, %d, %s, %s, %s)\n",
			name, type, publicId, systemId, content);
}

static	void
attributeDecl_(
	ValueContext	*value,
	xmlChar		*elem,
	xmlChar		*name,
	int			type,
	int			def,
	xmlChar		*defaultValue,
	xmlEnumerationPtr tree)
{
	if (defaultValue == NULL)
		fprintf(stderr, "SAX.attributeDecl(%s, %s, %d, %d, NULL, ...)\n",
				elem, name, type, def);
    else
		fprintf(stderr, "SAX.attributeDecl(%s, %s, %d, %d, %s, ...)\n",
				elem, name, type, def, defaultValue);
}

static	void
elementDecl_(
	ValueContext	*value,
	xmlChar		*name,
	int			type,
	xmlElementContentPtr	content)
{
	fprintf(stderr, "SAX.elementDecl(%s, %d, ...)\n",
			name, type);
}

static	void
notationDecl_(
	ValueContext	*value,
	xmlChar		*name,
	xmlChar		*publicId,
	xmlChar		*systemId)
{
	fprintf(stderr, "SAX.notationDecl(%s, %s, %s)\n",
			(char *) name, (char *) publicId, (char *) systemId);
}

static	void
unparsedEntityDecl_(
	ValueContext	*value,
	xmlChar		*name,
	xmlChar		*publicId,
	xmlChar		*systemId,
	xmlChar		*notationName)
{
	fprintf(stdout, "SAX.unparsedEntityDecl(%s, %s, %s, %s)\n",
			(char *) name, (char *) publicId, (char *) systemId,
			(char *) notationName);
}

static	void
setDocumentLocator_(
	ValueContext			*value,
	xmlSAXLocatorPtr	loc)
{
#ifdef	DEBUG
	fprintf(stderr, "SAX.setDocumentLocator()\n");
#endif
}

static	void
startDocument_(
	ValueContext	*value)
{
#ifdef	DEBUG
    fprintf(stderr, "SAX.startDocument()\n");
#endif
}

static	void
endDocument_(
	ValueContext	*value)
{
#ifdef	DEBUG
    fprintf(stderr, "SAX.endDocument()\n");
#endif
}

static	void
reference_(
	ValueContext	*value,
	xmlChar		*name)
{
	fprintf(stderr, "SAX.reference(%s)\n", name);
}

static void
ignorableWhitespace_(
	ValueContext	*value,
	xmlChar		*ch,
	int			len)
{
    char output[40];
    int i;

    for (i = 0;(i<len) && (i < 30);i++)
		output[i] = ch[i];
	output[i] = 0;
	fprintf(stderr, "SAX.ignorableWhitespace(%s, %d)\n", output, len);
}

static	void
processingInstruction_(
	ValueContext	*value,
	xmlChar		*target,
	xmlChar		*data)
{
	fprintf(stderr, "SAX.processingInstruction(%s, %s)\n",
			(char *) target, (char *) data);
}

static	void
cdataBlock_(
	ValueContext	*value,
	xmlChar		*data,
	int len)
{
	fprintf(stderr, "SAX.pcdata(%.20s, %d)\n",
			(char *) data, len);
}

static	void
comment_(
	ValueContext	*value,
	xmlChar		*comment)
{
	fprintf(stderr, "SAX.comment(%s)\n", comment);
}

static	void
warning_(
	ValueContext	*value,
	char		*msg, ...)
{
    va_list args;

    va_start(args, msg);
    fprintf(stdout, "SAX.warning: ");
    vfprintf(stdout, msg, args);
    va_end(args);
}

static	void
error_(
	ValueContext	*value,
	char		*msg, ...)
{
    va_list args;

    va_start(args, msg);
    fprintf(stdout, "SAX.error: ");
    vfprintf(stdout, msg, args);
    va_end(args);
}

static	void
fatalError_(
	ValueContext	*value,
	char		*msg, ...)
{
    va_list args;

    va_start(args, msg);
    fprintf(stderr, "fatalError: ");
    vfprintf(stderr, msg, args);
    va_end(args);
}

static	xmlChar	*
GetAttribute(
	xmlChar	**atts,
	xmlChar	*name)
{
	xmlChar	*ret;

	ret = NULL;
	while	( *atts  !=  NULL  ) {
		if		(  stricmp(*atts,name)  ==  0  ) {
			ret = *(atts + 1);
			break;
		}
		atts ++;
	}
	return	(ret);
}

static	void
startElement1_(
	ValueContext	*ctx,
	xmlChar	*name,
	xmlChar	**atts)
{
	xmlChar	*vname
	,		*p
	,		*q;

	p = NextName(ctx);
	q = p;
	if		(  ( vname = (char *)GetAttribute(atts,(xmlChar *)"name") )  !=  NULL  ) {
		if		(  (char *)p  !=  ctx->longname  ) {
			*p++ = '.';
		}
		sprintf(p,"%s",(char *)vname);
	}
#ifdef	DEBUG
	printf("startElement_(%s)[%s]\n",(char *)name,ctx->longname);
#endif
	if		(  stricmp((char *)name,"lm:item")  ==  0  ) {
		ctx->value = GetItemLongName(ctx->root,ctx->longname);
		*q = 0;
		ctx->fStart = TRUE;
	} else
	if		(  stricmp(name,"lm:record")  ==  0  ) {
		if		(  ctx->value  ==  NULL  ) {
			*q = 0;
		}
		ctx->value = GetItemLongName(ctx->root,ctx->longname);
		ctx->fStart = FALSE;
	} else
	if		(  stricmp(name,"lm:array")  ==  0  ) {
		*q = 0;
		ctx->fStart = FALSE;
	} else
	if		(  stricmp(name,"lm:block")  ==  0  ) {
		ctx->value = NULL;
		ctx->fStart = FALSE;
	}
}

static	void
endElement1_(
	ValueContext	*ctx,
	xmlChar	*name)
{
	xmlChar	*p;

#ifdef	DEBUG
	printf("endElement_(%s)\n",(char *)name);
#endif
	if		(	(  stricmp(name,"lm:array")  ==  0  )
			||	(  stricmp(name,"lm:record")  ==  0  ) ) {
		if		(  ( p = strrchr(ctx->longname,'.') )  !=  NULL  ) {
			*p = 0;
		} else {
			*ctx->longname = 0;
		}
	}
	ctx->fStart = FALSE;
}

static	void
characters_(
	ValueContext	*ctx,
	xmlChar	*ch,
	size_t	len)
{

	if		(  ctx->fStart  ) {
		if		(  ctx->size  <  len  ) {
			xfree(ctx->buff);
			ctx->size = ( len + 1 );
			ctx->buff = (xmlChar *)xmalloc(ctx->size * sizeof(xmlChar));
		}
		memcpy(ctx->buff,ch,len*sizeof(xmlChar));
		ctx->buff[len] = 0;
		dbgprintf("characters_[%s]\n",(char *)ctx->buff);
		if		(  ctx->value  !=  NULL  ) {
#ifdef	USE_XML2
			SetValueString(ctx->value,(char *)ctx->buff,NULL);
#else
			SetValueString(ctx->value,(char *)ctx->buff,LIBXML_CODE);
#endif
			ValueIsNonNil(ctx->value);
		}
	}
}

static	xmlSAXHandler mondaiSAXHandlerStruct1 = {
	(internalSubsetSAXFunc)internalSubset_,
	(isStandaloneSAXFunc)isStandalone_,
	(hasInternalSubsetSAXFunc)hasInternalSubset_,
	(hasExternalSubsetSAXFunc)hasExternalSubset_,
	(resolveEntitySAXFunc)resolveEntity_,
	(getEntitySAXFunc)getEntity_,
	(entityDeclSAXFunc)entityDecl_,
	(notationDeclSAXFunc)notationDecl_,
	(attributeDeclSAXFunc)attributeDecl_,
	(elementDeclSAXFunc)elementDecl_,
	(unparsedEntityDeclSAXFunc)unparsedEntityDecl_,
	(setDocumentLocatorSAXFunc)setDocumentLocator_,
	(startDocumentSAXFunc)startDocument_,
	(endDocumentSAXFunc)endDocument_,
	(startElementSAXFunc)startElement1_,
	(endElementSAXFunc)endElement1_,
	(referenceSAXFunc)reference_,
	(charactersSAXFunc)characters_,
	(ignorableWhitespaceSAXFunc)ignorableWhitespace_,
	(processingInstructionSAXFunc)processingInstruction_,
	(commentSAXFunc)comment_,
	(warningSAXFunc)warning_,
	(errorSAXFunc)error_,
	(fatalErrorSAXFunc)fatalError_,
	(getParameterEntitySAXFunc)getParameterEntity_,
	(cdataBlockSAXFunc)cdataBlock_,
#ifdef	USE_XML2
	(externalSubsetSAXFunc)externalSubset_,
    1
#endif
};

static	void
startElement2_(
	ValueContext	*ctx,
	xmlChar	*name,
	xmlChar	**atts)
{
	xmlChar	*vname
	,		*rname
	,		*p
	,		*q
	,		*type;
	xmlChar	buff[SIZE_LONGNAME+1];
	int		count;
	Bool	fArray;

	strcpy(buff,name);
	if		(  ( p = strchr(buff,':')  )  !=  NULL  ) {
		*p = 0;
		rname = buff;
		vname = p + 1;
	} else {
		rname = "";
		vname = buff;
	}
	if		(  ( p = strchr(vname,':')  )  !=  NULL  ) {
		*p = 0;
		count = atoi(p+1);
		fArray = TRUE;
	} else {
		fArray = FALSE;
		count = 0;
	}

	p = NextName(ctx);
	q = p;
	if		(  (char *)p  !=  ctx->longname  ) {
		*p++ = '.';
	}
	if		(  fArray  ) {
		sprintf(p,"%s[%d]",(char *)vname,count);
	} else {
		sprintf(p,"%s",(char *)vname);
	}
#ifdef	DEBUG
	printf("startElement_(%s)[%s]\n",(char *)name,ctx->longname);
#endif
	if		(  stricmp((char *)rname,ctx->opt->recname)  ==  0  ) {
		if		(  stricmp((char *)vname,"data")  ==  0  ) {
			*ctx->longname = 0;
			ctx->value = NULL;
			ctx->fStart = FALSE;
		} else {
			type = (char *)GetAttribute(atts,(xmlChar *)"type");
			if		(  stricmp(type,"record")  ==  0  ) {
				if		(  ctx->value  ==  NULL  ) {
					*q = 0;
				}
				ctx->value = GetItemLongName(ctx->root,ctx->longname);
				ctx->fStart = FALSE;
			} else
			if		(  stricmp(type,"array")  ==  0  ) {
				*q = 0;
				ctx->fStart = FALSE;
			} else {
				ctx->value = GetItemLongName(ctx->root,ctx->longname);
				ctx->fStart = TRUE;
			}
		}
	} else {
		*q = 0;
		ctx->fStart = FALSE;
	}
}

static	void
endElement2_(
	ValueContext	*ctx,
	xmlChar	*name)
{
	xmlChar	*vname
	,		*rname;
	xmlChar	*p
	,		*q;
	xmlChar	buff[SIZE_LONGNAME+1];

#ifdef	DEBUG
	printf("endElement_(%s)\n",(char *)name);
#endif
	strcpy(buff,name);
	if		(  ( p = strchr(buff,':')  )  !=  NULL  ) {
		*p = 0;
		rname = buff;
		vname = p + 1;
	} else {
		rname = "";
		vname = buff;
	}
	if		(  ( p = strchr(vname,':')  )  !=  NULL  ) {
		*p = 0;
	}

	if		(  ( p = strrchr(ctx->longname,'.') )  !=  NULL  ) {
		if		(  ( q = strchr(p+1,'[')  )  !=  NULL  ) {
			*q = 0;
		}
		if		(  strcmp(p+1,vname)  ==  0  ) {
			*p = 0;
		}
		if		(  q  !=  NULL  ) {
			*q = '[';
		}
	} else {
		*ctx->longname = 0;
	}
	ctx->fStart = FALSE;
}

static	xmlSAXHandler mondaiSAXHandlerStruct2 = {
	(internalSubsetSAXFunc)internalSubset_,
	(isStandaloneSAXFunc)isStandalone_,
	(hasInternalSubsetSAXFunc)hasInternalSubset_,
	(hasExternalSubsetSAXFunc)hasExternalSubset_,
	(resolveEntitySAXFunc)resolveEntity_,
	(getEntitySAXFunc)getEntity_,
	(entityDeclSAXFunc)entityDecl_,
	(notationDeclSAXFunc)notationDecl_,
	(attributeDeclSAXFunc)attributeDecl_,
	(elementDeclSAXFunc)elementDecl_,
	(unparsedEntityDeclSAXFunc)unparsedEntityDecl_,
	(setDocumentLocatorSAXFunc)setDocumentLocator_,
	(startDocumentSAXFunc)startDocument_,
	(endDocumentSAXFunc)endDocument_,
	(startElementSAXFunc)startElement2_,
	(endElementSAXFunc)endElement2_,
	(referenceSAXFunc)reference_,
	(charactersSAXFunc)characters_,
	(ignorableWhitespaceSAXFunc)ignorableWhitespace_,
	(processingInstructionSAXFunc)processingInstruction_,
	(commentSAXFunc)comment_,
	(warningSAXFunc)warning_,
	(errorSAXFunc)error_,
	(fatalErrorSAXFunc)fatalError_,
	(getParameterEntitySAXFunc)getParameterEntity_,
	(cdataBlockSAXFunc)cdataBlock_,
#ifdef	USE_XML2
	(externalSubsetSAXFunc)externalSubset_,
    1
#endif
};

static	xmlSAXHandlerPtr	mondaiSAXHandler1 = &mondaiSAXHandlerStruct1;
static	xmlSAXHandlerPtr	mondaiSAXHandler2 = &mondaiSAXHandlerStruct2;

static	void
SetNil(
	ValueStruct	*val)
{
	int		i;

	switch	(ValueType(val)) {
	  case	GL_TYPE_NUMBER:
	  case	GL_TYPE_BYTE:
	  case	GL_TYPE_CHAR:
	  case	GL_TYPE_VARCHAR:
	  case	GL_TYPE_DBCODE:
	  case	GL_TYPE_INT:
	  case	GL_TYPE_BOOL:
	  case	GL_TYPE_TEXT:
	  case	GL_TYPE_OBJECT:
		val->attr |= GL_ATTR_NIL;
		break;
	  case	GL_TYPE_ARRAY:
		for	( i = 0 ; i < ValueArraySize(val) ; i ++ ) {
			SetNil(ValueArrayItem(val,i));
		}
		break;
	  case	GL_TYPE_RECORD:
		for	( i = 0 ; i < ValueRecordSize(val) ; i ++ ) {
			SetNil(ValueRecordItem(val,i));
		}
		break;
	  default:
		break;
	}
}

extern	byte	*
XML_UnPackValue(
	CONVOPT		*opt,
	byte		*p,
	ValueStruct	*value)
{
	ValueContext	*ctx;

ENTER_FUNC;
	ctx = New(ValueContext);
	ctx->root = value;
	ctx->buff = (char *)xmalloc(1);
	ctx->size = 1;
	ctx->fStart = FALSE;
	ctx->opt = opt;
	memset(ctx->longname,0,SIZE_LONGNAME+1);
	SetNil(value);
	switch	(ConvXmlType(opt)) {
	  case	XML_TYPE1:
		xmlSAXUserParseMemory(mondaiSAXHandler1,ctx,p,strlen(p));
		break;
	  case	XML_TYPE2:
		xmlSAXUserParseMemory(mondaiSAXHandler2,ctx,p,strlen(p));
		break;
	}
    xmlCleanupParser();
	xfree(ctx->buff);
	xfree(ctx);
LEAVE_FUNC;
	return	(p);
}

static	size_t
_XML_SizeValue1(
	CONVOPT		*opt,
	char		*name,
	ValueStruct	*value,
	char		*buff)
{
	char	num[SIZE_NAME+1];
	int		i;
	size_t	size;

	size = 0;
	if		(  value  !=  NULL  ) {
		if		(  IS_VALUE_NIL(value)  )	return	(0);
		nIndent ++;
		if		(  ConvIndent(opt)  ) {
			size += nIndent;
		}
		switch	(ValueType(value)) {
		  case	GL_TYPE_ARRAY:
			size += sprintf(buff,"<lm:array name=\"%s\" count=\"%d\">"
							,name,ValueArraySize(value));
			size += PutCR(opt,buff);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				sprintf(num,"%s[%d]",name,i);
				size += _XML_SizeValue1(opt,num,ValueArrayItem(value,i),buff);
			}
			if		(  ConvIndent(opt)  ) {
				size += nIndent;
			}
			size += 11;		//	</lm:array>
			break;
		  case	GL_TYPE_RECORD:
			size += sprintf(buff,"<lm:record name=\"%s\" size=\"%d\">"
							,name,ValueRecordSize(value));
			size += PutCR(opt,buff);
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				size += _XML_SizeValue1(opt,ValueRecordName(value,i),ValueRecordItem(value,i),buff);
			}
			if		(  ConvIndent(opt)  ) {
				size += nIndent;
			}
			size += 12;		//	</lm:record>
			break;
		  default:
			size += sprintf(buff,"<lm:item name=\"%s\"",name);
			if		(  ConvType(opt)  ) {
				size += 6;		//	" type="
				switch	(ValueType(value)) {
				  case	GL_TYPE_INT:
					size += 5;			//	"int"
					break;
				  case	GL_TYPE_BOOL:
					size += 6;			//	"bool"
					break;
				  case	GL_TYPE_BYTE:
					size += sprintf(buff,
									"\"byte\" size=\"%d\"",ValueStringLength(value));
					break;
				  case	GL_TYPE_CHAR:
					size += sprintf(buff,
									"\"char\" size=\"%d\"",ValueStringLength(value));
					break;
				  case	GL_TYPE_VARCHAR:
					size += sprintf(buff,
									"\"varchar\" size=\"%d\"",ValueStringLength(value));
					break;
				  case	GL_TYPE_TEXT:
					size += sprintf(buff,
									"\"text\" size=\"%d\"",ValueStringLength(value));
					break;
				  case	GL_TYPE_DBCODE:
					size += sprintf(buff,
									"\"dbcode\" size=\"%d\"",ValueStringLength(value));
					break;
				  case	GL_TYPE_NUMBER:
					size += sprintf(buff,"\"number\" size=\"%d\" ssize=\"%d\"",
									ValueFixedLength(value),ValueFixedSlen(value));
					break;
				  case	GL_TYPE_FLOAT:
					size += 7;			//	"float"
					break;
				  case	GL_TYPE_ALIAS:
				  default:
					break;
				}
			}
			if		(  ConvIndent(opt)  ) {
				size += 1;		//	">"
			} else {
				size += 3;		//	" />"
			}
			if		(  !IS_VALUE_NIL(value)  ) {
#ifdef	USE_XML2
				size += sprintf(buff,"%s",XML_Encode(ValueToString(value,opt->coding),buff));
#else
				size += sprintf(buff,"%s",XML_Encode(ValueToString(value,LIBXML_CODE),buff));
#endif
			}
			if		(  ConvIndent(opt)  ) {
				size += 10;		//	"</lm:item>"
			}
			break;
		}
		size += PutCR(opt,buff);
		nIndent --;
	}
	return	(size);
}

static	size_t
_XML_SizeValue2(
	CONVOPT		*opt,
	char		*name,
	ValueStruct	*value,
	char		*buff)
{
	char	num[SIZE_NAME+1];
	int		i;
	size_t	size;

	size = 0;
	if		(  value  !=  NULL  ) {
		if		(  IS_VALUE_NIL(value)  )	return	(0);
		nIndent ++;
		if		(  ConvIndent(opt)  ) {
			size += nIndent;
		}
		switch	(ValueType(value)) {
		  case	GL_TYPE_ARRAY:
			size += sprintf(buff,"<%s:%s type=\"array\" count=\"%d\">"
							,opt->recname,name,ValueArraySize(value));
			size += PutCR(opt,buff);
			for	( i = 0 ; i < ValueArraySize(value) ; i ++ ) {
				sprintf(num,"%s:%d",name,i);
				size += _XML_SizeValue2(opt,num,ValueArrayItem(value,i),buff);
			}
			if		(  ConvIndent(opt)  ) {
				size += nIndent;
			}
			size += sprintf(buff,"</%s:%s>",opt->recname,name);
			break;
		  case	GL_TYPE_RECORD:
			size += sprintf(buff,"<%s:%s type=\"record\" size=\"%d\">"
							,opt->recname,name,ValueRecordSize(value));
			size += PutCR(opt,buff);
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				size += _XML_SizeValue2(opt,ValueRecordName(value,i),ValueRecordItem(value,i),buff);
			}
			if		(  ConvIndent(opt)  ) {
				size += nIndent;
			}
			size += sprintf(buff,"</%s:%s>",opt->recname,name);
			break;
		  default:
			size += sprintf(buff,"<%s:%s",opt->recname,name);
			size += 6;		//	" type="
			switch	(ValueType(value)) {
			  case	GL_TYPE_INT:
				size += 5;			//	"int"
				break;
			  case	GL_TYPE_BOOL:
				size += 6;			//	"bool"
				break;
			  case	GL_TYPE_BYTE:
				size += sprintf(buff,
								"\"byte\" size=\"%d\"",ValueStringLength(value));
				break;
			  case	GL_TYPE_CHAR:
				size += sprintf(buff,
								"\"char\" size=\"%d\"",ValueStringLength(value));
				break;
			  case	GL_TYPE_VARCHAR:
				size += sprintf(buff,
								"\"varchar\" size=\"%d\"",ValueStringLength(value));
				break;
			  case	GL_TYPE_TEXT:
				size += sprintf(buff,
								"\"text\" size=\"%d\"",ValueStringLength(value));
				break;
			  case	GL_TYPE_DBCODE:
				size += sprintf(buff,
								"\"dbcode\" size=\"%d\"",ValueStringLength(value));
				break;
			  case	GL_TYPE_NUMBER:
				size += sprintf(buff,"\"number\" size=\"%d\" ssize=\"%d\"",
								ValueFixedLength(value),ValueFixedSlen(value));
				break;
			  case	GL_TYPE_FLOAT:
				size += 7;			//	"float"
				break;
			  case	GL_TYPE_ALIAS:
			  default:
				break;
			}
			if		(  ConvIndent(opt)  ) {
				size += 1;		//	">"
			} else {
				size += 3;		//	" />"
			}
			if		(  !IS_VALUE_NIL(value)  ) {
#ifdef	USE_XML2
				size += sprintf(buff,"%s",XML_Encode(ValueToString(value,opt->coding),buff));
#else
				size += sprintf(buff,"%s",XML_Encode(ValueToString(value,LIBXML_CODE),buff));
#endif
			}
			if		(  ConvIndent(opt)  ) {
				size += sprintf(buff,"</%s:%s>",opt->recname,name);
			}
			break;
		}
		size += PutCR(opt,buff);
		nIndent --;
	}
	return	(size);
}

extern	size_t
XML_SizeValue(
	CONVOPT		*opt,
	ValueStruct	*value)
{
	char	buff[SIZE_BUFF+1];
	size_t	size;

	nIndent = 0;
	size = 19;			//	<?xml version="1.0"
#ifdef	USE_XML2
	if		(  opt->coding  !=  NULL  ) {
		size += 12 + strlen(opt->coding);	
		//	" encoding=\"%s\"",opt->coding
	}
#else
	size += 12 + strlen(LIBXML_CODE);
#endif
	size += 2;			//	?>
	size += PutCR(opt,buff);
	switch	(ConvXmlType(opt)) {
	  case	XML_TYPE1:
		size += sprintf(buff,"<lm:block xmlns:lm=\"%s\">",NS_URI);
		size += PutCR(opt,buff);
		size += _XML_SizeValue1(opt,opt->recname,value,buff);
		size += 11;			//	</lm:block>
		break;
	  case	XML_TYPE2:
		size += sprintf(buff,"<%s:data xmlns:%s=\"%s/%s.rec\">"
						,opt->recname,opt->recname,NS_URI,opt->recname);
		size += PutCR(opt,buff);
		size += _XML_SizeValue2(opt,opt->recname,value,buff);
		size += sprintf(buff,"</%s:data>",opt->recname);
		break;
	}
	return	(size);
}

#endif	/*	USE_XML	*/
