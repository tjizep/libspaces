//
// Extractor.cpp
//
// $Id: //poco/1.4/Data/SQLite/src/Extractor.cpp#1 $
//
// Library: Data/SQLite
// Package: SQLite
// Module:  Extractor
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
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


#include "Poco/Data/SQLite/Extractor.h"
#include "Poco/Data/SQLite/Utility.h"
#include "Poco/Data/BLOB.h"
#include "Poco/Data/DataException.h"
#include "Poco/Exception.h"
#if defined(POCO_UNBUNDLED)
#include <sqlite3.h>
#else
#include "sqlite3.h"
#endif
#include <cstdlib>


namespace Poco {
namespace Data {
namespace SQLite {


Extractor::Extractor(sqlite3_stmt* pStmt):
	_pStmt(pStmt)
{
}


Extractor::~Extractor()
{
}


bool Extractor::extract(std::size_t pos, Poco::Int32& val)
{
	if (isNull(pos))
		return false;
	val = sqlite3_column_int(_pStmt, (int) pos);
	return true;
}


bool Extractor::extract(std::size_t pos, Poco::Int64& val)
{
	if (isNull(pos))
		return false;
	val = sqlite3_column_int64(_pStmt, (int) pos);
	return true;
}


bool Extractor::extract(std::size_t pos, double& val)
{
	if (isNull(pos))
		return false;
	val = sqlite3_column_double(_pStmt, (int) pos);
	return true;
}


bool Extractor::extract(std::size_t pos, std::string& val)
{
	if (isNull(pos))
		return false;
	const char *pBuf = reinterpret_cast<const char*>(sqlite3_column_text(_pStmt, (int) pos));
	if (!pBuf)
		val.clear();
	else
		val = std::string(pBuf);
	return true;
}


bool Extractor::extract(std::size_t pos, Poco::Data::BLOB& val)
{
	if (isNull(pos))
		return false;
	int size = sqlite3_column_bytes(_pStmt, (int) pos);
	const char* pTmp = reinterpret_cast<const char*>(sqlite3_column_blob(_pStmt, (int) pos));
	val = Poco::Data::BLOB(pTmp, size);
	return true;
}


bool Extractor::extract(std::size_t pos, Poco::Int8& val)
{
	if (isNull(pos))
		return false;
	val = sqlite3_column_int(_pStmt, (int) pos);
	return true;
}


bool Extractor::extract(std::size_t pos, Poco::UInt8& val)
{
	if (isNull(pos))
		return false;
	val = sqlite3_column_int(_pStmt, (int) pos);
	return true;
}


bool Extractor::extract(std::size_t pos, Poco::Int16& val)
{
	if (isNull(pos))
		return false;
	val = sqlite3_column_int(_pStmt, (int) pos);
	return true;
}


bool Extractor::extract(std::size_t pos, Poco::UInt16& val)
{
	if (isNull(pos))
		return false;
	val = sqlite3_column_int(_pStmt, (int) pos);
	return true;
}


bool Extractor::extract(std::size_t pos, Poco::UInt32& val)
{
	if (isNull(pos))
		return false;
	val = sqlite3_column_int(_pStmt, (int) pos);
	return true;
}


bool Extractor::extract(std::size_t pos, Poco::UInt64& val)
{
	if (isNull(pos))
		return false;
	val = sqlite3_column_int64(_pStmt, (int) pos);
	return true;
}


bool Extractor::extract(std::size_t pos, bool& val)
{
	if (isNull(pos))
		return false;
	val = (0 != sqlite3_column_int(_pStmt, (int) pos));
	return true;
}


bool Extractor::extract(std::size_t pos, float& val)
{
	if (isNull(pos))
		return false;
	val = static_cast<float>(sqlite3_column_double(_pStmt, (int) pos));
	return true;
}


bool Extractor::extract(std::size_t pos, char& val)
{
	if (isNull(pos)) return false;
	val = sqlite3_column_int(_pStmt, (int) pos);
	return true;
}


bool Extractor::extract(std::size_t pos, Poco::Any& val)
{
	if (isNull(pos)) return false;

	bool ret = false;

	switch (Utility::getColumnType(_pStmt, pos))
	{
	case MetaColumn::FDT_BOOL:
	{
		bool i = false;
		ret = extract(pos, i); 
		val = i;
		break;
	}
	case MetaColumn::FDT_INT8:
	{
		Poco::Int8 i = 0;
		ret = extract(pos, i); 
		val = i;
		break;
	}
	case MetaColumn::FDT_UINT8:
	{
		Poco::UInt8 i = 0;
		ret = extract(pos, i); 
		val = i;
		break;
	}
	case MetaColumn::FDT_INT16:
	{
		Poco::Int16 i = 0;
		ret = extract(pos, i); 
		val = i;
		break;
	}
	case MetaColumn::FDT_UINT16:
	{
		Poco::UInt16 i = 0;
		ret = extract(pos, i); 
		val = i;
		break;
	}
	case MetaColumn::FDT_INT32:
	{
		Poco::Int32 i = 0;
		ret = extract(pos, i); 
		val = i;
		break;
	}
	case MetaColumn::FDT_UINT32:
	{
		Poco::UInt32 i = 0;
		ret = extract(pos, i); 
		val = i;
		break;
	}
	case MetaColumn::FDT_INT64:
	{
		Poco::Int64 i = 0;
		ret = extract(pos, i); 
		val = i;
		break;
	}
	case MetaColumn::FDT_UINT64:
	{
		Poco::UInt64 i = 0;
		ret = extract(pos, i); 
		val = i;
		break;
	}
	case MetaColumn::FDT_STRING:
	{
		std::string s;
		ret = extract(pos, s); 
		val = s;
		break;
	}
	case MetaColumn::FDT_DOUBLE:
	{
		double d(0.0);
		ret = extract(pos, d); 
		val = d;
		break;
	}
	case MetaColumn::FDT_FLOAT:
	{
		float f(0.0);
		ret = extract(pos, f); 
		val = f;
		break;
	}
	case MetaColumn::FDT_BLOB:
	{
		BLOB b;
		ret = extract(pos, b); 
		val = b;
		break;
	}
	default:
		throw Poco::Data::UnknownTypeException("Unknown type during extraction");
	}

	return ret;
}


bool Extractor::isNull(std::size_t pos)
{
	return sqlite3_column_type(_pStmt, (int) pos) == SQLITE_NULL;
}


} } } // namespace Poco::Data::SQLite
