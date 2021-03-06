#ifndef _STX_BSTORAGE_H_
#define _STX_BSTORAGE_H_
#ifdef _MSC_VER
#pragma warning(disable : 4503)
#endif
#include <stx/storage/types.h>
#include <stx/storage/leb128.h>
#include <Poco/Mutex.h>
#include <Poco/Timestamp.h>
#include <lz4.h>
#include <stdio.h>
#include <string.h>
#include <stx/storage/pool.h>
/// memuse variables
extern stx::storage::u64 store_max_mem_use ;
extern stx::storage::u64 store_current_mem_use ;
extern stx::storage::u64  _reported_memory_size();
extern stx::storage::u64 calc_total_use();
/// the allocation pool
extern stx::storage::allocation::pool allocation_pool;
extern stx::storage::allocation::pool buffer_allocation_pool;
extern "C"{
	#include "zlib.h"
};
#include <fse.h>
#include <zlibh.h>
#include "storage/transactions/system_timers.h"


namespace stx{
	template<typename _Ht>
	struct btree_hash{
		size_t operator()(const _Ht& k) const{
			return 0;///(size_t) std::hash<_Ht>()(k); ///
		};
	};
	namespace storage{

		namespace allocation{
			template <class T>
			class pool_alloc_tracker : public base_tracker<T>{
			public:

                typedef T        value_type;
				typedef T*       pointer;
				typedef const T* const_pointer;
				typedef T&       reference;
				typedef const T& const_reference;
				typedef std::size_t    size_type;
				typedef std::ptrdiff_t difference_type;
				// rebind allocator to type U
				template <typename U>
				struct rebind {
					typedef pool_alloc_tracker<U> other;
				};
				pool_alloc_tracker() throw() {
				}
				pool_alloc_tracker(const pool_alloc_tracker&) throw() {
				}
				template <class U>
				pool_alloc_tracker (const pool_alloc_tracker<U>&) throw() {
				}

				// allocate but don't initialize num elements of type T
				pointer allocate (size_type num, const void* = 0) {
					//bt_counter c;
					//c.add(num*sizeof(T)+this->overhead());
					return (pointer)allocation_pool.allocate(num*sizeof(T));
				}
				// deallocate storage p of deleted elements
				void deallocate (pointer p, size_type num) {

					allocation_pool.free((void*)p,num*sizeof(T));

					//bt_counter c;
					//c.remove(num*sizeof(T)+this->overhead());
				}
			};
			 // return that all specializations of this allocator are interchangeable
			template <class T1, class T2>
			bool operator== (const pool_alloc_tracker<T1>&,const pool_alloc_tracker<T2>&) throw() {
				return true;
			}
			template <class T1, class T2>
			bool operator!= (const pool_alloc_tracker<T1>&, const pool_alloc_tracker<T2>&) throw() {
				return false;
			}
			template <class T>
			class buffer_pool_alloc_tracker : public base_tracker<T>{
			public:

                typedef T        value_type;
				typedef T*       pointer;
				typedef const T* const_pointer;
				typedef T&       reference;
				typedef const T& const_reference;
				typedef std::size_t    size_type;
				typedef std::ptrdiff_t difference_type;
				// rebind allocator to type U
				template <class U>
				struct rebind {
					typedef buffer_pool_alloc_tracker<U> other;
				};
				buffer_pool_alloc_tracker() throw() {
				}
				buffer_pool_alloc_tracker(const buffer_pool_alloc_tracker&) throw() {
				}
				template <class U>
				buffer_pool_alloc_tracker (const buffer_pool_alloc_tracker<U>&) throw() {
				}

				// allocate but don't initialize num elements of type T
				pointer allocate (size_type num, const void* = 0) {
					//buffer_counter c;
					//c.add(num*sizeof(T)+this->overhead());
					return (pointer)buffer_allocation_pool.allocate(num*sizeof(T));
				}
				// deallocate storage p of deleted elements
				void deallocate (pointer p, size_type num) {

					buffer_allocation_pool.free((void*)p,num*sizeof(T));

					//buffer_counter c;
					//c.remove(num*sizeof(T)+this->overhead());
				}
			};
			 // return that all specializations of this allocator are interchangeable
			template <class T1, class T2>
			bool operator== (const buffer_pool_alloc_tracker<T1>&,const buffer_pool_alloc_tracker<T2>&) throw() {
				return true;
			}
			template <class T1, class T2>
			bool operator!= (const buffer_pool_alloc_tracker<T1>&, const buffer_pool_alloc_tracker<T2>&) throw() {
				return false;
			}
			template <class T>
			class buffer_tracker : public base_tracker<T>{
			public:
				// rebind allocator to type U
				typedef base_tracker<T> base_class;
				typedef T        value_type;
				typedef T*       pointer;
				typedef const T* const_pointer;
				typedef T&       reference;
				typedef const T& const_reference;
				typedef std::size_t    size_type;
				typedef std::ptrdiff_t difference_type;

				template <class U>
				struct rebind {
					typedef buffer_tracker<U> other;
				};
				buffer_tracker() throw() {
				}
				buffer_tracker(const buffer_tracker&) throw() {
				}
				template <class U>
				buffer_tracker (const buffer_tracker<U>&) throw() {
				}

				// allocate but don't initialize num elements of type T
				pointer allocate (size_type num, const void* = 0) {
					buffer_counter c;
					c.add(num*sizeof(T)+this->overhead());
					return base_class::allocate(num);
				}
				// deallocate storage p of deleted elements
				void deallocate (pointer p, size_type num) {
					base_class::deallocate(p, num);
					buffer_counter c;
					c.remove(num*sizeof(T)+this->overhead());
				}
			};

			 // return that all specializations of this allocator are interchangeable
			template <class T1, class T2>
			bool operator== (const buffer_tracker<T1>&,const buffer_tracker<T2>&) throw() {
				return true;
			}
			template <class T1, class T2>
			bool operator!= (const buffer_tracker<T1>&, const buffer_tracker<T2>&) throw() {
				return false;
			}
			template <class T>
			class col_tracker : public base_tracker<T>{
			public:
                typedef base_tracker<T> base_class;
				typedef T        value_type;
				typedef T*       pointer;
				typedef const T* const_pointer;
				typedef T&       reference;
				typedef const T& const_reference;
				typedef std::size_t    size_type;
				typedef std::ptrdiff_t difference_type;
				// rebind allocator to type U
				template <class U>
				struct rebind {
					typedef col_tracker<U> other;
				};
				col_tracker() throw() {
				}
				col_tracker(const col_tracker&) throw() {
				}
				template <class U>
				col_tracker (const col_tracker<U>&) throw() {
				}

				// allocate but don't initialize num elements of type T
				pointer allocate (size_type num, const void* = 0) {
					col_counter c;
					c.add(num*sizeof(T)+this->overhead());
					return base_class::allocate(num);
				}
				// deallocate storage p of deleted elements
				void deallocate (pointer p, size_type num) {
					base_class::deallocate(p, num);
					col_counter c;
					c.remove(num*sizeof(T)+this->overhead());
				}
			};

			 // return that all specializations of this allocator are interchangeable
			template <class T1, class T2>
			bool operator== (const col_tracker<T1>&,const col_tracker<T2>&) throw() {
				return true;
			}
			template <class T1, class T2>
			bool operator!= (const col_tracker<T1>&, const col_tracker<T2>&) throw() {
				return false;
			}
			template <class T>
			class stl_tracker : public base_tracker<T>{
			public:
				typedef base_tracker<T> base_class;
				typedef T        value_type;
				typedef T*       pointer;
				typedef const T* const_pointer;
				typedef T&       reference;
				typedef const T& const_reference;
				typedef std::size_t    size_type;
				typedef std::ptrdiff_t difference_type;
				// rebind allocator to type U
				template <class U>
				struct rebind {
					typedef stl_tracker<U> other;
				};
				stl_tracker() throw() {
				}
				stl_tracker(const stl_tracker&) throw() {
				}
				template <class U>
				stl_tracker (const stl_tracker<U>&) throw() {
				}

				// allocate but don't initialize num elements of type T
				pointer allocate (size_type num, const void* = 0) {
					stl_counter c;
					c.add(num*sizeof(T)+this->overhead());
					return base_class::allocate(num);
				}
				// deallocate storage p of deleted elements
				void deallocate (pointer p, size_type num) {
					base_class::deallocate(p, num);
					stl_counter c;
					c.remove(num*sizeof(T)+this->overhead());
				}
			};

			 // return that all specializations of this allocator are interchangeable
			template <class T1, class T2>
			bool operator== (const stl_tracker<T1>&,const stl_tracker<T2>&) throw() {
				return true;
			}
			template <class T1, class T2>
			bool operator!= (const stl_tracker<T1>&, const stl_tracker<T2>&) throw() {
				return false;
			}
			static void print_allocations(){
				double total_mb = ((double)(allocation_pool.get_total_allocated()+buffer_allocation_pool.get_total_allocated())/(1024.0*1024.0));
				double buff_mb = ((double)(buffer_allocation_pool.get_total_allocated())/(1024.0*1024.0));

				double max_alloc = ((double)allocation_pool.get_max_pool_size())/(1024.0*1024.0);
				double max_buf_alloc = buffer_allocation_pool.get_max_pool_size()/(1024.0*1024.0);
				inf_print(" resources: %.3f (buffers %.3f of %.3f) (structure max %.3f)",total_mb,buff_mb,max_buf_alloc,max_alloc);
			}

		}; /// allocations

		/// the general buffer type
		typedef std::vector<u8, sta::buffer_pool_alloc_tracker<u8>> buffer_type;
		/// typedef std::vector<u8, sta::buffer_tracker<u8>> buffer_type;

		/// ZLIBH
		static void inplace_compress_zlibh(buffer_type& buff){
			typedef char * encode_type_ref;
			buffer_type t;
			i32 origin = (i32) buff.size();
			t.resize(ZLIBH_compressBound(origin)+sizeof(i32));
			i32 cp = buff.empty() ? 0 : ZLIBH_compress((encode_type_ref)&t[sizeof(i32)], (const encode_type_ref)&buff[0], origin);
			*((i32*)&t[0]) = origin;
			t.resize(cp+sizeof(i32));
			buff = t;

		}
		template<typename _VectorType>
		static void decompress_zlibh(buffer_type &decoded,const _VectorType& buff){
			if(buff.empty()){
				decoded.clear();

			}else{
				typedef char * encode_type_ref;
				i32 d = *((i32*)&buff[0]) ;
				decoded.reserve(d);
				decoded.resize(d);
				ZLIBH_decompress((encode_type_ref)&decoded[0],(const encode_type_ref)&buff[sizeof(i32)]);
			}
		}

		static void inplace_decompress_zlibh(buffer_type& buff, buffer_type& dt){
			if(buff.empty()) return;
			decompress_zlibh(dt, buff);
			buff = dt;
		}

		static void inplace_decompress_zlibh(buffer_type& buff){
			if(buff.empty()) return;
			buffer_type dt;
			decompress_zlibh(dt, buff);
			buff = dt;
		}
		/// FSE
		static void inplace_compress_fse(buffer_type& buff){
			std::cout << "fse compression not included in this build" << std::endl;
			throw std::exception();
			buffer_type t;
			i32 origin = (i32)buff.size();
			t.resize(FSE_compressBound(origin)+sizeof(i32));
			i32 cp = buff.empty() ? 0 : (i32)0; // FSE_compress((encode_type_ref)&t[sizeof(i32)], (i32)(t.size()) - sizeof(i32), (const encode_type_ref)&buff[0], origin);
			if(FSE_isError(cp)!=0 || cp <= 1){
				cp = (i32)buff.size();
				origin = -cp;
				memcpy(&t[sizeof(i32)], &buff[0], buff.size());
			}
			*((i32*)&t[0]) = (i32)origin;
			t.resize(cp+sizeof(i32));
			buff = t;

		}
		template<typename _VectorType>
		static void decompress_fse(buffer_type &decoded,const _VectorType& buff){
		    std::cout << "fse compression not included in this build" << std::endl;
			throw std::exception();

			if(buff.empty()){
				decoded.clear();
			}else{
				i32 d = *((i32*)&buff[0]) ;
				if(d < 0){
					d = -d;
					decoded.reserve(d);
					decoded.resize(d);
					memcpy(&decoded[0], &buff[sizeof(i32)], d);
				}else{
					decoded.reserve(d);
					decoded.resize(d);
					//FSE_decompress((encode_type_ref)&decoded[0],d,(const encode_type_ref)&buff[sizeof(i32)],((i32)buff.size())-sizeof(i32));
				}
			}
		}

		static void inplace_decompress_fse(buffer_type& buff, buffer_type& dt){
			if(buff.empty()) return;
			decompress_fse(dt, buff);
			buff = dt;
		}

		static void inplace_decompress_fse(buffer_type& buff){
			if(buff.empty()) return;
			buffer_type dt;
			decompress_fse(dt, buff);
			buff = dt;
		}
		/// LZ4
		static void inplace_compress_lz4(buffer_type& buff, buffer_type& t){
			typedef char * encode_type_ref;

			i32 origin = (i32)buff.size();
			/// TODO: cannot compress sizes lt 200 mb
			size_t dest_size = LZ4_compressBound((int)buff.size())+sizeof(i32);
			if(t.size() < dest_size) t.resize(dest_size);
			i32 cp = buff.empty() ? 0 : LZ4_compress((const encode_type_ref)&buff[0], (encode_type_ref)&t[sizeof(i32)], origin);
			*((i32*)&t[0]) = origin;
			t.resize(cp+sizeof(i32));
			/// inplace_compress_zlibh(t);
			/// inplace_compress_fse(t);
			buff = t;

		}
		static void compress_lz4_fast(buffer_type& to, const buffer_type& from){
			typedef char * encode_type_ref;
			static const i32 max_static = 8192;

			i32 origin = (i32)from.size();
			/// TODO: cannot compress sizes lt 200 mb
			i32 tcl = LZ4_compressBound((int)from.size())+sizeof(i32);
			encode_type_ref to_ref;

			char s_buf[max_static];
			if(tcl < max_static){
				to_ref = s_buf;
			}else{
				/// TODO: use a pool allocator directly
				to_ref = new char[tcl];
			}
			i32 cp = from.empty() ? 0 : LZ4_compress((const encode_type_ref)&from[0], (encode_type_ref)&to_ref[sizeof(i32)], origin);
			*((i32*)&to_ref[0]) = origin;
			if(to.empty()){
				buffer_type temp((const u8*)to_ref,((const u8*)to_ref)+(cp+sizeof(i32)));
				to.swap(temp);
			}else{
				to.resize(cp+sizeof(i32));
				memcpy(&to[0], to_ref,to.size());
			}
			if(tcl >= max_static){
				delete to_ref;
			}
			/// inplace_compress_zlibh(t);
			/// inplace_compress_fse(t);
			//buff = t;

		}
		static void compress_lz4(buffer_type& to, const buffer_type& from){
			typedef char * encode_type_ref;

			i32 origin = (i32)from.size();
			/// TODO: cannot compress sizes lt 200 mb
			to.resize(LZ4_compressBound((int)from.size())+sizeof(i32));
			i32 cp = from.empty() ? 0 : LZ4_compress((const encode_type_ref)&from[0], (encode_type_ref)&to[sizeof(i32)], origin);
			*((i32*)&to[0]) = origin;
			to.resize(cp+sizeof(i32));
			/// inplace_compress_zlibh(t);
			/// inplace_compress_fse(t);
			//buff = t;

		}
		static void inplace_compress_lz4(buffer_type& buff){

			buffer_type t;
			inplace_compress_lz4(buff, t);
		}
		static void decompress_lz4(buffer_type &decoded,const buffer_type& buff){ /// input <-> buff
			if(buff.empty()){
				decoded.clear();
			}else{
				typedef char * encode_type_ref;
				/// buffer_type buff ;
				/// decompress_zlibh(buff, input);
				/// decompress_fse(buff, input);
				i32 d = *((i32*)&buff[0]) ;
				decoded.reserve(d);
				decoded.resize(d);
				LZ4_decompress_fast((const encode_type_ref)&buff[sizeof(i32)],(encode_type_ref)&decoded[0],d);
			}

		}
		static size_t r_decompress_lz4(buffer_type &decoded,const buffer_type& buff){ /// input <-> buff
			if(buff.empty()){
				decoded.clear();
			}else{
				typedef char * encode_type_ref;
				/// buffer_type buff ;
				/// decompress_zlibh(buff, input);
				/// decompress_fse(buff, input);
				i32 d = *((i32*)&buff[0]) ;
				if((i32)decoded.size() < d){
					i32 rs = d; //std::max<i32>(d,256000);
					decoded.reserve(rs);
					decoded.resize(rs);
				}
				LZ4_decompress_fast((const encode_type_ref)&buff[sizeof(i32)],(encode_type_ref)&decoded[0],d);
				return d;
			}
			return 0;
		}
		static void inplace_decompress_lz4(buffer_type& buff){
			if(buff.empty()) return;
			buffer_type dt;
			decompress_lz4(dt, buff);
			buff = dt;
		}
		static void inplace_decompress_lz4(buffer_type& buff, buffer_type& dt){
			if(buff.empty()) return;
			decompress_lz4(dt, buff);
			buff = dt;
		}
		static void inplace_compress_zlib(buffer_type& buff){
			static const int COMPRESSION_LEVEL = 1;
			static const int Z_MIN_MEM = 1024;
			buffer_type t;
			uLongf actual = (uLongf) buff.size();
			t.resize(actual+Z_MIN_MEM+sizeof(uLongf));
			uLongf compressed = actual+Z_MIN_MEM+sizeof(uLongf);
			*((uLongf*)&t[0]) = actual;
			int zErr = compress2((Bytef*)&t[sizeof(actual)],&compressed,&buff[0],actual,COMPRESSION_LEVEL);
			if(zErr != Z_OK){
				printf("ZLIB write Error\n");
			}
			t.resize(compressed+sizeof(actual));
			t.shrink_to_fit();
			buff = t;
		}
		static void decompress_zlib(buffer_type &decoded,const buffer_type& encoded){
			if(encoded.empty()) return;
			uLongf eos = (uLongf)encoded.size();
			uLongf actual = *((uLongf*)&encoded[0]);
			uLongf compressed = eos - sizeof(actual);
			decoded.resize(actual+1024);
			if(Z_OK != uncompress((Bytef*)&decoded[0], (uLongf*)&actual, (Bytef*)&encoded[sizeof(actual)], compressed)){
				printf("ZLIB read error eos %ld, actual %ld, compressed %ld\n",eos,actual,compressed);
				return;
			}
			decoded.resize(actual);
			buffer_type temp = decoded;
			decoded.swap(temp);

		}
		static void inplace_decompress_zlib(buffer_type &decoded){
			if(decoded.empty()) return;
			buffer_type t(decoded);
			decompress_zlib(decoded,t);
		}
		static void test_compression(){
		    buffer_type buff,buff2;
            inplace_compress_zlibh(buff);
            inplace_decompress_zlibh(buff);
            inplace_decompress_zlibh(buff,buff2);
            inplace_compress_fse(buff);
            inplace_decompress_fse(buff);

		}
		/// some primitive encodings
		namespace primitive {

		
			template<typename _Primitive>
			static buffer_type::iterator store(buffer_type::iterator w, _Primitive p) {
				buffer_type::iterator writer = w;
				const u8* s = (const u8*)&p;
				const u8* e = s+sizeof(_Primitive);
				writer = std::copy(s, e, writer);
				return writer;
			}
			template<typename _Primitive>
			static buffer_type::const_iterator read(_Primitive& p, buffer_type::const_iterator r) {
				buffer_type::const_iterator reader = r;
				u8* s = (u8 *)&p;				
				std::copy(reader, reader + sizeof(_Primitive), s);
				reader += sizeof(_Primitive);
				return reader;
			} 
		};// primitive
		
		/// allocation type read only ,write or
		enum storage_action{
			read = 0,
			write,
			create
		};

		/// basic storage functions based on vectors of 1 byte unsigned
		class basic_storage{
		protected:
			typedef u8 value_type;

		public:
			/// reading functions for basic types

			buffer_type::iterator write(buffer_type::iterator out, u8 some){
				return stx::storage::leb128::write_unsigned(out, some);
			}

			buffer_type::iterator write(buffer_type::iterator out, u16 some){
				return stx::storage::leb128::write_unsigned(out, some);
			}

			buffer_type::iterator write(buffer_type::iterator out, u32 some){
				return stx::storage::leb128::write_unsigned(out, some);
			}

			buffer_type::iterator write(buffer_type::iterator out, i8 some){
				return stx::storage::leb128::write_signed(out, some);
			}

			/// write the buffer to the out iterator
			buffer_type::iterator write(buffer_type::iterator out, buffer_type::iterator limit, const u8 *some, u32 l){
				const u8* e = some+l;
				for(;some != e && out !=limit ;++some){
					*out++ = *some;
					++some;
				}
				return out;
			}

			/// reading functions for basic types
			u32 read_unsigned(buffer_type::iterator &inout){
				return stx::storage::leb128::read_unsigned(inout);
			}

			i32 read_signed(buffer_type::iterator &inout){
				return stx::storage::leb128::read_signed(inout);
			}

			size_t read(u8 *some, buffer_type::iterator in, buffer_type::iterator limit){
				u8* v = some;
				for(; in != limit; ++in){
					*v++ = *in;
				}
				return v-some;
			}
		};
		/// versions for all

		typedef std::pair<u64, version_type> _VersionRequest;

		typedef std::vector<_VersionRequest> _VersionRequests;



	};
};
#endif
