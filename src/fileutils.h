/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2003 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2004-2008 Ogochan.
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

#ifndef _INC_FILEUTILS_H
#define _INC_FILEUTILS_H
#include <types.h>

extern Bool rm_r(char *dname);
extern Bool mkdir_p(char *dname, int mode);
extern Bool mkdir_p_clean(char *dir, int mode);
extern Bool MakeDir(char *dir, int mode);

#endif
