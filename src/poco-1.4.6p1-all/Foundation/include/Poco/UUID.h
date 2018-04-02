//
// UUID.h
//
// $Id: //poco/1.4/Foundation/include/Poco/UUID.h#1 $
//
// Library: Foundation
// Package: UUID
// Module:  UUID
//
// Definition of the UUID class.
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


#ifndef Foundation_UUID_INCLUDED
#define Foundation_UUID_INCLUDED


#include "Poco/Foundation.h"


namespace Poco {


class Foundation_API UUID
	/// A UUID is an identifier that is unique across both space and time,
	/// with respect to the space of all UUIDs. Since a UUID is a fixed
	/// size and contains a time field, it is possible for values to
	/// rollover (around A.D. 3400, depending on the specific algorithm
	/// used). A UUID can be used for multiple purposes, from tagging
	/// objects with an extremely short lifetime, to reliably identifying
	/// very persistent objects across a network.
	/// This class implements a Universal Unique Identifier,
	/// as specified in Appendix A of the DCE 1.1 Remote Procedure
	/// Call Specification (http://www.opengroup.org/onlinepubs/9629399/),
	/// RFC 2518 (WebDAV), section 6.4.1 and the UUIDs and GUIDs internet
	/// draft by Leach/Salz from February, 1998 
	/// (http://ftp.ics.uci.edu/pub/ietf/webdav/uuid-guid/draft-leach-uuids-guids-01.txt)
	/// and also
	/// http://www.ietf.org/internet-drafts/draft-mealling-uuid-urn-03.txt
{
public:
	enum Version
	{
		UUID_TIME_BASED = 0x01,
		UUID_DCE_UID    = 0x02,
		UUID_NAME_BASED = 0x03,
		UUID_RANDOM     = 0x04
	};

	UUID();
		/// Creates a nil (all zero) UUID.
		
	UUID(const UUID& uuid);
		/// Copy constructor.

	explicit UUID(const std::string& uuid);
		/// Parses the UUID from a string.
		
	explicit UUID(const char* uuid);
		/// Parses the UUID from a string.

	~UUID();
		/// Destroys the UUID.

	UUID& operator = (const UUID& uuid);
		/// Assignment operator.
		
	void swap(UUID& uuid);
		/// Swaps the UUID with another one.	
		
	void parse(const std::string& uuid);
		/// Parses the UUID from its string representation.

	bool tryParse(const std::string& uuid);
		/// Tries to interpret the given string as an UUID.
		/// If the UUID is syntactically valid, assigns the
		/// members and returns true. Otherwise leaves the 
		/// object unchanged and returns false.

	std::string toString() const;
		/// Returns a string representation of the UUID consisting
		/// of groups of hexadecimal digits separated by hyphens.

	void copyFrom(const char* buffer);
		/// Copies the UUID (16 bytes) from a buffer or byte array.
		/// The UUID fields are expected to be
		/// stored in network byte order.
		/// The buffer need not be aligned.

	void copyTo(char* buffer) const;
		/// Copies the UUID to the buffer. The fields
		/// are in network byte order.
		/// The buffer need not be aligned.
		/// There must have room for at least 16 bytes.

	Version version() const;
		/// Returns the version of the UUID.
		
	int variant() const;
		/// Returns the variant number of the UUID:
		///   - 0 reserved for NCS backward compatibility
		///   - 2 the Leach-Salz variant (used by this class)
		///   - 6 reserved, Microsoft Corporation backward compatibility
		///   - 7 reserved for future definition

	bool operator == (const UUID& uuid) const;
	bool operator != (const UUID& uuid) const;
	bool operator <  (const UUID& uuid) const;
	bool operator <= (const UUID& uuid) const;
	bool operator >  (const UUID& uuid) const;
	bool operator >= (const UUID& uuid) const;
	
	bool isNull() const;
		/// Returns true iff the UUID is nil (in other words,
		/// consists of all zeros).

	bool hasTime() const ;
		/// returns true if the time is set

	static const UUID& null();
		/// Returns a null/nil UUID.

	static const UUID& dns();
		/// Returns the namespace identifier for the DNS namespace.
		
	static const UUID& uri();
		/// Returns the namespace identifier for the URI (former URL) namespace.

	static const UUID& oid();
		/// Returns the namespace identifier for the OID namespace.

	static const UUID& x500();
		/// Returns the namespace identifier for the X500 namespace.

protected:
	UUID(UInt32 timeLow, UInt32 timeMid, UInt32 timeHiAndVersion, UInt16 clockSeq, UInt8 node[]);
	UUID(const char* bytes, Version version);
	int compare(const UUID& uuid) const;
	static void appendHex(std::string& str, UInt8 n);
	static void appendHex(std::string& str, UInt16 n);
	static void appendHex(std::string& str, UInt32 n);
	static UInt8 nibble(char hex);
	void fromNetwork();
	void toNetwork();

private:
	UInt32 _timeLow;
	UInt16 _timeMid;
	UInt16 _timeHiAndVersion;
	UInt16 _clockSeq;
	UInt8  _node[6];
	
	friend class UUIDGenerator;
};


//
// inlines
//
inline bool UUID::operator == (const UUID& uuid) const
{
	return compare(uuid) == 0;
}


inline bool UUID::operator != (const UUID& uuid) const
{
	return compare(uuid) != 0;
}


inline bool UUID::operator < (const UUID& uuid) const
{
	return compare(uuid) < 0;
}


inline bool UUID::operator <= (const UUID& uuid) const
{
	return compare(uuid) <= 0;
}


inline bool UUID::operator > (const UUID& uuid) const
{
	return compare(uuid) > 0;
}


inline bool UUID::operator >= (const UUID& uuid) const
{
	return compare(uuid) >= 0;
}


inline UUID::Version UUID::version() const
{
	return Version(_timeHiAndVersion >> 12);
}


inline bool UUID::isNull() const
{
	return compare(null()) == 0;
}

inline bool UUID::hasTime() const
{
	return (_timeLow || _timeMid);
}

inline void swap(UUID& u1, UUID& u2)
{
	u1.swap(u2);
}


} // namespace Poco


#endif // Foundation_UUID_INCLUDED
