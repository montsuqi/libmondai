/*
 * PANDA -- a simple transaction monitor
 * Copyright (C) 2007-2008 Ogochan.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <pthread.h>
#include "libmondai.h"
#include "lock.h"
#include "debug.h"

extern void _Lock(_LOCKOBJECT *lock, Bool fRead) {
  pthread_mutex_lock(&lock->mutex);
  if (fRead) {
    while ((lock->rwlock < 0) || (lock->wwrite > 0)) {
      pthread_cond_wait(&lock->reader_ok, &lock->mutex);
    }
    lock->rwlock++;
  } else {
    while (lock->rwlock != 0) {
      lock->wwrite++;
      pthread_cond_wait(&lock->writer_ok, &lock->mutex);
      lock->wwrite--;
    }
    lock->rwlock = -1;
  }
  pthread_mutex_unlock(&lock->mutex);
}

extern void _UnLock(_LOCKOBJECT *lock) {
  Bool fWaitWrite, fWaitRead;

  pthread_mutex_lock(&lock->mutex);
  if (lock->rwlock < 0) {
    lock->rwlock = 0;
  } else {
    lock->rwlock--;
  }
  fWaitWrite = ((lock->wwrite != 0) && (lock->rwlock == 0)) ? TRUE : FALSE;
  fWaitRead = (lock->wwrite == 0) ? TRUE : FALSE;
  pthread_mutex_unlock(&lock->mutex);
  if (fWaitWrite) {
    pthread_cond_signal(&lock->writer_ok);
  } else if (fWaitRead) {
    pthread_cond_signal(&lock->reader_ok);
  }
}

extern void _WaitLock(pthread_cond_t *cond, _LOCKOBJECT *lock) {
  Bool fWriter;

  fWriter = FALSE;
  pthread_mutex_lock(&lock->mutex);
  if (lock->rwlock < 0) {
    lock->rwlock = 0;
    fWriter = TRUE;
  } else {
    lock->rwlock--;
  }
  if ((lock->wwrite != 0) && (lock->rwlock == 0)) {
    pthread_cond_signal(&lock->writer_ok);
  } else if (lock->wwrite == 0) {
    pthread_cond_broadcast(&lock->reader_ok);
  }
  pthread_cond_wait(cond, &lock->mutex);
  pthread_mutex_unlock(&lock->mutex);
  _Lock(lock, !fWriter);
}
