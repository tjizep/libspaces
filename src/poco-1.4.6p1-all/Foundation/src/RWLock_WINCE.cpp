//
// RWLock_WINCE.cpp
//
// $Id: //poco/1.4/Foundation/src/RWLock_WINCE.cpp#1 $
//
// Library: Foundation
// Package: Threading
// Module:  RWLock
//
// Copyright (c) 2009-2010, Applied Informatics Software Engineering GmbH.
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

#include "Poco/RWLock_WINCE.h"
#include "Poco/Thread.h"


namespace Poco {


RWLockImpl::RWLockImpl(): 
	_readerCount(0),
	_readerWaiting(0),
	_writerCount(0),
	_writerWaiting(0),
	_writeLock(false)
	
{
	InitializeCriticalSection(&_cs);
	_readerGreen = CreateEventW(NULL, FALSE, TRUE, NULL);
	if (!_readerGreen) throw SystemException("Cannot create RWLock");
	_writerGreen = CreateEventW(NULL, FALSE, TRUE, NULL);
	if (!_writerGreen)
	{
		CloseHandle(_readerGreen);
		throw SystemException("Cannot create RWLock");
	}
}


RWLockImpl::~RWLockImpl()
{
	CloseHandle(_readerGreen);
	CloseHandle(_writerGreen);
	DeleteCriticalSection(&_cs);
}


void RWLockImpl::readLockImpl()
{
	tryReadLockImpl(INFINITE);
}


bool RWLockImpl::tryReadLockImpl(DWORD timeout)
{
	bool wait = false;
	do 
	{
		EnterCriticalSection(&_cs);
		if (!_writerCount && !_writerWaiting)
		{
			if (wait)
			{
				_readerWaiting--;
				wait = false;
			}
			_readerCount++;
		}
		else 
		{
			if (!wait) 
			{
				_readerWaiting++;
				wait = true;
			}
			ResetEvent(_readerGreen);
		}
		LeaveCriticalSection(&_cs); 
		if (wait) 
		{
			if (WaitForSingleObject(_readerGreen, timeout) != WAIT_OBJECT_0) 
			{
				EnterCriticalSection(&_cs);
				_readerWaiting--;
				SetEvent(_readerGreen); 
				SetEvent(_writerGreen);
				LeaveCriticalSection(&_cs);
				return false;
			}
		}
	} 
	while (wait);
   
	return true;
}


void RWLockImpl::writeLockImpl()
{
	tryWriteLockImpl(INFINITE);
}


bool RWLockImpl::tryWriteLockImpl(DWORD timeout)
{
	bool wait = false;

	do 
	{
		EnterCriticalSection(&_cs);
		if (!_readerCount && !_writerCount)
		{
			if (wait) 
			{
				_writerWaiting--;
				wait = false;
			}
			_writerCount++;
		}
		else 
		{
			if (!wait) 
			{
				_writerWaiting++;
				wait = true;
			}
			ResetEvent(_writerGreen);
		}
		LeaveCriticalSection(&_cs);
		if (wait) 
		{
			if (WaitForSingleObject(_writerGreen, timeout) != WAIT_OBJECT_0) 
			{
				EnterCriticalSection(&_cs);
				_writerWaiting--;
				SetEvent(_readerGreen);
				SetEvent(_writerGreen);
				LeaveCriticalSection(&_cs);
				return false;
			}
		}
	}
	while (wait);
	
	_writeLock = true;
	return true;
}


void RWLockImpl::unlockImpl()
{
	EnterCriticalSection(&_cs);
	
	if (_writeLock)
	{
		_writeLock = false;
		_writerCount--;
	}
	else
	{
		_readerCount--;
	}
	if (_writerWaiting)
		SetEvent(_writerGreen);
	else if (_readerWaiting)
		SetEvent(_readerGreen);   
		  
	LeaveCriticalSection(&_cs);
}


} // namespace Poco
#endif