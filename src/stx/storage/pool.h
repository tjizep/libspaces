#ifndef _POOL_TS_H_20141129_

#define _POOL_TS_H_20141129_
#include <limits>
#include <vector>
#include <atomic>
#include <Poco/Mutex.h>
#include <Poco/Thread.h>
#include <Poco/ThreadPool.h>
#include <Poco/ThreadLocal.h>
#include <Poco/TaskManager.h>
#include <Poco/Task.h>
#include <Poco/Timestamp.h>
#include "storage/transactions/NotificationQueueWorker.h"
#include <stx/storage/types.h>
#include <vector>
#include <rabbit/unordered_map>
#include <storage/transactions/system_timers.h>

#include <typeinfo>
#include <unordered_map>

extern void add_btree_totl_used(ptrdiff_t added);
extern void remove_btree_totl_used(ptrdiff_t added);
extern char	use_internal_pool;
#ifdef _MSC_VER
#define thread_local __declspec(thread)
#endif
#ifndef TRUE
#define FALSE 0
#define TRUE !FALSE
#endif
namespace stx{
	namespace storage{
		namespace unordered = rabbit;
		typedef Poco::ScopedLockWithUnlock<Poco::Mutex> synchronized;
		typedef Poco::ScopedLockWithUnlock<Poco::FastMutex> f_synchronized;
		typedef Poco::ScopedTryLock<Poco::Mutex> synchronizing;
		extern void add_buffer_use(long long added);
		extern void remove_buffer_use(long long removed);

		extern void add_total_use(long long added);
		extern void remove_total_use(long long removed);

		extern void add_col_use(long long added);
		extern void remove_col_use(long long removed);

		extern void add_stl_use(long long added);
		extern void remove_stl_use(long long added);
		extern long long total_use;
		extern long long buffer_use;
		extern long long col_use;
		extern long long stl_use;

		namespace allocation{
			struct bt_counter{
				void add(size_t bytes){
					add_btree_totl_used(bytes);
				};
				void remove(size_t bytes){
					remove_btree_totl_used(bytes);
				};
			};

			struct stl_counter{

				void add(size_t bytes){
					add_stl_use(bytes);
				};
				void remove(size_t bytes){
					remove_stl_use(bytes);
				};
			};

			struct col_counter{
				void add(size_t bytes){
					add_col_use(bytes);
				};
				void remove(size_t bytes){
					remove_col_use(bytes);
				};
			};

			struct buffer_counter{
				void add(size_t bytes){
					add_buffer_use(bytes);
				};
				void remove(size_t bytes){
					remove_buffer_use(bytes);
				};
			};
			/// set to true to enable heap corruption checking and delete order checking
			static const bool heap_alloc_check = false;
			/// derived from The C++ Standard Library - A Tutorial and Reference - Nicolai M. Josuttis
			/// the bean counting issue
			template <class T>
			class base_tracker{
			public:

			public:
       // type definitions

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
					typedef base_tracker<U> other;
				};

				// return address of values
				pointer address (reference value) const {
					return &value;
				}
				const_pointer address (const_reference value) const {
					return &value;
				}

				/* constructors and destructor
				* - nothing to do because the allocator has no state
				*/
				base_tracker() throw() {
				}
				base_tracker(const base_tracker&) throw() {
				}
				template <class U>
				base_tracker (const base_tracker<U>&) throw() {
				}
				~base_tracker() throw() {
				}
				// a guess of the overhead that malloc may incur per allocation
				size_type overhead() const throw() {
					return sizeof(void*);
				}
				// return maximum number of elements that can be allocated
				size_type max_size () const throw() {
					return ((std::numeric_limits<size_t>::max)()) / sizeof(T);
				}

				// allocate but don't initialize num elements of type T
				pointer allocate (size_type num, const void* = 0) {
					// print message and allocate memory with global new

					pointer ret = (pointer)(malloc(num*sizeof(T)));

					return ret;
				}
				/// for 'dense' hash map
				pointer reallocate(pointer p, size_type n) {

					return static_cast<pointer>(realloc(p, n * sizeof(value_type)));
				}
				// initialize elements of allocated storage p with value value
				void construct (T* p, const T& value) {
					// initialize memory with placement new
					new((void*)p)T(value);
				}

				// initialize elements with variyng args
#ifndef _MSC_VER
				template<typename... _Args>
				void construct(T* p, _Args&&... __args) {
					new((void*)p)T(std::forward<_Args>(__args)...);
				}

#endif
				// destroy elements of initialized storage p
				void destroy (pointer p) {
					// destroy objects by calling their destructor
					p->~T();
				}

				// deallocate storage p of deleted elements
				void deallocate (pointer p, size_type num) {
					// print message and deallocate memory with global delete
					free((void*)p);

				}

			};// tracker tracking allocator

			template <class T, class _Counter = stl_counter>
			class tracker{
			public:

			public:
       // type definitions

				typedef T        value_type;
				typedef T*       pointer;
				typedef const T* const_pointer;
				typedef T&       reference;
				typedef const T& const_reference;
				typedef std::size_t    size_type;
				typedef std::ptrdiff_t difference_type;
				typedef _Counter counter_type;
				// rebind allocator to type U
				template <class U>
				struct rebind {
					typedef tracker<U> other;
				};

				// return address of values
				pointer address (reference value) const {
					return &value;
				}
				const_pointer address (const_reference value) const {
					return &value;
				}

				/* constructors and destructor
				* - nothing to do because the allocator has no state
				*/
				tracker() throw() {
				}

				tracker(const tracker&) throw() {
				}

				template <class U>
				tracker (const tracker<U>&) throw() {
				}

				~tracker() throw() {
				}

				size_type overhead() const throw() {
					return sizeof(void*);
				}
				// return maximum number of elements that can be allocated
				size_type max_size () const throw() {
					return ((std::numeric_limits<size_t>::max)()) / sizeof(T);
				}

				// allocate but don't initialize num elements of type T
				pointer allocate (size_type num, const void* = 0) {
					pointer ret = (pointer)(malloc(num*sizeof(T)));
					counter_type c;
					c.add(num*sizeof(T)+overhead());
					return ret;
				}
				// deallocate storage p of deleted elements
				void deallocate (pointer p, size_type num) {
					free((void*)p);
					counter_type c;
					c.remove(num*sizeof(T));
				}
				/// for 'dense' hash map
				pointer reallocate(pointer p, size_type n) {

					return static_cast<pointer>(realloc(p, n * sizeof(value_type)));
				}
				// initialize elements of allocated storage p with value value
				void construct (pointer p, const T& value) {
					// initialize memory with placement new
					new((void*)p)T(value);
				}

				// initialize elements with variyng args
#ifndef _MSC_VER
				template<typename... _Args>
				void construct(T* p, _Args&&... __args) {
					new((void*)p)T(std::forward<_Args>(__args)...);
				}
#endif
				// destroy elements of initialized storage p
				void destroy (pointer p) {
					// destroy objects by calling their destructor
					p->~T();
				}



			};// tracker tracking allocator
			 // return that all specializations of this allocator are interchangeable
			template <class T1, class T2>
			bool operator== (const tracker<T1>&,const tracker<T2>&) throw() {
				return true;
			}
			template <class T1, class T2>
			bool operator!= (const tracker<T1>&, const tracker<T2>&) throw() {
				return false;
			}

			template <class T>
			class pool_tracker : public base_tracker<T>{
			public:
				/// this is realy *realy* silly, isnt the types inherited as well ?? someone deserves a rasberry
				// type definitions
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
					typedef pool_tracker<U> other;
				};
				pool_tracker() throw() {
				}
				pool_tracker(const pool_tracker&) throw() {
				}
				template <class U>
				pool_tracker (const pool_tracker<U>&) throw() {				}

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
			class pool_shared{
			public:
				pool_shared(){
					allocated = 0;
					used = 0;
					max_pool_size = 0;
					instances = 0;
					current = 0;

				}
				u64 max_pool_size;

				std::atomic<u64> allocated;
				std::atomic<u64> used;
				std::atomic<u64> instances;
				std::atomic<u64> current;

			};
			typedef std::vector<void*> _ThreadAlloc;
			struct _ThreadSizeAlloc{
				_ThreadSizeAlloc() : accesses(0){
				}
				size_t accesses;
				_ThreadAlloc buckets;
				_ThreadAlloc & access(){
					++accesses;
					return buckets;
				}
			};

			typedef unordered::unordered_map<size_t,_ThreadSizeAlloc*> _ThreadAllocPtrMap;
			typedef std::unordered_multimap<u64, size_t> _ThreadAllocCheckMap;

			class inner_pool;
			struct thread_instance{
				thread_instance(pool_shared* shared) : shared(shared),total_buckets(0){
				}
				pool_shared * shared;
				u64 total_buckets;
				_ThreadAllocPtrMap alloc_pairs;
				_ThreadAllocPtrMap active_pairs;

				size_t eviction_size(){
					size_t f = 0xFFFFFFFFFFFFFFll,s = 0;
					for(auto a = this->active_pairs.begin(); a != this->active_pairs.end(); ++a){
						auto &bucket = *(a->second);

						if(!bucket.buckets.empty() && bucket.accesses < f){
							f = bucket.accesses;
							s = a->first;
						}
					}
					return s;
				}

				size_t overhead() const throw(){
					return sizeof(void*);
				}
				/// allocations evicted goes back to the os and is therefor neither allocated nor used
				template<class _InnerPool>
				void evict(size_t size, _InnerPool& inner){

					_ThreadSizeAlloc* r = nullptr;
					auto e = active_pairs.find(size);
					if(e != active_pairs.end()){
						r = e->second;
						u64 removed = r->buckets.size();
						while(!r->buckets.empty()){

							//free_pairs.push_back(std::make_pair(r->buckets.back(),size));
							void * data = r->buckets.back(); //,size
							inner.remove_check_free(data,size);
							delete (u8*)data;

							u64 moved = size + overhead();
							if((*this).shared->allocated < moved){
								err_print("invalid de-allocation detected");
							}
							(*this).shared->allocated -= moved;

							r->buckets.pop_back();
						}
						total_buckets-=removed;
						active_pairs.erase(size);
					}

				}
				_ThreadAlloc & access(size_t size){
					_ThreadSizeAlloc* r = nullptr;
					auto si = active_pairs.find(size);
					if(si == active_pairs.end()){
						auto ai = alloc_pairs.find(size);
						if(ai != alloc_pairs.end()){
							r = ai->second;
						}else{
							r = new _ThreadSizeAlloc();
							alloc_pairs[size] = r;
						}
						active_pairs[size] = r;
					}else{
						r = si->second;
					}

					return r->access();
				}
				std::vector<std::pair<void *, size_t>> free_pairs;
			};

			class inner_pool{
			public:
				/// types
				struct _Allocated{
					_Allocated(u8 *data,size_t):data(data),is_new(false){
					}
					_Allocated(){
						data = nullptr;
						is_new = true;
					}

					u8* data;
					bool is_new;
				};

				typedef std::vector<_Allocated> _Bucket; ///, pool_tracker<_Allocated>
				struct _Clocked{
					_Clocked() : size(0),clock(0),min_size(0){
					}
					size_t size;
                    u64 clock;
                    size_t min_size;

					_Bucket bucket;
				};

				typedef unordered::unordered_map<size_t, _Clocked> _Buckets;


			protected:
				/// data/fields/state

				static const u64 MAX_ALLOCATION_SIZE = 25600000000000ll;
				static const u64 USED_PERIOD = 1000000000ll;
				static const u64 MIN_ALLOCATION_SIZE = 32ll;
				static const u64 MAX_SMALL_ALLOCATION_SIZE = MIN_ALLOCATION_SIZE*4ll;
				static const u64 MAX_THREAD_BUCKETS = 5000;
				pool_shared * shared;
				u64 last_full_flush;
				u64 clock;
				u64 used_period;
				u64 id;
				u64 last_allocation;
				u64 last_allocation_row;
				u64 threads;
				_Buckets buckets;
				Poco::Mutex lock;
				_ThreadAllocCheckMap check_pairs;

			
			public:

				inner_pool(pool_shared* shared)
				:	shared(shared)
				,	clock(0)
				,	used_period(0)
				,	last_allocation(0)
				,	last_allocation_row(0)
				,	threads(0)
				{
					//inner_pool::max_pool_size = max_pool_size;
					this->id = ++(shared->instances);
					dbg_print("creating unlocked pool");
					last_full_flush = clock;

				}

				~inner_pool(){
				}
				void check_overflow(){
					
				}

				void add_check_free(void * ptr, size_t size){
					synchronized l(lock);
					auto f = check_pairs.find((u64)ptr);
					if(f != check_pairs.end()){
						err_print("address already deleted");
					}else{
						check_pairs.insert(std::make_pair((u64)ptr, size));
					}

				}
				void remove_check_free(void * ptr, size_t size){
					if(!heap_alloc_check) return;
					synchronized l(lock);
					auto f = check_pairs.find((u64)ptr);
					if(f != check_pairs.end()){
						if(f->second != size){
							err_print("address different size");
						}
						check_pairs.erase(f);
					}else{
						err_print("reused address has never been freed");
					}
				}
				void remove_free(void * ptr, size_t size){
					synchronized l(lock);
					auto f = check_pairs.find((u64)ptr);
					if(f != check_pairs.end()){
						check_pairs.erase(f);
					}
				}


				pool_shared * get_shared(){
					return this->shared;
				}
				size_t get_allocated() const {
					return (*this).shared->allocated;
				}
				void set_max_pool_size(u64 max_pool_size){

					shared->max_pool_size = max_pool_size;

				}
				u64 get_used() {

					return (*this).shared->used;
				}
				u64 get_total_allocated(){

					return (*this).shared->allocated + (*this).shared->used;
				}

				void * allocate(size_t requested){

					_Allocated result = allocate_type(requested);
					return result.data;
				}

				size_t overhead() const throw(){
					return sizeof(void*);
				}

				void bucket_2_thread(thread_instance* thread, _Bucket &current, size_t size){
					
				}
				_Allocated allocate_type(size_t requested) {

					
						shared->allocated += requested + overhead() ;
						u8 * a = new u8[requested];
						//memset(a,0,requested);
						_Allocated result(a,requested);
						result.is_new = true;

						return result;
					
					
				}
				
				void free(void * data, size_t requested){
					free_type(data,requested);
				}
				
			
				void free_type(void * data, size_t requested){
					if(use_internal_pool == FALSE){// && requested < MAX_SMALL_ALLOCATION_SIZE){
						if((*this).shared->allocated < (requested + overhead())){
							err_print("invalid de-allocation detected");
						}
						shared->allocated -= requested + overhead();
						delete (u8*)data;
						return;
					}
					
				}


				/// returns true if the pool is depleted
				bool is_depleted() const {

					return ( shared->allocated >= shared->max_pool_size );
				}
				bool is_full() const {

					return ( ( shared->used + shared->allocated )  >= shared->max_pool_size );
				}
				bool is_near_full() const {

					return ( ( shared->used + shared->allocated )  >= shared->max_pool_size ) && ( shared->allocated > (shared->max_pool_size*0.1) );
				}

				/// returns true if the pool is nearing depletion
				bool is_near_depleted() const {
					return is_near_factor(0.95);
				}
				bool is_near_factor(double factor) const {
					return ( shared->allocated >= factor*shared->max_pool_size ) ;
				}
				u64 get_max_pool_size() const {
					return shared->max_pool_size;
				}
				/// simple template allocations
				template<typename T>
				T* allocate(){
					_Allocated allocated = (*this).allocate_type(sizeof(T));
					void * potential = allocated.data;
					new (potential) T();

					return reinterpret_cast<T*>(potential);
				}
				template<typename T,typename _Context>
				T* allocate(_Context * context){

					_Allocated allocated = (*this).allocate_type(sizeof(T));
					void * potential = allocated.data;
					new (potential) T();

					return reinterpret_cast<T*>(potential);
				}
				template<typename T>
				void free(T* v){
					free_type(v, sizeof(T));
				}

			};
			class pool{
			protected:

				pool_shared shared;

				inner_pool* inner;


			public:
				pool(u64 max_pool_size) : inner (nullptr){
					set_max_pool_size(max_pool_size);
					inner = new inner_pool(&shared);
				}

				~pool(){

				}
				void set_special(){

				}
				const inner_pool& get_pool() const {
					return const_cast<pool*>(this)->get_pool() ;
				}
				inner_pool& get_pool(){
					//if(unlocked)
					return *inner;

				}
				size_t get_allocated() const {

					return get_pool().get_allocated();
				}

				void set_max_pool_size(u64 max_pool_size){
					this->shared.max_pool_size = max_pool_size;
				}

				u64 get_used() {
					return this->shared.used;
				}
				u64 get_total_allocated(){

					return get_pool().get_total_allocated();
				}
				u64 get_max_pool_size() const {
					return get_pool().get_max_pool_size();
				}
				void * allocate(size_t requested){

					return get_pool().allocate(requested);
				}


				void free(void * data, size_t requested){

					get_pool().free(data, requested);
				}

				/// return true if the give size bytes can be allocated
				bool can_allocate(u64 size) const throw() {
					if(size <  this->shared.used) return true;
					u64 extra = size -  this->shared.used;
					return  this->shared.allocated + extra <  this->shared.max_pool_size;
				}

				/// returns true if the pool is depleted
				bool is_depleted() const {
					return get_pool().is_depleted();

				}
				bool is_full() const {
					return get_pool().is_full();
				}

				bool is_near_full() const {
					return get_pool().is_near_full();
				}

				/// returns true if the pool is nearing depletion
				bool is_near_depleted() const {
					return get_pool().is_near_depleted();
				}
				bool is_near_factor(double factor) const {
					return get_pool().is_near_factor(factor);
				}
				bool check(const void * data, size_t requested) const{
					return true;
				}
				void check_overflow(){
					get_pool().check_overflow();
				}
				/// simple template allocations
				template<typename T>
				T* allocate(){

					T* r = get_pool().allocate<T>();
					return r;
				}
				template<typename T,typename _Context>
				T* allocate(_Context * context){

					T* r = get_pool().allocate<T,_Context>(context);
					return r;
				}
				template<typename T>
				inline void free(T* v){

					if(!v) return;
					v->~T();
					free_ns<T>(v);
				}
				template<typename T>
				inline void free_ns(T* v){

					get_pool().free<T>(v);
				}
			};
		};
	};
};
/// short hand for the namespace
namespace sta = ::stx::storage::allocation;

#endif /// _POOL_TS_H_20141129_
