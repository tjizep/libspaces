//
// CryptoTransform.h
//
// $Id: //poco/1.4/Crypto/include/Poco/Crypto/CryptoTransform.h#2 $
//
// Library: Crypto
// Package: Cipher
// Module:  CryptoTransform
//
// Definition of the CryptoTransform class.
//
// Copyright (c) 2008, Applied Informatics Software Engineering GmbH.
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


#ifndef Crypto_CryptoTransform_INCLUDED
#define Crypto_CryptoTransform_INCLUDED


#include "Poco/Crypto/Crypto.h"
#include <ios>


namespace Poco {
namespace Crypto {


class Crypto_API CryptoTransform
	/// This interface represents the basic operations for cryptographic
	/// transformations to be used with a CryptoInputStream or a
	/// CryptoOutputStream.
	///
	/// Implementations of this class are returned by the Cipher class to
	/// perform encryption or decryption of data.
{
public:
	CryptoTransform();
		/// Creates a new CryptoTransform object.

	virtual ~CryptoTransform();
		/// Destroys the CryptoTransform.

	virtual std::size_t blockSize() const = 0;
		/// Returns the block size for this CryptoTransform.

	virtual int setPadding(int padding);
		/// Enables or disables padding. By default encryption operations are padded using standard block 
		/// padding and the padding is checked and removed when decrypting. If the padding parameter is zero then 
		/// no padding is performed, the total amount of data encrypted or decrypted must then be a multiple of 
		/// the block size or an error will occur.
		
	virtual std::streamsize transform(
		const unsigned char* input,
		std::streamsize		 inputLength,
		unsigned char*		 output,
		std::streamsize		 outputLength) = 0;
		/// Transforms a chunk of data. The inputLength is arbitrary and does not
		/// need to be a multiple of the block size. The output buffer has a maximum
		/// capacity of the given outputLength that must be at least
		///   inputLength + blockSize() - 1
		/// Returns the number of bytes written to the output buffer.

	virtual std::streamsize finalize(unsigned char* output, std::streamsize length) = 0;
		/// Finalizes the transformation. The output buffer must contain enough
		/// space for at least two blocks, ie.
		///   length >= 2*blockSize()
		/// must be true.  Returns the number of bytes written to the output
		/// buffer.
};


} } // namespace Poco::Crypto


#endif // Crypto_CryptoTransform_INCLUDED
