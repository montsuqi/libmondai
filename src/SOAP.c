/*
libmondai -- MONTSUQI data access library
Copyright (C) 2005 Ogochan.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

/*
#define	DEBUG
#define	TRACE
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#ifdef	USE_SOAP

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
#include	"misc_v.h"
#include	"hash_v.h"
#include	"value.h"
#include	"getset.h"
#include	"memory_v.h"
#include	"monstring.h"
#include	"others.h"
#include	"SOAP_v.h"
#include	"debug.h"

#define	XSI_URL			"http://www.w3.org/1999/XMLSchema-instance"
#define	SOAP_ENV		"http://schemas.xmlsoap.org/soap/envelope/"
#define	SOAP_ENC		"http://schemas.xmlsoap.org/soap/encoding/"

#define	XMLNodeType(cur)			((cur)->type)
#define	XMLNodeNext(cur)			((cur)->next)
#define	XMLNodeChildren(cur)		((cur)->children)
#define	XMLNodeContent(cur)			((cur)->content)
#define	XMLGetProp(tree,name)		xmlGetProp((tree),(name))
#define	XMLNs(cur)					((cur)->ns)
#define	XMLNsBody(cur)				(char *)((cur)->ns->href)
#define	XMLNsPrefix(cur)			((cur)->ns->prefix)
#define	XMLName(cur)				(char *)((cur)->name)
#define	XMLGetProp(tree,name)		xmlGetProp((tree),(name))
#define	XMLGetNsProp(tree,name,ns)	xmlGetNsProp((tree),(name),(ns))

#ifdef	DEBUG
static	int		Depth;
static	void
DumpIndentLine(void)
{
	int		i;

	for	( i = 0 ; i < Depth ; i ++ )	{
		printf(" ");
	}
}

static void	DumpNodes(xmlNode *node);
static void
DumpNode(xmlNode *node)
{
	xmlChar	*text;

	switch	(node->type)	{
	  case	XML_ELEMENT_NODE:
		DumpIndentLine();
		printf("node type: Element, name: %s\n", node->name);
		break;
	  case	XML_TEXT_NODE:
		if		(  ( text = XMLNodeContent(node) )  !=  NULL  ) {
			DumpIndentLine();
			printf("text: %s\n", text);
		}
	  default:
		break;
	}

	DumpNodes(node->children);
}

static void
DumpNodes(xmlNode	*node)
{
    xmlNode *cur_node;

	Depth ++;
    for (cur_node = node ; cur_node != NULL ; cur_node = cur_node->next) {
		DumpNode(cur_node);
    }
	Depth --;
}

static	void
DumpIdTable(
	GHashTable	*ids)
{
	void	Dump(
		char		*id,
		xmlNode		*node)
		{
			printf("id = [%s]\n",id);
			DumpNode(node);
		}

	printf("*** (id dump) ***\n");
	Depth = 0;
	g_hash_table_foreach(ids,(GHFunc)Dump,NULL);
	printf("*** (id dump end) ***\n");
}
#endif

static void
MakeIdTable(
	GHashTable	*ids,
	xmlNode * a_node)
{
    xmlNode *node;
	xmlChar	*id;

    for (node = a_node ; node != NULL ; node = node->next) {
        if		(  node->type  ==  XML_ELEMENT_NODE  ) {
			if		(  ( id = XMLGetProp(node,"id") )  !=  NULL  ) {
				if		(  g_hash_table_lookup(ids,id)  ==  NULL  ) {
					g_hash_table_insert(ids,id,node);
				}
			}
        }
        MakeIdTable(ids,node->children);
    }
}

static	xmlNodePtr
SearchNode(
	xmlNodePtr	tree,
	char		*ns,
	char		*tag,
	char		*prop,
	char		*pvalue)
{
	char	*thisns;
	char	*thisnn;
	xmlNodePtr	ret;

ENTER_FUNC;
	ret = NULL;
	while	(  tree  !=  NULL  ) {
		switch	(XMLNodeType(tree)) {
		  case	XML_ELEMENT_NODE:
			if		(  XMLNs(tree)  !=  NULL  ) {
				thisns = XMLNsBody(tree);
			} else {
				thisns = "";
			}
			if		(	(  tag  ==  NULL  )
					&&	(  ns   ==  NULL  )	) {
				if		(	(  ( thisnn = XMLGetProp(tree,prop) )  !=  NULL  )
						&&	(  !strcmp(thisnn,pvalue)  ) ) {
					ret = tree;
				} else {
					ret = SearchNode(XMLNodeChildren(tree),ns,tag,prop,pvalue);
				}
			} else
			if		(	(  pvalue  ==  NULL  )
					&&	(  !strcmp(thisns,ns)  )
					&&	(  !strcmp(XMLName(tree),tag)  ) ) {
				ret = tree;
			} else
			if		(	(  !strcmp(thisns,ns)  )
					&&	(  !strcmp(XMLName(tree),tag)  )
					&&	(  ( thisnn = XMLGetProp(tree,prop) )  !=  NULL  )
					&&	(  !strcmp(thisnn,pvalue)  ) ) {
				ret = tree;
			} else
			if		(  XMLNodeChildren(tree)  !=  NULL  ) {
				ret = SearchNode(XMLNodeChildren(tree),ns,tag,prop,pvalue);
			}
			break;
		  case	XML_TEXT_NODE:
			break;
		  default:
			break;
		}
		if		(  ret  !=  NULL  )	break;
		tree = XMLNodeNext(tree);
	}
LEAVE_FUNC;
	return	(ret);
}

static	void
_SOAP_UnPackValue(
	ValueStruct	*value,
	GHashTable	*ids,
	xmlNode		*node)
{
	int		i;
	char	*href
		,	*xsi_null;
	byte	*buff;
	xmlChar	*text;
	xmlNode		*child
		,		*rnode;
	size_t	size;

ENTER_FUNC;
	if		(  value  !=  NULL  ) {
		if		(	(  ( xsi_null = XMLGetNsProp(node,"null",XSI_URL) )  !=  NULL  )
				&&	(  strcmp(xsi_null,"true")  ==  0  ) ) {
			ValueIsNil(value);
		} else {
			ValueIsNonNil(value);
			switch	(ValueType(value)) {
			  case	GL_TYPE_INT:
			  case	GL_TYPE_BOOL:
			  case	GL_TYPE_FLOAT:
			  case	GL_TYPE_NUMBER:
			  case	GL_TYPE_CHAR:
			  case	GL_TYPE_VARCHAR:
			  case	GL_TYPE_DBCODE:
			  case	GL_TYPE_TEXT:
			  case	GL_TYPE_SYMBOL:
				if		(	(  XMLNodeType(node)  ==      XML_ELEMENT_NODE  )
						&&	(  ( child = XMLNodeChildren(node) )  !=  NULL  )
						&&	(  XMLNodeType(child)  ==     XML_TEXT_NODE     )
						&&	(  ( text = XMLNodeContent(child) )   !=  NULL  ) ) {
					SetValueString(value,text,"utf-8");
				}
				break;
			  case	GL_TYPE_BYTE:
				if		(	(  XMLNodeType(node)  ==      XML_ELEMENT_NODE  )
						&&	(  ( child = XMLNodeChildren(node) )  !=  NULL  )
						&&	(  XMLNodeType(child)  ==     XML_TEXT_NODE     )
						&&	(  ( text = XMLNodeContent(child) )   !=  NULL  ) ) {
					DecodeBase64(ValueByte(value),ValueByteLength(value),
								 text,strlen(text));
				}
				break;
			  case	GL_TYPE_BINARY:
				if		(	(  XMLNodeType(node)  ==      XML_ELEMENT_NODE  )
						&&	(  ( child = XMLNodeChildren(node) )  !=  NULL  )
						&&	(  XMLNodeType(child)  ==     XML_TEXT_NODE     )
						&&	(  ( text = XMLNodeContent(child) )   !=  NULL  ) ) {
					buff = (byte *)xmalloc(strlen(text));
					size = DecodeBase64(buff,strlen(text),text,strlen(text));
					if		(  size  >  ValueByteSize(value)  ) {
						if		(  ValueByte(value)  !=  NULL  ) {
							xfree(ValueByte(value));
						}
						ValueByteSize(value) = size;
						ValueByte(value) = (char *)xmalloc(size);
					}
					memclear(ValueByte(value),ValueByteSize(value));
					memcpy(ValueByte(value),buff,size);
					xfree(buff);
				}
				break;
			  case	GL_TYPE_ARRAY:
				dbgprintf("nspre= [%s]\n",XMLNsPrefix(node));
				dbgprintf("tag  = [%s]\n",XMLName(node));
				child = XMLNodeChildren(node);
				for	( i = 0 ; i < ValueArraySize(value) ; ) {
					if		(  child  ==  NULL  )	break;
					if		(  XMLNs(child)  !=  NULL  ) {
						dbgprintf("ns   = [%s]\n",XMLNsBody(child));
						dbgprintf("nspre= [%s]\n",XMLNsPrefix(child));
					} else {
						if		(  ( href = XMLGetProp(child,"href") )  !=  NULL  ) {
							if		(	(  *href  ==  '#'  )
									&&	(  ( rnode = g_hash_table_lookup(ids,href+1) )
										   !=  NULL  ) ) {
								_SOAP_UnPackValue(ValueArrayItem(value,i),ids,rnode);
							}
						} else {
							_SOAP_UnPackValue(ValueArrayItem(value,i),ids,child);
						}
						i ++;
					}
					child = XMLNodeNext(child);
				}
				break;
			  case	GL_TYPE_RECORD:
				child = XMLNodeChildren(node);
				for	( i = 0 ; i < ValueRecordSize(value) ; ) {
					if		(  child  ==  NULL  )	break;
					dbgprintf("name = [%s]\n",ValueRecordName(value,i));
					dbgprintf("tag  = [%s]\n",XMLName(child));
					if		(  XMLNs(child)  !=  NULL  ) {
						dbgprintf("ns   = [%p]\n",XMLNsBody(child));
						dbgprintf("nspre= [%p]\n",XMLNsPrefix(child));
					} else {
						if		(  strcmp(ValueRecordName(value,i),XMLName(child))  ==  0  ) {
							if		(  ( href = XMLGetProp(child,"href") )  !=  NULL  ) {
								if		(	(  *href  ==  '#'  )
										&&	(  ( rnode = g_hash_table_lookup(ids,href+1) )
											   !=  NULL  ) ) {
									_SOAP_UnPackValue(ValueRecordItem(value,i),ids,rnode);
								}
							} else {
								_SOAP_UnPackValue(ValueRecordItem(value,i),ids,child);
							}
						}
						i ++;
					}
					child = XMLNodeNext(child);
				}
				break;
			  case	GL_TYPE_OBJECT:
				break;
			  default:
				ValueIsNil(value);
				break;
			}
		}
	}
LEAVE_FUNC;
}

extern	void
SOAP_UnPackValue(
	ValueStruct	*val,
	char		*data,
	char		*method)
{
	xmlDocPtr	doc;
	xmlNode		*body
		,		*top;
	GHashTable	*ids;

ENTER_FUNC;
    if		(  ( doc = xmlReadMemory(data,strlen(data), "", NULL, XML_PARSE_NOBLANKS) )  ==  NULL  ) {
		exit(1);
	}
#ifdef	DEBUG
	Depth = 0;
	DumpNodes(xmlDocGetRootElement(doc));
#endif

	ids = NewNameHash();
	MakeIdTable(ids,xmlDocGetRootElement(doc));
#ifdef	DEBUG
	DumpIdTable(ids);
#endif

	body = SearchNode(xmlDocGetRootElement(doc),SOAP_ENV,"Body",NULL,NULL);

	if		(  body  ==  NULL  ) {
		fprintf(stderr,"body not found\n");
	} else {
		if		(  ( top = XMLNodeChildren(body) )  !=  NULL  ) {
			strcpy(method,XMLName(top));
			_SOAP_UnPackValue(val,ids,top);
		}
	}

    xmlFreeDoc(doc);
	g_hash_table_destroy(ids);
    xmlCleanupParser();
LEAVE_FUNC;
}

static	ValueStruct	*
_SOAP_LoadValue(
	ValueStruct	*upper,
	GHashTable	*ids,
	xmlNode		*node)
{
	char		*href
		,		*tname
		,		*xsi_null
		,		*text;
	xmlNode		*rnode
		,		*child;
	ValueStruct	*value
		,		*elem;
	Bool		fBin;
	byte		*buff;
	size_t		size;

ENTER_FUNC;
	if		(  node  !=  NULL  ) {
		dbgprintf("name  = [%s]\n",XMLName(node));
		if		( ( tname = XMLGetNsProp(node,"type",XSI_URL) )  !=  NULL  ) {
			dbgprintf("tname = [%s]\n",tname);
			fBin = FALSE;
			if		(  strcmp(tname,"SOAP-ENC:Array")  ==  0  ) {
				value = NewValue(GL_TYPE_ARRAY);
				ValueIsNonNil(value);
				node = XMLNodeChildren(node);
				while	(  node  !=  NULL  ) {
					if		(  ( elem = _SOAP_LoadValue(value,ids,node) )  !=  NULL  ) {
						ValueAddArrayItem(value,-1,elem);
					}
					node = XMLNodeNext(node);
				}
			} else {
				if		(  strcmp(tname,"SOAP-ENC:int")  ==  0  ) {
					value = NewValue(GL_TYPE_INT);
				} else
				if		(  strcmp(tname,"SOAP-ENC:boolean")  ==  0  ) {
					value = NewValue(GL_TYPE_BOOL);
				} else
				if		(  strcmp(tname,"SOAP-ENC:base64")  ==  0  ) {
					value = NewValue(GL_TYPE_BINARY);
					fBin = TRUE;
				} else {
					value = NewValue(GL_TYPE_TEXT);
				}
				if		(  !fBin  ) {
					if		(	(  XMLNodeType(node)  ==      XML_ELEMENT_NODE  )
							&&	(  ( child = XMLNodeChildren(node) )  !=  NULL  )
							&&	(  XMLNodeType(child)  ==     XML_TEXT_NODE     )
							&&	(  ( text = XMLNodeContent(child) )   !=  NULL  ) ) {
						SetValueString(value,text,"utf-8");
					}
				} else {
					if		(	(  XMLNodeType(node)  ==      XML_ELEMENT_NODE  )
							&&	(  ( child = XMLNodeChildren(node) )  !=  NULL  )
							&&	(  XMLNodeType(child)  ==     XML_TEXT_NODE     )
							&&	(  ( text = XMLNodeContent(child) )   !=  NULL  ) ) {
						buff = (byte *)xmalloc(strlen(text));
						size = DecodeBase64(buff,strlen(text),text,strlen(text));
						ValueByteSize(value) = size;
						ValueByteLength(value) = size;
						ValueByte(value) = (char *)xmalloc(size);
						memclear(ValueByte(value),ValueByteSize(value));
						memcpy(ValueByte(value),buff,size);
						xfree(buff);
					}
				}
				if		(	(  ( xsi_null = XMLGetNsProp(node,"null",XSI_URL) )  !=  NULL  )
						&&	(  strcmp(xsi_null,"true")  ==  0  ) ) {
					ValueIsNil(value);
				} else {
					ValueIsNonNil(value);
				}
			}
		} else
		if		(  ( href = XMLGetProp(node,"href") )  !=  NULL  ) {
			dbgprintf("href  = [%s]\n",href);
			if		(	(  *href  ==  '#'  )
					&&	(  ( rnode = g_hash_table_lookup(ids,href+1) )  !=  NULL  ) ) {
				value = _SOAP_LoadValue(upper,ids,rnode);
			} else {
				value = NULL;
			}
		} else {
			value = NewValue(GL_TYPE_RECORD);
			ValueIsNonNil(value);
			node = XMLNodeChildren(node);
			while	(  node  !=  NULL  ) {
				if		(  ( elem = _SOAP_LoadValue(value,ids,node) )  !=  NULL  ) {
					ValueAddRecordItem(value,XMLName(node),elem);
				}
				node = XMLNodeNext(node);
			}
		}
	} else {
		value = NULL;
	}
LEAVE_FUNC;
	return	(value);
}

extern	ValueStruct	*
SOAP_LoadValue(
	char		*data,
	char		*method)
{
	xmlDocPtr	doc;
	xmlNode		*body
		,		*top
		,		*node;
	GHashTable	*ids;
	ValueStruct	*val
		,		*elem;

ENTER_FUNC;
    if		(  ( doc = xmlReadMemory(data,strlen(data), "", NULL, XML_PARSE_NOBLANKS) )  ==  NULL  ) {
		exit(1);
	}
#ifdef	DEBUG
	Depth = 0;
	DumpNodes(xmlDocGetRootElement(doc));
#endif

	ids = NewNameHash();
	MakeIdTable(ids,xmlDocGetRootElement(doc));
#ifdef	DEBUG
	DumpIdTable(ids);
#endif

	body = SearchNode(xmlDocGetRootElement(doc),SOAP_ENV,"Body",NULL,NULL);

	if		(  body  ==  NULL  ) {
		fprintf(stderr,"body not found\n");
		val = NULL;
	} else {
		if		(  ( top = XMLNodeChildren(body) )  !=  NULL  ) {
			val = NewValue(GL_TYPE_RECORD);
			strcpy(method,XMLName(top));
			node = XMLNodeChildren(top);
			while	(  node  !=  NULL  ) {
				if		(  ( elem = _SOAP_LoadValue(val,ids,node) )  !=  NULL  ) {
					ValueAddRecordItem(val,XMLName(node),elem);
				}
				node = XMLNodeNext(node);
			}
		} else {
			val = NULL;
		}
	}

    xmlFreeDoc(doc);
	g_hash_table_destroy(ids);
    xmlCleanupParser();
LEAVE_FUNC;
	return	(val);
}


static	size_t
_SOAP_PackValue(
	CONVOPT		*opt,
	byte		*p,
	char		*ns,
	char		*name,
	char		*id,
	ValueStruct	*value,
	char		*path)
{
	ValueStruct	*ival;
	int		i;
	byte	*pp;
	Bool	fOut;
	char	ibuff[SIZE_LONGNAME+1];

ENTER_FUNC;
	pp = p;
	if		(  value  !=  NULL  ) {
		switch	(ValueType(value)) {
		  case	GL_TYPE_ARRAY:
			p += IndentLine(opt,p);
			if		(  ns  !=  NULL  ) {
				p += sprintf(p,"<%s:%s",ns,name);
			} else {
				p += sprintf(p,"<%s",name);
			}
			if		(  id  !=  NULL  ) {
				p += sprintf(p," id=\"%s\"",id);
			}
			p += sprintf(p," xsi:type=\"SOAP-ENC:Array\">");
			p += PutCR(opt,p);
			opt->nIndent ++;
			fOut = FALSE;
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				ival = ValueArrayItem(value,i);
				snprintf(ibuff,SIZE_LONGNAME,"%s.%d",path,i);
				p += _SOAP_PackValue(opt,p,
									 ns,name,
									 ibuff,ival,ibuff);
			}
			opt->nIndent --;
			p += IndentLine(opt,p);
			if		(  ns  !=  NULL  ) {
				p += sprintf(p,"</%s:%s>",ns,name);
			} else {
				p += sprintf(p,"</%s>",name);
			}
			p += PutCR(opt,p);
			break;
		  case	GL_TYPE_RECORD:
			p += IndentLine(opt,p);
			if		(  ns  !=  NULL  ) {
				p += sprintf(p,"<%s:%s",ns,name);
			} else {
				p += sprintf(p,"<%s",name);
			}
			if		(  id  !=  NULL  ) {
				p += sprintf(p," id=\"%s\"",id);
			}
			p += sprintf(p,">");
			p += PutCR(opt,p);

			fOut = FALSE;
			opt->nIndent ++;
			for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
				ival = ValueRecordItem(value,i);
				if		(	(  IS_VALUE_STRUCTURE(ival)  )
						||	(  IS_VALUE_BITS(ival)       ) ) {
					p += IndentLine(opt,p);
					p += sprintf(p,"<%s",ValueRecordName(value,i));
					if		(  ValueType(ival)  ==  GL_TYPE_ALIAS  ) {
						p += sprintf(p," href=\"#%s.%s\"/>",path,
									 ValueAliasName(ival));
					} else
					if		(  ValueType(ival)  ==  GL_TYPE_ARRAY  ) {
						p += sprintf(p," href=\"#%s.%d\"/>",path,i);
					} else {
						p += sprintf(p," href=\"#%s.%s\"/>",path,ValueRecordName(value,i));
					}
					p += PutCR(opt,p);
					fOut = TRUE;
				} else {
					snprintf(ibuff,SIZE_LONGNAME,"%s.%s",path,ValueRecordName(value,i));
					p += _SOAP_PackValue(opt,p,
											NULL,ValueRecordName(value,i),
											ibuff,ival,path);
				}
			}
			opt->nIndent --;
			p += IndentLine(opt,p);
			if		(  ns  !=  NULL  ) {
				p += sprintf(p,"</%s:%s>",ns,name);
			} else {
				p += sprintf(p,"</%s>",name);
			}
			p += PutCR(opt,p);
			if		(  fOut  ) {
				for	( i = 0 ; i < ValueRecordSize(value) ; i ++ ) {
					ival = ValueRecordItem(value,i);
					if		(	(  IS_VALUE_STRUCTURE(ival)  )
							||	(  IS_VALUE_BITS(ival)       ) ) {
						if		(  ValueType(ival)  ==  GL_TYPE_ARRAY  ) {
							snprintf(ibuff,SIZE_LONGNAME,"%s.%d",path,i);
						} else {
							snprintf(ibuff,SIZE_LONGNAME,"%s.%s",path,
									 ValueRecordName(value,i));
						}
						p += _SOAP_PackValue(opt,p,
												ns,
												ValueRecordName(value,i),
												ibuff,
												ival,ibuff);
					} 
				}
			}

			break;
		  case	GL_TYPE_ALIAS:
			break;
		  default:
			p += IndentLine(opt,p);
			if		(  ns  !=  NULL  ) {
				p += sprintf(p,"<%s:%s",ns,name);
			} else {
				p += sprintf(p,"<%s",name);
			}
			if		(  id  !=  NULL  ) {
				p += sprintf(p," id=\"%s\"",id);
			}
			switch	(ValueType(value)) {
			  case	GL_TYPE_OBJECT:
			  case	GL_TYPE_INT:
				if		(  IS_VALUE_NIL(value)  ) {
					p += sprintf(p," xsi:type=\"SOAP-ENC:int\" xsi:null=\"true\"/>");
				} else {
					p += sprintf(p," xsi:type=\"SOAP-ENC:int\">");
					p += sprintf(p,"%s",ValueToString(value,"utf-8"));
				}
				break;
			  case	GL_TYPE_BOOL:
				if		(  IS_VALUE_NIL(value)  ) {
					p += sprintf(p," xsi:type=\"SOAP-ENC:boolean\" xsi:null=\"true\"/>");
				} else {
					p += sprintf(p," xsi:type=\"SOAP-ENC:boolean\">");
					p += sprintf(p,"%s",ValueToString(value,"utf-8"));
				}
				break;
			  case	GL_TYPE_FLOAT:
				if		(  IS_VALUE_NIL(value)  ) {
					p += sprintf(p," xsi:type=\"SOAP-ENC:double\" xsi:null=\"true\"/>");
				} else {
					p += sprintf(p," xsi:type=\"SOAP-ENC:double\">");
					p += sprintf(p,"%s",ValueToString(value,"utf-8"));
				}
				break;
			  case	GL_TYPE_NUMBER:
				if		(  IS_VALUE_NIL(value)  ) {
					p += sprintf(p," xsi:type=\"SOAP-ENC:decimal\" xsi:null=\"true\"/>");
				} else {
					p += sprintf(p," xsi:type=\"SOAP-ENC:decimal\">");
					p += sprintf(p,"%s",ValueToString(value,"utf-8"));
				}
				break;
			  case	GL_TYPE_CHAR:
			  case	GL_TYPE_TEXT:
			  case	GL_TYPE_SYMBOL:
			  case	GL_TYPE_VARCHAR:
			  case	GL_TYPE_DBCODE:
				if		(  IS_VALUE_NIL(value)  ) {
					p += sprintf(p," xsi:type=\"SOAP-ENC:string\" xsi:null=\"true\"/>");
				} else {
					p += sprintf(p," xsi:type=\"SOAP-ENC:string\">");
					p += sprintf(p,"%s",ValueToString(value,"utf-8"));
				}
				break;
			  case	GL_TYPE_BINARY:
			  case	GL_TYPE_BYTE:
				if		(  IS_VALUE_NIL(value)  ) {
					p += sprintf(p," xsi:type=\"SOAP-ENC:base64\" xsi:null=\"true\"/>");
				} else {
					p += sprintf(p," xsi:type=\"SOAP-ENC:base64\">");
					p += EncodeBase64(p,-1,ValueByte(value),ValueByteLength(value));
				}
			  default:
				break;
			}
			if		(  !IS_VALUE_NIL(value)  ) {
				if		(  ns  !=  NULL  ) {
					p += sprintf(p,"</%s:%s>",ns,name);
				} else {
					p += sprintf(p,"</%s>",name);
				}
			}
			p += PutCR(opt,p);
			break;
		}
	}
LEAVE_FUNC;
	return	(p-pp);
}

extern	size_t
SOAP_PackValue(
	byte		*p,
	ValueStruct	*value,
	char		*method,
	char		*prefix,
	char		*uri,
	Bool		fIndent,
	Bool		fBodyOnly)
{
	char	buff[SIZE_BUFF+1];
	byte	*pp;
	CONVOPT		*opt;

ENTER_FUNC;
	opt = NewConvOpt();

	ConvSetIndent(opt,fIndent);
	ConvSetCodeset(opt,"utf-8");

	pp = p;
	if		(  !fBodyOnly  ) {
		p += sprintf(p,"<SOAP-ENV:Envelope");
		p += PutCR(opt,p);
		p += sprintf(p,"  xmlns:xsi=\"%s\"",XSI_URL);
		p += PutCR(opt,p);
		p += sprintf(p,"  xmlns:SOAP-ENV=\"%s\"",SOAP_ENV);
		p += PutCR(opt,p);
		p += sprintf(p,"  xmlns:SOAP-ENC=\"%s\">",SOAP_ENC);
		p += PutCR(opt,p);
	}
	opt->nIndent = 1;
	p += IndentLine(opt,p);
	if		(  uri  !=  NULL  ) {
		p += sprintf(p,"<SOAP-ENV:Body xmlns:%s=\"%s\">",prefix,uri);
	} else {
		p += sprintf(p,"<SOAP-ENV:Body>");
	}
	p += PutCR(opt,p);
	opt->nIndent ++;
	strcpy(buff,prefix);
	p +=_SOAP_PackValue(opt,p,prefix,method,NULL,value,buff);
	opt->nIndent --;
	p += IndentLine(opt,p);
	p += sprintf(p,"</SOAP-ENV:Body>");
	p += PutCR(opt,p);
	if		(  !fBodyOnly  ) {
		p += sprintf(p,"</SOAP-ENV:Envelope>");
	}
	*p = 0;

	DestroyConvOptXML(opt);
LEAVE_FUNC;
	return	(p-pp);
}

#endif
