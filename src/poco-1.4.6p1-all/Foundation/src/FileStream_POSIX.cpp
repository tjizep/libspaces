//
// FileStream_POSIX.cpp
//
// $Id: //poco/1.4/Foundation/src/FileStream_POSIX.cpp#2 $
//
// Library: Foundation
// Package: Streams
// Module:  FileStream
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

#ifdef _OS_SELECTOR_INCLUDE_

#include "Poco/FileStream.h"
#include "Poco/File.h"
#include "Poco/Exception.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


namespace Poco {


FileStreamBuf::FileStreamBuf():
	BufferedBidirectionalStreamBuf(BUFFER_SIZE, std::ios::in | std::ios::out),
	_fd(-1),
	_pos(0)
{
}


FileStreamBuf::~FileStreamBuf()
{
	close();
}


void FileStreamBuf::open(const std::string& path, std::ios::openmode mode)
{
	poco_assert (_fd == -1);

	_pos = 0;
	_path = path;
	setMode(mode);
	resetBuffers();

	int flags(0);
	if (mode & std::ios::trunc)
		flags |= O_TRUNC;
	if (mode & std::ios::app)
		flags |= O_APPEND;
	if (mode & std::ios::out)
		flags |= O_CREAT;
	if ((mode & std::ios::in) && (mode & std::ios::out))
		flags |= O_RDWR;
	else if (mode & std::ios::in)
		flags |= O_RDONLY;
	else
		flags |= O_WRONLY;
			
	_fd = ::open(path.c_str(), flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (_fd == -1)
		File::handleLastError(_path);
		
	if ((mode & std::ios::app) || (mode & std::ios::ate))
		seekoff(0, std::ios::end, mode);
}


int FileStreamBuf::readFromDevice(char* buffer, std::streamsize length)
{
	if (_fd == -1) return -1;
	
	if (getMode() & std::ios::out)
		sync();
	
	int n = read(_fd, buffer, length);
	if (n == -1)
		File::handleLastError(_path);
	_pos += n;
	return n;
}


int FileStreamBuf::writeToDevice(const char* buffer, std::streamsize length)
{
	if (_fd == -1) return -1;

#if defined(POCO_VXWORKS)
	int n = write(_fd, const_cast<char*>(buffer), length);
#else
	int n = write(_fd, buffer, length);
#endif
	if (n == -1)
		File::handleLastError(_path);
	_pos += n;
	return n;
}


bool FileStreamBuf::close()
{
	bool success = true;
	if (_fd != -1)
	{
		try
		{
			sync();
		}
		catch (...)
		{
			success = false;
		}
		::close(_fd);
		_fd = -1;
	}
	return success;
}


std::streampos FileStreamBuf::seekoff(std::streamoff off, std::ios::seekdir dir, std::ios::openmode mode)
{
	if (_fd == -1 || !(getMode() & mode)) 
		return -1;

	if (getMode() & std::ios::out)
		sync();

	std::streamoff adj;
	if (mode & std::ios::in)
		adj = static_cast<std::streamoff>(egptr() - gptr());
	else
		adj = 0;

	resetBuffers();

	int whence = SEEK_SET;
	if (dir == std::ios::cur)
	{
		whence = SEEK_CUR;
		off -= adj;
	}
	else if (dir == std::ios::end)
	{
		whence = SEEK_END;
	}
	_pos = lseek(_fd, off, whence);
	return _pos;
}


std::streampos FileStreamBuf::seekpos(std::streampos pos, std::ios::openmode mode)
{
	if (_fd == -1 || !(getMode() & mode)) 
		return -1;

	if (getMode() & std::ios::out)
		sync();

	resetBuffers();

	_pos = lseek(_fd, pos, SEEK_SET);
	return _pos;
}


} // namespace Poco
#endif