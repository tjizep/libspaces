//
// DataTypes.h
//
// $Id: //poco/1.4/Data/ODBC/include/Poco/Data/ODBC/DataTypes.h#2 $
//
// Library: Data/ODBC
// Package: ODBC
// Module:  DataTypes
//
// Definition of DataTypes.
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


#ifndef ODBC_DataTypes_INCLUDED
#define ODBC_DataTypes_INCLUDED


#include "Poco/Data/ODBC/ODBC.h"
#include <map>
#if defined(POCO_OS_FAMILY_WINDOWS)
#include <windows.h>
#endif
#include <sqlext.h>


namespace Poco {
namespace Data {
namespace ODBC {


class ODBC_API DataTypes
	/// C <==> SQL datatypes mapping utility class.
{
public:
	typedef std::map<int, int> DataTypeMap;
	typedef DataTypeMap::value_type ValueType;

	DataTypes();
		/// Creates the DataTypes.

	~DataTypes();
		/// Destroys the DataTypes.

	int cDataType(int sqlDataType) const;
		/// Returns C data type corresponding to supplied SQL data type.

	int sqlDataType(int cDataType) const;
		/// Returns SQL data type corresponding to supplied C data type.

private:
	DataTypeMap _cDataTypes; 
	DataTypeMap _sqlDataTypes; 
};


} } } // namespace Poco::Data::ODBC


#endif
