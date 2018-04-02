#pragma once
#include <memory>
#include <vector>
extern "C" {
	#include <lualib.h>
	#include <lauxlib.h>
	#include <lua.h>	
};

namespace spaces {
	class spaces_key {
	};
	class spaces_instance {
	};
	class spaces_link {
	public:
		//TODO: change back to _rio_object_ptr<>

		//typedef spaces_link* ptr;
		template < typename _Allocated >
		class gc_alloc {
		public:

		private:
			struct _EnAllocated {
				_Allocated alloc;
				typename gc_alloc<_Allocated> * allocref;
			};

			umi away;
			typedef std::vector<_EnAllocated*> _Links;
			std::vector<_EnAllocated*> links;
			std::allocator<_EnAllocated> al;
			void deallocate() {
				for (_Links::iterator l = links.begin(); l != links.end(); ++l) {
					al.deallocate((*l));
				}
			}

		public:


			gc_alloc() : away(0) {
			}

			~gc_alloc() {

			}
			_Allocated* allocate() {
				_EnAllocated* r = al.allocate(1);
				links.push_back(r);
				r->allocref = this;
				return (_Allocated*)r;
			}
			static gc_alloc* get_allocator(_Allocated * allocated) {
				_EnAllocated* en = (_EnAllocated*)allocated;
				return en->allocref;
			}
			void reference() {
				away++;
			}
			void dereference() {
				remove();
			}
			bool remove() {
				away--;
				if (!away) {
					deallocate();
				}
				return (away == 0);
			}

		};


	private:


		//static gc_alloc* get_allocator(bool del = false){
		//	static  gc_alloc* loc =0;//__declspec(thread)
		//	if(!loc && !del)
		//		loc = new gc_alloc();
		//	if(loc && del){
		//		delete loc;
		//		loc = 0;
		//	}
		//	return loc;
		//}

		/*void operator delete(void* p){
			//if(get_allocator()->remove()) get_allocator(true);
		}*/
		//void* operator new(size_t n){
		//	
		//	//return get_allocator()->allocate();
		//	return 0;
		//}
	public:

		template < typename T > class gc_p
		{
		private:
			T*    ptr;
		public:
			gc_p(T* p = 0) throw()
				: ptr(p)
			{
				use();
			}
			gc_p(const gc_p<T>& p) throw()
				: ptr(p.ptr)
			{
				use();
			}
			virtual ~gc_p() throw() {
				dispose();
			}

			gc_p &operator=(T * p) throw() {
				if (ptr != p) {
					dispose();
					ptr = p;
					use();
				}
				return *this;
			}
			gc_p& operator=(const gc_p<T> &p) throw() {
				if (this != &p) {
					dispose();
					ptr = p.ptr;
					use();
				}
				return *this;
			}
			gc_p& operator=(gc_p<T> &p) throw() {
				if (this != &p) {
					dispose();
					ptr = p.ptr;
					use();
				}
				return *this;
			}
			inline virtual operator T*() {
				return ptr;
			}
			inline virtual operator T*() const {
				return ptr;
			}
			inline virtual T& operator*() {
				if (!ptr) throwUnassignedPtrError<T>();

				return *ptr;
			}
			inline virtual T& operator*() const {
				if (!ptr) throwUnassignedPtrError<T>();
				return *ptr;
			}
			inline virtual T* operator->() {
				if (!ptr) throwUnassignedPtrError<T>();
				return ptr;
			}
			inline virtual T* operator->() const {
				if (!ptr) throwUnassignedPtrError<T>();
				return ptr;
			}

			inline bool assigned() {
				return (ptr != 0);
			}
			void clear() {
				dispose();
			}
		protected:
			gc_alloc<T> *get_allocator() {
				return gc_alloc<T>::get_allocator(ptr);
			}
			void dispose() {
				if (ptr)
					get_allocator()->dereference();
			}
			void use() {
				if (ptr)
					get_allocator()->reference();
			}
		};
		typedef  gc_alloc<spaces_link> allocator;
		typedef std::shared_ptr<spaces_link> ptr;
		//gc_p<>
		spaces_link(const spaces_key& k, const ptr &context) : context(context), _k(0) {
			_instance = context->_instance;
			set_key(k);
			//usage.arguments.add(sizeof(*this));
		}
		spaces_link(spaces_key* k, const ptr &context) : context(context), _k(0) {
			_instance = context->_instance;
			set_key(k);
			//usage.arguments.add(sizeof(*this));
		}
		spaces_link(const spaces_key& k, spaces_instance* _instance) : context(0), _k(0) {
			(*this)._instance = _instance;
			set_key(k);
			//usage.arguments.add(sizeof(*this));
		}
		spaces_link(spaces_key* k, spaces_instance* _instance) : context(0), _k(0) {
			(*this)._instance = _instance;
			set_key(k);
			//usage.arguments.add(sizeof(*this));
		}
		~spaces_link() {
			clear_key();
			//usage.arguments.remove(sizeof(*this));
		}
		void clear_key() {
			//if(_k) 
				//get_instance()->map.deallocate(_k);
			_k = 0;
		}
		const ptr context;
		//TODO: make this a pointer

		spaces_instance* get_instance() {
			return _instance;
		}

		spaces_key& get_key() {
			return *_k;
		}
		void set_key(const spaces_key& k) {
			clear_key();
			//_k = get_instance ()->map.allocate();
			*_k = k;
		}
		void set_key(spaces_key* k) {
			clear_key();
			//get_instance ()->map.ref(k);
			_k = k;
		}
#ifndef _DEBUG
#ifdef _USE_NED_
		typedef nedalloc::nedallocator<spaces_link, nedalloc::nedpolicy::reserveN<26>::policy > _Allocator;
#else
		typedef std::allocator<spaces_link> _Allocator;
#endif

#else
		typedef std::allocator<spaces_link> _Allocator;
#endif
		void operator delete(void* p) {
			_Allocator().deallocate((spaces_link*)p, 1);
		}
		void* operator new(size_t n) {

			return _Allocator().allocate(1);
		}
	private:
		spaces_instance* _instance;
		spaces_key* _k;

	};
};
