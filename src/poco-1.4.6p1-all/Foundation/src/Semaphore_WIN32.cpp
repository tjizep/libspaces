//
// Semaphore_WIN32.cpp
//
// $Id: //poco/1.4/Foundation/src/Semaphore_WIN32.cpp#1 $
//
// Library: Foundation
// Package: Threading
// Module:  Semaphore
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

#include "Poco/Semaphore_WIN32.h"


namespace Poco {


SemaphoreImpl::SemaphoreImpl(int n, int max)
{
	poco_assert (n >= 0 && max > 0 && n <= max);

	_sema = CreateSemaphoreW(NULL, n, max, NULL);
	if (!_sema)
	{
		throw SystemException("cannot create semaphore");
	}
}


SemaphoreImpl::~SemaphoreImpl()
{
	CloseHandle(_sema);
}


void SemaphoreImpl::waitImpl()
{
	switch (WaitForSingleObject(_sema, INFINITE))
	{
	case WAIT_OBJECT_0:
		return;
	default:
		throw SystemException("wait for semaphore failed");
	}
}


bool SemaphoreImpl::waitImpl(long milliseconds)
{
	switch (WaitForSingleObject(_sema, milliseconds + 1))
	{
	case WAIT_TIMEOUT:
		return false;
	case WAIT_OBJECT_0:
		return true;
	default:
		throw SystemException("wait for semaphore failed");		
	}
}


} // namespace Poco
#endif //#ifdef _OS_SELECTOR_INCLUDE_