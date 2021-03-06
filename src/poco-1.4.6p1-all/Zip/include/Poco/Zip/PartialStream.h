//
// PartialStream.h
//
// $Id: //poco/1.4/Zip/include/Poco/Zip/PartialStream.h#1 $
//
// Library: Zip
// Package: Zip
// Module:  PartialStream
//
// Definition of the PartialStream class.
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


#ifndef Zip_PartialStream_INCLUDED
#define Zip_PartialStream_INCLUDED


#include "Poco/Zip/Zip.h"
#include "Poco/BufferedStreamBuf.h"
#include "Poco/Buffer.h"
#include <istream>
#include <ostream>


namespace Poco {
namespace Zip {


class Zip_API PartialStreamBuf: public Poco::BufferedStreamBuf
	/// A PartialStreamBuf is a class that limits one view on an inputstream to a selected view range
{
public:
	PartialStreamBuf(std::istream& in, std::ios::pos_type start, std::ios::pos_type end, const std::string& prefix, const std::string& postfix, bool initStream);
		/// Creates the PartialStream. 
		/// If initStream is true the status of the stream will be cleared on the first access, and the stream will be repositioned
		/// to position start

	PartialStreamBuf(std::ostream& out, std::size_t start, std::size_t end, bool initStream);
		/// Creates the PartialStream. 
		/// If initStream is true the status of the stream will be cleared on the first access.
		/// start and end acts as offset values for the written content. A start value greater than zero,
		/// means that the first bytes are not written but discarded instead,
		/// an end value not equal to zero means that the last end bytes are not written!
		/// Examples:
		///     start = 3; end = 1
		///     write("hello", 5) -> "l"

	~PartialStreamBuf();
		/// Destroys the PartialStream.

	void close();
		/// Flushes a writing streambuf

	Poco::UInt64 bytesWritten() const;

protected:
	int readFromDevice(char* buffer, std::streamsize length);

	int writeToDevice(const char* buffer, std::streamsize length);

private:
	enum 
	{
		STREAM_BUFFER_SIZE  = 1024
	};

	bool           _initialized;
	std::ios::pos_type  _start;
	Poco::UInt64   _numBytes;
	Poco::UInt64   _bytesWritten;
	std::istream*  _pIstr;
	std::ostream*  _pOstr;
	std::string    _prefix;
	std::string    _postfix;
	std::size_t    _ignoreStart;
	Poco::Buffer<char> _buffer;
	Poco::UInt32   _bufferOffset;
};


inline Poco::UInt64 PartialStreamBuf::bytesWritten() const
{
	return _bytesWritten;
}


class Zip_API PartialIOS: public virtual std::ios
	/// The base class for PartialInputStream and PartialOutputStream.
	///
	/// This class is needed to ensure the correct initialization
	/// order of the stream buffer and base classes.
{
public:
	PartialIOS(std::istream& istr, std::ios::pos_type start, std::ios::pos_type end, const std::string& prefix, const std::string& postfix, bool initStream);
		/// Creates the basic stream and connects it
		/// to the given input stream.
		/// If initStream is true the status of the stream will be cleared on the first access, and the stream will be repositioned
		/// to position start

	PartialIOS(std::ostream& ostr, std::size_t start, std::size_t end, bool initStream);
		/// Creates the basic stream and connects it
		/// to the given output stream.
		/// If initStream is true the status of the stream will be cleared on the first access.
		/// start and end acts as offset values for the written content. A start value greater than zero,
		/// means that the first bytes are not written but discarded instead,
		/// an end value not equal to zero means that the last end bytes are not written!
		/// Examples:
		///     start = 3; end = 1
		///     write("hello", 5) -> "l"

	~PartialIOS();
		/// Destroys the stream.

	PartialStreamBuf* rdbuf();
		/// Returns a pointer to the underlying streambuf.

protected:
	PartialStreamBuf _buf;
};


class Zip_API PartialInputStream: public PartialIOS, public std::istream
	/// This stream copies all characters read through it
	/// to one or multiple output streams.
{
public:
	PartialInputStream(std::istream& istr, std::ios::pos_type start, std::ios::pos_type end, bool initStream = true, const std::string& prefix = std::string(), const std::string& postfix = std::string());
		/// Creates the PartialInputStream and connects it
		/// to the given input stream. Bytes read are guaranteed to be in the range [start, end-1]
		/// If initStream is true the status of the stream will be cleared on the first access, and the stream will be repositioned
		/// to position start

	~PartialInputStream();
		/// Destroys the PartialInputStream.
};


class Zip_API PartialOutputStream: public PartialIOS, public std::ostream
	/// This stream copies all characters written to it
	/// to one or multiple output streams.
{
public:
	PartialOutputStream(std::ostream& ostr, std::size_t start, std::size_t end, bool initStream = true);
		/// Creates the PartialOutputStream and connects it
		/// to the given output stream. Bytes written are guaranteed to be in the range [start, realEnd - end].
		/// If initStream is true the status of the stream will be cleared on the first access.
		/// start and end acts as offset values for the written content. A start value greater than zero,
		/// means that the first bytes are not written but discarded instead,
		/// an end value not equal to zero means that the last end bytes are not written!
		/// Examples:
		///     start = 3; end = 1
		///     write("hello", 5) -> "l"
		///     
		///     start = 3; end = 0
		///     write("hello", 5) -> "lo"

	~PartialOutputStream();
		/// Destroys the PartialOutputStream.

	void close();
		/// must be called for the stream to properly terminate it

	Poco::UInt64 bytesWritten() const;
		/// Returns the number of bytes actually forwarded to the inner ostream
};


inline void PartialOutputStream::close()
{
	flush();
	_buf.close();
}


inline Poco::UInt64 PartialOutputStream::bytesWritten() const
{
	return _buf.bytesWritten();
}


} } // namespace Poco::Zip


#endif // Zip_PartialStream_INCLUDED
