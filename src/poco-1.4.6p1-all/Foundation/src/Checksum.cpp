//
// Checksum.cpp
//
// $Id: //poco/1.4/Foundation/src/Checksum.cpp#1 $
//
// Library: Foundation
// Package: Core
// Module:  Checksum
//
// Copyright (c) 2007, Applied Informatics Software Engineering GmbH.
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


#include "Poco/Checksum.h"
#if defined(POCO_UNBUNDLED)
#include <zlib.h>
#else
#include <zlib.h>
#endif


namespace Poco {


Checksum::Checksum():
	_type(TYPE_CRC32),
	_value(crc32(0L, Z_NULL, 0))
{
}


Checksum::Checksum(Type t):
	_type(t),
	_value(0)
{
	if (t == TYPE_CRC32)
		_value = crc32(0L, Z_NULL, 0);
	else
		_value = adler32(0L, Z_NULL, 0);
}


Checksum::~Checksum()
{
}


void Checksum::update(const char* data, unsigned length)
{
	if (_type == TYPE_ADLER32)
		_value = adler32(_value, reinterpret_cast<const Bytef*>(data), length);
	else
		_value = crc32(_value, reinterpret_cast<const Bytef*>(data), length);
}


} // namespace Poco
