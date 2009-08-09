/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 1989-2009 Ogochan.
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
*	debug macros
*/

#ifndef	_INC_DEBUG_H
#define	_INC_DEBUG_H
#include	<stdio.h>
#include	<stdarg.h>

#ifdef	TRACE
#define	dbgmsg(s)			MessageDebug(s)
#define	dbgprintf(fmt,...)	MessageDebugPrintf((fmt), __VA_ARGS__)
#define	PASS(s)				MessageDebug(s)
#define	ENTER_FUNC			MessageDebugPrintf(">%s", __func__)
#define	LEAVE_FUNC			MessageDebugPrintf("<%s", __func__)
#define	RETURN(v)			MessageDebugPrintf("<%s", __func__),return(v)
#else
#define	dbgmsg(s)			/*	*/
#define	dbgprintf(fmt,...)	/*	*/
#define	PASS(s)				/*	*/
#define	ENTER_FUNC			/*	*/
#define	LEAVE_FUNC			/*	*/
#define	RETURN(v)			return(v)
#endif

#define	EXIT(c)	{ printf("exit at %s(%d) %s\n",__FILE__,__LINE__, __func__);exit(c);}

#ifdef	_INC_MESSAGE_H
#define	Error(...)                                                      \
do {                                                                    \
    _MessageLevelPrintf(MESSAGE_ERROR,__FILE__,__LINE__,__VA_ARGS__);   \
    exit(1);                                                            \
} while (0)
#define	Warning(...)                                                \
_MessageLevelPrintf(MESSAGE_WARN,__FILE__,__LINE__,__VA_ARGS__);
#define	Message(...)                                  \
_MessageLevelPrintf(MESSAGE_PRINT,__FILE__,__LINE__,__VA_ARGS__);
#else

#if	( defined(_WIN32) || defined(_WIN64) ) && !defined(CONSOLE_DEBUG)
extern	void	debug_printf(const _TCHAR *fmt, ...);
extern	void	debug_line_start(void);
extern	void	debug_line_flush(void);

#define	LINE_START		debug_line_start()
#define	LINE_FEED_OUT	debug_line_flush()
#else
#define	LINE_START		/*	*/
#define	debug_printf	printf
#define	LINE_FEED_OUT	printf("\n");fflush(stdout)
#endif

#define	Error(...)								\
do {											\
	LINE_START;									\
	debug_printf("E:%s:%d:",__FILE__,__LINE__);	\
	debug_printf(__VA_ARGS__);					\
	debug_printf("\n");							\
	LINE_FEED_OUT;								\
} while (0)
#define	Warning(...)							\
do {											\
	LINE_START;									\
	debug_printf("W:%s:%d:",__FILE__,__LINE__);	\
	debug_printf(__VA_ARGS__);					\
	LINE_FEED_OUT;								\
} while (0)
#define	Message(l, ...)							\
do {											\
	LINE_START;									\
	debug_printf("M:%s:%d:",__FILE__,__LINE__);	\
	debug_printf(__VA_ARGS__);					\
	LINE_FEED_OUT;								\
} while (0)
#define	_MessageLevelPrintf(m,f,l,...)			\
do {											\
	LINE_START;									\
	debug_printf("M:%s:%d:",(f),(l));			\
	debug_printf(__VA_ARGS__);					\
	LINE_FEED_OUT;								\
} while (0)
#define	MessageLog(s)							\
do {											\
	LINE_START;									\
	debug_printf("L:%s:%d:%s",__FILE__,__LINE__,(s))	\
	LINE_FEED_OUT;								\
}	while(0)
#define MessageLogPrintf(...)					\
do {											\
	LINE_START;									\
	debug_printf("L:%s:%d:",__FILE__,__LINE__);	\
	debug_printf(__VA_ARGS__);					\
	LINE_FEED_OUT;								\
}	while (0)
#define	MessageDebug(s)							\
do {											\
	LINE_START;									\
	debug_printf("D:%s:%d:%s",__FILE__,__LINE__,(s));	\
	LINE_FEED_OUT;								\
}	while (0)
#define MessageDebugPrintf(...)					\
do {											\
	LINE_START;									\
	debug_printf("D:%s:%d:",__FILE__,__LINE__);	\
	debug_printf(__VA_ARGS__);					\
	LINE_FEED_OUT;								\
} while (0)
#endif

#endif
