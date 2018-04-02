//
// Binding.h
//
// $Id: //poco/1.4/Data/include/Poco/Data/Binding.h#1 $
//
// Library: Data
// Package: DataCore
// Module:  Binding
//
// Definition of the Binding class.
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


#ifndef Data_Binding_INCLUDED
#define Data_Binding_INCLUDED


#include "Poco/Data/Data.h"
#include "Poco/Data/AbstractBinding.h"
#include "Poco/Data/DataException.h"
#include "Poco/Data/TypeHandler.h"
#include <set>
#include <vector>
#include <map>
#include <cstddef>


namespace Poco {
namespace Data {


template <class T>
class Binding: public AbstractBinding
	/// A Binding maps a value to a column.
{
public:
	explicit Binding(const T& val): _val(val), _bound(false)
		/// Creates the Binding.
	{
	}

	~Binding()
		/// Destroys the Binding.
	{
	}

	std::size_t numOfColumnsHandled() const
	{
		return TypeHandler<T>::size();
	}

	std::size_t numOfRowsHandled() const
	{
		return 1;
	}

	bool canBind() const
	{
		return !_bound;
	}

	void bind(std::size_t pos)
	{
		poco_assert_dbg(getBinder() != 0);
		TypeHandler<T>::bind(pos, _val, getBinder());
		_bound = true;
	}

	void reset ()
	{
		_bound = false;
	}

private:
	const T& _val;
	bool     _bound;
};


template <class T>
class Binding<std::vector<T> >: public AbstractBinding
	/// Specialization for std::vector.
{
public:
	explicit Binding(const std::vector<T>& val): _val(val), _begin(val.begin()), _end(val.end())
		/// Creates the Binding.
	{
		if (numOfRowsHandled() == 0)
			throw BindingException("It is illegal to bind to an empty data collection");
	}

	~Binding()
		/// Destroys the Binding.
	{
	}

	std::size_t numOfColumnsHandled() const
	{
		return TypeHandler<T>::size();
	}

	std::size_t numOfRowsHandled() const
	{
		return _val.size();
	}

	bool canBind() const
	{
		return _begin != _end;
	}

	void bind(std::size_t pos)
	{
		poco_assert_dbg(getBinder() != 0);
		poco_assert_dbg(canBind());
		TypeHandler<T>::bind(pos, *_begin, getBinder());
		++_begin;
	}

	void reset()
	{
		_begin = _val.begin();
		_end   = _val.end();
	}

private:
	const std::vector<T>&                   _val;
	typename std::vector<T>::const_iterator _begin;
	typename std::vector<T>::const_iterator _end;
};


template <class T>
class Binding<std::set<T> >: public AbstractBinding
	/// Specialization for std::set.
{
public:
	explicit Binding(const std::set<T>& val): AbstractBinding(), _val(val), _begin(val.begin()), _end(val.end())
		/// Creates the Binding.
	{
		if (numOfRowsHandled() == 0)
			throw BindingException("It is illegal to bind to an empty data collection");
	}

	~Binding()
		/// Destroys the Binding.
	{
	}

	std::size_t numOfColumnsHandled() const
	{
		return TypeHandler<T>::size();
	}

	std::size_t numOfRowsHandled() const
	{
		return _val.size();
	}

	bool canBind() const
	{
		return _begin != _end;
	}

	void bind(std::size_t pos)
	{
		poco_assert_dbg(getBinder() != 0);
		poco_assert_dbg(canBind());
		TypeHandler<T>::bind(pos, *_begin, getBinder());
		++_begin;
	}

	void reset()
	{
		_begin = _val.begin();
		_end   = _val.end();
	}

private:
	const std::set<T>&                   _val;
	typename std::set<T>::const_iterator _begin;
	typename std::set<T>::const_iterator _end;
};


template <class T>
class Binding<std::multiset<T> >: public AbstractBinding
	/// Specialization for std::multiset.
{
public:
	explicit Binding(const std::multiset<T>& val): AbstractBinding(), _val(val), _begin(val.begin()), _end(val.end())
		/// Creates the Binding.
	{
		if (numOfRowsHandled() == 0)
			throw BindingException("It is illegal to bind to an empty data collection");
	}

	~Binding()
		/// Destroys the Binding.
	{
	}

	std::size_t numOfColumnsHandled() const
	{
		return TypeHandler<T>::size();
	}

	std::size_t numOfRowsHandled() const
	{
		return _val.size();
	}

	bool canBind() const
	{
		return _begin != _end;
	}

	void bind(std::size_t pos)
	{
		poco_assert_dbg(getBinder() != 0);
		poco_assert_dbg(canBind());
		TypeHandler<T>::bind(pos, *_begin, getBinder());
		++_begin;
	}

	void reset()
	{
		_begin = _val.begin();
		_end   = _val.end();
	}

private:
	const std::multiset<T>&                   _val;
	typename std::multiset<T>::const_iterator _begin;
	typename std::multiset<T>::const_iterator _end;
};


template <class K, class V>
class Binding<std::map<K, V> >: public AbstractBinding
	/// Specialization for std::map.
{
public:
	explicit Binding(const std::map<K, V>& val): AbstractBinding(), _val(val), _begin(val.begin()), _end(val.end())
		/// Creates the Binding.
	{
		if (numOfRowsHandled() == 0)
			throw BindingException("It is illegal to bind to an empty data collection");
	}

	~Binding()
		/// Destroys the Binding.
	{
	}

	std::size_t numOfColumnsHandled() const
	{
		return TypeHandler<V>::size();
	}

	std::size_t numOfRowsHandled() const
	{
		return _val.size();
	}

	bool canBind() const
	{
		return _begin != _end;
	}

	void bind(std::size_t pos)
	{
		poco_assert_dbg(getBinder() != 0);
		poco_assert_dbg(canBind());
		TypeHandler<V>::bind(pos, _begin->second, getBinder());
		++_begin;
	}

	void reset()
	{
		_begin = _val.begin();
		_end   = _val.end();
	}

private:
	const std::map<K, V>&                   _val;
	typename std::map<K, V>::const_iterator _begin;
	typename std::map<K, V>::const_iterator _end;
};


template <class K, class V>
class Binding<std::multimap<K, V> >: public AbstractBinding
	/// Specialization for std::multimap.
{
public:
	explicit Binding(const std::multimap<K, V>& val): AbstractBinding(), _val(val), _begin(val.begin()), _end(val.end())
		/// Creates the Binding.
	{
		if (numOfRowsHandled() == 0)
			throw BindingException("It is illegal to bind to an empty data collection");
	}

	~Binding()
		/// Destroys the Binding.
	{
	}

	std::size_t numOfColumnsHandled() const
	{
		return TypeHandler<V>::size();
	}

	std::size_t numOfRowsHandled() const
	{
		return _val.size();
	}

	bool canBind() const
	{
		return _begin != _end;
	}

	void bind(std::size_t pos)
	{
		poco_assert_dbg(getBinder() != 0);
		poco_assert_dbg(canBind());
		TypeHandler<V>::bind(pos, _begin->second, getBinder());
		++_begin;
	}

	void reset()
	{
		_begin = _val.begin();
		_end   = _val.end();
	}

private:
	const std::multimap<K, V>&                   _val;
	typename std::multimap<K, V>::const_iterator _begin;
	typename std::multimap<K, V>::const_iterator _end;
};


template <typename T> Binding<T>* use(const T& t)
	/// Convenience function for a more compact Binding creation.
{
	return new Binding<T>(t);
}


} } // namespace Poco::Data


#endif // Data_Binding_INCLUDED
