//
// SharedLibrary_VMS.cpp
//
// $Id: //poco/1.4/Foundation/src/SharedLibrary_VMS.cpp#2 $
//
// Library: Foundation
// Package: SharedLibrary
// Module:  SharedLibrary
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

#include "Poco/SharedLibrary_VMS.h"
#include "Poco/Path.h"
#include <lib$routines.h>
#include <libdef.h>
#include <descrip.h>
#include <chfdef.h>
#include <libfisdef.h>


namespace Poco {


FastMutex SharedLibraryImpl::_mutex;


SharedLibraryImpl::SharedLibraryImpl()
{
}


SharedLibraryImpl::~SharedLibraryImpl()
{
}


void SharedLibraryImpl::loadImpl(const std::string& path, int /*flags*/)
{
	FastMutex::ScopedLock lock(_mutex);

	if (!_path.empty()) throw LibraryAlreadyLoadedException(path);
	_path = path;
}


void SharedLibraryImpl::unloadImpl()
{
	_path.clear();
}


bool SharedLibraryImpl::isLoadedImpl() const
{
	return !_path.empty();
}


void* SharedLibraryImpl::findSymbolImpl(const std::string& name)
{
	FastMutex::ScopedLock lock(_mutex);

	if (_path.empty()) return NULL;

	Path p(_path);
	std::string filename = p.getBaseName();
	std::string ext = p.getExtension();
	std::string imageSpec = p.makeParent().toString();
	if (!imageSpec.empty() && !ext.empty())
	{
		imageSpec.append(".");
		imageSpec.append(ext);
	}
	int value = 0;
	long flags = LIB$M_FIS_MIXEDCASE;
	POCO_DESCRIPTOR_STRING(filenameDsc, filename);
	POCO_DESCRIPTOR_STRING(symbolDsc, name);
	POCO_DESCRIPTOR_STRING(imageSpecDsc, imageSpec);

	try
	{
		// lib$find_image_symbol only accepts 32-bit pointers
		#pragma pointer_size save
		#pragma pointer_size 32
		lib$find_image_symbol(&filenameDsc, &symbolDsc, &value, imageSpec.empty() ? 0 : &imageSpecDsc, flags);
		#pragma pointer_size restore
	}
	catch (struct chf$signal_array& sigarr)
	{
		unsigned sig = sigarr.chf$is_sig_name;
		unsigned act = LIB$_ACTIMAGE;
		if (lib$match_cond(&sig, &act)) 
			throw LibraryLoadException(_path);
	}
	return (void*) value;
}


const std::string& SharedLibraryImpl::getPathImpl() const
{
	return _path;
}


std::string SharedLibraryImpl::suffixImpl()
{
#if defined(_DEBUG)
	return "d.exe";
#else
	return ".exe";
#endif
}


} // namespace Poco
#endif