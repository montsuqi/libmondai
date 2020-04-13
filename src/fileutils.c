/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2003 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2004-2009 Ogochan.
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
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <glib.h>
#include <errno.h>
#include <dirent.h>

#include "types.h"
#include "debug.h"

#define SIZE_PATH 2048

extern Bool rm_r(char *dname) {
  DIR *dir;
  struct dirent *ent;
  struct stat st;
  char path[SIZE_PATH];

  if (dname == NULL) {
    return FALSE;
  }

  if (stat(dname, &st) == 0) {
    if (S_ISDIR(st.st_mode)) {
      /* directory */
      if ((dir = opendir(dname)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
          if (strcmp(".",ent->d_name) == 0 || strcmp("..",ent->d_name) == 0) {
            // skip
          } else {
            snprintf(path, sizeof(path), "%s/%s", dname, ent->d_name);
            path[sizeof(path) - 1] = 0;
            if (!rm_r(path)) {
              return FALSE;
            }
          }
        }
        closedir(dir);
        if (remove(dname)) {
          return FALSE;
        }
      } else {
        return FALSE;
      }
    } else {
      /* file */
      if (remove(dname)) {
        return FALSE;
      }
      return TRUE;
    }
  } else {
    return FALSE;
  }
  return TRUE;
}

extern Bool mkdir_p(char *dname, int mode) {
  gchar path[SIZE_PATH];
  gchar *p, *q;
  size_t s;

  if (dname == NULL) {
    return FALSE;
  }
  p = q = dname;
  while (*p) {
    while (*p && !(G_DIR_SEPARATOR == (*p))) {
      p++;
    }
    if (p == q) {
      p++;
      continue;
    }
    s = (p - q) > sizeof(path) ? sizeof(path) : (p - q);
    strncpy(path, q, s);
    path[s] = 0;

    if (mkdir(path, mode) && errno != EEXIST) {
      if (*p == '\0') {
        return FALSE;
      }
    }
    if (*p) {
      p++;
    } else {
      break;
    }
  }
  return TRUE;
}

extern Bool mkdir_p_clean(char *dir, int mode) {
  rm_r(dir);
  if (mkdir_p(dir, mode)) {
    return TRUE;
  }
  return FALSE;
}

extern Bool MakeDir(char *dir, int mode) { return mkdir_p_clean(dir, mode); }
