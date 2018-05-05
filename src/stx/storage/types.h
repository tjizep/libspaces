#ifndef _STX_STORAGE_TYPES_H_
#define _STX_STORAGE_TYPES_H_

#include <type_traits>

template <typename T, typename NameGetter>
class has_member_impl {
private:
    typedef char matched_return_type;
    typedef long unmatched_return_type;

    template <typename C>
    static matched_return_type f(typename NameGetter::template get<C>*);

    template <typename C>
    static unmatched_return_type f(...);

public:
    static const bool value = (sizeof(f<T>(0)) == sizeof(matched_return_type));
};

template <typename T, typename NameGetter>
struct has_member
    : std::integral_constant<bool, has_member_impl<T, NameGetter>::value>
{};
/**
 * NOTE on store: vector is use as a contiguous array. If a version of STL
 * is encountered that uses a different allocation scheme then a replacement
 * will be provided here.
 */

/*
* Base types and imperfect hash Template Classes v 0.1
*/
/*****************************************************************************

Copyright (c) 2013, Christiaan Pretorius

Portions of this file contain modifications contributed and copyrighted by
Google, Inc. Those modifications are gratefully acknowledged and are described
briefly in the InnoDB documentation. The contributions by Google are
incorporated with their permission, and subject to the conditions contained in
the file COPYING.Google.

Portions of this file contain modifications contributed and copyrighted
by Percona Inc.. Those modifications are
gratefully acknowledged and are described briefly in the InnoDB
documentation. The contributions by Percona Inc. are incorporated with
their permission, and subject to the conditions contained in the file
COPYING.Percona.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA

*****************************************************************************/
#include <vector>
#include <Poco/Types.h>
#include <Poco/UUID.h>
#include <stdio.h>
#include <Poco/UUIDGenerator.h>
/// files that use these macros need to include <iostream> or <stdio.h>
#define __LOG_SPACES__ "SPACES"
#define __LOG_NAME__ __LOG_SPACES__

namespace stx{
	namespace storage{
			extern bool storage_debugging;
			extern bool storage_info;
		/// unsigned integer primitive types
			typedef unsigned char u8;
			typedef Poco::UInt16 u16;
			typedef Poco::UInt32 u32;
			typedef Poco::UInt64 u64;
		/// signed integer primitive types
			typedef char i8;
			typedef Poco::Int16 i16;
			typedef Poco::Int32 i32;
			typedef Poco::Int64 i64;
			typedef long long int lld; /// cast for %lld
			typedef double f64;
		/// virtual allocator address type
			typedef u64 stream_address ;
		/// the version type
			typedef Poco::UUID version_type;
		/// format spec casts
			typedef long long unsigned int fi64; /// cast for %llu

			static const char * tostring(const version_type& v) {
				static std::string _t_str;
				_t_str = v.toString();
				return _t_str.c_str();
			}

			inline version_type create_version(){
				return Poco::UUIDGenerator::defaultGenerator().create();
			}
			/// some resource descriptors for low latency verification
			typedef std::pair<stream_address,version_type> resource_descriptor;
			typedef std::vector<resource_descriptor> resource_descriptors;



	};
	extern bool memory_low_state;
};
namespace nst = stx::storage;
namespace std {

	static const char * to_string(const nst::version_type& v) {
		return nst::tostring(v);
	}
	static const char * to_string(const nst::resource_descriptor& v) {
		static std::string result;
		result = "{";
		result += std::to_string(v.first);
		result += std::to_string(v.second);
		result += "}";
		return result.c_str();
	}

	static const char * to_string(const nst::resource_descriptors& v) {
		static std::string result;
		result = "[";
		for(auto vi = v.begin(); vi != v.end(); ++vi){
			if(vi != v.begin()){
				result += ",";
			}
			result += std::to_string((*vi));

		}
		result += "]";
		return result.c_str();
	}


	template <>
    class hash<stx::storage::version_type>{
    public:
		size_t operator()(const stx::storage::version_type &version ) const{
			using namespace stx::storage;
			u64 r = 0;
			if(sizeof(version_type) >= sizeof(u64)){
				const u64 * data = reinterpret_cast<const u64*>(&version);
				r = data[0];
				if(sizeof(version_type) == sizeof(u64) * 2){
					 r ^= data[1];
				}
			}
			return r;
		}
    };
};

#define dbg_print(x,...)          do {  if (nst::storage_debugging) (printf("[DBG][%s] " x "\n", __LOG_NAME__, ##__VA_ARGS__)); } while(0)
#define wrn_print(x,...)          do {  if (nst::storage_info) (printf("[WRN][%s] " x "\n", __LOG_NAME__, ##__VA_ARGS__)); } while(0)
#define err_print(x,...)          do {  if (true) (printf("[ERR][%s] " x "\n", __LOG_NAME__, ##__VA_ARGS__)); } while(0)
#define inf_print(x,...)          do {  if (nst::storage_info) (printf("[INF][%s] " x "\n", __LOG_NAME__, ##__VA_ARGS__)); } while(0)
#endif

///_STX_STORAGE_TYPES_H_
