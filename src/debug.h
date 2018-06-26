/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 1989-2008 Ogochan.
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

#ifndef _INC_DEBUG_H
#define _INC_DEBUG_H
#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef TRACE
#define dbgmsg(s) MessageDebug(s)
#define dbgprintf(fmt, ...) MessageDebugPrintf((fmt), __VA_ARGS__)
#define PASS(s) MessageDebug(s)
#define ENTER_FUNC MessageDebugPrintf(">%s", __func__)
#define LEAVE_FUNC MessageDebugPrintf("<%s", __func__)
#define RETURN(v) MessageDebugPrintf("<%s", __func__), return (v)
#else
#define dbgmsg(s)           /*	*/
#define dbgprintf(fmt, ...) /*	*/
#define PASS(s)             /*	*/
#define ENTER_FUNC          /*	*/
#define LEAVE_FUNC          /*	*/
#define RETURN(v) return (v)
#endif

#define MessageDebug(s) printf("D:%s:%d:%s\n", __FILE__, __LINE__, (s))
#define MessageDebugPrintf(...)                                                \
  do {                                                                         \
    printf("D:%s:%d:", __FILE__, __LINE__);                                    \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  } while (0)

#define EXIT(c)                                                                \
  {                                                                            \
    printf("exit at %s(%d) %s\n", __FILE__, __LINE__, __func__);               \
    exit(c);                                                                   \
  }

#define MonError(s)                                                            \
  fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, (s));                      \
  syslog(LOG_ERR, "%s:%d:%s", __FILE__, __LINE__, (s));                        \
  exit(1)

#define MonErrorPrintf(fmt, ...)                                               \
  fprintf(stderr, "%s:%d:" fmt "\n", __FILE__, __LINE__, __VA_ARGS__);         \
  syslog(LOG_ERR, "%s:%d:" fmt, __FILE__, __LINE__, __VA_ARGS__);              \
  exit(1)

#define MonWarning(s)                                                          \
  fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, (s));                      \
  syslog(LOG_WARNING, "%s:%d:%s", __FILE__, __LINE__, (s))

#define MonWarningPrintf(fmt, ...)                                             \
  fprintf(stderr, "%s:%d:" fmt "\n", __FILE__, __LINE__, __VA_ARGS__);         \
  syslog(LOG_WARNING, "%s:%d:" fmt, __FILE__, __LINE__, __VA_ARGS__)

#endif
