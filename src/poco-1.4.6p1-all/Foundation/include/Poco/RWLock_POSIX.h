//
// RWLock_POSIX.h
//
// $Id: //poco/1.4/Foundation/include/Poco/RWLock_POSIX.h#1 $
//
// Library: Foundation
// Package: Threading
// Module:  RWLock
//
// Definition of the RWLockImpl class for POSIX Threads.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//

#ifdef _OS_SELECTOR_INCLUDE_

#ifndef Foundation_RWLock_POSIX_INCLUDED
#define Foundation_RWLock_POSIX_INCLUDED


#include "Poco/Foundation.h"
#include "Poco/Exception.h"
#include <pthread.h>
#include <errno.h>


namespace Poco {


class Foundation_API RWLockImpl
{
protected:
	RWLockImpl();
	~RWLockImpl();
	void readLockImpl();
	bool tryReadLockImpl();
	void writeLockImpl();
	bool tryWriteLockImpl();
	void unlockImpl();
	
private:
	pthread_rwlock_t _rwl;
};


//
// inlines
//
inline void RWLockImpl::readLockImpl()
{
	if (pthread_rwlock_rdlock(&_rwl)) 
		throw SystemException("cannot lock reader/writer lock");
}


inline bool RWLockImpl::tryReadLockImpl()
{
	int rc = pthread_rwlock_tryrdlock(&_rwl);
	if (rc == 0)
		return true;
	else if (rc == EBUSY)
		return false;
	else
		throw SystemException("cannot lock reader/writer lock");

}


inline void RWLockImpl::writeLockImpl()
{
	if (pthread_rwlock_wrlock(&_rwl)) 
		throw SystemException("cannot lock reader/writer lock");
}


inline bool RWLockImpl::tryWriteLockImpl()
{
	int rc = pthread_rwlock_trywrlock(&_rwl);
	if (rc == 0)
		return true;
	else if (rc == EBUSY)
		return false;
	else
		throw SystemException("cannot lock reader/writer lock");

}


inline void RWLockImpl::unlockImpl()
{
	if (pthread_rwlock_unlock(&_rwl))
		throw SystemException("cannot unlock mutex");
}


} // namespace Poco


#endif // Foundation_RWLock_POSIX_INCLUDED
#endif //#ifdef _OS_SELECTOR_INCLUDE_