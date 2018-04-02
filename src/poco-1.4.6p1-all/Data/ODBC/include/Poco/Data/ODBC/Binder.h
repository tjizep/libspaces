//
// Binder.h
//
// $Id: //poco/1.4/Data/ODBC/include/Poco/Data/ODBC/Binder.h#2 $
//
// Library: Data/ODBC
// Package: ODBC
// Module:  Binder
//
// Definition of the Binder class.
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


#ifndef DataConnectors_ODBC_Binder_INCLUDED
#define DataConnectors_ODBC_Binder_INCLUDED


#include "Poco/Data/ODBC/ODBC.h"
#include "Poco/Data/AbstractBinder.h"
#include "Poco/Data/BLOB.h"
#include "Poco/Data/ODBC/Handle.h"
#include "Poco/Data/ODBC/Parameter.h"
#include "Poco/Data/ODBC/ODBCColumn.h"
#include "Poco/Data/ODBC/Utility.h"
#include "Poco/Exception.h"
#ifdef POCO_OS_FAMILY_WINDOWS
#include <windows.h>
#endif
#include <sqlext.h>


namespace Poco {
namespace Data {
namespace ODBC {


class ODBC_API Binder: public Poco::Data::AbstractBinder
	/// Binds placeholders in the sql query to the provided values. Performs data types mapping.
{
public:
	enum ParameterBinding
	{
		PB_IMMEDIATE,
		PB_AT_EXEC
	};

	Binder(const StatementHandle& rStmt,
		ParameterBinding dataBinding = PB_IMMEDIATE);
		/// Creates the Binder.

	~Binder();
		/// Destroys the Binder.

	void bind(std::size_t pos, const Poco::Int8& val);
		/// Binds an Int8.

	void bind(std::size_t pos, const Poco::UInt8& val);
		/// Binds an UInt8.

	void bind(std::size_t pos, const Poco::Int16& val);
		/// Binds an Int16.

	void bind(std::size_t pos, const Poco::UInt16& val);
		/// Binds an UInt16.

	void bind(std::size_t pos, const Poco::Int32& val);
		/// Binds an Int32.

	void bind(std::size_t pos, const Poco::UInt32& val);
		/// Binds an UInt32.

	void bind(std::size_t pos, const Poco::Int64& val);
		/// Binds an Int64.

	void bind(std::size_t pos, const Poco::UInt64& val);
		/// Binds an UInt64.

	void bind(std::size_t pos, const bool& val);
		/// Binds a boolean.

	void bind(std::size_t pos, const float& val);
		/// Binds a float.

	void bind(std::size_t pos, const double& val);
		/// Binds a double.

	void bind(std::size_t pos, const char& val);
		/// Binds a single character.

	void bind(std::size_t pos, const std::string& val);
		/// Binds a string.

	void bind(std::size_t pos, const Poco::Data::BLOB& val);
		/// Binds a BLOB.

	void bind(std::size_t pos);
		/// Binds a NULL value.

	void setDataBinding(ParameterBinding binding);
		/// Set data binding type.

	ParameterBinding getDataBinding() const;
		/// Return data binding type.

	std::size_t dataSize(SQLPOINTER pAddr) const;
		/// Returns bound data size for parameter at specified position.

private:
	typedef std::map<SQLPOINTER, SQLLEN> SizeMap;

	void bind(std::size_t pos, const char* const &pVal);
		/// Binds a const char ptr. 
		/// This is a private no-op in this implementation
		/// due to security risk.

	template <typename T>
	void bindImpl(std::size_t pos, T& val, SQLSMALLINT cDataType)
	{
		if (pos == 0) 
		{
			reset();
		}
		
		_lengthIndicator.push_back(0);
		_dataSize.insert(SizeMap::value_type((SQLPOINTER) &val, sizeof(T)));

		int sqlDataType = Utility::sqlDataType(cDataType);
		
		SQLUINTEGER columnSize = 0;
		SQLSMALLINT decimalDigits = 0;

		// somewhat funky flow control, but not all
		// ODBC drivers will cooperate here
		try 
		{ 
			Parameter p(_rStmt, pos); 
			columnSize = (SQLUINTEGER) p.columnSize();
			decimalDigits = (SQLSMALLINT) p.decimalDigits();
		}
		catch (StatementException&)
		{	
			try 
			{ 
				ODBCColumn c(_rStmt, pos);
				columnSize = (SQLUINTEGER) c.length();
				decimalDigits = (SQLSMALLINT) c.precision();
			}
			catch (StatementException&) { }
		}

		if (Utility::isError(SQLBindParameter(_rStmt, 
			(SQLUSMALLINT) pos + 1, 
			SQL_PARAM_INPUT, 
			cDataType, 
			sqlDataType, 
			columnSize,
			decimalDigits,
			(SQLPOINTER) &val, 
			0, 
			_lengthIndicator.back())))
		{
			throw StatementException(_rStmt, "SQLBindParameter()");
		}
	}
	
	void reset();

	const StatementHandle& _rStmt;
	std::vector<SQLLEN*> _lengthIndicator;
	SizeMap _dataSize;
	ParameterBinding _paramBinding;
};


//
// inlines
//
inline void Binder::bind(std::size_t pos, const Poco::Int8& val)
{
	bindImpl(pos, val, SQL_C_STINYINT);
}


inline void Binder::bind(std::size_t pos, const Poco::UInt8& val)
{
	bindImpl(pos, val, SQL_C_UTINYINT);
}


inline void Binder::bind(std::size_t pos, const Poco::Int16& val)
{
	bindImpl(pos, val, SQL_C_SSHORT);
}


inline void Binder::bind(std::size_t pos, const Poco::UInt16& val)
{
	bindImpl(pos, val, SQL_C_USHORT);
}


inline void Binder::bind(std::size_t pos, const Poco::UInt32& val)
{
	bindImpl(pos, val, SQL_C_ULONG);
}


inline void Binder::bind(std::size_t pos, const Poco::Int32& val)
{
	bindImpl(pos, val, SQL_C_SLONG);
}


inline void Binder::bind(std::size_t pos, const Poco::UInt64& val)
{
	bindImpl(pos, val, SQL_C_UBIGINT);
}


inline void Binder::bind(std::size_t pos, const Poco::Int64& val)
{
	bindImpl(pos, val, SQL_C_SBIGINT);
}


inline void Binder::bind(std::size_t pos, const float& val)
{
	bindImpl(pos, val, SQL_C_FLOAT);
}


inline void Binder::bind(std::size_t pos, const double& val)
{
	bindImpl(pos, val, SQL_C_DOUBLE);
}


inline void Binder::bind(std::size_t pos, const bool& val)
{
	bindImpl(pos, val, SQL_C_BIT);
}


inline void Binder::bind(std::size_t pos, const char& val)
{
	bindImpl(pos, val, SQL_C_STINYINT);
}


inline void Binder::setDataBinding(Binder::ParameterBinding binding)
{
	_paramBinding = binding;
}


inline Binder::ParameterBinding Binder::getDataBinding() const
{
	return _paramBinding;
}


} } } // namespace Poco::Data::ODBC


#endif // DataConnectors_ODBC_Binder_INCLUDED
