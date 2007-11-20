/*
 * PANDA -- a simple transaction monitor
 * Copyright (C) 2007 Ogochan.
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

#ifndef	_PANDA_LOCK_H
#define	_PANDA_LOCK_H

#include	<pthread.h>

typedef	struct	{
	pthread_mutex_t	mutex;
	int				rwlock;
	int				wwrite;
	pthread_cond_t	reader_ok;
	pthread_cond_t	writer_ok;
}	_LOCKOBJECT;

#define	LOCKOBJECT	_LOCKOBJECT	lock

#define	InitLock(obj)	\
	{					\
		pthread_mutex_init(&(obj)->lock.mutex,NULL);	\
		pthread_cond_init(&(obj)->lock.reader_ok,NULL);	\
		pthread_cond_init(&(obj)->lock.writer_ok,NULL);	\
		(obj)->lock.rwlock = 0;							\
		(obj)->lock.wwrite = 0;							\
	}

#define	DestroyLock(obj)	\
	{					\
		pthread_mutex_destroy(&(obj)->lock.mutex);	\
		pthread_cond_destroy(&(obj)->lock.reader_ok);		\
		pthread_cond_destroy(&(obj)->lock.writer_ok);		\
	}

#define	LockRead(obj)		_Lock(&((obj)->lock),TRUE)
#define	LockWrite(obj)		_Lock(&((obj)->lock),FALSE)
#define	UnLock(obj)			_UnLock(&((obj)->lock))

extern	void	_Lock(_LOCKOBJECT *lock, Bool fRead);
extern	void	_UnLock(_LOCKOBJECT *lock);
extern	void	_WaitLock(pthread_cond_t *cond, _LOCKOBJECT *lock);

#endif
