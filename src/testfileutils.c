/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 2000-2002 Ogochan & JMA (Japan Medical Association).
 * Copyright (C) 2003-2007 Ogochan.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include "fileutils.h"

#define TEST_ROOT_DIR "/tmp/libmondai-test-root"
#define TEST_MAKE_DIR "/tmp/libmondai-test-root/MakeDir"
#define TEST_RM_R_OLD "/tmp/libmondai-test-root/rm_r_old"
#define TEST_RM_R_OLD2 "/tmp/libmondai-test-root/rm_r_old/1"
#define TEST_RM_R_OLD3 "/tmp/libmondai-test-root/rm_r_old/1/2"

static void touch(const char *path) {
  char *command = g_strdup_printf("touch %s/test.txt",path);
  fprintf(stderr, "%s\n", command);
  system(command);
  g_free(command);
}

static void ls_R() {
  char *command = g_strdup_printf("ls -R %s",TEST_ROOT_DIR);
  fprintf(stderr, "ls -R\n");
  system(command);
  g_free(command);
  fprintf(stderr, "\n");
}

extern int main(int argc, char **argv) {
  fprintf(stderr, "---- \n");
  fprintf(stderr, "rm_r[%s] [%d]\n", TEST_ROOT_DIR, rm_r(TEST_ROOT_DIR));
  fprintf(stderr, "mkdir_p[%s] [%d]\n", TEST_ROOT_DIR, mkdir_p(TEST_ROOT_DIR, 0700));
  touch(TEST_ROOT_DIR);
  ls_R();

  fprintf(stderr, "---- \n");
  fprintf(stderr, "MakeDir[%s] [%d]\n", TEST_MAKE_DIR, MakeDir(TEST_MAKE_DIR, 0700));
  touch(TEST_MAKE_DIR);
  ls_R();

  fprintf(stderr, "---- \n");
  fprintf(stderr, "rm_r[%s] [%d]\n", TEST_MAKE_DIR, rm_r(TEST_MAKE_DIR));
  ls_R();

  fprintf(stderr, "---- \n");
  fprintf(stderr, "mkdir_p[%s] [%d]\n", TEST_RM_R_OLD, mkdir_p(TEST_RM_R_OLD, 0700));
  touch(TEST_RM_R_OLD);
  fprintf(stderr, "mkdir_p[%s] [%d]\n", TEST_RM_R_OLD3, mkdir_p(TEST_RM_R_OLD3, 0700));
  touch(TEST_RM_R_OLD2);
  touch(TEST_RM_R_OLD3);
  ls_R();

  fprintf(stderr, "rm_r_old_depth(%s,500,2) \n", TEST_RM_R_OLD);
  rm_r_old_depth(TEST_RM_R_OLD,500,2);
  ls_R();

  fprintf(stderr, "sleep(5)\n");
  sleep(5);

  fprintf(stderr, "rm_r_old_depth(%s,4,2) \n", TEST_RM_R_OLD);
  rm_r_old_depth(TEST_RM_R_OLD,4,2);
  ls_R();

  return 0;
}
